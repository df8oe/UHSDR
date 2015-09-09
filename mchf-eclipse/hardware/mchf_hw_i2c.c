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

#include "mchf_hw_i2c.h"

#define I2C1_FLAG_TIMEOUT          		((uint32_t)0x500)
#define I2C1_LONG_TIMEOUT          		((uint32_t)(300 * I2C1_FLAG_TIMEOUT))

//#define I2C1_SPEED             		400000
#define I2C1_SPEED             			100000
//#define I2C1_SPEED           			25000

// I2C peripheral configuration defines (control interface of the si570)
#define SI570_I2C                      	I2C1
#define I2C1_CLK                  		RCC_APB1Periph_I2C1
#define SI570_I2C_GPIO_AF              	GPIO_AF_I2C1

__IO uint32_t  I2C1_Timeout = I2C1_LONG_TIMEOUT;

/*static void i2c_delay(void)
{
	__asm("nop\n");
	__asm("nop\n");
	__asm("nop\n");
	__asm("nop\n");
}*/

//Wait till the specified SR1 Bits are set
//More than 1 Flag can be "or"ed. This routine reads only SR1.
static uint32_t WaitSR1FlagsSet (uint32_t Flags)
{
	uint32_t TimeOut = HSI_VALUE;

	while(((SI570_I2C->SR1) & Flags) != Flags)
	{
		if (!(TimeOut--))
			return 1;
	}

	return 0;
}

//Check to see if the Line is busy
//This bit is set automatically when a start condition is broadcasted on the line (even from another master)
//and is reset when stop condition is detected.
static uint32_t WaitLineIdle(void)
{
	uint32_t TimeOut = HSI_VALUE;

	while((SI570_I2C->SR2) & (I2C_SR2_BUSY))
	{
		if (!(TimeOut--))
			return 1;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       :
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_hw_i2c_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	// CODEC_I2C SCL and SDA pins configuration
	GPIO_InitStructure.GPIO_Pin = I2C1_SCL_PIN|I2C1_SDA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;

	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;		// - strong ringing on the bus
	//GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN; 	// - makes it better
	//GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;	// - same as pull down

	GPIO_Init(I2C1_SCL_GPIO, &GPIO_InitStructure);

	// Connect pins to I2C peripheral
	GPIO_PinAFConfig(I2C1_SCL_GPIO, I2C1_SCL_PINSRC, SI570_I2C_GPIO_AF);
	GPIO_PinAFConfig(I2C1_SDA_GPIO, I2C1_SDA_PINSRC, SI570_I2C_GPIO_AF);

	// Enable the CODEC_I2C peripheral clock
	RCC_APB1PeriphClockCmd(I2C1_CLK, ENABLE);

	// CODEC_I2C peripheral configuration
	I2C_DeInit(SI570_I2C);
	I2C_InitStructure.I2C_Mode 					= I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle 			= I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 			= 0x33;
	I2C_InitStructure.I2C_Ack 					= I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress 	= I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed 			= I2C1_SPEED;

	// Enable the I2C peripheral
	I2C_Cmd (SI570_I2C, ENABLE);
	I2C_Init(SI570_I2C, &I2C_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : trx4m_i2c_reset
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void mchf_hw_i2c_reset(void)
{
	//printf("===========================\n\r");
	//printf("i2c reset bus\n\r");
	//printf("===========================\n\r");

	I2C_SoftwareResetCmd(SI570_I2C, ENABLE);
	non_os_delay();
	I2C_SoftwareResetCmd(SI570_I2C, DISABLE);

	I2C_Cmd (SI570_I2C, DISABLE);
	non_os_delay();
	I2C_Cmd (SI570_I2C, ENABLE);

	// Init again
	mchf_hw_i2c_init();
}

//*----------------------------------------------------------------------------
//* Function Name       :
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar mchf_hw_i2c_WriteRegister(uchar I2CAddr,uchar RegisterAddr, uchar RegisterValue)
{
	//printf("i2c write 0x%02x 0x%02x 0x%02x\n\r",I2CAddr,RegisterAddr,RegisterValue);

	// While the bus is busy
	I2C1_Timeout = I2C1_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BUSY))
	{
		if((I2C1_Timeout--) == 0)
			return 1;
	}

	// Start the config sequence
	I2C_GenerateSTART(SI570_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C1_Timeout--) == 0)
			return 2;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(SI570_I2C, I2CAddr, I2C_Direction_Transmitter);

	// Test on EV6 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((I2C1_Timeout--) == 0)
			return 3;
	}

	// Transmit the first address for write operation
	I2C_SendData(SI570_I2C, RegisterAddr);

	// Test on EV8 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
	{
		if((I2C1_Timeout--) == 0)
			return 4;
	}

	// Prepare the register value to be sent
	I2C_SendData(SI570_I2C, RegisterValue);

	// Wait till all data have been physically transferred on the bus
	I2C1_Timeout = I2C1_LONG_TIMEOUT;
	while(!I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BTF))
	{
		if((I2C1_Timeout--) == 0)
			return 5;
	}

	// End the configuration sequence
	I2C_GenerateSTOP(SI570_I2C, ENABLE);

	// stop bit flag
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_STOPF))
	{
		if((I2C1_Timeout--) == 0)
			return 6;
	}

	return 0;
}

uchar mchf_hw_i2c_WriteBlock(uchar I2CAddr,uchar RegisterAddr, uchar *data,ulong size)
{
	ulong i;

	//printf("i2c write 0x%02x 0x%02x 0x%02x\n\r",I2CAddr,RegisterAddr,RegisterValue);

	// While the bus is busy
	I2C1_Timeout = I2C1_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BUSY))
	{
		if((I2C1_Timeout--) == 0)
			return 1;
	}

	// Start the config sequence
	I2C_GenerateSTART(SI570_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C1_Timeout--) == 0)
			return 2;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(SI570_I2C, I2CAddr, I2C_Direction_Transmitter);

	// Test on EV6 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((I2C1_Timeout--) == 0)
			return 3;
	}

	// Transmit the first address for write operation
	I2C_SendData(SI570_I2C, RegisterAddr);

	// Test on EV8 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
	{
		if((I2C1_Timeout--) == 0)
			return 4;
	}

	for(i = 0; i < size; i++)
	{
		// Prepare the register value to be sent
		I2C_SendData(SI570_I2C, *data++);

		// Wait till all data have been physically transferred on the bus
		I2C1_Timeout = I2C1_LONG_TIMEOUT;
		while(!I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BTF))
		{
			if((I2C1_Timeout--) == 0)
				return 5;
		}
	}

	// End the configuration sequence
	I2C_GenerateSTOP(SI570_I2C, ENABLE);

	// stop bit flag
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_STOPF))
	{
		if((I2C1_Timeout--) == 0)
			return 6;
	}

	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : trx4m_hw_i2c_ReadRegister
//* Object              : ver1 - working (but stalls next write sometimes)
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
/*uchar trx4m_hw_i2c_ReadRegister(uchar I2CAddr,uchar RegisterAddr, uchar *RegisterValue)
{
	//trx4m_i2c_switch_pins(I2CAddr);

	printf("i2c read\n\r");

	//  While the bus is busy
	I2C1_Timeout = I2C1_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BUSY))
	{
		if((I2C1_Timeout--) == 0)
			return 1;
	}

	// Start the config sequence
	I2C_GenerateSTART(SI570_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C1_Timeout--) == 0)
			return 2;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(SI570_I2C, I2CAddr, I2C_Direction_Transmitter);

	// Test on EV6 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((I2C1_Timeout--) == 0)
			return 3;
	}

	// Transmit the first address for write operation
	I2C_SendData(SI570_I2C, RegisterAddr);

	// Test on EV8 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
	{
		if((I2C1_Timeout--) == 0)
			return 4;
	}

	// Start the config sequence
	I2C_GenerateSTART(SI570_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C1_Timeout--) == 0)
			return 6;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(SI570_I2C, I2CAddr, I2C_Direction_Receiver);

	// Test on EV6 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
		if((I2C1_Timeout--) == 0)
			return 7;
	}

	// Enable NACK bit on next read and read final register
	I2C_NACKPositionConfig(SI570_I2C, I2C_NACKPosition_Current);
	I2C_AcknowledgeConfig(SI570_I2C, DISABLE);

	// Test on EV8 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
		if((I2C1_Timeout--) == 0)
			return 8;
	}

	// Read data
	*RegisterValue = I2C_ReceiveData(SI570_I2C);

	// End the configuration sequence
	I2C_GenerateSTOP(SI570_I2C, ENABLE);

	// stop bit flag
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_STOPF));
	{
		if((I2C1_Timeout--) == 0)
		{
			I2C_AcknowledgeConfig(SI570_I2C, ENABLE);
			return 9;
		}
	}

	// Re-enable ACK bit incase it was disabled last cal
	I2C_AcknowledgeConfig(SI570_I2C, ENABLE);

	printf(" %d ",*RegisterValue);

	printf("read done\n\r");
	return 0;
}*/

//*----------------------------------------------------------------------------
//* Function Name       : trx4m_hw_i2c_ReadRegister
//* Object              : ver2 - under test
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar mchf_hw_i2c_ReadRegister(uchar I2CAddr,uchar RegisterAddr, uchar *RegisterValue)
{
	//printf("i2c read\n\r");

	//  While the bus is busy
	I2C1_Timeout = I2C1_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BUSY))
	{
		if((I2C1_Timeout--) == 0)
			return 1;
	}

	// Start the config sequence
	I2C_GenerateSTART(SI570_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C1_Timeout--) == 0)
			return 2;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(SI570_I2C, I2CAddr, I2C_Direction_Transmitter);

	// Test on EV6 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((I2C1_Timeout--) == 0)
			return 3;
	}

	// Transmit the first address for write operation
	I2C_SendData(SI570_I2C, RegisterAddr);

	// Test on EV8 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BTF) == RESET)
	{
		if((I2C1_Timeout--) == 0)
			return 4;
	}

	// Start the config sequence
	I2C_GenerateSTART(SI570_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C1_Timeout--) == 0)
			return 6;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(SI570_I2C, I2CAddr, I2C_Direction_Receiver);

	// Test on EV6 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_ADDR) == RESET)
	{
		if((I2C1_Timeout--) == 0)
			return 7;
	}

	// Dissable acknoledgment
	I2C_AcknowledgeConfig(SI570_I2C, DISABLE);

	// Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read)
	(void)SI570_I2C->SR2;

	// < Send STOP Condition
	I2C_GenerateSTOP(SI570_I2C, ENABLE);

	// Test on EV8 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_RXNE) == RESET)
	{
		if((I2C1_Timeout--) == 0)
			return 8;
	}

	// Read data
	*RegisterValue = I2C_ReceiveData(SI570_I2C);

	// stop bit flag
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while(SI570_I2C->CR1 & I2C_CR1_STOP)
	{
		if((I2C1_Timeout--) == 0)
		{
			I2C_AcknowledgeConfig(SI570_I2C, ENABLE);
			return 9;
		}
	}

	// Re-enable ACK bit incase it was disabled last cal
	I2C_AcknowledgeConfig(SI570_I2C, ENABLE);

	// Clear AF flag for next communication
	I2C_ClearFlag(SI570_I2C, I2C_FLAG_AF);

	//printf(" %d ",*RegisterValue);

	//printf("read done\n\r");
	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : trx4m_hw_i2c_ReadData
//* Object              : read more than one byte
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar mchf_hw_i2c_ReadData(uchar I2CAddr,uchar RegisterAddr, uchar *data, ulong size)
{
	//printf("i2c read block\n\r");

	//  While the bus is busy
	I2C1_Timeout = I2C1_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BUSY))
	{
		if((I2C1_Timeout--) == 0)
			return 1;
	}

	// Start the config sequence
	I2C_GenerateSTART(SI570_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C1_Timeout--) == 0)
			return 2;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(SI570_I2C, I2CAddr, I2C_Direction_Transmitter);

	// Test on EV6 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((I2C1_Timeout--) == 0)
			return 3;
	}

	// Transmit the first address for write operation
	I2C_SendData(SI570_I2C, RegisterAddr);

	// Test on EV8 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_BTF) == RESET)
	{
		if((I2C1_Timeout--) == 0)
			return 4;
	}

	// Start the config sequence
	I2C_GenerateSTART(SI570_I2C, ENABLE);

	// Test on EV5 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while (!I2C_CheckEvent(SI570_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((I2C1_Timeout--) == 0)
			return 6;
	}

	// Transmit the slave address and enable writing operation
	I2C_Send7bitAddress(SI570_I2C, I2CAddr, I2C_Direction_Receiver);

	// Test on EV6 and clear it
	I2C1_Timeout = I2C1_FLAG_TIMEOUT;
	while(I2C_GetFlagStatus(SI570_I2C, I2C_FLAG_ADDR) == RESET)
	{
		if((I2C1_Timeout--) == 0)
			return 7;
	}

	// Handle different block sizes
	switch(size)
	{
		// Read one byte
		case 1:
		{
			// Before Clearing Addr bit by reading SR2, we have to cancel ack.
			SI570_I2C->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);

			//Now Read the SR2 to clear ADDR
			(void)SI570_I2C->SR2;

			// Order a STOP condition
			// Note: Spec_p583 says this should be done just after clearing ADDR
			// If it is done before ADDR is set, a STOP is generated immediately
			// as the clock is being streched
			SI570_I2C->CR1 |= I2C_CR1_STOP;

			// Be carefull that till the stop condition is actually transmitted
			// the clock will stay active even if a NACK is generated after the next received byte.

			// Read data
			*data = I2C_ReceiveData(SI570_I2C);

			// Make Sure Stop bit is cleared and Line is now Idle
			WaitLineIdle();

			// Enable the Acknowledgement again
			SI570_I2C->CR1 |= ((uint16_t)I2C_CR1_ACK);

			break;
		}

		// Read two bytes
		case 2:
		{
			// Before Clearing Addr, reset ACK, set POS
			SI570_I2C->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
			SI570_I2C->CR1 |= I2C_CR1_POS;

			// Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read)
			(void)SI570_I2C->SR2;

			// Wait for the next 2 bytes to be received (1st in the DR, 2nd in the shift register)
			WaitSR1FlagsSet(I2C_SR1_BTF);

			// Order a stop condition (as the clock is being strecthed, the stop condition is generated immediately)
			SI570_I2C->CR1 |= I2C_CR1_STOP;

			// Read data
			while(size)
			{
				*data++ = I2C_ReceiveData(SI570_I2C);
				size--;
			}

			// Make Sure Stop bit is cleared and Line is now Iddle
			WaitLineIdle();

			//Enable the ack and reset Pos
			SI570_I2C->CR1 |= ((uint16_t)I2C_CR1_ACK);
			SI570_I2C->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_POS);

			break;
		}

		// Read more
		default:
		{
			// -----------------------------------------------------------------------------------
			// Suppose to be correct version - not working on SI570
			// Read the SR2 to clear ADDR
			/*(void)SI570_I2C->SR2;

			while((size--) > 3)
			{
				// Read till the last 3 bytes
				*data++ = I2C_ReceiveData(SI570_I2C);
			}

			// 3 more bytes to read. Wait till the next to is actually received
			WaitSR1FlagsSet(I2C_SR1_BTF);
			// Here the clock is strecthed. One more to read.

			// Reset Ack
			SI570_I2C->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);

			// Read N-2
			*data++ = I2C_ReceiveData(SI570_I2C);
			// Once we read this, N is going to be read to the shift register and NACK is generated

			// Wait for the BTF
			WaitSR1FlagsSet(I2C_SR1_BTF); //N-1 is in DR, N is in shift register
			// Here the clock is stretched

			// Generate a stop condition
			SI570_I2C->CR1 |= I2C_CR1_STOP;

			// Read the last two bytes (N-1 and N)
			// Read the next two bytes
			*data++ = I2C_ReceiveData(SI570_I2C);
			*data   = I2C_ReceiveData(SI570_I2C);

			// Make Sure Stop bit is cleared and Line is now Iddle
			WaitLineIdle();

			// Enable the ack
			SI570_I2C->CR1 |= ((uint16_t)I2C_CR1_ACK);*/
			// -----------------------------------------------------------------------------------


			// NOT working - to fix on si570....

			// Read the SR2 to clear ADDR
			(void)SI570_I2C->SR2;

			while((size--) > 3)
			{
				// Read till the last 3 bytes
				*data++ = I2C_ReceiveData(SI570_I2C);

				WaitSR1FlagsSet(I2C_SR1_BTF);
			}

			// 3 more bytes to read. Wait till the next to is actually received
			//WaitSR1FlagsSet(I2C_SR1_BTF);
			// Here the clock is strecthed. One more to read.

			// Reset Ack
			SI570_I2C->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);

			// Read N-2
			*data++ = I2C_ReceiveData(SI570_I2C);
			// Once we read this, N is going to be read to the shift register and NACK is generated

			// Wait for the BTF
			WaitSR1FlagsSet(I2C_SR1_BTF); //N-1 is in DR, N is in shift register
			// Here the clock is stretched

			// Generate a stop condition
			SI570_I2C->CR1 |= I2C_CR1_STOP;

			// Read the last two bytes (N-1 and N)
			// Read the next two bytes
			*data++ = I2C_ReceiveData(SI570_I2C);
			*data   = I2C_ReceiveData(SI570_I2C);

			// Make Sure Stop bit is cleared and Line is now Iddle
			WaitLineIdle();

			// Enable the ack
			SI570_I2C->CR1 |= ((uint16_t)I2C_CR1_ACK);

			break;
		}
	}

	//printf("read done\n\r");
	return 0;
}

