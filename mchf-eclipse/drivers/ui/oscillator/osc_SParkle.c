/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  File name:     osc_SParkle.c                                                  **
 **  Description:   SParkle interface, FPGA DDC management                         **
 **  Licence:       GNU GPLv3                                                      **
 **  Author:        Slawomir Balon/SP9BSL                                          **
 ************************************************************************************/
#include "main.h"
#include "uhsdr_board.h"
#include <math.h>
#include "osc_SParkle.h"
#include "radio_management.h"
#include "rfboard_interface.h"

SParkleState_t SParkleState;
#ifdef USE_OSC_SParkle

#include <spi.h>
#include "uhsdr_hw_i2c.h"

extern SAI_HandleTypeDef hsai_BlockA2;
extern SAI_HandleTypeDef hsai_BlockB2;
extern DMA_HandleTypeDef hdma_sai2_a;
extern DMA_HandleTypeDef hdma_sai2_b;

#define FPGA_CS_PIN GPIO_PIN_13
#define FPGA_CS_PORT GPIOC
#define hspiFPGA        (hspi6)
#define SParkleBB_I2C   (&hi2c2)

//SParkle Base Board I2C registers
#define SParkleBB_I2C_adr1 0x40     //IC204
    #define SParkleBB_I2C_adr1_LNA          (1<<0)     //LNA enable
    #define SParkleBB_I2C_adr1_ANT          (1<<1)     //HF antenna switch
    #define SParkleBB_I2C_adr1_AMP_EN1      (1<<2)     //HF amplifier enable
    #define SParkleBB_I2C_adr1_AMP_EN2      (1<<3)     //VHF amplifier enable
    #define SParkleBB_I2C_adr1_BND_all      (0<<4)     //all pas filter for band selection (exactly the low pass up to 6m)
    #define SParkleBB_I2C_adr1_BND_6m       (1<<4)
    #define SParkleBB_I2C_adr1_BND_160m     (2<<4)
    #define SParkleBB_I2C_adr1_BND_60_40m   (3<<4)
    #define SParkleBB_I2C_adr1_BND_12_10m   (4<<4)
    #define SParkleBB_I2C_adr1_BND_30_20m   (5<<4)
    #define SParkleBB_I2C_adr1_BND_80m      (6<<4)
    #define SParkleBB_I2C_adr1_BND_17_15m   (7<<4)
#define SParkleBB_I2C_adr2 0x42
    #define SParkleBB_I2C_adr2_AMP_EN3      (1<<7)

#define SParkle_AMPselHF 0
#define SParkle_AMPselVHF 1

#define PCA_9554_RegInput 0
#define PCA_9554_RegOutput 1
#define PCA_9554_RegPolarityInv 2
#define PCA_9554_RegConfig 3

//SPI registers of DDC board
#define DDCboard_REG_STAT 0
#define DDCboard_REG_CTRL 1
    #define DDCboard_REG_CTRL_SAIen     (1<<0)  //enable SAI outputs in FPGA
    #define DDCboard_REG_CTRL_SAITEST   (1<<1)  //test pattern for receiver (FPGA transmits: L:0x12345678 R:0x90123456 on its SAI output, increment on DAC output)
    #define DDCboard_REG_CTRL_RCV1revIQ (1<<2)  //exchange Receiver1 IQ (when working in even Nyquist Zone)
    #define DDCboard_REG_CTRL_AMP1      (1<<3)  //20dB ADC amplifier on off signal
    #define DDCboard_REG_CTRL_LNA       (1<<4)  //26dB 10m> LNA on off signal
    #define DDCboard_REG_CTRL_RXANT     (1<<5)  //RX mux switch (1=IN#1, 0=IN#2)
    #define DDCboard_REG_CTRL_TXANT     (1<<6)  //TX mux switch (1=OUT#1, 0=OUT#2)
    #define DDCboard_REG_CTRL_ADCFLTRmsk (3<<7)
        #define DDCboard_REG_CTRL_ADCFLTR_2mBPF (3<<7)  //RX/TX ADC filter mux,0 or 2=2m BPF, 1=4m BPF, 3=52MHz LPF
        #define DDCboard_REG_CTRL_ADCFLTR_4mBPF (1<<7)  //RX/TX ADC filter mux,0 or 2=2m BPF, 1=4m BPF, 3=52MHz LPF
        #define DDCboard_REG_CTRL_ADCFLTR_LPF   (0<<7)  //RX/TX ADC filter mux,0 or 2=2m BPF, 1=4m BPF, 3=52MHz LPF
#define DDCboard_REG_CTRL_AdcRes (1<<9)
#define DDCboard_REG_CTRL_RevDAC (1<<10)    //if set, turns on the chinese clone fault by not proper handling of U2 data
#define DDCboard_REG_CTRL_AttDAC (1<<11)    //shifts two bits down the output amplitude (attenuate by 12dB for f<144MHz to compensate 3rd alias attenuation)
#define DDCboard_REG_CTRL_LED2  (1<<15)

#define DDCboard_REG_RXfreq     2
#define DDCboard_REG_TXfreq     3
#define DDCboard_REG_ATT        4
#define DDCboard_REG_TXlvl      5

#define DDCboard_RegPeriphSpi   16

//ADS6145 SPI registers
#define ADS6145_Reg_00 (0x00<<11)
    #define ADS6145_Reg_00_CoarseGainEnable (1<<9)
#define ADS6145_Reg_0a (0x0a<<11)
    #define ADS6145_Reg_0a_TestPattern(x)  (x<<5)
    #define ADS6145_Reg_0a_formatBinary  (0<<10)
    #define ADS6145_Reg_0a_format2sCompl (1<<10)
#define ADS6145_Reg_0b (0x0b<<11)
    #define ADS6145_Reg_0b_CustomLow(x) ((x&0x1ff)<<2)
#define ADS6145_Reg_0c (0x0c<<11)
    #define ADS6145_Reg_0c_FineGain(x) ((x&7)<<8)
    #define ADS6145_Reg_0c_CustomHigh(x) ((x>>9)&0x1f)

//Attenuator SPI address
#define Att_RX 2
#define Att_TX 3

enum {
    att_30=0,
    att_20,
    att_10,
    att_off,
    amp10,
    amp20
};
const int8_t att_table[]={-30,-20,-10,0,10,20};

#define SParkle_att_max att_30
#define SParkleHF_amp_max amp10
#define SParkleVHF_amp_max amp20

static inline void SP_DDCboard_CSon(void)
{
    GPIO_ResetBits(FPGA_CS_PORT,FPGA_CS_PIN);
}

static inline void SP_DDCboard_CSoff(void)
{
    GPIO_SetBits(FPGA_CS_PORT,FPGA_CS_PIN);
}

static uint32_t SP_DDCboard_read(uint8_t regaddr)
{
    uint8_t tx_data[5];
    uint8_t rx_data[5];
    uint32_t retval=0;

    tx_data[0] = regaddr;
    tx_data[1] = 0;
    tx_data[2] = 0;
    tx_data[3] = 0;
    tx_data[4] = 0;

    SP_DDCboard_CSon();
    if (HAL_SPI_TransmitReceive(&hspiFPGA,tx_data,rx_data, 5,100) == HAL_OK)
    {
            retval = rx_data[1] << 24 | rx_data[2] << 16 | rx_data[3] << 8 |rx_data[4];
    }
    SP_DDCboard_CSoff();
    return retval;
}

static uint32_t SP_DDCboard_write(uint8_t regaddr,uint32_t data)
{
    uint8_t tx_data[5];
    uint32_t retval=SParkle_DDCboard_Fail;

    tx_data[0] = regaddr;
    tx_data[1] = (uint8_t)(data>>24);
    tx_data[2] = (uint8_t)(data>>16);
    tx_data[3] = (uint8_t)(data>>8);
    tx_data[4] = (uint8_t)data;

    SP_DDCboard_CSon();
    if (HAL_SPI_Transmit(&hspiFPGA,tx_data,5,100) == HAL_OK)
    {
        retval=SParkle_DDCboard_OK;
    }
    SP_DDCboard_CSoff();
    return retval;
}

static uint32_t SP_DDCboard_writePeriphSPI(uint32_t data)
{
    uint8_t tx_data[8];
    uint32_t retval=SParkle_DDCboard_Fail;

    tx_data[0] = DDCboard_RegPeriphSpi;
    tx_data[1] = (uint8_t)(data>>24);
    tx_data[2] = (uint8_t)(data>>16);
    tx_data[3] = (uint8_t)(data>>8);
    tx_data[4] = (uint8_t)data;
    tx_data[5]=0;
    tx_data[6]=0;
    tx_data[7]=0;

    SP_DDCboard_CSon();
    if (HAL_SPI_Transmit(&hspiFPGA,tx_data,5,100) == HAL_OK)
    {
        retval=SParkle_DDCboard_OK;
    }
    SP_DDCboard_CSoff();

    if (HAL_SPI_Transmit(&hspiFPGA,&tx_data[5],3,100) == HAL_OK)
    {
        retval=SParkle_DDCboard_OK;
    }
    else
        retval=SParkle_DDCboard_Fail;

    return retval;
}

static bool SParkle_IsPresent(void)
{
    return SParkleState.is_present;
}

//we use SPI6 for communicating with the DDC board, if DDC Tag is received after checking REG0, function returns true
bool SParkle_CheckPresence(void)
{
    HAL_GPIO_WritePin(FPGA_CS_PORT, FPGA_CS_PIN, GPIO_PIN_SET);

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = FPGA_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(FPGA_CS_PORT, &GPIO_InitStruct);

    // setup SPI with correct data
    hspiFPGA.Init.Mode = SPI_MODE_MASTER;
    hspiFPGA.Init.Direction = SPI_DIRECTION_2LINES;
    hspiFPGA.Init.DataSize = SPI_DATASIZE_8BIT;
    //hspiFPGA.Init.CLKPolarity = SPI_POLARITY_HIGH;
    //hspiFPGA.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspiFPGA.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspiFPGA.Init.CLKPhase = SPI_PHASE_1EDGE;

    hspiFPGA.Init.NSS = SPI_NSS_SOFT;
    hspiFPGA.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspiFPGA.Init.FirstBit = SPI_FIRSTBIT_MSB;

    if (HAL_SPI_Init(&hspi6) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_SPI_ENABLE(&hspi6);

    SParkleState.version_major=0;
    SParkleState.version_minor=0;

    uint32_t stat=SP_DDCboard_read(DDCboard_REG_STAT);
    uint16_t marker=stat>>16;

    if(marker==0xa55a)
    {
        SParkleState.version_major=stat>>8;
        SParkleState.version_minor=stat;

        SParkleState.TestStatus&=~SParkleStat_BaseBoardPresent;
        if(UhsdrHw_I2C_DeviceReady(SParkleBB_I2C,SParkleBB_I2C_adr1) == HAL_OK)
        {
            if(UhsdrHw_I2C_DeviceReady(SParkleBB_I2C,SParkleBB_I2C_adr2) == HAL_OK)
            {
                SParkleState.TestStatus|=SParkleStat_BaseBoardPresent;
            }
        }

        return true;
    }

    return false;
}


//Reconfiguration of peripherals for DDC board usage
//this is necessary to keep compatibility with "conventional" analog RF boards.
//The main difference is that the source of I2S(SAI) clock is provided by FPGA board as the source of RF signal.
static void SParkle_ConfigureSAI(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /**SAI2_B_Block_B GPIO Configuration
        PE6     ------> SAI2_MCLK_B
        PC0     ------> SAI2_FS_B
        PA2     ------> SAI2_SCK_B
        PG10     ------> SAI2_SD_B
     */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_SAI2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_SAI2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);



    /*Configure GPIO pin : PC9 */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;  //strange alternate name, but there is no I2S_CKIN definition in HAL :)
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    __HAL_RCC_SAI1_CONFIG(RCC_SAI1CLKSOURCE_PIN);
    __HAL_RCC_SAI2_CONFIG(RCC_SAI1CLKSOURCE_PIN);

    hsai_BlockA2.Instance = SAI2_Block_A;
    hsai_BlockA2.Init.AudioMode = SAI_MODESLAVE_RX;
    hsai_BlockA2.Init.Synchro = SAI_SYNCHRONOUS;
    hsai_BlockA2.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
    hsai_BlockA2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
    hsai_BlockA2.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
    hsai_BlockA2.Init.MonoStereoMode = SAI_STEREOMODE;
    hsai_BlockA2.Init.CompandingMode = SAI_NOCOMPANDING;
    hsai_BlockA2.Init.TriState = SAI_OUTPUT_NOTRELEASED;
    if (HAL_SAI_InitProtocol(&hsai_BlockA2, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_32BIT, 2) != HAL_OK)
    {
      Error_Handler();
    }

    hsai_BlockB2.Instance = SAI2_Block_B;
    hsai_BlockB2.Init.AudioMode = SAI_MODESLAVE_TX;     //SAI_MODEMASTER_TX;
    hsai_BlockB2.Init.Synchro = SAI_ASYNCHRONOUS;       //SAI_ASYNCHRONOUS;
    hsai_BlockB2.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
    hsai_BlockB2.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    hsai_BlockB2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
    hsai_BlockB2.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
    hsai_BlockB2.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
    hsai_BlockB2.Init.MonoStereoMode = SAI_STEREOMODE;
    hsai_BlockB2.Init.CompandingMode = SAI_NOCOMPANDING;
    hsai_BlockB2.Init.TriState = SAI_OUTPUT_NOTRELEASED;
    if (HAL_SAI_InitProtocol(&hsai_BlockB2, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_32BIT, 2) != HAL_OK)
    {
      Error_Handler();
    }

    //changing default DMA settings generated by CubeMX and used by normal UI board, to setting required for DDC/DUC
    //most of the structure data is the same like the CUbeMX generated, but for some more readability copied here too

    hdma_sai2_a.Instance = DMA2_Stream2;
    hdma_sai2_a.Init.Channel = DMA_CHANNEL_10;
    hdma_sai2_a.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sai2_a.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sai2_a.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sai2_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sai2_a.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sai2_a.Init.Mode = DMA_CIRCULAR;
    hdma_sai2_a.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sai2_a.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sai2_a.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai2_a.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_sai2_a.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_sai2_a) != HAL_OK)
    {
        Error_Handler();
    }

    hdma_sai2_b.Instance = DMA2_Stream6;
    hdma_sai2_b.Init.Channel = DMA_CHANNEL_3;
    hdma_sai2_b.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sai2_b.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sai2_b.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sai2_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sai2_a.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sai2_b.Init.Mode = DMA_CIRCULAR;
    hdma_sai2_b.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sai2_b.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sai2_b.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai2_b.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_sai2_b.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_sai2_b) != HAL_OK)
    {
        Error_Handler();
    }
}

static void SParkle_SetPPM(float32_t ppm)
{
    SParkleState.ppm=ppm;
}

static void SParkle_UpdateConfig(uint32_t cfgValue, bool state)
{
    if(state)
        SParkleState.DDC_RegConfig|=cfgValue;
    else
        SParkleState.DDC_RegConfig&=~cfgValue;

    SP_DDCboard_write(DDCboard_REG_CTRL,SParkleState.DDC_RegConfig);
}

static Oscillator_ResultCodes_t SParkle_DDCboard_PrepareNextFrequency(uint32_t freq, int temp_factor)
{
    //determining the Nyquist zone and set frequency to fit in 1st Nyquist zone

    uint8_t next_ampSel=0;
    uint8_t next_bandSel=SParkleBB_I2C_adr1_BND_all;

    if(freq>=(oscDDC_f_sample))
    {
        freq-=oscDDC_f_sample;
        SParkleState.Nyquist_Zone=3;
        SParkleState.AntiAliasFilterSeting=DDCboard_REG_CTRL_ADCFLTR_2mBPF;
    }
    else if(freq>=(oscDDC_f_sample/2))
    {
        freq=oscDDC_f_sample/2-(freq-oscDDC_f_sample/2);
        SParkleState.Nyquist_Zone=2;
        SParkleState.AntiAliasFilterSeting=DDCboard_REG_CTRL_ADCFLTR_4mBPF;
    }
    else
    {
        SParkleState.Nyquist_Zone=1;
        SParkleState.AntiAliasFilterSeting=DDCboard_REG_CTRL_ADCFLTR_LPF;
        next_ampSel=SParkle_AMPselHF;

        uint32_t dial_freq=freq-AudioDriver_GetTranslateFreq();
        //prepare the selection of the right filter in filter board regarding of wanted frequency
        if((dial_freq>=1810000) && (dial_freq<2000000))   //band 160m
        {
            next_bandSel=SParkleBB_I2C_adr1_BND_160m;
        }
        else if((dial_freq>=3500000) && (dial_freq<3800000))   //band 80m
        {
            next_bandSel=SParkleBB_I2C_adr1_BND_80m;
        }
        else if((dial_freq>=5100000) && (dial_freq<7200000))   //bands 60m..40m
        {
            next_bandSel=SParkleBB_I2C_adr1_BND_60_40m;
        }
        else if((dial_freq>=10100000) && (dial_freq<14350000))   //bands 60m..40m
        {
            next_bandSel=SParkleBB_I2C_adr1_BND_30_20m;
        }
        else if((dial_freq>=18000000) && (dial_freq<21450000))   //bands 17m..15m
        {
            next_bandSel=SParkleBB_I2C_adr1_BND_17_15m;
        }
        else if((dial_freq>=24890000) && (dial_freq<29700000))   //bands 12m..10m
        {
            next_bandSel=SParkleBB_I2C_adr1_BND_12_10m;
        }
        else if((dial_freq>=50000000) && (dial_freq<52000000))   //band6m
        {
            next_bandSel=SParkleBB_I2C_adr1_BND_6m;
        }
        else
        {
            next_bandSel=SParkleBB_I2C_adr1_BND_all;  //default the low pass filter for 6m
        }


    }


    SParkleState.next_BB_reg1=next_bandSel;
    SParkleState.next_BB_reg2&=~SParkleBB_I2C_adr2_AMP_EN3; //amp3 is future option, not used anywhere yet

    switch(next_ampSel)
    {
        case SParkle_AMPselHF:
            SParkleState.next_BB_reg1|=SParkleBB_I2C_adr1_AMP_EN1;
            break;
        case SParkle_AMPselVHF:
            SParkleState.next_BB_reg1|=SParkleBB_I2C_adr1_AMP_EN2;
            break;
    }

    //CORDIC oscillator works by adding phase offset,
    //thus we need to recalculate the needed frequency to exact phase offset added by each edge of the sampling clock
    //this code was implemented in fpga, moving it here we save about 100 macrocells in the fpga

    int64_t phase=1172812403;  // B57 = 2^57.   M2 = B57/122880000;
    phase *=freq;
    phase+=16777216;           // M3 = 2^24, used to round the result
    phase >>=25;
    SParkleState.next_frequency=phase; //phase = (rx_freq * M2 + M3)>>25;

    return OSC_OK;
}

static uint16_t SParkle_WriteBBRegister1(uint8_t reg, uint8_t val)
{
    return UhsdrHw_I2C_WriteRegister(SParkleBB_I2C, SParkleBB_I2C_adr1, reg, 1, val);
}

static Oscillator_ResultCodes_t SParkle_DDCboard_ChangeToNextFrequency()
{
    Oscillator_ResultCodes_t retval = OSC_OK;

    SP_DDCboard_write(DDCboard_REG_RXfreq,SParkleState.next_frequency);
    SP_DDCboard_write(DDCboard_REG_TXfreq,SParkleState.next_frequency);
    if(SParkleState.prevAntiAliasFilterSeting!=SParkleState.AntiAliasFilterSeting)
    {
        SParkleState.prevAntiAliasFilterSeting=SParkleState.AntiAliasFilterSeting;
        SParkleState.DDC_RegConfig&=~DDCboard_REG_CTRL_ADCFLTRmsk;
        if(SParkleState.Nyquist_Zone==2)
        {
            SParkleState.DDC_RegConfig|=DDCboard_REG_CTRL_RCV1revIQ;        //reversing the order of IQ data for even Nyquist zone (2nd)
        }
        else
        {

            SParkleState.DDC_RegConfig&=~DDCboard_REG_CTRL_RCV1revIQ;
        }

        if(SParkleState.Nyquist_Zone==3)
        {
            SParkleState.DDC_RegConfig&=~DDCboard_REG_CTRL_AttDAC;  //full power on dac for 2m (attenuation compensation for 3rd nyquist zone)
        }
        else
        {
            SParkleState.DDC_RegConfig|=DDCboard_REG_CTRL_AttDAC;   //setting attenuation for all other than 2m bands
        }

        SParkle_UpdateConfig(SParkleState.AntiAliasFilterSeting,ENABLE);
    }

    if(SParkleState.current_BB_reg1!=SParkleState.next_BB_reg1)
    {
        SParkleState.current_BB_reg1=SParkleState.next_BB_reg1;
        SParkle_WriteBBRegister1(PCA_9554_RegOutput,SParkleState.current_BB_reg1);
    }

    return retval;
}

static bool SParkle_DDCboard_IsNextStepLarge()
{
    return false;
}

static uint32_t SParkle_DDCboard_getMinFrequency()
{
    return (uint32_t)100000;
}

static uint32_t SParkle_DDCboard_getMaxFrequency()
{
    return (uint32_t)150000000;
}

/**
 * @brief Checks if all oscillator resources are available for switching frequency
 * It basically checks if the SPI is currently in use
 * This function must be called before changing the oscillator in interrupts
 * otherwise deadlocks may happen
 * @return true if it is safe to call oscillator functions in an interrupt
 */
static bool SParkle_DDCboard_ReadyForIrqCall()
{
    return true;
}

static uint8_t SParkle_SetAttenuator(uint16_t Chan, float32_t att)
{
    uint8_t tx_data[7];
    uint32_t retval=SParkle_DDCboard_Fail;
    int16_t data=att*2;
    data<<=10;

    tx_data[0] = DDCboard_RegPeriphSpi;
    tx_data[1] = (uint8_t)(Chan>>8);
    tx_data[2] = (uint8_t)(Chan);
    tx_data[3] = (uint8_t)(data>>8);
    tx_data[4] = (uint8_t)data;
    tx_data[5]=0;
    tx_data[6]=0;

    SP_DDCboard_CSon();
    if (HAL_SPI_Transmit(&hspiFPGA,tx_data,5,100) == HAL_OK)
    {
        retval=SParkle_DDCboard_OK;
    }
    SP_DDCboard_CSoff();

    if (HAL_SPI_Transmit(&hspiFPGA,&tx_data[5],2,100) == HAL_OK)
    {
        retval=SParkle_DDCboard_OK;
    }
    else
        retval=SParkle_DDCboard_Fail;

    return retval;
}

/**
 * @brief sets the output attenuator according to required power percentage
 * @param pf  the value of output power to set
 */
bool SParkle_SetTXpower(float32_t pf)
{
    static float32_t oldPF=0;
    float32_t oldPF_=oldPF;
    bool power_changed=oldPF_!=pf;

    if(power_changed)
    {
        float32_t newpwr=10*log10f(pf);
        if(newpwr<-31.5)
        {
            newpwr=-31.5;
        }

        SParkle_SetAttenuator(Att_TX,-newpwr);
    }

    oldPF=pf;   //store new value
    return power_changed;
}

static void osc_SParkle_CheckMaxATT()
{
    uint8_t maxtval=SParkleHF_amp_max;
    if(SParkleState.Nyquist_Zone>1)
    {
        maxtval=SParkleVHF_amp_max;
    }

    if(SParkleState.RX_amp_idx>127)
    {
        SParkleState.RX_amp_idx=0;
    }

    if(SParkleState.RX_amp_idx>maxtval)
    {
        SParkleState.RX_amp_idx=maxtval;
    }
}

static void osc_SParkle_updateRXATT()
{
    if(SParkleState.current_RX_amp_idx!=SParkleState.RX_amp_idx)
    {
        SParkleState.current_RX_amp_idx=SParkleState.RX_amp_idx;

        int8_t gain=att_table[SParkleState.RX_amp_idx];
        int8_t atten=0;
        if(gain<=0)
        {
            atten=gain*(-1);
            SParkleState.next_BB_reg1&=~SParkleBB_I2C_adr1_LNA;
        }
        else if(gain>0)
        {
            SParkleState.next_BB_reg1|=SParkleBB_I2C_adr1_LNA;
        }

        SParkle_SetAttenuator(Att_RX,atten);
        if(SParkleState.current_BB_reg1!=SParkleState.next_BB_reg1)
        {
            SParkleState.current_BB_reg1=SParkleState.next_BB_reg1;
            SParkle_WriteBBRegister1(PCA_9554_RegOutput,SParkleState.current_BB_reg1);
        }

    }
}
static int8_t osc_SParkle_ATTgetCurrent(void)
{
    osc_SParkle_CheckMaxATT();
    osc_SParkle_updateRXATT();
    return att_table[SParkleState.RX_amp_idx];
}

static int8_t osc_SParkle_ATTsetNext(void)
{
    SParkleState.RX_amp_idx++;
    osc_SParkle_CheckMaxATT();
    return att_table[SParkleState.RX_amp_idx];
}

static int8_t osc_SParkle_ATTsetPrev(void)
{
    SParkleState.RX_amp_idx--;
    osc_SParkle_CheckMaxATT();
    return att_table[SParkleState.RX_amp_idx];
}
bool SParkle_GetDacType(void)
{
    return (SParkleState.DDC_RegConfig&DDCboard_REG_CTRL_RevDAC)!=0;
}

void SParkle_SetDacType(bool DacType)
{
    if(DacType)
    {
        SParkle_UpdateConfig(DDCboard_REG_CTRL_RevDAC,ENABLE);
        SParkleState.EEPROM_Flags|=EEPROM_SParkleFLAG_DACtype;
    }
    else
    {
        SParkle_UpdateConfig(DDCboard_REG_CTRL_RevDAC,DISABLE);
        SParkleState.EEPROM_Flags&=~EEPROM_SParkleFLAG_DACtype;
    }

}
//this raoutine is called after eeprom read
void SParkle_ConfigurationInit(void)
{
    SParkle_UpdateConfig(DDCboard_REG_CTRL_RevDAC,(SParkleState.EEPROM_Flags&EEPROM_SParkleFLAG_DACtype)==0?DISABLE:ENABLE);
    SParkleState.EEPROM_Flags&=EEPROM_SParkleFLAGS_MASK;
}

bool osc_SParkle_Init(void)
{
    SParkleState.current_frequency = 0;
    SParkleState.next_frequency = 0;
    SParkleState.next_BB_reg1=0;
    SParkleState.next_BB_reg2=0;
    SParkleState.current_BB_reg1=255;    //255 to trigger update of i2c expander
    SParkleState.current_BB_reg2=255;    //255 to trigger update of i2c expander
    SParkleState.current_RX_amp_idx=255;

    SParkleState.is_present = SParkle_CheckPresence();

    if (SParkleState.is_present)
    {
        SParkleState.DDC_RegConfig=DDCboard_REG_CTRL_AMP1;
        RFboard.AMP_ATT_getCurrent=osc_SParkle_ATTgetCurrent;
        RFboard.AMP_ATT_next=osc_SParkle_ATTsetNext;
        RFboard.AMP_ATT_prev=osc_SParkle_ATTsetPrev;

        SParkle_ConfigureSAI();
        SParkle_UpdateConfig(DDCboard_REG_CTRL_SAIen | DDCboard_REG_CTRL_AdcRes | DDCboard_REG_CTRL_AMP1 | DDCboard_REG_CTRL_LED2,ENABLE);   //enable MCLK, Reset ADC to default state
        SParkle_UpdateConfig(DDCboard_REG_CTRL_AdcRes,DISABLE);

        SParkle_UpdateConfig(DDCboard_REG_CTRL_ADCFLTR_LPF | DDCboard_REG_CTRL_RXANT | DDCboard_REG_CTRL_TXANT,ENABLE);

        //DDCboard_UpdateConfig(DDCboard_REG_CTRL_SAITEST,ENABLE);

        SParkle_SetAttenuator(Att_RX,0);
        SParkle_SetAttenuator(Att_TX,0);

        //DDCboard_writePeriphSPI(0x00010000|ADS6145_Reg_0a|ADS6145_Reg_0a_TestPattern(4)); //increment for every cycle (for pin soldering test)


        //disabled for future optional usage of ADC internal gain setting

        //DDCboard_writePeriphSPI(0x00010000|ADS6145_Reg_00|ADS6145_Reg_00_CoarseGainEnable); //gain set to 3dB
        //DDCboard_writePeriphSPI(0x00010000|ADS6145_Reg_0c|ADS6145_Reg_0c_FineGain(6)); //gain set to 3dB


        if(SParkleState.TestStatus&SParkleStat_BaseBoardPresent)
        {
            SParkle_WriteBBRegister1(PCA_9554_RegConfig,0);    //expander#1 setup all to outputs
        }

        ts.DisableTCXOdisplay=1;    //disable the tcxo field in layout
        SParkleState.RX_amp_idx=att_off;
        ts.ATT_Gain=att_table[SParkleState.RX_amp_idx];  //enable display of attenuation/amplification control
        ts.TX_at_zeroIF=1;      //DUC input interpolating FIR filter has roll off around 12kHz causing -12kHz USB and +12kHz LSB not being transmitted, so there is a must for transmit at zero if
                                //TODO: proof that FM/SAM transmit works as expected

    }

    return  SParkle_IsPresent();

}

const OscillatorInterface_t osc_SParkle_DDC =
{
        .init = osc_SParkle_Init,
        .isPresent = SParkle_IsPresent,
        .setPPM = SParkle_SetPPM,
        .prepareNextFrequency = SParkle_DDCboard_PrepareNextFrequency,
        .changeToNextFrequency = SParkle_DDCboard_ChangeToNextFrequency,
        .isNextStepLarge = SParkle_DDCboard_IsNextStepLarge,
        .readyForIrqCall = SParkle_DDCboard_ReadyForIrqCall,
        .getMinFrequency = SParkle_DDCboard_getMinFrequency,
        .getMaxFrequency = SParkle_DDCboard_getMaxFrequency,
        .name = "SParkle DDC",
        .type = OSC_SPARKLE,
};

#endif
