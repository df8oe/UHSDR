/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

// Common
#include "mchf_board.h"
#include "profiling.h"
#include "mchf_hw_i2s.h"

#include "audio_driver.h"

#ifdef STM32F4
#include "i2s.h"
#endif

#ifdef STM32F7
#include "sai.h"
#endif




__IO dma_audio_buffer_t audio_buf[DMA_AUDIO_NUM];


static uint32_t szbuf;


#ifdef USE_24_BITS
typedef int32_t audio_data_t;
#else
typedef int16_t audio_data_t;
#endif

#ifdef STM32F4
#define CODEC_IQ_IDX 0
#define CODEC_ANA_IDX  0
#endif

#ifdef STM32F7
#define CODEC_IQ_IDX 1
#define CODEC_ANA_IDX 0
#endif

static void MchfHw_Codec_HandleBlock(uint16_t which)
{
#ifdef PROFILE_EVENTS
    // we stop during interrupt
    // at the end we start again
    // profileCycleCount_stop();
    profileTimedEventStart(ProfileAudioInterrupt);
#endif

    static audio_data_t *src, *dst, sz;

#ifdef EXEC_PROFILING
    // Profiling pin (high level)
    GPIOE->BSRRL = GPIO_Pin_10;
#endif

    ts.audio_int_counter++;   // generating a time base for encoder handling

    // Transfer complete interrupt
    // Point to 2nd half of buffers
    sz = szbuf/2;
    uint16_t offset = which == 0?sz:0;

    if (ts.txrx_mode != TRX_MODE_TX)
    {
        src = (audio_data_t*)&audio_buf[CODEC_IQ_IDX].in[offset];
        dst = (audio_data_t*)&audio_buf[CODEC_ANA_IDX].out[offset];
    }
    else
    {
        src = (audio_data_t*)&audio_buf[CODEC_ANA_IDX].in[offset];
        dst = (audio_data_t*)&audio_buf[CODEC_IQ_IDX].out[offset];
    }

    // Handle half
    AudioDriver_I2SCallback(src, dst, sz, which);

#ifdef EXEC_PROFILING
    // Profiling pin (low level)
    GPIOE->BSRRH = GPIO_Pin_10;
#endif
#ifdef PROFILE_EVENTS
    // we stopped during interrupt
    // now we start again
    // profileCycleCount_start();
    profileTimedEventStop(ProfileAudioInterrupt);
#endif
}

#ifdef STM32F4
/**
 * @brief HAL Handler for Codec DMA Interrupt
 */
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    MchfHw_Codec_HandleBlock(0);
}

/**
 * @brief HAL Handler for Codec DMA Interrupt
 */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    MchfHw_Codec_HandleBlock(1);
}
#endif

#ifdef STM32F7
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hi2s)
{
    if (hi2s == &hsai_BlockA2)
    {
        MchfHw_Codec_HandleBlock(0);
    }
}




void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hi2s)
{
    if (hi2s == &hsai_BlockA2)
    {
        MchfHw_Codec_HandleBlock(1);
    }
}
#endif

void MchfHw_Codec_StartDMA()
{
    szbuf = BUFF_LEN;

#ifdef STM32F4
    HAL_I2SEx_TransmitReceive_DMA(&hi2s3,(uint16_t*)audio_buf[0].out,(uint16_t*)audio_buf[0].in,szbuf);
#endif
#ifdef STM32F7
    HAL_SAI_Receive_DMA(&hsai_BlockA1,(uint8_t*)audio_buf[0].in,szbuf);
    HAL_SAI_Transmit_DMA(&hsai_BlockB1,(uint8_t*)audio_buf[0].out,szbuf);

    HAL_SAI_Receive_DMA(&hsai_BlockA2,(uint8_t*)audio_buf[1].in,szbuf);
    HAL_SAI_Transmit_DMA(&hsai_BlockB2,(uint8_t*)audio_buf[1].out,szbuf);

#endif
}


void MchfHw_Codec_StopDMA(void)
{
#ifdef STM32F4
    HAL_I2S_DMAStop(&hi2s3);
#endif
#ifdef STM32F7
    HAL_SAI_DMAStop(&hsai_BlockA1);
    HAL_SAI_DMAStop(&hsai_BlockB1);
    HAL_SAI_DMAStop(&hsai_BlockA2);
    HAL_SAI_DMAStop(&hsai_BlockB2);
#endif
}
