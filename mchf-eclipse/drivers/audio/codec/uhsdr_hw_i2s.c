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
#include "uhsdr_board_config.h"
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
    IqSample_t out[2*IQ_BLOCK_SIZE];
    IqSample_t in[2*IQ_BLOCK_SIZE];
} dma_iq_buffer_t;

typedef struct
{
    AudioSample_t out[2*AUDIO_BLOCK_SIZE];
    AudioSample_t in[2*AUDIO_BLOCK_SIZE];
} dma_audio_buffer_t;


// we do something tricky here:
// if we have a single codec both buffers are in fact the same, so we use a union
// if we have two codecs we use a struct hence two separate buffers

typedef
#if CODEC_NUM == 1
    union
#else
    struct
#endif
    {
        dma_iq_buffer_t iq_buf;
        dma_audio_buffer_t audio_buf;
    } I2S_DmaBuffers_t;

static __UHSDR_DMAMEM I2S_DmaBuffers_t dma;


void UhsdrHwI2s_Codec_ClearTxDmaBuffer()
{
    memset((void*)&dma.iq_buf.out, 0, sizeof(dma.iq_buf.out));
}

// #define PROFILE_APP
static void MchfHw_Codec_HandleBlock(uint16_t which)
{
#ifdef PROFILE_EVENTS
    // we stop during interrupt
    // at the end we start again
#ifdef PROFILE_APP
    profileCycleCount_stop();
#else
    profileTimedEventStart(ProfileAudioInterrupt);
#endif
#endif

#ifdef EXEC_PROFILING
    // Profiling pin (high level)
    GPIOE->BSRRL = GPIO_Pin_10;
#endif

    ts.audio_int_counter++;   // generating a time base for encoder handling

    // Transfer complete interrupt
    // Point to 2nd half of buffers
    const size_t sz = IQ_BLOCK_SIZE;
    const uint16_t offset = which == 0?sz:0;

    AudioSample_t *audio;
    IqSample_t    *iq;

    if (ts.txrx_mode != TRX_MODE_TX)
    {
        iq = &dma.iq_buf.in[offset];
        audio = &dma.audio_buf.out[offset];
    }
    else
    {
        audio = &dma.audio_buf.in[offset];
        iq = &dma.iq_buf.out[offset];
    }

    AudioSample_t *audioDst = &dma.audio_buf.out[offset];

    // Handle
    AudioDriver_I2SCallback(audio, iq, audioDst, sz);

#ifdef EXEC_PROFILING
    // Profiling pin (low level)
    GPIOE->BSRRH = GPIO_Pin_10;
#endif
#ifdef PROFILE_EVENTS
    // we stopped during interrupt
    // now we start again
#ifdef PROFILE_APP
    profileCycleCount_start();
#else
    profileTimedEventStop(ProfileAudioInterrupt);
#endif
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

#if defined(UI_BRD_OVI40)
static void UhsdrHWI2s_Sai32Bits(SAI_HandleTypeDef* hsai)
{
    hsai->hdmarx->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hsai->hdmarx->Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    HAL_DMA_Init(hsai->hdmarx);

    HAL_SAI_InitProtocol(hsai, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_32BIT, 2);
}
#endif

static void UhsdrHwI2s_SetBitWidth()
{
#if defined(USE_32_IQ_BITS)
    #if defined(UI_BRD_MCHF)
    hi2s3.Init.DataFormat = I2S_DATAFORMAT_32B;
    HAL_I2S_Init(&hi2s3);

    #endif
    #if defined(UI_BRD_OVI40)
    UhsdrHWI2s_Sai32Bits(&hsai_BlockA2);
    UhsdrHWI2s_Sai32Bits(&hsai_BlockB2);
    #endif
#endif

#if defined(USE_32_AUDIO_BITS)
    #if defined(UI_BRD_OVI40)
    UhsdrHWI2s_Sai32Bits(&hsai_BlockA1);
    UhsdrHWI2s_Sai32Bits(&hsai_BlockB1);
    #endif
#endif
}



void UhsdrHwI2s_Codec_StartDMA()
{
    UhsdrHwI2s_SetBitWidth();

#ifdef UI_BRD_MCHF
    HAL_I2SEx_TransmitReceive_DMA(&hi2s3,(uint16_t*)dma.iq_buf.out,(uint16_t*)dma.iq_buf.in,sizeof(dma.iq_buf.in)/sizeof(dma.iq_buf.in[0].l));
#endif
#ifdef UI_BRD_OVI40
    // we clean the buffers since we don't know if we are in a "cleaned" memory segement
    memset((void*)&dma.audio_buf,0,sizeof(dma.audio_buf));
    memset((void*)&dma.iq_buf,0,sizeof(dma.iq_buf));

    HAL_SAI_Receive_DMA(&hsai_BlockA1,(uint8_t*)dma.audio_buf.in,sizeof(dma.audio_buf.in)/sizeof(dma.audio_buf.in[0].l));
    HAL_SAI_Transmit_DMA(&hsai_BlockB1,(uint8_t*)dma.audio_buf.out,sizeof(dma.audio_buf.out)/sizeof(dma.audio_buf.out[0].l));

    HAL_SAI_Receive_DMA(&hsai_BlockA2,(uint8_t*)dma.iq_buf.in,sizeof(dma.iq_buf.in)/sizeof(dma.iq_buf.in[0].l));
    HAL_SAI_Transmit_DMA(&hsai_BlockB2,(uint8_t*)dma.iq_buf.out,sizeof(dma.iq_buf.out)/sizeof(dma.iq_buf.out[0].l));

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
