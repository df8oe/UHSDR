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
#include "uhsdr_board.h"
#include "profiling.h"
#include "uhsdr_hw_i2s.h"

#include "audio_driver.h"

#ifdef UI_BRD_MCHF
#include "i2s.h"
#endif

#ifdef UI_BRD_OVI40
#include "sai.h"
#endif


typedef struct
{
    IqSample_t out[2*IQ_SAMPLES_PER_BLOCK];
    IqSample_t in[2*IQ_SAMPLES_PER_BLOCK];
} dma_iq_buffer_t;
__UHSDR_DMAMEM dma_iq_buffer_t iq_buf;

#if defined(UI_BRD_OVI40)

typedef struct
{
    AudioSample_t out[2*AUDIO_SAMPLES_PER_BLOCK];
    AudioSample_t in[2*AUDIO_SAMPLES_PER_BLOCK];
} dma_audio_buffer_t;

__UHSDR_DMAMEM dma_audio_buffer_t audio_buf;
#else

#define audio_buf iq_buf

#endif


void UhsdrHwI2s_Codec_ClearTxDmaBuffer()
{
    memset((void*)&iq_buf.out, 0, sizeof(iq_buf.out));
}

static void MchfHw_Codec_HandleBlock(uint16_t which)
{
#ifdef PROFILE_EVENTS
    // we stop during interrupt
    // at the end we start again
    // profileCycleCount_stop();
    profileTimedEventStart(ProfileAudioInterrupt);
#endif


#ifdef EXEC_PROFILING
    // Profiling pin (high level)
    GPIOE->BSRRL = GPIO_Pin_10;
#endif

    ts.audio_int_counter++;   // generating a time base for encoder handling

    // Transfer complete interrupt
    // Point to 2nd half of buffers
    const size_t sz = IQ_SAMPLES_PER_BLOCK;
    const uint16_t offset = which == 0?sz:0;

    AudioSample_t *audio;
    IqSample_t    *iq;

    if (ts.txrx_mode != TRX_MODE_TX)
    {
        iq = &iq_buf.in[offset];
        audio = &audio_buf.out[offset];
    }
    else
    {
        audio = &audio_buf.in[offset];
        iq = &iq_buf.out[offset];
    }

    AudioSample_t *audioDst = &audio_buf.out[offset];

    // Handle
    AudioDriver_I2SCallback(audio, iq, audioDst, sz);

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

#ifdef UI_BRD_MCHF
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

#ifdef UI_BRD_OVI40
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

void UhsdrHwI2s_Codec_StartDMA()
{

#ifdef UI_BRD_MCHF
    HAL_I2SEx_TransmitReceive_DMA(&hi2s3,(uint16_t*)audio_buf.out,(uint16_t*)audio_buf.in,sizeof(audio_buf.in));
#endif
#ifdef UI_BRD_OVI40
    // we clean the buffers since we don't know if we are in a "cleaned" memory segement
    memset((void*)&audio_buf,0,sizeof(audio_buf));
    memset((void*)&iq_buf,0,sizeof(iq_buf));

    HAL_SAI_Receive_DMA(&hsai_BlockA1,(uint8_t*)audio_buf.in,sizeof(audio_buf.in)/sizeof(audio_buf.in[0].l));
    HAL_SAI_Transmit_DMA(&hsai_BlockB1,(uint8_t*)audio_buf.out,sizeof(audio_buf.out)/sizeof(audio_buf.out[0].l));

    HAL_SAI_Receive_DMA(&hsai_BlockA2,(uint8_t*)iq_buf.in,sizeof(iq_buf.in)/sizeof(iq_buf.in[0].l));
    HAL_SAI_Transmit_DMA(&hsai_BlockB2,(uint8_t*)iq_buf.out,sizeof(iq_buf.out)/sizeof(iq_buf.out[0].l));

#endif
}


void UhsdrHwI2s_Codec_StopDMA(void)
{
#ifdef UI_BRD_MCHF
    HAL_I2S_DMAStop(&hi2s3);
#endif
#ifdef UI_BRD_OVI40
    HAL_SAI_DMAStop(&hsai_BlockA1);
    HAL_SAI_DMAStop(&hsai_BlockB1);
    HAL_SAI_DMAStop(&hsai_BlockA2);
    HAL_SAI_DMAStop(&hsai_BlockB2);
#endif
}
