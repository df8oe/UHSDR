/**
  ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team
  * @version V2.4.2
  * @date    11-December-2015
  * @brief   This file provides the Audio core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                AUDIO Class  Description
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
  *          The current audio class version supports the following audio features:
  *             - Pulse Coded Modulation (PCM) format
  *             - sampling rate: 48KHz. 
  *             - Bit resolution: 16
  *             - Number of channels: 2
  *             - No volume control
  *             - Mute/Unmute capability
  *             - Asynchronous Endpoints 
  *          
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
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
#include "usbd_audio_cdc_comp.h"
#include "usbd_cdc_if.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "mchf_board.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_AUDIO 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */ 
#define AUDIO_SAMPLE_FREQ(frq)      (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

                                         
/**
  * @}
  */ 




/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */

#define USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE 0x0B
#define USB_INTERFACE_ASSOCIATION_DESC_SIZE 0x08

#define USB_DEVICE_CLASS_MISCELLANEOUS 0xEF


#define CDC_DESCRIPTOR_TYPE                     0x21

#define DEVICE_CLASS_CDC                        0x02
#define DEVICE_SUBCLASS_CDC                     0x00


#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define AUDIO_CONTROL_MUTE                            0x0001
#define AUDIO_CONTROL_VOLUME                          0x0002

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_III                         0x03

#define USB_ENDPOINT_TYPE_ISOCHRONOUS                 0x01
#define AUDIO_ENDPOINT_GENERAL                        0x01

#define AUDIO_REQ_GET_CUR                             0x81
#define AUDIO_REQ_GET_MIN                             0x82
#define AUDIO_REQ_GET_MAX                             0x83
#define AUDIO_REQ_GET_RES                             0x84


// mcHF USB Config below

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define MCHF_AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ * USBD_AUDIO_OUT_CHANNELS * 2) /1000))
#define AUDIO_IN_PACKET                              (uint32_t)(((USBD_AUDIO_IN_FREQ * USBD_AUDIO_IN_CHANNELS * 2) /1000))

#define CDC_DATA_MAX_PACKET_SIZE       32   /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SZE             8    /* Control Endpoint Packet size */

#define CDC_IN_FRAME_INTERVAL          5    /* Number of frames between IN transfers */
#define APP_RX_DATA_SIZE               2048 /* Total size of IN buffer: */


#define CDC_IN_EP                               0x81  /* EP1 for data IN */
#define CDC_OUT_EP                              0x01  /* EP1 for data OUT */
#define CDC_CMD_EP                              0x82  /* EP2 for CDC commands */
#define CDC_CTRL_IF                 0x00
#define CDC_DATA_IF                 0x01
#define CDC_TOTAL_IF_NUM            0x02

#define AUDIO_OUT_EP                    0x02
#define AUDIO_IN_EP                     0x83
#define AUDIO_CTRL_IF                   0x02
#define AUDIO_OUT_IF                    0x03
#define AUDIO_IN_IF                 0x04
#define AUDIO_TOTAL_IF_NUM              0x03

#define USBD_AUDIO_OUT_CHANNELS 2

#define USBD_AUDIO_IN_OUT_DIV 1
// USBD_IN_AUDIO_IN_OUT_DIV must be set to an integer number between 3 and 1
// in order to keep within the limits of the existing code in audio_driver.c audio_rx_processor

#define USBD_AUDIO_IN_CHANNELS 2
#define USBD_AUDIO_IN_FREQ (USBD_AUDIO_FREQ/USBD_AUDIO_IN_OUT_DIV)


#define AUDIO_PACKET_SZE(frq,channels)          (uint8_t)(((frq * channels * 2)/1000) & 0xFF), \
                                       (uint8_t)((((frq * channels * 2)/1000) >> 8) & 0xFF)
#define SAMPLE_FREQ(frq)               (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

// local stuff
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
          USBD_LL_Transmit(pdev,AUDIO_IN_EP, pkt, AUDIO_IN_PACKET);
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
          USBD_LL_Transmit(pdev,AUDIO_IN_EP, (uint8_t*)Silence, AUDIO_IN_PACKET);
      }
  }

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


static uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_AUDIO_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_AUDIO_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_AUDIO_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_AUDIO_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_Private_Variables
  * @{
  */ 

USBD_ClassTypeDef  USBD_AUDIO = 
{
  USBD_AUDIO_Init,
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup,
  USBD_AUDIO_EP0_TxReady,  
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn,
  USBD_AUDIO_DataOut,
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete,
  USBD_AUDIO_IsoOutIncomplete,      
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc, 
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc,
};

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
{
        /* Configuration 1 */
        0x09,                                 /* bLength */
        USB_DESC_TYPE_CONFIGURATION,            // USB_CONFIGURATION_DESCRIPTOR_TYPE,    /* bDescriptorType */
        LOBYTE(USB_AUDIO_CONFIG_DESC_SIZ),       /* wTotalLength  109 bytes*/
        HIBYTE(USB_AUDIO_CONFIG_DESC_SIZ),
        0x05,                                 /* bNumInterfaces */
        0x01,                                 /* bConfigurationValue */
        0x00,                                 /* iConfiguration */
        0xC0,                                 /* bmAttributes  BUS Powred*/
        0x32,                                 /* bMaxPower = 100 mA*/
        /* 09 byte*/
        /*---------------------------------------------------------------------------*/
        USB_INTERFACE_ASSOCIATION_DESC_SIZE,
        USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,
        CDC_CTRL_IF,                          /* first interface */
        CDC_TOTAL_IF_NUM,                     /* bNumInterfaces */
        0x02,               /* bInterfaceClass */
        0x02,          /* bInterfaceSubClass */
        0x01,             /* bInterfaceProtocol */
        0x00,                                 /* String Index */

        /*Interface Descriptor */
        0x09,   /* bLength: Interface Descriptor size */
        USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
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
        USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
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
        AUDIO_CTRL_IF,                        /* first interface */
        AUDIO_TOTAL_IF_NUM,                   /* bNumInterfaces */
        USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
        AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
        AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
        0x00,                                 /* String Index */

        /* USB Sound Standard interface descriptor */
        AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
        USB_DESC_TYPE_INTERFACE,        /* bDescriptorType */
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
        AUDIO_IN_IF,                                    /* baInterfaceNr */
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
        0x06,                                 /* bUnitID */
        0x04,                                 /* bSourceID */
        0x01,                                 /* bControlSize */
        AUDIO_CONTROL_VOLUME,                 /* bmaControls(0) */
        0x00,                                 /* bmaControls(1) */
        0x00,                                 /* iTerminal */
        /* 09 byte*/

        /* AC Selector Unit Descriptor */
        0x07, //    bLength
        0x24, //    bDescriptorType
        0x05, //    bDescriptorSubtype
        0x07, //    bUnitID
        0x01, //    bBrInPins
        0x06, //    baSourceID(1)
        0x00, //    iSelector
    #endif
        // ========================================== END AudioControl
        /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
        /* Interface 1, Alternate Setting 0                                             */
        AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
        USB_DESC_TYPE_INTERFACE,        /* bDescriptorType */
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
        USB_DESC_TYPE_INTERFACE,        /* bDescriptorType */
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
        USB_DESC_TYPE_INTERFACE,    // INTERFACE descriptor type (bDescriptorType) 0x04
        AUDIO_IN_IF, // Index of this interface. (bInterfaceNumber) ?????????? (3<) (1<<) (1<M)
        0x00,                         // Index of this alternate setting. (bAlternateSetting)
        0x00,                         // 0 endpoints.   (bNumEndpoints)
        USB_DEVICE_CLASS_AUDIO,       // AUDIO (bInterfaceClass)
        AUDIO_SUBCLASS_AUDIOSTREAMING, // AUDIO_STREAMING (bInterfaceSubclass)
        0x00,                         // Unused. (bInterfaceProtocol)
        0x00,                         // Unused. (iInterface)

        /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 1) (CODE == 4)*/
        0x09,                         // Size of the descriptor, in bytes (bLength)
        USB_DESC_TYPE_INTERFACE,     // INTERFACE descriptor type (bDescriptorType)
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

#if 0
        // CubeMX Demo Desc
  /* Configuration 1 */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_CONFIGURATION,          /* bDescriptorType */
  LOBYTE(USB_AUDIO_CONFIG_DESC_SIZ),    /* wTotalLength  109 bytes*/
  HIBYTE(USB_AUDIO_CONFIG_DESC_SIZ),
  0x02,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  0xC0,                                 /* bmAttributes  BUS Powred*/
  0x32,                                 /* bMaxPower = 100 mA*/
  /* 09 byte*/

  /* USB Speaker Standard interface descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
  /* 09 byte*/

  /* USB Speaker Class-specific AC Interface Descriptor */
  AUDIO_INTERFACE_DESC_SIZE,            /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
  0x00,          /* 1.00 */             /* bcdADC */
  0x01,
  0x27,                                 /* wTotalLength = 39*/
  0x00,
  0x01,                                 /* bInCollection */
  0x01,                                 /* baInterfaceNr */
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
  AUDIO_CONTROL_MUTE,// |AUDIO_CONTROL_VOLUME, /* bmaControls(0) */
  0,                                    /* bmaControls(1) */
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

  /* USB Speaker Standard AS Interface Descriptor - Audio Streaming Zero Bandwith */
  /* Interface 1, Alternate Setting 0                                             */
  AUDIO_INTERFACE_DESC_SIZE,  /* bLength */
  USB_DESC_TYPE_INTERFACE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
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
  USB_DESC_TYPE_INTERFACE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
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
  AUDIO_SAMPLE_FREQ(USBD_AUDIO_FREQ),         /* Audio sampling frequency coded on 3 bytes */
  /* 11 byte*/

  /* Endpoint 1 - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType */
  AUDIO_OUT_EP,                         /* bEndpointAddress 1 out endpoint*/
  USBD_EP_TYPE_ISOC,                    /* bmAttributes */
  AUDIO_PACKET_SZE(USBD_AUDIO_FREQ),    /* wMaxPacketSize in Bytes (Freq(Samples)*2(Stereo)*2(HalfWord)) */
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

#endif
} ;

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END=
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_AUDIO_Init
  *         Initialize the AUDIO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static void* cdcClassData;
static void* audioClassData;
static void* cdcUserData;
static void* audioUserData;

static inline void switchToCdc(USBD_HandleTypeDef *pdev)
{
    pdev->pClassData = cdcClassData;
    pdev->pUserData = cdcUserData;
}

static inline void switchToAudio(USBD_HandleTypeDef *pdev)
{
    pdev->pClassData = audioClassData;
    pdev->pUserData = audioUserData;
}

static uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  
  /* Open EP OUT */
  USBD_LL_OpenEP(pdev,
                 AUDIO_OUT_EP,
                 USBD_EP_TYPE_ISOC,
                 AUDIO_OUT_PACKET);
  
  /* Allocate Audio structure */
  pdev->pClassData = USBD_malloc(sizeof (USBD_AUDIO_HandleTypeDef));
  
  if(pdev->pClassData == NULL)
  {
    return USBD_FAIL; 
  }
  else
  {
    haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
    memset(haudio,0,sizeof(USBD_AUDIO_HandleTypeDef));
    haudio->offset = AUDIO_OFFSET_UNKNOWN;
    haudio->wr_ptr = 0; 
    haudio->rd_ptr = 0;  
    haudio->rd_enable = 0;
    
    /* Initialize the Audio output Hardware layer */
    if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0) != USBD_OK)
    {
      return USBD_FAIL;
    }
    
    /* Prepare Out endpoint to receive 1st packet */ 
    USBD_LL_PrepareReceive(pdev,
                           AUDIO_OUT_EP,
                           haudio->buffer,                        
                           AUDIO_OUT_PACKET);      
  }
  // return USBD_OK;

  audioClassData = pdev->pClassData;
  audioUserData = pdev->pUserData;
  uint8_t retval = USBD_CDC.Init(pdev,cfgidx);
  cdcClassData = pdev->pClassData;
  cdcUserData = pdev->pUserData;
  return retval;
}

/**
  * @brief  USBD_AUDIO_Init
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
  
  /* Open EP OUT */
  USBD_LL_CloseEP(pdev,
              AUDIO_OUT_EP);

  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
   ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->DeInit(0);
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  if (cdcClassData != NULL)
  {
      switchToCdc(pdev);
      ((USBD_CDC_ItfTypeDef *)pdev->pUserData)->DeInit();
       USBD_free(pdev->pClassData);
       pdev->pClassData = NULL;
  }
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Setup
  *         Handle the AUDIO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_AUDIO_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  uint16_t len;
  uint8_t *pbuf;
  uint8_t ret = USBD_OK;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
      if (req->wIndex == CDC_CTRL_IF)
      {
          switchToCdc(pdev);
          ret = USBD_CDC.Setup(pdev,req);
          switchToAudio(pdev);
      }
      else
      {
          switch (req->bRequest)
          {
          case AUDIO_REQ_GET_MIN:
          case AUDIO_REQ_GET_MAX:
          case AUDIO_REQ_GET_RES:
          case AUDIO_REQ_GET_CUR:
              AUDIO_REQ_GetCurrent(pdev, req);
              break;

          case AUDIO_REQ_SET_CUR:
              AUDIO_REQ_SetCurrent(pdev, req);
              break;

          default:
              USBD_CtlError (pdev, req);
              ret = USBD_FAIL;
          }
      }
      break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:      
      if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
      {
        pbuf = USBD_AUDIO_CfgDesc + 18;
        len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);
        
        
        USBD_CtlSendData (pdev, 
                          pbuf,
                          len);
      }
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&(haudio->alt_setting),
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES)
      {
        haudio->alt_setting[req->wIndex] = (uint8_t)(req->wValue);

#ifdef AUDIO_IN
// interface 2 is AUDIO_IN
                if (haudio->alt_setting[AUDIO_IN_IF] == 1)
                {
                    if (!haudio->SendFlag)
                    {
                        haudio->SendFlag = 1;
                    }
                }
                else
                {
                    haudio->SendFlag = 0;
                    USBD_LL_FlushEP(pdev,AUDIO_IN_EP);
                }
#endif
      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      ret = USBD_FAIL;     
    }
  }
  return ret;
}


/**
  * @brief  USBD_AUDIO_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_AUDIO_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_AUDIO_CfgDesc);
  return USBD_AUDIO_CfgDesc;
}


/**
  * @brief  USBD_AUDIO_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DataIn (USBD_HandleTypeDef *pdev, 
                              uint8_t epnum)
{
    uint8_t retval = USBD_OK;
#ifdef AUDIO_IN
    if (epnum == (AUDIO_IN_EP & 0x7f))
    {
        USBD_LL_FlushEP(pdev,AUDIO_IN_EP); //very important!!!
        audio_in_fill_ep_fifo(pdev);
    }
#endif
    if (epnum == (CDC_IN_EP & 0x7f))
    {
        switchToCdc(pdev);
        retval = USBD_CDC.DataIn(pdev,epnum);
        switchToAudio(pdev);
    }
  return retval;
}

/**
  * @brief  USBD_AUDIO_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
  uint8_t retval = USBD_OK;

  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
  {/* In this driver, to simplify code, only SET_CUR request is managed */

    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
    {
     ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);     
      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
  } 
  else
  {
      switchToCdc(pdev);
      retval = USBD_CDC.EP0_RxReady(pdev);
      switchToAudio(pdev);
  }


  return retval;
}
/**
  * @brief  USBD_AUDIO_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_EP0_TxReady (USBD_HandleTypeDef *pdev)
{
  /* Only OUT control data are processed */
  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_AUDIO_SOF (USBD_HandleTypeDef *pdev)
{
  return USBD_OK;
}

/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
void  USBD_AUDIO_Sync (USBD_HandleTypeDef *pdev, AUDIO_OffsetTypeDef offset)
{
  int8_t shift = 0;
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  haudio->offset =  offset; 
  
  
  if(haudio->rd_enable == 1)
  {
    haudio->rd_ptr += AUDIO_TOTAL_BUF_SIZE/2;
    
    if (haudio->rd_ptr == AUDIO_TOTAL_BUF_SIZE)
    {
      /* roll back */
      haudio->rd_ptr = 0;
    }
  }
  
  if(haudio->rd_ptr > haudio->wr_ptr)
  {
    if((haudio->rd_ptr - haudio->wr_ptr) < AUDIO_OUT_PACKET)
    {
      shift = -4;
    }
    else if((haudio->rd_ptr - haudio->wr_ptr) > (AUDIO_TOTAL_BUF_SIZE - AUDIO_OUT_PACKET))
    {
      shift = 4;
    }    

  }
  else
  {
    if((haudio->wr_ptr - haudio->rd_ptr) < AUDIO_OUT_PACKET)
    {
      shift = 4;
    }
    else if((haudio->wr_ptr - haudio->rd_ptr) > (AUDIO_TOTAL_BUF_SIZE - AUDIO_OUT_PACKET))
    {
      shift = -4;
    }  
  }

  if(haudio->offset == AUDIO_OFFSET_FULL)
  {
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0],
                                                         AUDIO_TOTAL_BUF_SIZE/2 - shift,
                                                         AUDIO_CMD_PLAY); 
      haudio->offset = AUDIO_OFFSET_NONE;           
  }
}

/**
  * @brief  USBD_AUDIO_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_AUDIO_DataOut (USBD_HandleTypeDef *pdev, 
                              uint8_t epnum)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  if (epnum == AUDIO_OUT_EP)
  {
    /* Increment the Buffer pointer or roll it back when all buffers are full */
    
    haudio->wr_ptr += AUDIO_OUT_PACKET;
    
    if (haudio->wr_ptr == AUDIO_TOTAL_BUF_SIZE)
    {/* All buffers are full: roll back */
      haudio->wr_ptr = 0;
      
      if(haudio->offset == AUDIO_OFFSET_UNKNOWN)
      {
        ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->AudioCmd(&haudio->buffer[0],
                                                             AUDIO_TOTAL_BUF_SIZE/2,
                                                             AUDIO_CMD_START);
          haudio->offset = AUDIO_OFFSET_NONE;
      }
    }
    
    if(haudio->rd_enable == 0)
    {
      if (haudio->wr_ptr == (AUDIO_TOTAL_BUF_SIZE / 2))
      {
        haudio->rd_enable = 1; 
      }
    }
    
    /* Prepare Out endpoint to receive next audio packet */
    USBD_LL_PrepareReceive(pdev,
                           AUDIO_OUT_EP,
                           &haudio->buffer[haudio->wr_ptr], 
                           AUDIO_OUT_PACKET);  
      
  }
  
  return USBD_OK;
}

/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{  
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  memset(haudio->control.data, 0, 64);
  /* Send the current mute state */
  USBD_CtlSendData (pdev, 
                    haudio->control.data,
                    req->wLength);
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{ 
  USBD_AUDIO_HandleTypeDef   *haudio;
  haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;
  
  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx (pdev,
                       haudio->control.data,                                  
                       req->wLength);    
    
    haudio->control.cmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    haudio->control.len = req->wLength;          /* Set the request data length */
    haudio->control.unit = HIBYTE(req->wIndex);  /* Set the request target unit */
  }
}


/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_AUDIO_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_AUDIO_DeviceQualifierDesc);
  return USBD_AUDIO_DeviceQualifierDesc;
}

/**
* @brief  USBD_AUDIO_RegisterInterface
* @param  fops: Audio interface callback
* @retval status
*/
uint8_t  USBD_AUDIO_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                        USBD_AUDIO_ItfTypeDef *fops)
{
  if(fops != NULL)
  {
    pdev->pUserData= fops;
  }
  return 0;
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
