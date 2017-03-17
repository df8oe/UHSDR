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

#ifndef __CODEC_H
#define __CODEC_H

#define WORD_SIZE_16 					0
#define WORD_SIZE_32 					1

#define CODEC_I2S                      SPI3
#define CODEC_I2S_EXT                  I2S3ext
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S_ADDRESS              (SPI3_BASE    + 0x0C)
#define CODEC_I2S_EXT_ADDRESS          (I2S3ext_BASE + 0x0C)
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI3
#define CODEC_I2S_IRQ                  SPI3_IRQn
#define CODEC_I2S_EXT_IRQ              SPI3_IRQn

#define AUDIO_I2S_IRQHandler           SPI3_IRQHandler
#define AUDIO_I2S_EXT_IRQHandler       SPI3_IRQHandler

#define AUDIO_I2S_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_I2S_DMA_STREAM           DMA1_Stream5
#define AUDIO_I2S_DMA_DREG             CODEC_I2S_ADDRESS
#define AUDIO_I2S_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_I2S_DMA_IRQ              DMA1_Stream5_IRQn
#define AUDIO_I2S_DMA_FLAG_TC          DMA_FLAG_TCIF5
#define AUDIO_I2S_DMA_FLAG_HT          DMA_FLAG_HTIF5
#define AUDIO_I2S_DMA_FLAG_FE          DMA_FLAG_FEIF5
#define AUDIO_I2S_DMA_FLAG_TE          DMA_FLAG_TEIF5
#define AUDIO_I2S_DMA_FLAG_DME         DMA_FLAG_DMEIF5
#define AUDIO_I2S_EXT_DMA_STREAM       DMA1_Stream2
#define AUDIO_I2S_EXT_DMA_DREG         CODEC_I2S_EXT_ADDRESS
#define AUDIO_I2S_EXT_DMA_CHANNEL      DMA_Channel_2
#define AUDIO_I2S_EXT_DMA_IRQ          DMA1_Stream2_IRQn
//#define AUDIO_I2S_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF2
//#define AUDIO_I2S_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF2
//#define AUDIO_I2S_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF2
//#define AUDIO_I2S_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF2
//#define AUDIO_I2S_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF2

// Mask for the bit EN of the I2S CFGR register
// #define I2S_ENABLE_MASK                 0x0400
// #define I2S_STANDARD                   	I2S_Standard_Phillips

// --------------------------------------------------

/**
 * @brief setups up the codec according to tx/rx mode and selected sources
 * @param txrx_mode the mode for which it should be configured
 *
 */
void     Codec_SwitchTxRxMode(uint8_t txrx_mode);
void     Codec_MCUInterfaceInit(uint32_t AudioFreq);
void 	 Codec_VolumeLineOut(uint8_t txrx_mode);
void     Codec_VolumeSpkr(uint8_t vol);
void 	 Codec_LineInGainAdj(uint8_t gain);
void 	 Codec_MuteDAC(bool state);

void     Codec_Reset(uint32_t AudioFreq,uint32_t word_size);
void     Codec_RestartI2S();
void     Codec_TxSidetoneSetgain(uint8_t mode);
void     Codec_SwitchMicTxRxMode(uint8_t mode);
void     Codec_PrepareTx(uint8_t current_txrx_mode);

#endif
