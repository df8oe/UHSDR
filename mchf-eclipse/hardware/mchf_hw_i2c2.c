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
#include "ui_lcd_hy28.h"
#include "mchf_hw_i2c.h"
#include "mchf_hw_i2c2.h"

#define I2C2_SPEED             			25000

// I2C peripheral configuration defines (control interface of the si570)
#define I2C2_CLK                  	RCC_APB1Periph_I2C2
#define I2C2_GPIO_AF              	GPIO_AF_I2C2

__IO uint32_t  I2C2_Timeout = I2C2_LONG_TIMEOUT;

//*----------------------------------------------------------------------------
//* Function Name       : mchf_hw_i2c2_init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_hw_i2c2_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // I2C2 SCL and SDA pins configuration
    GPIO_InitStructure.GPIO_Pin   = I2C2_SCL_PIN|I2C2_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;

    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;		// we also have ext pullups
    GPIO_Init(I2C2_SCL_GPIO, &GPIO_InitStructure);

    // Connect pins to I2C peripheral
    GPIO_PinAFConfig(I2C2_SCL_GPIO, I2C2_SCL_PINSRC, I2C2_GPIO_AF);
    GPIO_PinAFConfig(I2C2_SDA_GPIO, I2C2_SDA_PINSRC, I2C2_GPIO_AF);

    // Enable the I2C2 peripheral clock
    RCC_APB1PeriphClockCmd(I2C2_CLK, ENABLE);

    // I2C2 peripheral configuration
    I2C_DeInit(I2C2);
    I2C_InitStructure.I2C_Mode 				= I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle 			= I2C_DutyCycle_16_9;
    I2C_InitStructure.I2C_OwnAddress1 			= 0x33;
    I2C_InitStructure.I2C_Ack 				= I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress 		= I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed 			= I2C2_SPEED;

    // Enable the I2C peripheral
    I2C_Cmd (I2C2, ENABLE);
    I2C_Init(I2C2, &I2C_InitStructure);
}



//*----------------------------------------------------------------------------
//* Function Name       : mchf_hw_i2c2_reset
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/* void mchf_hw_i2c2_reset(void)
{
	I2C_SoftwareResetCmd(I2C2, ENABLE);
	non_os_delay();
	I2C_SoftwareResetCmd(I2C2, DISABLE);

	I2C_Cmd (I2C2, DISABLE);
	non_os_delay();
	I2C_Cmd (I2C2, ENABLE);

	// Init again
	mchf_hw_i2c2_init();
}
 */
