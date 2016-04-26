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

// Common
#include "mchf_board.h"

#include "codec.h"
#include "audio_driver.h"

#include "i2s.h"

static DMA_InitTypeDef DMA_InitStructure, DMA_InitStructure2;

static uint32_t txbuf, rxbuf, szbuf;

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
    // Enable the DMA clock
    RCC_AHB1PeriphClockCmd(AUDIO_I2S_DMA_CLOCK, ENABLE);

    // Configure the TX DMA Stream
    DMA_Cmd(AUDIO_I2S_DMA_STREAM, DISABLE);
    DMA_DeInit(AUDIO_I2S_DMA_STREAM);

    // Set the parameters to be configured
    DMA_InitStructure.DMA_Channel = AUDIO_I2S_DMA_CHANNEL;
    DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_I2S_DMA_DREG;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;
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
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);

    // Enable the I2S DMA request
    SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);

    // Configure the RX DMA Stream
    DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
    DMA_DeInit(AUDIO_I2S_EXT_DMA_STREAM);

    // Set the parameters to be configured
    DMA_InitStructure2.DMA_Channel = AUDIO_I2S_EXT_DMA_CHANNEL;
    DMA_InitStructure2.DMA_PeripheralBaseAddr = AUDIO_I2S_EXT_DMA_DREG;
    DMA_InitStructure2.DMA_Memory0BaseAddr = (uint32_t)0;
    DMA_InitStructure2.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure2.DMA_BufferSize = (uint32_t)0xFFFE;
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
    DMA_InitStructure2.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure2.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitStructure2.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure2.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &DMA_InitStructure2);

    // Enable the Half & Complete DMA interrupts
    DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, ENABLE);

    // I2S DMA IRQ Channel configuration
    NVIC_EnableIRQ(AUDIO_I2S_EXT_DMA_IRQ);

    // Enable the I2S DMA request
    SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);
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

    // Configure the tx buffer address and size
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)txAddr;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)Size;

    // Configure the DMA Stream with the new parameters
    DMA_Init(AUDIO_I2S_DMA_STREAM, &DMA_InitStructure);

    // Configure the rx buffer address and size
    DMA_InitStructure2.DMA_Memory0BaseAddr = (uint32_t)rxAddr;
    DMA_InitStructure2.DMA_BufferSize = (uint32_t)Size;

    // Configure the DMA Stream with the new parameters
    DMA_Init(AUDIO_I2S_EXT_DMA_STREAM, &DMA_InitStructure2);

    // Enable the I2S DMA Streams
    DMA_Cmd(AUDIO_I2S_DMA_STREAM, ENABLE);
    DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, ENABLE);

    // If the I2S peripheral is still not enabled, enable it
    if ((CODEC_I2S->I2SCFGR & 0x0400) == 0)
    {
        I2S_Cmd(CODEC_I2S, ENABLE);
    }

    if ((CODEC_I2S_EXT->I2SCFGR & 0x0400) == 0)
    {
        I2S_Cmd(CODEC_I2S_EXT, ENABLE);
    }
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
    I2S_Cmd(CODEC_I2S_EXT, DISABLE);
    I2S_Cmd(CODEC_I2S,     DISABLE);

    DMA_Cmd(AUDIO_I2S_EXT_DMA_STREAM, DISABLE);
    DMA_Cmd(AUDIO_I2S_DMA_STREAM,     DISABLE);

    SPI_I2S_DMACmd(CODEC_I2S_EXT, SPI_I2S_DMAReq_Rx, DISABLE);

    NVIC_DisableIRQ(AUDIO_I2S_EXT_DMA_IRQ);

    DMA_ITConfig(AUDIO_I2S_EXT_DMA_STREAM, DMA_IT_TC | DMA_IT_HT, DISABLE);
}

//*----------------------------------------------------------------------------
//* Function Name       : DMA1_Stream2_IRQHandler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void DMA1_Stream2_IRQHandler(void)
{
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
    if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC) != RESET)
    {
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
        I2S_RX_CallBack(src, dst, sz, 0);

        // Clear the Interrupt flag
        DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_TC);
    }

    // Half Transfer complete interrupt
    if (DMA_GetFlagStatus(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_HT) != RESET)
    {
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
        I2S_RX_CallBack(src, dst, sz, 1);

        // Clear the Interrupt flag
        DMA_ClearFlag(AUDIO_I2S_EXT_DMA_STREAM, AUDIO_I2S_EXT_DMA_FLAG_HT);
    }

#ifdef EXEC_PROFILING
    // Profiling pin (low level)
    GPIOE->BSRRH = GPIO_Pin_10;
#endif
}
