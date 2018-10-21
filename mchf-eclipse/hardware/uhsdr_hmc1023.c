/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/
#include "uhsdr_board.h"
#include "uhsdr_hmc1023.h"

HMC1023_t hmc1023;

#ifdef USE_HMC1023

#include <spi.h>

#define HMC1023_REG3_FINE_LIMIT  (11)

#define HMC1023_REG2_OPAMP_LIMIT (3)
#define HMC1023_REG2_OPAMP_SHIFT (0)
#define HMC1023_REG2_OPAMP_MASK  (0x03)

#define HMC1023_REG2_DRVR_LIMIT (3)
#define HMC1023_REG2_DRVR_SHIFT (2)
#define HMC1023_REG2_DRVR_MASK (0x03)

#define HMC1023_REG2_GAIN_LIMIT (1)
#define HMC1023_REG2_GAIN_SHIFT (4)
#define HMC1023_REG2_GAIN_MASK (0x01)

#define HMC1023_REG2_BYPASS_LIMIT (1)
#define HMC1023_REG2_BYPASS_SHIFT (5)
#define HMC1023_REG2_BYPASS_MASK (0x01)

#define HMC1023_REG2_COARSE_LIMIT (8)
#define HMC1023_REG2_COARSE_SHIFT (6)
#define HMC1023_REG2_COARSE_MASK (0x0f)

#define HMC1023_REG1_USE_SPI_SETTINGS (1 << 1)
#define HMC1023_REG1_FORCE_CAL_CODE (1 << 4)

static void hmc1023_ll_spi_tx(bool is_tx)
{

  hspi6.Init.CLKPhase = is_tx == true? SPI_PHASE_1EDGE : SPI_PHASE_2EDGE;
  if (HAL_SPI_Init(&hspi6) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_SPI_ENABLE(&hspi6);
}


static void hmc1023_ll_cs(bool on)
{
    if (on)
    {
        GPIO_ResetBits(GPIOG,GPIO_PIN_9);
    }
    else
    {
        GPIO_SetBits(GPIOG,GPIO_PIN_9);
    }
}

static uint32_t hmc1023_ll_write(uint32_t regaddr, uint32_t value)
{
    uint8_t tx_data[4];
    uint32_t retval = HMC1023_ERROR;

    tx_data[0] = value >> 16;
    tx_data[1] = value >> 8;
    tx_data[2] = value;
    tx_data[3] = 0x5 | (regaddr << 3); // chip id & register to write;

    hmc1023_ll_cs(true);
    if (HAL_SPI_Transmit(&hspi6,tx_data,4,100) == HAL_OK)
    {
        retval = HMC1023_OK;
    }
    hmc1023_ll_cs(false);
    return retval;
}

static uint32_t hmc1023_ll_read(uint32_t regaddr)
{

    uint32_t retval = HMC1023_ERROR;
    // write register address to read to register 0
    if (hmc1023_ll_write(0,regaddr) == HAL_OK)
    {
        uint8_t tx_data[4] = { 0, 0, 0, 0};
        uint8_t rx_data[4];

        hmc1023_ll_spi_tx(false);
        // reconfig through HAL should cause enough delay
        hmc1023_ll_cs(true);

        // now all tx_data is zero and we try to read using
        // the rx spi config
        if (HAL_SPI_TransmitReceive(&hspi6,tx_data,rx_data, 4,100) == HAL_OK)
        {
            if (( rx_data[3] & 0x07) == 5)  // we found the correct "chip id"
            {
                retval = rx_data[0] << 16 | rx_data[1] << 8 | rx_data[2];
            }
        }

        hmc1023_ll_cs(false);
        hmc1023_ll_spi_tx(true);

    }

    return retval;
}

static bool hmc1023_ll_presence()
{
    uint32_t value = hmc1023_ll_read(0);
    return (value == 0x114080);
}

/**
 * Set bits in register 2
 * @param bits actual bits to set (zero based, bits will be later shifted)
 * @param limit maximum permitted value for bits (0...limit) is permitted range
 * @param mask bit mask for bits (zero based, bits will be later shifted)
 * @param shift position of mask (how many bits to shift left for final position)
 */

static void hmc1023_set_reg2_bits(uint8_t bits,uint8_t limit, uint32_t mask, uint8_t shift)
{
    if (bits <= limit)
    {
        if ( (hmc1023.reg2 & (mask <<shift))  != bits << shift)
        {
            hmc1023.reg2 &= ~mask;
            hmc1023.reg2 |= (bits << shift);
            hmc1023_ll_write(2,hmc1023.reg2);
        }
    }
}

/**
 * Sets the coarse bandwidth from about 5 / 7 / 10 / 14 / 20 / 28 / 40 / 50 / 72
 * Note +/-20% variation if not calibrated!
 * @param coarse 0 - 8
 */

void hmc1023_set_coarse(uint8_t value)
{
    hmc1023_set_reg2_bits(value,HMC1023_REG2_COARSE_LIMIT,HMC1023_REG2_COARSE_MASK, HMC1023_REG2_COARSE_SHIFT);
}

void hmc1023_set_bias_opamp(uint8_t value)
{
    hmc1023_set_reg2_bits(value,HMC1023_REG2_OPAMP_LIMIT,HMC1023_REG2_OPAMP_MASK, HMC1023_REG2_OPAMP_SHIFT);
}

void hmc1023_set_bias_drvr(uint8_t value)
{
    hmc1023_set_reg2_bits(value,HMC1023_REG2_DRVR_LIMIT,HMC1023_REG2_DRVR_MASK, HMC1023_REG2_DRVR_SHIFT);
}

/**
 * Sets the 10db amplifier on or off
 * @param on true on / false off
 */
void hmc1023_set_gain(bool on)
{
    hmc1023_set_reg2_bits(on?1:0,HMC1023_REG2_GAIN_LIMIT,HMC1023_REG2_GAIN_MASK, HMC1023_REG2_GAIN_SHIFT);
}

/**
 * Sets the LPF bypass on or off
 * @param on true on / false off
 */
void hmc1023_set_bypass(bool on)
{
    hmc1023_set_reg2_bits(on?1:0,HMC1023_REG2_BYPASS_LIMIT,HMC1023_REG2_BYPASS_MASK, HMC1023_REG2_BYPASS_SHIFT);
}

/**
 * adjusts the lpf bandwidth from about 0.803 to about 1.183 of the coarse frequency
 * Note +/-20% variation if not calibrated!
 * @param fine 0 - 11
 */
void hmc1023_set_fine(uint8_t fine)
{
    if (fine < HMC1023_REG3_FINE_LIMIT)
    {
        if ( (hmc1023.reg3)  != fine )
        {
            hmc1023.reg3 = fine;
            hmc1023_ll_write(3,hmc1023.reg3);
        }
    }
}

// call after hmc1023_set_fine / hmc1023_set_coarse
void hmc1023_activate_settings()
{
    if ((hmc1023.reg1 & (HMC1023_REG1_USE_SPI_SETTINGS | HMC1023_REG1_FORCE_CAL_CODE))
            != (HMC1023_REG1_USE_SPI_SETTINGS | HMC1023_REG1_FORCE_CAL_CODE))
    {
        hmc1023.reg1 |= HMC1023_REG1_USE_SPI_SETTINGS | HMC1023_REG1_FORCE_CAL_CODE;
        hmc1023_ll_write(1,hmc1023.reg1);
    }
}

void hmc1023_init()
{

    // TODO: Move to HAL Config
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_9, GPIO_PIN_SET);

    GPIO_InitTypeDef GPIO_InitStruct;
    /*Configure GPIO pins : PFPin PFPin PFPin */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    // setup SPI with correct data
    hspi6.Init.Mode = SPI_MODE_MASTER;
    hspi6.Init.Direction = SPI_DIRECTION_2LINES;
    hspi6.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi6.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi6.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi6.Init.NSS = SPI_NSS_SOFT;
    hspi6.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi6.Init.FirstBit = SPI_FIRSTBIT_MSB;

    hmc1023_ll_spi_tx(true);
    // make sure we are in right config for TX,
    // which is assumed by the read/write code


    hmc1023.present = hmc1023_ll_presence();

    if (hmc1023.present)
    {
        hmc1023.reg1 = hmc1023_ll_read(1);
        hmc1023.reg2 = hmc1023_ll_read(2);
        hmc1023.reg2 = hmc1023_ll_read(3);
        hmc1023_activate_settings();
    }
}
#endif // UI_BRD_OVI40