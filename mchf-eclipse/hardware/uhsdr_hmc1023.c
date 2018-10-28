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

#define HMC1023_CS_PIN GPIO_PIN_9
#define HMC1023_CS_PORT GPIOG
#define hspiHmc1023 (hspi6)

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

/**
 * Switch hardware SPI settings from tx to rx mode and vice versa. Also provides necessary delay between
 * tx and rx.
 * HMC1023LP5E needs phase == 1EDGE for transmit by STM and 2EDGE for RX sampling by STM. Who designs such an SPI protocol?
 * @param is_tx true -> configure for  tx mode, false -> for rx mode
 */
static void hmc1023_ll_spi_tx(bool is_tx)
{

  hspiHmc1023.Init.CLKPhase = is_tx == true? SPI_PHASE_1EDGE : SPI_PHASE_2EDGE;
  if (HAL_SPI_Init(&hspi6) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_SPI_ENABLE(&hspi6);
}

/**
 * Control selection of HMC1023LP5E. Please note, output of SDO does not go tristate if deselected!
 * @param on true -> chip is selected (HMC1023LP5E input EN == LOW), false -> not selected
 */
static void hmc1023_ll_cs(bool on)
{
    if (on)
    {
        GPIO_ResetBits(HMC1023_CS_PORT,HMC1023_CS_PIN);
    }
    else
    {
        GPIO_SetBits(HMC1023_CS_PORT,HMC1023_CS_PIN);
    }
}

/**
 * Write  register value into the HMC1023LP5E. It is not checked if the value was received correctly!
 * @param regaddr which register to write to, not range checked!
 * @param value 24 bit value
 * @return HMC1023_ERROR if failed, HMC1023_OK otherwise.
 */
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
/**
 *
 * @param regaddr which register to read, not range checked!
 * @return 24 bit value in case of success, HMC1023_ERROR otherwise
 */
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

/**
 *
 * @return true if HMC1023LP5E was detected by reading the correct id
 */
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
        const uint32_t shifted_mask = (mask << shift);
        const uint32_t shifted_bits = (bits << shift);

        if ( (hmc1023.reg2 & shifted_mask)  != shifted_bits)
        {
            MODIFY_REG(hmc1023.reg2,shifted_mask,shifted_bits);
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

/**
 * Control HMC1023LP5E opamp bias
 * @param value: 0 - 3 (higher value, higher bias)
 */
void hmc1023_set_bias_opamp(uint8_t value)
{
    hmc1023_set_reg2_bits(value,HMC1023_REG2_OPAMP_LIMIT,HMC1023_REG2_OPAMP_MASK, HMC1023_REG2_OPAMP_SHIFT);
}

/**
 * Control HMC1023LP5E driver bias
 * @param value: 0 - 3 (higher value, higher bias)
 */
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

/**
 * use HMC1023LP5E register settings for coarse and fine LPF bandwidth control
 * please note that this enables both coarse and fine bandwidth from SPI, this is incompatible with
 * automatic calibration as discussed in data sheet.
 * @param on true -> use bandwidth settings in reg2/reg3, false -> use internal defaults
 */
void hmc1023_use_spi_settings(bool on)
{
    const uint32_t target_bits = on == true ? (HMC1023_REG1_USE_SPI_SETTINGS | HMC1023_REG1_FORCE_CAL_CODE) : 0;
    const uint32_t target_mask = (HMC1023_REG1_USE_SPI_SETTINGS | HMC1023_REG1_FORCE_CAL_CODE);
    if ((hmc1023.reg1 & target_mask) != target_bits)
    {
        MODIFY_REG(hmc1023.reg1, target_mask, target_bits);
        hmc1023_ll_write(1,hmc1023.reg1);
    }
}

/**
 * Configures SPI bus 6 AND CS GPIO PG9 to detect and control an HMC1023LP5E LPF
 * presence of an LPF is return via hmc1023.present
 * Does not return hardware configuration to original state, even if LPF is not detected!
 */
void hmc1023_init()
{

    // TODO: Move to HAL Config
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(HMC1023_CS_PORT, HMC1023_CS_PIN, GPIO_PIN_SET);

    GPIO_InitTypeDef GPIO_InitStruct;
    /*Configure GPIO pins : PFPin PFPin PFPin */
    GPIO_InitStruct.Pin = HMC1023_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(HMC1023_CS_PORT, &GPIO_InitStruct);

    // setup SPI with correct data
    hspiHmc1023.Init.Mode = SPI_MODE_MASTER;
    hspiHmc1023.Init.Direction = SPI_DIRECTION_2LINES;
    hspiHmc1023.Init.DataSize = SPI_DATASIZE_8BIT;
    hspiHmc1023.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspiHmc1023.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspiHmc1023.Init.NSS = SPI_NSS_SOFT;
    hspiHmc1023.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspiHmc1023.Init.FirstBit = SPI_FIRSTBIT_MSB;

    hmc1023_ll_spi_tx(true);
    // make sure we are in right config for TX,
    // which is assumed by the read/write code


    hmc1023.present = hmc1023_ll_presence();

    if (hmc1023.present)
    {
        hmc1023.reg1 = hmc1023_ll_read(1);
        hmc1023.reg2 = hmc1023_ll_read(2);
        hmc1023.reg2 = hmc1023_ll_read(3);
        hmc1023_use_spi_settings(true);
        hmc1023_set_bias_opamp(1);
        hmc1023_set_bias_opamp(0);
    }
}
#endif // UI_BRD_OVI40
