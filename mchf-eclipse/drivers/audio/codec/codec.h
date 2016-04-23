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
**  Licence:		CC BY-NC-SA 3.0                                                **
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
#define AUDIO_I2S_EXT_DMA_FLAG_TC      DMA_FLAG_TCIF2
#define AUDIO_I2S_EXT_DMA_FLAG_HT      DMA_FLAG_HTIF2
#define AUDIO_I2S_EXT_DMA_FLAG_FE      DMA_FLAG_FEIF2
#define AUDIO_I2S_EXT_DMA_FLAG_TE      DMA_FLAG_TEIF2
#define AUDIO_I2S_EXT_DMA_FLAG_DME     DMA_FLAG_DMEIF2
//

// Mask for the bit EN of the I2S CFGR register
#define I2S_ENABLE_MASK                 0x0400

#define CODEC_STANDARD                	0x04
#define I2S_STANDARD                   	I2S_Standard_Phillips

#define W8731_ADDR_0 					0x1A		// CS = 0, MODE to GND
#define W8731_ADDR_1 					0x1B		// CS = 1, MODE to GND

// The 7 bits Codec address (sent through I2C interface)
#define CODEC_ADDRESS           		(W8731_ADDR_0<<1)
// --------------------------------------------------
// Registers
#define W8731_LEFT_LINE_IN				0x00		// 0000000
#define W8731_RIGHT_LINE_IN				0x01		// 0000001

#define W8731_LEFT_HEADPH_OUT			0x02		// 0000010
#define W8731_RIGHT_HEADPH_OUT			0x03		// 0000011

#define W8731_ANLG_AU_PATH_CNTR			0x04		// 0000100
#define W8731_DIGI_AU_PATH_CNTR			0x05		// 0000101

#define W8731_POWER_DOWN_CNTR			0x06		// 0000110
#define W8731_DIGI_AU_INTF_FORMAT		0x07		// 0000111

#define W8731_SAMPLING_CNTR				0x08		// 0001000
#define W8731_ACTIVE_CNTR				0x09		// 0001001
#define W8731_RESET						0x0F		// 0001111

// -------------------------------------------------

//#define W8731_DEEMPH_CNTR 				0x06		// WM8731 codec De-emphasis enabled
#define W8731_DEEMPH_CNTR 				0x00		// WM8731 codec De-emphasis disabled

//

uint32_t Codec_Init(uint32_t AudioFreq,ulong word_size);
void 	 Codec_RX_TX(uint8_t mode);
void 	 Codec_Volume(uchar vol, uint8_t txrx_mode);
void 	Codec_Line_Gain_Adj(uchar gain);
void 	 Codec_Mute(uchar state);

void     Codec_AudioInterface_Init(uint32_t AudioFreq);
void     Codec_Reset(uint32_t AudioFreq,ulong word_size);
uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue);
void     Codec_GPIO_Init(void);
void Codec_SidetoneSetgain(uint8_t mode);
void Codec_MicBoostCheck(uint8_t mode);

#endif
