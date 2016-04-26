/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
 ******************************************************************************
 * @file    usbd_audio_core.c
 * @author  MCD Application Team
 * @version V1.1.0
 * @date    19-March-2012
 * @brief   This file provides the high layer firmware functions to manage the
 *          following functionalities of the USB Audio Class:
 *           - Initialization and Configuration of high and low layer
 *           - Enumeration as Audio Streaming Device
 *           - Audio Streaming data transfer
 *           - AudioControl requests management
 *           - Error management
 *
 *  @verbatim
 *
 *          ===================================================================
 *                                Audio Class Driver Description
 *          ===================================================================
 *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
 *           Audio Devices V1.0 Mar 18, 98".
 *           This driver implements the following aspects of the specification:
 *             - Device descriptor management
 *             - Configuration descriptor management
 *             - Standard AC Interface Descriptor management
 *             - 1 Audio Streaming Interface (with single channel, PCM, Stereo mode)
 *             - 1 Audio Streaming Endpoint
 *             - 1 Audio Terminal Input (1 channel)
 *             - Audio Class-Specific AC Interfaces
 *             - Audio Class-Specific AS Interfaces
 *             - AudioControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
 *             - Audio Feature Unit (limited to Mute control)
 *             - Audio Synchronization type: Asynchronous
 *             - Single fixed audio sampling rate (configurable in usbd_conf.h file)
 *
 *           @note
 *            The Audio Class 1.0 is based on USB Specification 1.0 and thus supports only
 *            Low and Full speed modes and does not allow High Speed transfers.
 *            Please refer to "USB Device Class Definition for Audio Devices V1.0 Mar 18, 98"
 *            for more details.
 *
 *           These aspects may be enriched or modified for a specific user application.
 *
 *            This driver doesn't implement the following aspects of the specification
 *            (but it is possible to manage these features with some modifications on this driver):
 *             - AudioControl Endpoint management
 *             - AudioControl requsests other than SET_CUR and GET_CUR
 *             - Abstraction layer for AudioControl requests (only Mute functionality is managed)
 *             - Audio Synchronization type: Adaptive
 *             - Audio Compression modules and interfaces
 *             - MIDI interfaces and modules
 *             - Mixer/Selector/Processing/Extension Units (Feature unit is limited to Mute control)
 *             - Any other application-specific modules
 *             - Multiple and Variable audio sampling rates
 *             - Out Streaming Endpoint/Interface (microphone)
 *
 *  @endverbatim
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "usbd_audio_core.h"
#include "usbd_cdc_core.h"
#include "usbd_audio_out_if.h"
#include "math.h"
/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @{
 */


/** @defgroup usbd_audio
 * @brief usbd core module
 * @{
 */

/** @defgroup usbd_audio_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */


/** @defgroup usbd_audio_Private_Defines
 * @{
 */
/**
 * @}
 */


/** @defgroup usbd_audio_Private_Macros
 * @{
 */
/**
 * @}
 */


/** @defgroup usbd_audio_Private_FunctionPrototypes
 * @{
 */

/*********************************************
   AUDIO Device library callbacks
 *********************************************/
static uint8_t  usbd_audio_Init       (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_audio_DeInit     (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_audio_Setup      (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_audio_EP0_RxReady(void *pdev);
static uint8_t  usbd_audio_DataIn     (void *pdev, uint8_t epnum);
static uint8_t  usbd_audio_DataOut    (void *pdev, uint8_t epnum);
static uint8_t  usbd_audio_SOF        (void *pdev);
static uint8_t  usbd_audio_OUT_Incplt (void  *pdev);

/*********************************************
   AUDIO Requests management functions
 *********************************************/
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req);
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req);
static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length);
/**
 * @}
 */

/** @defgroup usbd_audio_Private_Variables
 * @{
 */
/* Main Buffer for Audio Data Out transfers and its relative pointers */
uint8_t  IsocOutBuff [TOTAL_OUT_BUF_SIZE * 2];
uint8_t* IsocOutWrPtr = IsocOutBuff;
uint8_t* IsocOutRdPtr = IsocOutBuff;

#define USB_AUDIO_IN_NUM_BUF 8
#define USB_AUDIO_IN_PKT_SIZE   (AUDIO_IN_PACKET/2)
// size in samples not size in bytes
#define USB_AUDIO_IN_BUF_SIZE (USB_AUDIO_IN_NUM_BUF * USB_AUDIO_IN_PKT_SIZE)

static volatile int16_t in_buffer[USB_AUDIO_IN_BUF_SIZE]; //buffer for filtered PCM data from Recv.
static int16_t Silence[USB_AUDIO_IN_PKT_SIZE];
static volatile uint16_t in_buffer_tail;
static volatile uint16_t in_buffer_head;
static volatile uint16_t in_buffer_overflow;


void audio_in_put_buffer(int16_t sample)
{
    in_buffer[in_buffer_head] = sample;
    in_buffer_head=  (in_buffer_head + 1) %USB_AUDIO_IN_BUF_SIZE;
    // now test buffer full
    if (in_buffer_head == in_buffer_tail)
    {
        // ok. We loose data now, should never ever happen, but so what
        // will cause minor distortion if only a few bytes.
        in_buffer_overflow++;
    }
}
volatile int16_t* audio_in_buffer_next_pkt()
{
    uint16_t room;
    uint16_t temp_head = in_buffer_head;
    room = ((((temp_head < in_buffer_tail)?USB_AUDIO_IN_BUF_SIZE:0) + temp_head) - in_buffer_tail);
    if (room >= USB_AUDIO_IN_PKT_SIZE)
    {
        return &in_buffer[in_buffer_tail];
    }
    else
    {

        return NULL;
    }
}
void audio_in_buffer_pop_pkt(int16_t* ptr)
{
    if (ptr)
    {
        // there was data and pkt has been used
        // free  the space
        in_buffer_tail = (in_buffer_tail+USB_AUDIO_IN_PKT_SIZE)%USB_AUDIO_IN_BUF_SIZE;
    }
}

static void audio_in_fill_ep_fifo(void *pdev)
{
    uint8_t *pkt = (uint8_t*)audio_in_buffer_next_pkt();
    static uint16_t fill_buffer = (USB_AUDIO_IN_NUM_BUF/2) + 1;
    if (fill_buffer == 0 && pkt)
    {
        DCD_EP_Tx (pdev,AUDIO_IN_EP, pkt, AUDIO_IN_PACKET);
        audio_in_buffer_pop_pkt((int16_t*)pkt);
    }
    else
    {
        if (fill_buffer == 0)
        {
            fill_buffer = USB_AUDIO_IN_NUM_BUF/2 + 1;
        }
        fill_buffer--;
        // transmit something if we do not have enough in buffer
        DCD_EP_Tx (pdev,AUDIO_IN_EP, (uint8_t*)Silence, AUDIO_IN_PACKET);
    }
}


/* Main Buffer for Audio Control Rrequests transfers and its relative variables */
uint8_t  AudioCtl[64];
uint8_t  AudioCtlCmd = 0;
uint32_t AudioCtlLen = 0;
uint8_t  AudioCtlUnit = 0;
uint8_t  AudioCtlCS = 0;
uint8_t  AudioCtlCN = 0;
uint8_t  AudioCtlIf = 0;

extern USBD_Class_cb_TypeDef  USBD_CDC_cb;
static uint32_t PlayFlag = 0;
static uint32_t SendFlag = 0;


UsbAudioUnit usbUnits[UnitMax] =
{
    {
        .cs  = AUDIO_CONTROL_VOLUME,
        .cn  = AUDIO_OUT_STREAMING_CTRL,
        .min = -31 * 0x100,
        .max = 0 * 0x100,
        .res = 0x100,
        .cur = -16* 0x100,
        .ptr = &ts.tx_gain[TX_AUDIO_DIG]
    },
    {
        .cs  = AUDIO_CONTROL_VOLUME,
        .cn  = 0x06,
        .min = -31 * 0x100,
        .max = 0 * 0x100,
        .res = 0x100,
        .cur = -16* 0x100,
        .ptr = &ts.rx_gain[RX_AUDIO_DIG].value
    }

};


static __IO uint32_t  usbd_audio_AltSet[USBD_ITF_MAX_NUM];
// each interface could potentially be audio and have alternate setting, EP0 gets "unused" entry


static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE];

/* AUDIO interface class callbacks structure */
USBD_Class_cb_TypeDef  AUDIO_cb =
{
    usbd_audio_Init,
    usbd_audio_DeInit,
    usbd_audio_Setup,
    NULL, /* EP0_TxSent */
    usbd_audio_EP0_RxReady,
    usbd_audio_DataIn,
    usbd_audio_DataOut,
    usbd_audio_SOF,
    NULL,
    usbd_audio_OUT_Incplt,
    USBD_audio_GetCfgDesc,
#ifdef USB_OTG_HS_CORE
    USBD_audio_GetCfgDesc, /* use same config as per FS */
#endif
};

/* USB AUDIO device Configuration Descriptor */
// https://www.lpcware.com/content/forum/trying-combined-usb-audio-and-out-lpc1857
// Tsuneos Post has perfect instructions how to join the the IN and OUT Interface descriptors into
// one.

static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE] =
{
    /* Configuration 1 */
    0x09,                                 /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,    /* bDescriptorType */
    LOBYTE(AUDIO_CONFIG_DESC_SIZE),       /* wTotalLength  109 bytes*/
    HIBYTE(AUDIO_CONFIG_DESC_SIZE),
    0x05,                                 /* bNumInterfaces */
    0x01,                                 /* bConfigurationValue */
    0x00,                                 /* iConfiguration */
    0xC0,                                 /* bmAttributes  BUS Powred*/
    0x32,                                 /* bMaxPower = 100 mA*/
    /* 09 byte*/
    /*---------------------------------------------------------------------------*/
    USB_INTERFACE_ASSOCIATION_DESC_SIZE,
    USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,
    CDC_CTRL_IF,						  /* first interface */
    CDC_TOTAL_IF_NUM,					  /* bNumInterfaces */
    0x02,               /* bInterfaceClass */
    0x02,          /* bInterfaceSubClass */
    0x01,             /* bInterfaceProtocol */
    0x00,								  /* String Index */

    /*Interface Descriptor */
    0x09,   /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    CDC_CTRL_IF,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */

    /*Header Functional Descriptor*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,

    /*Call Management Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x01,   /* bDataInterface: 1 */

    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */

    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    CDC_CTRL_IF,   /* bMasterInterface: Communication class interface */
    CDC_DATA_IF,   /* bSlaveInterface0: Data Class Interface */

    /*Endpoint 2 Descriptor*/
    0x07,                           /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                     /* bEndpointAddress */
    0x03,                           /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SZE),     /* wMaxPacketSize: */
    HIBYTE(CDC_CMD_PACKET_SZE),
#ifdef USE_USB_OTG_HS
    0x10,                           /* bInterval: */
#else
    0xFF,                           /* bInterval: */
#endif /* USE_USB_OTG_HS */

    /*---------------------------------------------------------------------------*/

    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
    CDC_DATA_IF,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */

    /*Endpoint OUT Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                        /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
    0x00,                              /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
    CDC_IN_EP,                         /* bEndpointAddress */
    0x02,                              /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
    HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
    0x00,                               /* bInterval: ignore for Bulk transfer */

    USB_INTERFACE_ASSOCIATION_DESC_SIZE,
    USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,
    AUDIO_CTRL_IF,						  /* first interface */
    AUDIO_TOTAL_IF_NUM,					  /* bNumInterfaces */
    USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
    0x00,								  /* String Index */

    /* USB Sound Standard interface descriptor */
    AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
    AUDIO_CTRL_IF,                                 /* bInterfaceNumber */
    0x00,                                 /* bAlternateSetting */
    0x00,                                 /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
    0x00,                                 /* iInterface */

    /* 09 byte*/
    // Begin AudioControl Descriptors
    /* USB Speaker Class-specific AC Interface Descriptor */
#ifdef AUDIO_IN
    AUDIO_INTERFACE_DESC_SIZE + 1,            /* bLength */
#else
    AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
#endif
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
    0x00,          /* 1.00 */             /* bcdADC */
    0x01,
#ifdef AUDIO_IN
    61  + 9 + 7,
#else
    39,
#endif
    0x00,
#ifdef AUDIO_IN
    0x02,                                 /* bInCollection */
    AUDIO_OUT_IF,                                 /* baInterfaceNr */
    AUDIO_IN_IF,        							/* baInterfaceNr */
#else
    0x01,                                 /* bInCollection */
    0x01,                                 /* baInterfaceNr */
#endif
    /* 09 byte*/
    /* USB Speaker Input Terminal Descriptor */
    AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype */
    0x01,                                 /* bTerminalID */
    0x01,                                 /* wTerminalType AUDIO_TERMINAL_USB_STREAMING   0x0101 */
    0x01,
    0x00,                                 /* bAssocTerminal */
    0x01,                                 /* bNrChannels */
    0x00,                                 /* wChannelConfig 0x0000  Mono */
    0x00,
    0x00,                                 /* iChannelNames */
    0x00,                                 /* iTerminal */
    /* 12 byte*/

    /* USB Speaker Audio Feature Unit Descriptor */
    0x09,                                 /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
    AUDIO_OUT_STREAMING_CTRL,             /* bUnitID */
    0x01,                                 /* bSourceID */
    0x01,                                 /* bControlSize */
    AUDIO_CONTROL_VOLUME,                   /* bmaControls(0) */
    0x00,                                 /* bmaControls(1) */
    0x00,                                 /* iTerminal */
    /* 09 byte*/

    /*USB Speaker Output Terminal Descriptor */
    0x09,      /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype */
    0x03,                                 /* bTerminalID */
    0x01,                                 /* wTerminalType  0x0301*/
    0x03,
    0x00,                                 /* bAssocTerminal */
    0x02,                                 /* bSourceID */
    0x00,                                 /* iTerminal */
    /* 09 byte*/
#ifdef AUDIO_IN
    /*USB Microphone Input Terminal Descriptor */
    0x0C,                         // Size of the descriptor, in bytes
    AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type
    AUDIO_CONTROL_INPUT_TERMINAL,    // INPUT_TERMINAL descriptor subtype
    0x04,                         // ID of this Terminal.
    0x01,0x02,                    // Terminal is Microphone (0x01,0x02)
    0x00,                         // No association
    USBD_AUDIO_IN_CHANNELS,       // One or two channel
    0x00,0x00,                    // Mono sets no position bits
    0x00,                         // Unused.
    0x00,                         // Unused.

    /* USB Microphone Output Terminal Descriptor */
    0x09,                            // Size of the descriptor, in bytes (bLength)
    AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type (bDescriptorType)
    AUDIO_CONTROL_OUTPUT_TERMINAL,   // OUTPUT_TERMINAL descriptor subtype (bDescriptorSubtype)
    0x05,                            // ID of this Terminal. (bTerminalID)
    0x01, 0x01,                      // USB Streaming. (wTerminalType
    0x00,                            // unused         (bAssocTerminal)
    0x07,                            // From Input Terminal.(bSourceID)
    0x00,                            // unused  (iTerminal)

    /* USB Speaker Audio Feature Unit Descriptor */
    0x09,                                 /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
    0x06,             					  /* bUnitID */
    0x04,                                 /* bSourceID */
    0x01,                                 /* bControlSize */
    AUDIO_CONTROL_VOLUME,                 /* bmaControls(0) */
    0x00,                                 /* bmaControls(1) */
    0x00,                                 /* iTerminal */
    /* 09 byte*/

    /* AC Selector Unit Descriptor */
    0x07, //	bLength
    0x24, //	bDescriptorType
    0x05, //	bDescriptorSubtype
    0x07, //	bUnitID
    0x01, //	bBrInPins
    0x06, //	baSourceID(1)
    0x00, //	iSelector
#endif
    // ========================================== END AudioControl
    /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
    /* Interface 1, Alternate Setting 0                                             */
    AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
    AUDIO_OUT_IF,                                 /* bInterfaceNumber */
    0x00,                                 /* bAlternateSetting */
    0x00,                                 /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
    0x00,                                 /* iInterface */
    /* 09 byte*/

    /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Operational */
    /* Interface 1, Alternate Setting 1                                           */
    AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
    AUDIO_OUT_IF,                         /* bInterfaceNumber */
    0x01,                                 /* bAlternateSetting */
    0x01,                                 /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
    0x00,                                 /* iInterface */
    /* 09 byte*/

    /* USB Speaker Audio Streaming Interface Descriptor */
    AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype */
    0x01,                                 /* bTerminalLink */
    0x01,                                 /* bDelay */
    0x01,                                 /* wFormatTag AUDIO_FORMAT_PCM  0x0001*/
    0x00,
    /* 07 byte*/

    /* USB Speaker Audio Type III Format Interface Descriptor */
    0x0B,                                 /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
    AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype */
    AUDIO_FORMAT_TYPE_III,                /* bFormatType */
    0x02,                                 /* bNrChannels */
    0x02,                                 /* bSubFrameSize :  2 Bytes per frame (16bits) */
    16,                                   /* bBitResolution (16-bits per sample) */
    0x01,                                 /* bSamFreqType only one frequency supported */
    SAMPLE_FREQ(USBD_AUDIO_FREQ),         /* Audio sampling frequency coded on 3 bytes */
    /* 11 byte*/

    /* Endpoint 1 - Standard Descriptor */
    AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType */
    AUDIO_OUT_EP,                         /* bEndpointAddress 1 out endpoint*/
    USB_ENDPOINT_TYPE_ISOCHRONOUS,        /* bmAttributes */
    AUDIO_PACKET_SZE(USBD_AUDIO_FREQ,USBD_AUDIO_OUT_CHANNELS),    /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
    0x01,                                 /* bInterval */
    0x00,                                 /* bRefresh */
    0x00,                                 /* bSynchAddress */
    /* 09 byte*/

    /* Endpoint - Audio Streaming Descriptor*/
    AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
    AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType */
    AUDIO_ENDPOINT_GENERAL,               /* bDescriptor */
    0x00,                                 /* bmAttributes */
    0x00,                                 /* bLockDelayUnits */
    0x00,                                 /* wLockDelay */
    0x00,
    /* 07 byte*/

#ifdef AUDIO_IN
    /* From Here is the Microphone */
    /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 0) (CODE == 3)*/ //zero-bandwidth interface
    0x09,                         // Size of the descriptor, in bytes (bLength)
    USB_INTERFACE_DESCRIPTOR_TYPE,    // INTERFACE descriptor type (bDescriptorType) 0x04
    AUDIO_IN_IF, // Index of this interface. (bInterfaceNumber) ?????????? (3<) (1<<) (1<M)
    0x00,                         // Index of this alternate setting. (bAlternateSetting)
    0x00,                         // 0 endpoints.   (bNumEndpoints)
    USB_DEVICE_CLASS_AUDIO,       // AUDIO (bInterfaceClass)
    AUDIO_SUBCLASS_AUDIOSTREAMING, // AUDIO_STREAMING (bInterfaceSubclass)
    0x00,                         // Unused. (bInterfaceProtocol)
    0x00,                         // Unused. (iInterface)

    /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 1) (CODE == 4)*/
    0x09,                         // Size of the descriptor, in bytes (bLength)
    USB_INTERFACE_DESCRIPTOR_TYPE,     // INTERFACE descriptor type (bDescriptorType)
    AUDIO_IN_IF, // Index of this interface. (bInterfaceNumber)
    0x01,                         // Index of this alternate setting. (bAlternateSetting)
    0x01,                         // 1 endpoint (bNumEndpoints)
    USB_DEVICE_CLASS_AUDIO,       // AUDIO (bInterfaceClass)
    AUDIO_SUBCLASS_AUDIOSTREAMING,   // AUDIO_STREAMING (bInterfaceSubclass)
    0x00,                         // Unused. (bInterfaceProtocol)
    0x00,                         // Unused. (iInterface)

    /*  USB Microphone Class-specific AS General Interface Descriptor (CODE == 5)*/
    0x07,                         // Size of the descriptor, in bytes (bLength)
    AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type (bDescriptorType) 0x24
    AUDIO_STREAMING_GENERAL,         // GENERAL subtype (bDescriptorSubtype) 0x01
    0x05,             // Unit ID of the Output Terminal.(bTerminalLink)
    0x01,                         // Interface delay. (bDelay)
    0x01,0x00,                    // PCM Format (wFormatTag)

    /*  USB Microphone Type I Format Type Descriptor (CODE == 6)*/
    0x0B,                        // Size of the descriptor, in bytes (bLength)
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,// CS_INTERFACE Descriptor Type (bDescriptorType) 0x24
    AUDIO_STREAMING_FORMAT_TYPE,   // FORMAT_TYPE subtype. (bDescriptorSubtype) 0x02
    0x01,                        // FORMAT_TYPE_I. (bFormatType)
    USBD_AUDIO_IN_CHANNELS,                        // One or two channel.(bNrChannels)
    0x02,                        // Two bytes per audio subframe.(bSubFrameSize)
    0x10,                        // 16 bits per sample.(bBitResolution)
    0x01,                        // One frequency supported. (bSamFreqType)
    (USBD_AUDIO_IN_FREQ&0xFF),((USBD_AUDIO_IN_FREQ>>8)&0xFF),0x00,  // (tSamFreq) (NOT COMPLETE!!!)

    /*  USB Microphone Standard Endpoint Descriptor (CODE == 8)*/ //Standard AS Isochronous Audio Data Endpoint Descriptor
    0x09,                       // Size of the descriptor, in bytes (bLength)
    0x05,                       // ENDPOINT descriptor (bDescriptorType)
    AUDIO_IN_EP,                    // IN Endpoint 1. (bEndpointAddress)
    USB_ENDPOINT_TYPE_ISOCHRONOUS, // Isochronous, not shared. (bmAttributes)//USB_ENDPOINT_TYPE_asynchronous USB_ENDPOINT_TYPE_ISOCHRONOUS
    (AUDIO_IN_PACKET&0xFF),((AUDIO_IN_PACKET>>8)&0xFF),                  //bytes per packet (wMaxPacketSize)
    0x01,                       // One packet per frame.(bInterval)
    0x00,                       // Unused. (bRefresh)
    0x00,                       // Unused. (bSynchAddress)

    /* USB Microphone Class-specific Isoc. Audio Data Endpoint Descriptor */
    0x07,                       // Size of the descriptor, in bytes (bLength)
    AUDIO_ENDPOINT_DESCRIPTOR_TYPE,    // CS_ENDPOINT Descriptor Type (bDescriptorType) 0x25
    AUDIO_ENDPOINT_GENERAL,            // GENERAL subtype. (bDescriptorSubtype) 0x01
    0x00,                              // No sampling frequency control, no pitch control, no packet padding.(bmAttributes)
    0x00,                              // Unused. (bLockDelayUnits)
    0x00,0x00,                         // Unused. (wLockDelay)
#endif
} ;

/**
 * @}
 */

/** @defgroup usbd_audio_Private_Functions
 * @{
 */

/**
 * @brief  usbd_audio_Init
 *         Initilaizes the AUDIO interface.
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t  usbd_audio_Init (void  *pdev,
                                 uint8_t cfgidx)
{

#ifdef AUDIO_IN

    DCD_EP_Open(pdev,
                AUDIO_IN_EP,
                AUDIO_IN_PACKET,
                USB_OTG_EP_ISOC);
#endif
#ifdef AUDIO_OUT
    DCD_EP_Open(pdev,
                AUDIO_OUT_EP,
                AUDIO_OUT_PACKET,
                USB_OTG_EP_ISOC);
#endif

#define DEFAULT_VOLUME 0
    /* Initialize the Audio output Hardware layer */
    if (AUDIO_OUT_fops.Init(USBD_AUDIO_FREQ, DEFAULT_VOLUME, 0) != USBD_OK)
    {
        // return USBD_FAIL;
    }

    /* Prepare Out endpoint to receive audio data */
#ifdef AUDIO_OUT
    IsocOutRdPtr = IsocOutBuff;
    IsocOutWrPtr = IsocOutBuff;
    DCD_EP_PrepareRx(pdev,
                     AUDIO_OUT_EP,
                     (uint8_t*)IsocOutBuff,
                     AUDIO_OUT_PACKET);
#endif
    // return USBD_OK;
    return USBD_CDC_cb.Init(pdev,cfgidx);
}

/**
 * @brief  usbd_audio_Init
 *         DeInitializes the AUDIO layer.
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t  usbd_audio_DeInit (void  *pdev,
                                   uint8_t cfgidx)
{
#ifdef AUDIO_IN
    DCD_EP_Close (pdev , AUDIO_IN_EP);
#endif
#ifdef AUDIO_OUT
    DCD_EP_Close (pdev , AUDIO_OUT_EP);
#endif
    /* DeInitialize the Audio output Hardware layer */
    if (AUDIO_OUT_fops.DeInit(0) != USBD_OK)
    {
        return USBD_FAIL;
    }

    // return USBD_OK;
    return USBD_CDC_cb.Init(pdev,cfgidx);
}

/**
 * @brief  usbd_audio_Setup
 *         Handles the Audio control request parsing.
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t  usbd_audio_Setup (void  *pdev,
                                  USB_SETUP_REQ *req)
{
    uint16_t len=USB_AUDIO_DESC_SIZ;
    uint8_t  *pbuf=usbd_audio_CfgDesc + 18;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    /* AUDIO Class Requests -------------------------------*/
    case USB_REQ_TYPE_CLASS :
        if (req->wIndex == CDC_CTRL_IF)
        {
            return USBD_CDC_cb.Setup(pdev,req);
        }
        else
        {
            switch (req->bRequest)
            {
            case AUDIO_REQ_GET_MIN:
            case AUDIO_REQ_GET_MAX:
            case AUDIO_REQ_GET_RES:
            case AUDIO_REQ_GET_CUR:
                AUDIO_Req_GetCurrent(pdev, req);
                break;

            case AUDIO_REQ_SET_CUR:
                AUDIO_Req_SetCurrent(pdev, req);
                break;

            default:
                USBD_CtlError (pdev, req);
                return USBD_FAIL;
            }
        }
        break;

    /* Standard Requests -------------------------------*/
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR:
            if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
            {
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
                //!        pbuf = usbd_audio_Desc;
#else
                pbuf = usbd_audio_CfgDesc + 18;
#endif
                len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);
            }

            USBD_CtlSendData (pdev,
                              pbuf,
                              len);
            break;

        case USB_REQ_GET_INTERFACE :

            USBD_CtlSendData (pdev,
                              (uint8_t *)&usbd_audio_AltSet[req->wIndex],
                              1);
            break;

        case USB_REQ_SET_INTERFACE :
            if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
            {
                usbd_audio_AltSet[req->wIndex] = (uint8_t)(req->wValue);
#ifdef AUDIO_IN
// interface 2 is AUDIO_IN
                if (usbd_audio_AltSet[AUDIO_IN_IF] == 1)
                {
                    if (!SendFlag)
                    {
                        SendFlag = 1;
                    }
                }
                else
                {
                    SendFlag = 0;
                    DCD_EP_Flush (pdev,AUDIO_IN_EP);
                }
#endif

            }
            else
            {
                /* Call the error management function (command will be nacked) */
                USBD_CtlError (pdev, req);
            }
            break;
        }
    }
    return USBD_OK;
}

/**
 * @brief  usbd_audio_EP0_RxReady
 *         Handles audio control requests data.
 * @param  pdev: device device instance
 * @retval status
 */
static uint8_t  usbd_audio_EP0_RxReady (void  *pdev)
{
    uint8_t retval = USBD_OK;
    /* Check if an AudioControl request has been issued */
    if (AudioCtlCmd == AUDIO_REQ_SET_CUR)
    {
        /* In this driver, to simplify code, only SET_CUR request is managed */
        /* Check for which addressed unit the AudioControl request has been issued */
        if (AudioCtlUnit == AUDIO_OUT_STREAMING_CTRL)
        {
            /* In this driver, to simplify code, only one unit is manage */
            /* Call the audio interface volume function */
            int16_t val = (int16_t)((uint16_t)AudioCtl[0] | ((uint16_t)AudioCtl[1])<<8) ;
            retval = AUDIO_OUT_fops.VolumeCtl((val/0x100)+31);
            usbUnits[UnitVolumeTX].cur = val;
            /* Reset the AudioCtlCmd variable to prevent re-entering this function */
        }
        else
        {
            int16_t val = (int16_t)((uint16_t)AudioCtl[0] | ((uint16_t)AudioCtl[1])<<8) ;
            retval = AUDIO_IN_fops.VolumeCtl((val/0x100)+31);
            usbUnits[UnitVolumeRX].cur = val;
        }
        /* Reset the AudioCtlCmd variable to prevent re-entering this function */
        AudioCtlCmd = 0;
        AudioCtlLen = 0;
        AudioCtlCS = 0;
        AudioCtlCN = 0;
        AudioCtlIf = 0;

    }
    else
    {
        retval = USBD_CDC_cb.EP0_RxReady(pdev);
    }
    return retval;
}

/**
 * @brief  usbd_audio_DataIn
 *         Handles the audio IN data stage.
 * @param  pdev: instance
 * @param  epnum: endpoint number
 * @retval status
 */


static uint8_t  usbd_audio_DataIn (void *pdev, uint8_t epnum)
{

#ifdef AUDIO_IN
    if (epnum == (AUDIO_IN_EP & 0x7f))
    {
        DCD_EP_Flush(pdev,AUDIO_IN_EP); //very important!!!
        audio_in_fill_ep_fifo(pdev);
    }
#endif
    if (epnum == (CDC_IN_EP & 0x7f))
    {
        return USBD_CDC_cb.DataIn(pdev,epnum);
    }
    return USBD_OK;
}

/**
 * @brief  usbd_audio_DataOut
 *         Handles the Audio Out data stage.
 * @param  pdev: instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t  usbd_audio_DataOut (void *pdev, uint8_t epnum)
{

    if (epnum == AUDIO_OUT_EP)
    {
        // mchf_board_green_led(PlayFlag != 0);
        // during init the first buffer part was already set up to receive data
        // if we end up here, one chunk of data has arrived. Thats cool. And it is
        // a good time to hand over the packet to the next layer.
        // we do that now, since we can be sure that there is at least one packet of
        // data.
        // The buffer implementation is quite interesting, since it uses
        // OUT_PACKET_NUM + 1 slots but allocates OUT_PACKET_NUM *  2 slots
        // just to be on the save side I guess. Don't know which side this is, though.

        /* If all available buffers have been consumed, stop playing */
        if (IsocOutWrPtr >= (IsocOutBuff + (AUDIO_OUT_PACKET * OUT_PACKET_NUM)))
        {
            /* All buffers are full: roll back */
            IsocOutWrPtr = IsocOutBuff;
        }
        else
        {
            /* Increment the buffer pointer */
            IsocOutWrPtr += AUDIO_OUT_PACKET;
        }

        /* Toggle the frame index */
        ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame =
            (((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame)? 0:1;

        /* Prepare Out endpoint to receive next audio packet */
        DCD_EP_PrepareRx(pdev,
                         AUDIO_OUT_EP,
                         (uint8_t*)(IsocOutWrPtr),
                         AUDIO_OUT_PACKET);

        if (PlayFlag)
        {
            PlayFlag = 5;
            /* Start playing received packet */
            AUDIO_OUT_fops.AudioCmd((uint8_t*)(IsocOutRdPtr),  /* Samples buffer pointer */
                                    AUDIO_OUT_PACKET,          /* Number of samples in Bytes */
                                    AUDIO_CMD_PLAY);           /* Command to be processed */

            /* Increment the Buffer pointer or roll it back when all buffers all full */
            if (IsocOutRdPtr >= (IsocOutBuff + (AUDIO_OUT_PACKET * OUT_PACKET_NUM)))
            {
                /* Roll back to the start of buffer */
                IsocOutRdPtr = IsocOutBuff;
            }
            else
            {
                /* Increment to the next sub-buffer */
                IsocOutRdPtr += AUDIO_OUT_PACKET;
            }
        }

        /* Increment the Buffer pointer or roll it back when all buffers are full */

        /* Trigger the start of streaming only when half buffer is full */
        if (PlayFlag == 0)
        {
            if (IsocOutWrPtr >= (IsocOutBuff + ((AUDIO_OUT_PACKET * OUT_PACKET_NUM) / 2)))
            {
                /* Enable start of Streaming */
                PlayFlag = 5;
            }
        }

        // mchf_board_green_led(0);
    }
    if (epnum == (CDC_OUT_EP & 0x7f))
    {
        return USBD_CDC_cb.DataOut(pdev,epnum);
    }

    return USBD_OK;
}

/**
 * @brief  usbd_audio_SOF
 *         Handles the SOF event (data buffer update and synchronization).
 * @param  pdev: instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t  usbd_audio_SOF (void *pdev)
{
    /* Check if there are available data in stream buffer.
    In this function, a single variable (PlayFlag) is used to avoid software delays.
    The play operation must be executed as soon as possible after the SOF detection. */
#ifdef AUDIO_IN
    if (SendFlag == 1)
    {
        DCD_EP_Flush(pdev,AUDIO_IN_EP);//very important!!!
        audio_in_fill_ep_fifo(pdev);
        SendFlag = 2;
    }
#endif
    if (PlayFlag)
    {
        PlayFlag--;
        if (PlayFlag == 0)
        {
            /* Pause the audio stream */
            AUDIO_OUT_fops.AudioCmd((uint8_t*)(IsocOutBuff),   /* Samples buffer pointer */
                                    AUDIO_OUT_PACKET,          /* Number of samples in Bytes */
                                    AUDIO_CMD_PAUSE);          /* Command to be processed */

            /* Stop entering play loop */
            PlayFlag = 0;

            /* Reset buffer pointers */
            IsocOutRdPtr = IsocOutBuff;
            IsocOutWrPtr = IsocOutBuff;
        }
    }

    return USBD_CDC_cb.SOF(pdev);
}

/**
 * @brief  usbd_audio_OUT_Incplt
 *         Handles the iso out incomplete event.
 * @param  pdev: instance
 * @retval status
 */
static uint8_t  usbd_audio_OUT_Incplt (void  *pdev)
{
    return USBD_OK;
}

/******************************************************************************
     AUDIO Class requests management
 ******************************************************************************/
/**
 * @brief  AUDIO_Req_GetCurrent
 *         Handles the GET_CUR Audio control request.
 * @param  pdev: instance
 * @param  req: setup class request
 * @retval status
 */
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req)
{
    UsbAudioUnit *unit = &usbUnits[HIBYTE(req->wIndex) == AUDIO_OUT_STREAMING_CTRL?UnitVolumeTX:UnitVolumeRX];
    // find the right unit by index

    int16_t* word =(int16_t*) &AudioCtl[0];

    switch(req->bRequest)
    {
    case AUDIO_REQ_GET_MIN:
        word[0] = unit->min;
        break;
    case AUDIO_REQ_GET_MAX:
        word[0] = unit->max;
        break;
    case AUDIO_REQ_GET_RES:
        word[0] = unit->res;
        break;
    case AUDIO_REQ_GET_CUR:
        // we load the current value from the application layer and convert it.
        // not the most beautiful approach
        unit->cur = (((int16_t)(*unit->ptr))-31)*0x100;
        word[0] = unit->cur;
        break;
    }
    USBD_CtlSendData (pdev,
                      AudioCtl,
                      req->wLength);
}

/**
 * @brief  AUDIO_Req_SetCurrent
 *         Handles the SET_CUR Audio control request.
 * @param  pdev: instance
 * @param  req: setup class request
 * @retval status
 */
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req)
{
    if (req->wLength)
    {
        /* Prepare the reception of the buffer over EP0 */
        USBD_CtlPrepareRx (pdev,
                           AudioCtl,
                           req->wLength);

        /* Set the global variables indicating current request and its length
        to the function usbd_audio_EP0_RxReady() which will process the request */
        AudioCtlCmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
        AudioCtlLen = req->wLength;          /* Set the request data length */
        AudioCtlUnit = HIBYTE(req->wIndex);  /* Set the request target unit */
        AudioCtlIf   = LOBYTE(req->wIndex);  /* Set the request target unit */
        AudioCtlCS   = HIBYTE(req->wValue);  /* Set the request target unit */
        AudioCtlCN   = LOBYTE(req->wValue);  /* Set the request target unit */

    }
}

/**
 * @brief  USBD_audio_GetCfgDesc
 *         Returns configuration descriptor.
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length)
{
    *length = sizeof (usbd_audio_CfgDesc);
    return usbd_audio_CfgDesc;
}
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
