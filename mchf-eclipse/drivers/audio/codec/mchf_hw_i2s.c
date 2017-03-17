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
#include "i2s.h"

static uint32_t txbuf, rxbuf, szbuf;


#ifdef USE_24_BITS
typedef int32_t audio_data_t;
#else
typedef int16_t audio_data_t;
#endif


void MchfHw_Codec_HandleBlock(uint16_t which)
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

    src = (audio_data_t *)(rxbuf) + offset;
    dst = (audio_data_t *)(txbuf) + offset;

    // Handle 2nd half
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

void MchfHw_Codec_StartDMA(uint32_t txAddr, uint32_t rxAddr, uint32_t Size)
{
    txbuf = txAddr;
    rxbuf = rxAddr;
    szbuf = Size;

    HAL_I2SEx_TransmitReceive_DMA(&hi2s3,(uint16_t*)txAddr,(uint16_t*)rxAddr,Size);
}

void MchfHw_Codec_StopDMA(void)
{
    HAL_I2S_DMAStop(&hi2s3);
}
