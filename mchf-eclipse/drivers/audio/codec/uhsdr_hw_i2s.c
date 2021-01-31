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
#include <assert.h>


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
static void UhsdrHWI2s_SaiConfig(SAI_HandleTypeDef* hsai, uint32_t bits, uint32_t rate)
{

    typedef struct
    {
        uint32_t bits;
        uint32_t word_id;
        uint32_t PeriphDataAlignment;
        uint32_t MemDataAlignment;
    } protocol_config_t;

    static const protocol_config_t word_config[] =
    {
            { 16, SAI_PROTOCOL_DATASIZE_16BIT,  DMA_PDATAALIGN_HALFWORD, DMA_MDATAALIGN_HALFWORD },
            { 24, SAI_PROTOCOL_DATASIZE_32BIT,  DMA_PDATAALIGN_WORD, DMA_MDATAALIGN_WORD },
            { 32, SAI_PROTOCOL_DATASIZE_32BIT,  DMA_PDATAALIGN_WORD, DMA_MDATAALIGN_WORD },
            // keep at end
            { 0, 0, 0, 0 },
    };

    // get bits_idx, is valid if word_config[bits_idx].bits != 0
    uint32_t bits_idx;
    for (bits_idx = 0; word_config[bits_idx].bits != 0 && word_config[bits_idx].bits != bits; bits_idx++) {}

    typedef struct
    {
        uint32_t rate;
        uint32_t rate_id;
    } rate_config_t;

    static const rate_config_t sr_config[] =
    {
                { 48000,  SAI_AUDIO_FREQUENCY_48K},
                { 96000, SAI_AUDIO_FREQUENCY_96K },
                { 192000, SAI_AUDIO_FREQUENCY_192K },
                // keep at end
                { 0, 0 },
    };
    // get sample rate_idx, is valid if sr_config[sr_idx].rate != 0
    uint32_t sr_idx;
    for (sr_idx = 0; sr_config[sr_idx].rate != 0 && sr_config[sr_idx].rate != rate; sr_idx++) {}



    if (sr_config[sr_idx].rate != 0 && word_config[bits_idx].bits != 0)
    {

        hsai->hdmarx->Init.PeriphDataAlignment = word_config[bits_idx].PeriphDataAlignment;
        hsai->hdmarx->Init.MemDataAlignment = word_config[bits_idx].MemDataAlignment;
        HAL_DMA_Init(hsai->hdmarx);

        HAL_SAI_InitProtocol(hsai, SAI_I2S_STANDARD, word_config[bits_idx].word_id, 2);
    }
    else
    {
        assert(false && "Unsupported SAI Sample Rate or SAI Word Lenght");
        Error_Handler();
    }
}
#endif

/***
 * This handles reconfiguring the I2S connection to use the desired parameters namely the used data format
 * NOTE: It can only be used with disabled DMA transfers!
 */
static void UhsdrHwI2s_ApplyConfig()
{

#if defined(UI_BRD_MCHF)
    if (IQ_SAMPLE_BITS == 32)
    {
        hi2s3.Init.DataFormat = I2S_DATAFORMAT_32B;
    }
    HAL_I2S_Init(&hi2s3);
#endif

#if defined(UI_BRD_OVI40)
    UhsdrHWI2s_SaiConfig(&hsai_BlockA2, IQ_SAMPLE_BITS, IQ_SAMPLE_RATE);
    UhsdrHWI2s_SaiConfig(&hsai_BlockB2, IQ_SAMPLE_BITS, IQ_SAMPLE_RATE);

    UhsdrHwI2s_Codec_IqAsSlave(ts.rf_board == RF_BOARD_DDCDUC_DF8OE || ts.rf_board == RF_BOARD_SPARKLE);

    UhsdrHWI2s_SaiConfig(&hsai_BlockA1, AUDIO_SAMPLE_BITS, AUDIO_SAMPLE_RATE);
    UhsdrHWI2s_SaiConfig(&hsai_BlockB1, AUDIO_SAMPLE_BITS, AUDIO_SAMPLE_RATE);
#endif
}



void UhsdrHwI2s_Codec_StartDMA()
{
    UhsdrHwI2s_ApplyConfig();

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

void UhsdrHwI2s_Codec_Restart()
{
    if (ts.rf_board != RF_BOARD_SPARKLE)  //SAI for SParkle is already configured and there is no need to change here anything.
    {
        UhsdrHwI2s_Codec_StopDMA();

        bool temp_mute = ts.audio_dac_muting_flag;
        ts.audio_dac_muting_flag = true;
#ifdef UI_BRD_MCHF
        MX_I2S3_Init();
#endif
#ifdef UI_BRD_OVI40
        MX_SAI2_Init();
#endif
        UhsdrHwI2s_Codec_StartDMA();
        ts.audio_dac_muting_flag = temp_mute;
    }
}

#ifdef UI_BRD_OVI40
static void UhsdrHwI2s_Codec_EnableExternalMasterClock(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /*Configure GPIO pin : PC9 */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;  //strange alternate name, but there is no I2S_CKIN definition in HAL :)
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    __HAL_RCC_SAI1_CONFIG(RCC_SAI1CLKSOURCE_PIN);
    __HAL_RCC_SAI2_CONFIG(RCC_SAI1CLKSOURCE_PIN);
}

void UhsdrHwI2s_Codec_IqAsSlave(bool is_slave)
{

    if (ts.rf_board == RF_BOARD_DDCDUC_DF8OE)
    {
        uint32_t target_mode = is_slave? SAI_MODESLAVE_TX: SAI_MODEMASTER_TX;
        if (target_mode != hsai_BlockB2.Init.AudioMode)
        {
            UhsdrHwI2s_Codec_StopDMA();
            if (target_mode == SAI_MODESLAVE_TX)
            {
                UhsdrHwI2s_Codec_EnableExternalMasterClock();
            }
            hsai_BlockB2.Init.AudioMode = is_slave? SAI_MODESLAVE_TX: SAI_MODEMASTER_TX;
            UhsdrHwI2s_Codec_StartDMA();
        }
    }
}
#endif
