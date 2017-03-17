/**
 ******************************************************************************
 * @file    USBD_COMP.c
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
#include <drivers/usb/device/class/composite/usbd_composite.h>
#include <drivers/usb/device/class/composite/usbd_composite_desc.h>
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "mchf_board.h"
#include "usbd_cdc.h"
#include "usbd_audio_cdc_comp.h"
#include "usbd_audio_if.h"
#include "usbd_cdc_if.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
 * @{
 */


/** @defgroup USBD_COMP
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_COMP_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */


/** @defgroup USBD_COMP_Private_Defines
 * @{
 */

/**
 * @}
 */


/** @defgroup USBD_COMP_Private_Macros
 * @{
 */
#define AUDIO_SAMPLE_FREQ(frq)      (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))


/**
 * @}
 */




/** @defgroup USBD_COMP_Private_FunctionPrototypes
 * @{
 */







/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_COMP_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
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
        LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
        HIBYTE(CDC_CMD_PACKET_SIZE),
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
        LOBYTE(CDC_DATA_FS_OUT_PACKET_SIZE),  /* wMaxPacketSize: */
        HIBYTE(CDC_DATA_FS_OUT_PACKET_SIZE),
        0x00,                              /* bInterval: ignore for Bulk transfer */

        /*Endpoint IN Descriptor*/
        0x07,   /* bLength: Endpoint Descriptor size */
        USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
        CDC_IN_EP,                         /* bEndpointAddress */
        0x02,                              /* bmAttributes: Bulk */
        LOBYTE(CDC_DATA_FS_IN_PACKET_SIZE),  /* wMaxPacketSize: */
        HIBYTE(CDC_DATA_FS_IN_PACKET_SIZE),
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
        AUDIO_INTERFACE_DESC_SIZE + 1,            /* bLength */
        AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
        AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
        0x00,          /* 1.00 */             /* bcdADC */
        0x01,
        61  + 9 + 7,
        0x00,
        0x02,                                 /* bInCollection */
        AUDIO_OUT_IF,                         /* baInterfaceNr */
        AUDIO_IN_IF,                          /* baInterfaceNr */

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
} ;


const usbd_ep_map_t usbdEpMap =
{
        .in = { CLASS_UNUSED, CLASS_CDC, CLASS_CDC, CLASS_AUDIO },
        .out = { CLASS_UNUSED, CLASS_CDC, CLASS_AUDIO, CLASS_UNUSED }
};

USBD_ClassCompInfo dev_instance[CLASS_NUM] =
{
        {
                .class = &USBD_AUDIO,
                .userData = &USBD_AUDIO_fops_FS,
                .ctrlIf = AUDIO_CTRL_IF,
                .minIf = AUDIO_CTRL_IF,
                .maxIf = AUDIO_IN_IF
        },
        {
                .class = &USBD_CDC,
                .userData = &USBD_Interface_fops_FS,
                .ctrlIf = CDC_CTRL_IF,
                .minIf = CDC_CTRL_IF,
                .maxIf = CDC_DATA_IF
        }
};

