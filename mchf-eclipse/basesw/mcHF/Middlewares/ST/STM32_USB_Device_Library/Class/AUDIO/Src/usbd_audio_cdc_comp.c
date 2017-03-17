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

                                         
/**
  * @}
  */ 




/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */


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


#define AUDIO_OUT_EP                0x02
#define AUDIO_IN_EP                 0x83
#define AUDIO_CTRL_IF               0x02
#define AUDIO_OUT_IF                0x03
#define AUDIO_IN_IF                 0x04
#define AUDIO_TOTAL_IF_NUM          0x03



#define OUT_PACKET_NUM                                   4
/* Total size of the audio transfer buffer */
#define TOTAL_OUT_BUF_SIZE                           ((uint32_t)(AUDIO_OUT_PACKET * OUT_PACKET_NUM))


uint8_t  IsocOutBuff [TOTAL_OUT_BUF_SIZE * 2];
uint8_t* IsocOutWrPtr = IsocOutBuff;
uint8_t* IsocOutRdPtr = IsocOutBuff;

static void audio_out_packet(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef   *haudio, USBD_AUDIO_ItfTypeDef *ops)
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
    /*
    ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame =
        (((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].even_odd_frame)? 0:1;

*/
    /* Prepare Out endpoint to receive next audio packet */
   USBD_LL_PrepareReceive(pdev,
                     AUDIO_OUT_EP,
                     (uint8_t*)(IsocOutWrPtr),
                     AUDIO_OUT_PACKET);

    if (haudio->PlayFlag)
    {
        haudio->PlayFlag = 5;
        /* Start playing received packet */
        ops->AudioCmd(IsocOutRdPtr,
                AUDIO_OUT_PACKET,
                AUDIO_CMD_PLAY);

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
    if (haudio->PlayFlag == 0)
    {
        if (IsocOutWrPtr >= (IsocOutBuff + ((AUDIO_OUT_PACKET * OUT_PACKET_NUM) / 2)))
        {
            /* Enable start of Streaming */
            haudio->PlayFlag = 5;
        }
    }
}

static void audio_out_packet_prepare(USBD_HandleTypeDef* pdev, USBD_AUDIO_HandleTypeDef   *haudio, USBD_AUDIO_ItfTypeDef *ops)
{
    if (haudio->PlayFlag)
    {
        haudio->PlayFlag--;
        if (haudio->PlayFlag == 0)
        {
            /* Pause the audio stream */
            ops->AudioCmd(IsocOutBuff,   /* Samples buffer pointer */
                    AUDIO_OUT_PACKET,          /* Number of samples in Bytes */
                    AUDIO_CMD_PAUSE);          /* Command to be processed */

            /* Stop entering play loop */
            haudio->PlayFlag = 0;

            /* Reset buffer pointers */
            IsocOutRdPtr = IsocOutBuff;
            IsocOutWrPtr = IsocOutBuff;
        }
    }
}




// local stuff
#define USB_AUDIO_IN_NUM_BUF 8
#define USB_AUDIO_IN_PKT_SIZE   (AUDIO_IN_PACKET/2)
// size in samples not size in bytes
#define USB_AUDIO_IN_BUF_SIZE (USB_AUDIO_IN_NUM_BUF * USB_AUDIO_IN_PKT_SIZE)




static int16_t Silence[USB_AUDIO_IN_PKT_SIZE];

typedef struct {
    int16_t  buffer[USB_AUDIO_IN_BUF_SIZE]; //buffer for filtered PCM data from Recv.
    uint16_t buffer_tail;
    uint16_t buffer_head;
    uint16_t buffer_overflow;
} audio_buffer_t;

static volatile audio_buffer_t in;

void audio_in_put_buffer(int16_t sample)
{
    in.buffer[in.buffer_head] = sample;
    in.buffer_head=  (in.buffer_head + 1) %USB_AUDIO_IN_BUF_SIZE;
    // now test buffer full
    if (in.buffer_head == in.buffer_tail)
    {
        // ok. We loose data now, should never ever happen, but so what
        // will cause minor distortion if only a few bytes.
        in.buffer_overflow++;
    }
}

volatile int16_t* audio_in_buffer_next_pkt()
{
    int16_t *retval;
    uint16_t room;
    uint16_t temp_head = in.buffer_head;
    room = ((((temp_head < in.buffer_tail)?USB_AUDIO_IN_BUF_SIZE:0) + temp_head) - in.buffer_tail);
    if (room >= USB_AUDIO_IN_PKT_SIZE)
    {
        retval = (int16_t*)&in.buffer[in.buffer_tail];
    }
    else
    {
        retval = NULL;
    }
    return retval;
}
void audio_in_buffer_pop_pkt(int16_t* ptr)
{
    if (ptr)
    {
        // there was data and pkt has been used
        // free  the space
        in.buffer_tail = (in.buffer_tail+USB_AUDIO_IN_PKT_SIZE)%USB_AUDIO_IN_BUF_SIZE;
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
        .cn  = AUDIO_IN_STREAMING_CTRL,
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


static uint8_t  USBD_AUDIO_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  USBD_AUDIO_HandleTypeDef   *haudio;
  
  /* Open EP OUT */
  USBD_LL_OpenEP(pdev,
                 AUDIO_OUT_EP,
                 USBD_EP_TYPE_ISOC,
                 AUDIO_OUT_PACKET);

  /* Open EP IN */
  USBD_LL_OpenEP(pdev,
              AUDIO_IN_EP,
              USBD_EP_TYPE_ISOC,
              AUDIO_IN_PACKET);

  
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
#if 0
    haudio->offset = AUDIO_OFFSET_UNKNOWN;
    haudio->wr_ptr = 0; 
    haudio->rd_ptr = 0;  
    haudio->rd_enable = 0;
#endif
    /* Initialize the Audio output Hardware layer */
    if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Init(USBD_AUDIO_FREQ, AUDIO_DEFAULT_VOLUME, 0) != USBD_OK)
    {
      return USBD_FAIL;
    }
    
    /* Prepare Out endpoint to receive 1st packet */ 
    USBD_LL_PrepareReceive(pdev,
                           AUDIO_OUT_EP,
                           IsocOutBuff,
                           AUDIO_OUT_PACKET);      
  }
  return USBD_OK;
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

  /* Open EP OUT */
  USBD_LL_CloseEP(pdev,
              AUDIO_IN_EP);

  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
   ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->DeInit(0);
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
  uint8_t ret = USBD_OK;
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
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
      break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:      
        // handled at composite level
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&(haudio->alt_setting),
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wIndex) <= USBD_MAX_NUM_INTERFACES)
      {
        haudio->alt_setting[req->wIndex] = (uint8_t)(req->wValue);

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
  *length = 0;
  return NULL;
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
    if (epnum == (AUDIO_IN_EP & 0x7f))
    {
        USBD_LL_FlushEP(pdev,AUDIO_IN_EP); //very important!!!
        audio_in_fill_ep_fifo(pdev);
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
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  
  if (haudio->control.cmd == AUDIO_REQ_SET_CUR)
  {/* In this driver, to simplify code, only SET_CUR request is managed */

    if (haudio->control.unit == AUDIO_OUT_STREAMING_CTRL)
    {
      // ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);
      int16_t val = (int16_t)((uint16_t)haudio->control.data[0] | ((uint16_t)haudio->control.data[1])<<8) ;
      retval =((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->VolumeCtl((val/0x100)+31);
      usbUnits[UnitVolumeTX].cur = val;

      haudio->control.cmd = 0;
      haudio->control.len = 0;
    }
    else
    {
        // ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->MuteCtl(haudio->control.data[0]);
        int16_t val = (int16_t)((uint16_t)haudio->control.data[0] | ((uint16_t)haudio->control.data[1])<<8) ;
        retval =((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->InVolumeCtl((val/0x100)+31);
        usbUnits[UnitVolumeTX].cur = val;

        haudio->control.cmd = 0;
        haudio->control.len = 0;
    }
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
    USBD_AUDIO_HandleTypeDef* haudio = (USBD_AUDIO_HandleTypeDef*) pdev->pClassData;

    if (haudio->SendFlag == 1)
    {
        USBD_LL_FlushEP(pdev,AUDIO_IN_EP);//very important!!!
        audio_in_fill_ep_fifo(pdev);
        haudio->SendFlag = 2;
    }

    audio_out_packet_prepare(pdev, haudio, (USBD_AUDIO_ItfTypeDef *)pdev->pUserData);
    return USBD_OK;
}

#if 0
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
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  
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
#endif
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
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  
  if (epnum == AUDIO_OUT_EP)
  {

    audio_out_packet(pdev, haudio, (USBD_AUDIO_ItfTypeDef *)pdev->pUserData);

#if 0
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
#endif
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
  
  UsbAudioUnit *unit = &usbUnits[HIBYTE(req->wIndex) == AUDIO_OUT_STREAMING_CTRL?UnitVolumeTX:UnitVolumeRX];
  // find the right unit by index

  memset(haudio->control.data, 0, 64);
  int16_t* word =(int16_t*) &haudio->control.data[0];

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
  haudio = (USBD_AUDIO_HandleTypeDef*)pdev->pClassData;
  
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
