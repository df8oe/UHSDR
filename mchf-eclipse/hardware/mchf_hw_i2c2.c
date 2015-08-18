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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

// Common
#include "mchf_board.h"

#include "mchf_hw_i2c2.h"

#define I2C2_FLAG_TIMEOUT          		((uint32_t)0x500)
#define I2C2_LONG_TIMEOUT          		((uint32_t)(300 * I2C2_FLAG_TIMEOUT))

#define I2C2_SPEED             			25000

// I2C peripheral configuration defines (control interface of the si570)
#define CODEC_I2C                      	I2C2
#define I2C2_CLK                  		RCC_APB1Periph_I2C2
#define CODEC_I2C_GPIO_AF              	GPIO_AF_I2C2

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

	// CODEC_I2C SCL and SDA pins configuration
	GPIO_InitStructure.GPIO_Pin   = I2C2_SCL_PIN|I2C2_SDA_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;

	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;		// we also have ext pullups
	GPIO_Init(I2C2_SCL_GPIO, &GPIO_InitStructure);

	// Connect pins to I2C peripheral
	GPIO_PinAFConfig(I2C2_SCL_GPIO, I2C2_SCL_PINSRC, CODEC_I2C_GPIO_AF);
	GPIO_PinAFConfig(I2C2_SDA_GPIO, I2C2_SDA_PINSRC, CODEC_I2C_GPIO_AF);

	// Enable the CODEC_I2C peripheral clock
	RCC_APB1PeriphClockCmd(I2C2_CLK, ENABLE);

	// CODEC_I2C peripheral configuration
	I2C_DeInit(CODEC_I2C);
	I2C_InitStructure.I2C_Mode 					= I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle 			= I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 			= 0x33;
	I2C_InitStructure.I2C_Ack 					= I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress 	= I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed 			= I2C2_SPEED;

	// Enable the I2C peripheral
	I2C_Cmd (CODEC_I2C, ENABLE);
	I2C_Init(CODEC_I2C, &I2C_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : mchf_hw_i2c2_WriteRegister
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar mchf_hw_i2c2_WriteRegister(uchar I2CAddr,uchar RegisterAddr, uchar RegisterValue)
{
	//printf("i2c write 0x%02x\n\r",I2CAddr);

	// While the bus is busy
	I2C2_Timeout = I2C2_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BUSY))
	{
		if((I2C2_Timeout--) == 0)
			return 1;
	}

	// Start the config sequence
	I2C_GenerateSTART(CODEC_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C2_Timeout = I2C2_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C2_Timeout--) == 0)
			return 2;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(CODEC_I2C, I2CAddr, I2C_Direction_Transmitter);

	// Test on EV6 and clear it
	I2C2_Timeout = I2C2_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((I2C2_Timeout--) == 0)
			return 3;
	}

	// Transmit the first address for write operation
	I2C_SendData(CODEC_I2C, RegisterAddr);

	// Test on EV8 and clear it
	I2C2_Timeout = I2C2_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
	{
		if((I2C2_Timeout--) == 0)
			return 4;
	}

	// Prepare the register value to be sent
	I2C_SendData(CODEC_I2C, RegisterValue);

	// Wait till all data have been physically transferred on the bus
	I2C2_Timeout = I2C2_LONG_TIMEOUT;
	while(!I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BTF))
	{
		if((I2C2_Timeout--) == 0)
			return 5;
	}

	// End the configuration sequence
	I2C_GenerateSTOP(CODEC_I2C, ENABLE);

	// stop bit flag
	I2C2_Timeout = I2C2_FLAG_TIMEOUT;
	while(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_STOPF))
	{
		if((I2C2_Timeout--) == 0)
			return 6;
	}

	//printf("i2c write ok\n\r");
	return 0;
}
