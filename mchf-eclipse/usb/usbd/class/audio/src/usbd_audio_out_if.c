/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/**
  ******************************************************************************
  * @file    usbd_audio_out_if.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the Audio Out (palyback) interface API.
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
#include "mchf_board.h"
#include "usbd_audio_core.h"
#include "usbd_audio_out_if.h"
/*
 * Theory of operation:
 * 1.Audio Out ( AKA digital line in) is only used  when TX is active,
 * otherwise audio out data can be discarded.
 * 2. Digital Audio is always delivered using 48khz, Stereo, 16bit.
 * 3. Both I/Q or plain audio can be send. The audio driver will do the necessary conversions.
 *    Here only plain data straight from USB is transferred.
 * 4. User needs to select DIG or DIQ mode for tx signal input in order to use digital signal.
 * 5. If no data is available for whatever reason, the audio driver gets silence delivered, i.e.
 *    the audio driver will not/does not have to know if there is an issue with USB data stream.
 *    Device will transmit silence :-)
 *
 * States:
 * 1. :Init: All Buffers are reset to be empty, asking for buffer data will deliver silence.
 * 			 Next State is :Idle:
 * 2. :Idle: All buffers are empty, asking for buffer data will deliver silence.
 * 			 It is checked if TX is being requested and the TX signal input is set to DIG or DIQ,
 * 			 then transfer to :TxActive:
 * 3. :Tx:   If USB delivers data, these are now stored in buffer. Asking for buffer data before
 *           buffer has been filled sufficiently (half) will deliver silence in order to prevent
 *           output jitter.
 *           If TX is no longer requested, state will change to :Init:
 *
 *	Implementation Steps:
 *	- Add DIG / DIQ to Audio Input Settings
 *	- At least DIG could be directly switched to by CAT Interface (Setting to DIG mode), needs
 *	  modification of cat_driver to do so. CAT driver could record previous setting in order to switch
 *	  back if DIG is disabled
 *	- audio_tx_processor reads from out_buffer instead of using the data from the I2S
 *	- USB driver has to copy data to output buffer if Txing and DIG is enabled.
 *	- DIQ will not be enabled for now.
 *
 */
/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup usbd_audio_out_if
  * @brief usbd out interface module
  * @{
  */

/** @defgroup usbd_audio_out_if_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup usbd_audio_out_if_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup usbd_audio_out_if_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup usbd_audio_out_if_Private_FunctionPrototypes
  * @{
  */
static uint8_t  Init         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
static uint8_t  DeInit       (uint32_t options);
static uint8_t  AudioCmd     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
static uint8_t  VolumeCtl    (uint8_t vol);
static uint8_t  MuteCtl      (uint8_t cmd);
static uint8_t  PeriodicTC   (uint8_t cmd);
static uint8_t  GetState     (void);


// static uint8_t  InInit         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
// static uint8_t  InDeInit       (uint32_t options);
// static uint8_t  InAudioCmd     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
static uint8_t  InVolumeCtl    (uint8_t vol);
// static uint8_t  InMuteCtl      (uint8_t cmd);
// static uint8_t  InPeriodicTC   (uint8_t cmd);
// static uint8_t  INGetState     (void);

/**
  * @}
  */

/** @defgroup usbd_audio_out_if_Private_Variables
  * @{
  */
AUDIO_FOPS_TypeDef  AUDIO_OUT_fops =
{
    Init,
    DeInit,
    AudioCmd,
    VolumeCtl,
    MuteCtl,
    PeriodicTC,
    GetState
};

AUDIO_FOPS_TypeDef  AUDIO_IN_fops =
{
    Init,
    DeInit,
    AudioCmd,
    InVolumeCtl,
    MuteCtl,
    PeriodicTC,
    GetState
};


static uint8_t AudioState = AUDIO_STATE_INACTIVE;

enum USB_DIG_AUDIO
{
    USB_DIG_AUDIO_OUT_INIT = 0,
    USB_DIG_AUDIO_OUT_IDLE,
    USB_DIG_ADUIO_OUT_TX,
} usb_dig_audio_state = USB_DIG_AUDIO_OUT_INIT;


/**
  * @}
  */

/** @defgroup usbd_audio_out_if_Private_Functions
  * @{
  */

#define USB_AUDIO_OUT_NUM_BUF 16
#define USB_AUDIO_OUT_PKT_SIZE   (AUDIO_OUT_PACKET/2)
#define USB_AUDIO_OUT_BUF_SIZE (USB_AUDIO_OUT_NUM_BUF * USB_AUDIO_OUT_PKT_SIZE)

static volatile int16_t out_buffer[USB_AUDIO_OUT_BUF_SIZE]; //buffer for filtered PCM data from Recv.
static volatile uint16_t out_buffer_tail;
static volatile uint16_t out_buffer_head;
static volatile uint16_t out_buffer_overflow;

static void audio_out_put_buffer(int16_t sample)
{

    uint32_t next_head = (out_buffer_head + 1) %USB_AUDIO_OUT_BUF_SIZE;

    if (next_head != out_buffer_head)
    {
        out_buffer[out_buffer_head] = sample;
        out_buffer_head = next_head;
    }
    else
    {
        // ok. We loose data now, should never ever happen, but so what
        // will cause minor distortion if only a few bytes.
        out_buffer_overflow++;
    }
}
volatile int16_t* audio_out_buffer_next_pkt(uint32_t len)
{
    uint16_t room;
    uint16_t temp_head = out_buffer_head;
    room = ((((temp_head < out_buffer_tail)?USB_AUDIO_OUT_BUF_SIZE:0) + temp_head) - out_buffer_tail);
    if (room >= len)
    {
        return &out_buffer[out_buffer_tail];
    }
    else
    {

        return NULL;
    }
}
static void audio_out_buffer_pop_pkt(volatile int16_t* ptr, uint32_t len)
{
    if (ptr)
    {
        // there was data and pkt has been used
        // free  the space
        out_buffer_tail = (out_buffer_tail+len)%USB_AUDIO_OUT_BUF_SIZE;
    }
}

/* len is length in 16 bit samples */
void audio_out_fill_tx_buffer(int16_t *buffer, uint32_t len)
{
    volatile int16_t *pkt = audio_out_buffer_next_pkt(len);

    static uint16_t fill_buffer = 1;
    if (fill_buffer == 0 && pkt)
    {
        uint32_t idx;
        for (idx = len; idx; idx--)
        {
            *buffer++ = *pkt++;
        }
        audio_out_buffer_pop_pkt(pkt,len);
    }
    else
    {
        if (fill_buffer == 0)
        {
            fill_buffer = 1;
        }
        if (audio_out_buffer_next_pkt((USB_AUDIO_OUT_BUF_SIZE*2)/3) != NULL)
        {
            fill_buffer = 0;
        }
        // Deliver silence if not enough data is stored in buffer
        // TODO: Make this more efficient by providing 4byte aligned buffers only (and requesting len in 4 byte increments)
        for (; len; len--)
        {
            *buffer++=0;
        }
    }
}



/**
  * @brief  Init
  *         Initialize and configures all required resources for audio play function.
  * @param  AudioFreq: Statrtup audio frequency.
  * @param  Volume: Startup volume to be set.
  * @param  options: specific options passed to low layer function.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  Init         (uint32_t AudioFreq,
                              uint32_t Volume,
                              uint32_t options)
{
    static uint32_t Initialized = 0;

    /* Check if the low layer has already been initialized */
    if (Initialized == 0)
    {
        /* Call low layer function */
//    if (EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, Volume, AudioFreq) != 0)
//    {
//      AudioState = AUDIO_STATE_ERROR;
//      return AUDIO_FAIL;
//    }

        /* Set the Initialization flag to prevent reinitializing the interface again */
        Initialized = 1;
    }

    /* Update the Audio state machine */
    AudioState = AUDIO_STATE_ACTIVE;

    return AUDIO_OK;
}

/**
  * @brief  DeInit
  *         Free all resources used by low layer and stops audio-play function.
  * @param  options: options passed to low layer function.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  DeInit       (uint32_t options)
{
    /* Update the Audio state machine */
    AudioState = AUDIO_STATE_INACTIVE;

    return AUDIO_OK;
}

/**
  * @brief  AudioCmd
  *         Play, Stop, Pause or Resume current file.
  * @param  pbuf: address from which file shoud be played.
  * @param  size: size of the current buffer/file.
  * @param  cmd: command to be executed, can be AUDIO_CMD_PLAY , AUDIO_CMD_PAUSE,
  *              AUDIO_CMD_RESUME or AUDIO_CMD_STOP.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */

static uint8_t  AudioCmd(uint8_t* pbuf,
                         uint32_t size,
                         uint8_t cmd)
{
    /* Check the current state */
    if ((AudioState == AUDIO_STATE_INACTIVE) || (AudioState == AUDIO_STATE_ERROR))
    {
        AudioState = AUDIO_STATE_ERROR;
        return AUDIO_FAIL;
    }

    switch (cmd)
    {
    /* Process the PLAY command ----------------------------*/
    case AUDIO_CMD_PLAY:
        /* If current state is Active or Stopped */
        if ((AudioState == AUDIO_STATE_ACTIVE) || \
                (AudioState == AUDIO_STATE_STOPPED) || \
                (AudioState == AUDIO_STATE_PAUSED) || \
                (AudioState == AUDIO_STATE_PLAYING))
        {


            if (ts.txrx_mode == TRX_MODE_TX)
            {
                uint16_t* pkt = (uint16_t*)pbuf;
                uint32_t count;
                for (count =0; count < size/2; count++)
                {

                    audio_out_put_buffer(pkt[count]);
                }
            }
            AudioState = AUDIO_STATE_PLAYING;
            return AUDIO_OK;
        }
        /* If current state is Paused */
        //  else if (AudioState == AUDIO_STATE_PAUSED)
        //	{
        //     if (EVAL_AUDIO_PauseResume(AUDIO_RESUME, (uint32_t)pbuf, (size/2)) != 0)
        //     {
        //       AudioState = AUDIO_STATE_ERROR;
        //       return AUDIO_FAIL;
        //      }
        //      else
        //     {
        //        AudioState = AUDIO_STATE_PLAYING;
        //        return AUDIO_OK;
        else /* Not allowed command */
        {
            return AUDIO_FAIL;
        }
        break;

    /* Process the STOP command ----------------------------*/
    case AUDIO_CMD_STOP:
        if (AudioState != AUDIO_STATE_PLAYING)
        {
            /* Unsupported command */
            return AUDIO_FAIL;
        }
//   else if (EVAL_AUDIO_Stop(CODEC_PDWN_SW) != 0)
//   {
//     AudioState = AUDIO_STATE_ERROR;
//     return AUDIO_FAIL;
//   }
        else
        {
            AudioState = AUDIO_STATE_STOPPED;
            return AUDIO_OK;
        }

    /* Process the PAUSE command ---------------------------*/
    case AUDIO_CMD_PAUSE:
        if (AudioState != AUDIO_STATE_PLAYING)
        {
            /* Unsupported command */
            return AUDIO_FAIL;
        }
//    else if (EVAL_AUDIO_PauseResume(AUDIO_PAUSE, (uint32_t)pbuf, (size/2)) != 0)
//    {
//     AudioState = AUDIO_STATE_ERROR;
//     return AUDIO_FAIL;
//   }
        else
        {
            AudioState = AUDIO_STATE_PAUSED;
            return AUDIO_OK;
        }

    /* Unsupported command ---------------------------------*/
    default:
        return AUDIO_FAIL;
    }
    return AUDIO_OK;
}

/**
  * @brief  VolumeCtl
  *         Set the volume level in %
  * @param  vol: volume level to be set in % (from 0% to 100%)
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  VolumeCtl    (uint8_t vol)
{
    /* Call low layer volume setting function */
    /* if (EVAL_AUDIO_VolumeCtl(vol) != 0)
      {
        AudioState = AUDIO_STATE_ERROR;
        return AUDIO_FAIL;
      }
    */
    uint16_t source = ts.tx_audio_source == TX_AUDIO_DIGIQ?TX_AUDIO_DIGIQ:TX_AUDIO_DIG;
    ts.tx_gain[source] = vol;
    return AUDIO_OK;
}
static uint8_t  InVolumeCtl    (uint8_t vol)
{
    /* Call low layer volume setting function */
    /* if (EVAL_AUDIO_VolumeCtl(vol) != 0)
      {
        AudioState = AUDIO_STATE_ERROR;
        return AUDIO_FAIL;
      }
    */
    ts.rx_gain[RX_AUDIO_DIG].value = vol;
    return AUDIO_OK;
}



/**
  * @brief  MuteCtl
  *         Mute or Unmute the audio current output
  * @param  cmd: can be 0 to unmute, or 1 to mute.
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  MuteCtl      (uint8_t cmd)
{
    /* Call low layer mute setting function */
// aaaa  if (EVAL_AUDIO_Mute(cmd) != 0)
//  {
//    AudioState = AUDIO_STATE_ERROR;
//    return AUDIO_FAIL;
//  }

    return AUDIO_OK;
}

/**
  * @brief
  *
  * @param
  * @param
  * @retval AUDIO_OK if all operations succeed, AUDIO_FAIL else.
  */
static uint8_t  PeriodicTC   (uint8_t cmd)
{


    return AUDIO_OK;
}


/**
  * @brief  GetState
  *         Return the current state of the audio machine
  * @param  None
  * @retval Current State.
  */
static uint8_t  GetState   (void)
{
    return AudioState;
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
