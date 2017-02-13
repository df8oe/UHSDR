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

#include "codec.h"
#include "audio_driver.h"

#include "i2s.h"

static uint32_t txbuf, rxbuf, szbuf;

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
#ifdef PROFILE_EVENTS
    // we stop during interrupt
    // at the end we start again
    // profileCycleCount_stop();
    profileTimedEventStart(ProfileAudioInterrupt);
#endif

#ifdef USE_24_BITS
    static int32_t *src, *dst, sz;
#else
    static int16_t *src, *dst, sz;
#endif

#ifdef EXEC_PROFILING
    // Profiling pin (high level)
    GPIOE->BSRRL = GPIO_Pin_10;
#endif

    ts.audio_int_counter++;   // generating a time base for encoder handling

    // Transfer complete interrupt
    // Point to 2nd half of buffers
    sz = szbuf/2;

#ifdef USE_24_BITS
    src = (int32_t *)(rxbuf) + sz;
    dst = (int32_t *)(txbuf) + sz;
#else
    src = (int16_t *)(rxbuf) + sz;
    dst = (int16_t *)(txbuf) + sz;
#endif

    // Handle 2nd half
    AudioDriver_I2SCallback(src, dst, sz, 0);

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

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
#ifdef PROFILE_EVENTS
    // we stop during interrupt
    // at the end we start again
    // profileCycleCount_stop();
    profileTimedEventStart(ProfileAudioInterrupt);
#endif

#ifdef USE_24_BITS
    static int32_t *src, *dst, sz;
#else
    static int16_t *src, *dst, sz;
#endif

#ifdef EXEC_PROFILING
    // Profiling pin (high level)
    GPIOE->BSRRL = GPIO_Pin_10;
#endif

    ts.audio_int_counter++;   // generating a time base for encoder handling

    // Half Transfer complete interrupt
    // Point to 1st half of buffers
    sz = szbuf/2;


#ifdef USE_24_BITS
    src = (int32_t *)(rxbuf);
    dst = (int32_t *)(txbuf);
#else
    src = (int16_t *)(rxbuf);
    dst = (int16_t *)(txbuf);
#endif

    // Handle 1st half
    AudioDriver_I2SCallback(src, dst, sz, 1);

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

//*----------------------------------------------------------------------------
//* Function Name       : I2S_Block_Init
//* Object              :
//* Object              : Init I2S channel for DMA with IRQ per block
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void I2S_Block_Init(void)
{
#if 0
    // Enable the DMA clock
    RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK, ENABLE);

    // Configure the TX DMA Stream
    DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
    DMA_DeInit(AUDIO_I2S_DMA_STREAM);

    // Set the parameters to be configured
    DMA_StructInit(&DMA_InitStructure);

    DMA_InitStructure.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_I2S_DMA_DREG;
    DMA_InitStructure.DMA_Memory0BaseAddr = 0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

#ifdef USE_24_BITS
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize 	 = DMA_MemoryDataSize_Word;
#else
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize 	 = DMA_MemoryDataSize_HalfWord;
#endif

    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);

    // Enable the I2S DMA request
    SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);

    // Configure the RX DMA Stream
    DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
    DMA_DeInit(AUDIO_I2S_EXT_DMA_STREAM);

    DMA_StructInit(&DMA_InitStructure2);

    // Set the parameters to be configured
    DMA_InitStructure2.DMA_Channel = AUDIO_I2S_EXT_DMA_CHANNEL;
    DMA_InitStructure2.DMA_PeripheralBaseAddr = AUDIO_I2S_EXT_DMA_DREG;
    DMA_InitStructure2.DMA_Memory0BaseAddr = 0;
    DMA_InitStructure2.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure2.DMA_BufferSize = 0;
    DMA_InitStructure2.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure2.DMA_MemoryInc = DMA_MemoryInc_Enable;

#ifdef USE_24_BITS
    DMA_InitStructure2.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure2.DMA_MemoryDataSize 	  = DMA_MemoryDataSize_Word;
#else
    DMA_InitStructure2.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure2.DMA_MemoryDataSize 	  = DMA_MemoryDataSize_HalfWord;
#endif

    DMA_InitStructure2.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure2.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure2.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStructure2.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure2.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
    DMA_InitStructure2.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &DMA_InitStructure2);

    // Enable the Half & Complete DMA interrupts
    DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);

    // I2S DMA IRQ Channel configuration
    NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);

    // Enable the I2S DMA request
    SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);
#endif
}

//*----------------------------------------------------------------------------
//* Function Name       : I2S_Block_Process
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void I2S_Block_Process(uint32_t txAddr, uint32_t rxAddr, uint32_t Size)
{
    txbuf = txAddr;
    rxbuf = rxAddr;
    szbuf = Size;

    HAL_I2SEx_TransmitReceive_DMA(&hi2s3,(uint16_t*)txAddr,(uint16_t*)rxAddr,Size);
}

//*----------------------------------------------------------------------------
//* Function Name       : I2S_Block_Stop
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void I2S_Block_Stop(void)
{
    HAL_I2S_DMAStop(&hi2s3);
}

//*----------------------------------------------------------------------------
//* Function Name       : DMA1_Stream2_IRQHandler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
