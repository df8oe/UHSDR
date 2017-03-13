/**
 ******************************************************************************
 * @file           : usbd_audio_if.c
 * @brief          : Generic media access Layer.
 ******************************************************************************
 *
 * Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"
/* USER CODE BEGIN INCLUDE */
#include "mchf_board.h"
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @{
 */

/** @defgroup USBD_AUDIO 
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
 * @{
 */
/* USER CODE BEGIN PRIVATE_TYPES */
/* USER CODE END PRIVATE_TYPES */ 
/**
 * @}
 */

/** @defgroup USBD_AUDIO_Private_Defines
 * @{
 */
/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
 * @}
 */

/** @defgroup USBD_AUDIO_Private_Macros
 * @{
 */
/* USER CODE BEGIN PRIVATE_MACRO */
/* USER CODE END PRIVATE_MACRO */

/**
 * @}
 */

/** @defgroup USBD_AUDIO_IF_Private_Variables
 * @{
 */
/* USER CODE BEGIN PRIVATE_VARIABLES */
/* USER CODE END PRIVATE_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_AUDIO_IF_Exported_Variables
 * @{
 */
extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE BEGIN EXPORTED_VARIABLES */
/* USER CODE END EXPORTED_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
 * @{
 */
static int8_t  AUDIO_Init_FS         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
static int8_t  AUDIO_DeInit_FS       (uint32_t options);
static int8_t  AUDIO_AudioCmd_FS     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
static int8_t  AUDIO_VolumeCtl_FS    (uint8_t vol);
static int8_t  AUDIO_MuteCtl_FS      (uint8_t cmd);
static int8_t  AUDIO_PeriodicTC_FS   (uint8_t cmd);
static int8_t  AUDIO_GetState_FS     (void);
static int8_t  AUDIO_InVolumeCtl_FS  (uint8_t vol);


/* Audio Machine States */
typedef enum
{
    AUDIO_STATE_INACTIVE =           0x00,
    AUDIO_STATE_ACTIVE,
    AUDIO_STATE_PLAYING,
    AUDIO_STATE_PAUSED,
    AUDIO_STATE_STOPPED,
    AUDIO_STATE_ERROR
} USB_AUDIO_MACHINE_State_t;



/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
static USB_AUDIO_MACHINE_State_t AudioState = AUDIO_STATE_INACTIVE;

typedef enum
{
    USB_DIG_AUDIO_OUT_INIT = 0,
    USB_DIG_AUDIO_OUT_IDLE,
    USB_DIG_ADUIO_OUT_TX,
} USB_AUDIO_State_t;

USB_AUDIO_State_t usb_dig_audio_state = USB_DIG_AUDIO_OUT_INIT;


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
static volatile uint16_t out_buffer_underflow;

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

volatile uint16_t audio_out_buffer_fill()
{
    uint16_t temp_head = out_buffer_head;
    return ((((temp_head < out_buffer_tail)?USB_AUDIO_OUT_BUF_SIZE:0) + temp_head) - out_buffer_tail);
}

volatile int16_t* audio_out_buffer_next_pkt(uint32_t len)
{
    if (audio_out_buffer_fill() >= len)
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
            out_buffer_underflow++;
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

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
 * @}
 */

USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS = 
{
        AUDIO_Init_FS,
        AUDIO_DeInit_FS,
        AUDIO_AudioCmd_FS,
        AUDIO_VolumeCtl_FS,
        AUDIO_MuteCtl_FS,
        AUDIO_PeriodicTC_FS,
        AUDIO_GetState_FS,
        AUDIO_InVolumeCtl_FS,
};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  AUDIO_Init_FS
 *         Initializes the AUDIO media low layer over USB FS IP
 * @param  AudioFreq: Audio frequency used to play the audio stream.
 * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
 * @param  options: Reserved for future use
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t AUDIO_Init_FS(uint32_t  AudioFreq, uint32_t Volume, uint32_t options)
{ 
    /* USER CODE BEGIN 0 */
    AudioState = AUDIO_STATE_ACTIVE;
    return (USBD_OK);
    /* USER CODE END 0 */
}

/**
 * @brief  AUDIO_DeInit_FS
 *         DeInitializes the AUDIO media low layer
 * @param  options: Reserved for future use
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t AUDIO_DeInit_FS(uint32_t options)
{
    /* USER CODE BEGIN 1 */
    AudioState = AUDIO_STATE_INACTIVE;
    return (USBD_OK);
    /* USER CODE END 1 */
}

/**
 * @brief  AUDIO_AudioCmd_FS
 *         Handles AUDIO command.
 * @param  pbuf: Pointer to buffer of data to be sent
 * @param  size: Number of data to be sent (in bytes)
 * @param  cmd: Command opcode
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */

static uint16_t fill;

static int8_t AUDIO_AudioCmd_FS (uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
    /* USER CODE BEGIN 2 */
    /* Check the current state */
    if ((AudioState == AUDIO_STATE_INACTIVE) || (AudioState == AUDIO_STATE_ERROR))
    {
        AudioState = AUDIO_STATE_ERROR;
        return USBD_FAIL;
    }

    switch (cmd)
    {
    /* Process the PLAY command ----------------------------*/
    case AUDIO_CMD_START:
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
                static bool too_high = false;

                fill =  audio_out_buffer_fill();
                // USB_AUDIO_OUT_NUM_BUF * USB_AUDIO_OUT_PKT_SIZE
                bool is_low_space = fill > (3*(USB_AUDIO_OUT_NUM_BUF/4) * USB_AUDIO_OUT_PKT_SIZE);
                bool is_high_space = fill < (USB_AUDIO_OUT_NUM_BUF/4 * USB_AUDIO_OUT_PKT_SIZE);
                bool is_high_ok = fill > (3*(USB_AUDIO_OUT_NUM_BUF/4) * USB_AUDIO_OUT_PKT_SIZE);
                // bool is_low_ok = fill < ((USB_AUDIO_OUT_NUM_BUF/4) * USB_AUDIO_OUT_PKT_SIZE);

                uint16_t num_samples = size/2;

                if (is_high_ok)
                {
                    too_high = false;
                } else  {
                    too_high = is_high_space;
                }


                if (is_low_space)
                {
                    num_samples-=2;
                }
                for (count =0; count < num_samples; count++)
                {
                    audio_out_put_buffer(pkt[count]);
                }
                if (too_high)
                {
                    audio_out_put_buffer(pkt[num_samples-2]);
                    audio_out_put_buffer(pkt[num_samples-1]);
                }

            }
            AudioState = AUDIO_STATE_PLAYING;
            return USBD_OK;
        }
        else /* Not allowed command */
        {
            return USBD_FAIL;
        }
        break;

    /* Process the STOP command ----------------------------*/
    case AUDIO_CMD_STOP:
        if (AudioState != AUDIO_STATE_PLAYING)
        {
            /* Unsupported command */
            return USBD_FAIL;
        }
        else
        {
            AudioState = AUDIO_STATE_STOPPED;
            return USBD_OK;
        }

    /* Process the PAUSE command ---------------------------*/
    case AUDIO_CMD_PAUSE:
        if (AudioState != AUDIO_STATE_PLAYING)
        {
            /* Unsupported command */
            return USBD_FAIL;
        }
        else
        {
            AudioState = AUDIO_STATE_PAUSED;
            return USBD_OK;
        }

    /* Unsupported command ---------------------------------*/
    default:
        return USBD_FAIL;
    }
    return USBD_OK;
    /* USER CODE END 2 */

}

/**
 * @brief  AUDIO_VolumeCtl_FS
 *         Controls AUDIO Volume.
 * @param  vol: volume level (0..100)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t AUDIO_VolumeCtl_FS (uint8_t vol)
{
    /* USER CODE BEGIN 3 */
    uint16_t source = ts.tx_audio_source == TX_AUDIO_DIGIQ?TX_AUDIO_DIGIQ:TX_AUDIO_DIG;
    ts.tx_gain[source] = vol;
    return (USBD_OK);
    /* USER CODE END 3 */
}

/**
 * @brief  AUDIO_MuteCtl_FS
 *         Controls AUDIO Mute.
 * @param  cmd: command opcode
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t AUDIO_MuteCtl_FS (uint8_t cmd)
{
    /* USER CODE BEGIN 4 */
    return (USBD_OK);
    /* USER CODE END 4 */
}

/**
 * @brief  AUDIO_PeriodicT_FS
 * @param  cmd: Command opcode
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t AUDIO_PeriodicTC_FS (uint8_t cmd)
{
    /* USER CODE BEGIN 5 */
    return (USBD_OK);
    /* USER CODE END 5 */
}

/**
 * @brief  AUDIO_GetState_FS
 *         Gets AUDIO State.
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t AUDIO_GetState_FS (void)
{
    /* USER CODE BEGIN 6 */
    return (USBD_OK);
    /* USER CODE END 6 */
}

#if 0
/**
 * @brief  Manages the DMA full Transfer complete event.
 * @param  None
 * @retval None
 */
void TransferComplete_CallBack_FS(void)
{
    /* USER CODE BEGIN 7 */
    USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_FULL);
    /* USER CODE END 7 */
}

/**
 * @brief  Manages the DMA Half Transfer complete event.
 * @param  None
 * @retval None
 */
void HalfTransfer_CallBack_FS(void)
{ 
    /* USER CODE BEGIN 8 */
    USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_HALF);
    /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
 * @}
 */

/**
 * @}
 */
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
static int8_t  AUDIO_InVolumeCtl_FS    (uint8_t vol)
{
    /* Call low layer volume setting function */
    ts.rx_gain[RX_AUDIO_DIG].value = vol;
    return USBD_OK;
}

