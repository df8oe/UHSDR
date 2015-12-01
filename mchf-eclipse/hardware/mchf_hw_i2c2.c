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
#define I2C2_CLK                  	RCC_APB1Periph_I2C2
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
	I2C_InitStructure.I2C_Mode 				= I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle 			= I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 			= 0x33;
	I2C_InitStructure.I2C_Ack 				= I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress 		= I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed 			= I2C2_SPEED;

	// Enable the I2C peripheral
	I2C_Cmd (CODEC_I2C, ENABLE);
	I2C_Init(CODEC_I2C, &I2C_InitStructure);
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
	I2C_SoftwareResetCmd(CODEC_I2C, ENABLE);
	non_os_delay();
	I2C_SoftwareResetCmd(CODEC_I2C, DISABLE);

	I2C_Cmd (CODEC_I2C, DISABLE);
	non_os_delay();
	I2C_Cmd (CODEC_I2C, ENABLE);

	// Init again
	mchf_hw_i2c2_init();
}
*/


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



// serial eeprom functions by DF8OE

uint8_t Write_24Cxx(uint32_t Addr, uint8_t Data, uint8_t Mem_Type)
{
uint8_t memwa = MEM_DEVICE_WRITE_ADDR;
uint32_t timeout = I2C2_LONG_TIMEOUT;
uint8_t upper_addr,lower_addr;

if(Mem_Type == 17 && Addr > 0xFFFF)
    {
    memwa = memwa + 8;			// 24LC1025
    Addr = Addr - 0x10000;
    }
if(Mem_Type == 18 && Addr > 0xFFFF)
    {
    memwa = memwa + 2;			// 24LC1026
    Addr = Addr - 0x10000;
    }
if(Mem_Type == 19)
    {
    if(Addr > 0xFFFF && Addr < 0x20000)
	{
	memwa = memwa + 4;			// 24CM02
	Addr = Addr - 0x10000;
	}
    if(Addr > 0x1FFFF && Addr < 0x30000)
	{
	memwa = memwa + 2;			// 24CM02
	Addr = Addr - 0x20000;
	}
    if(Addr > 0x2FFFF && Addr < 0x40000)
	{
	memwa = memwa + 3;			// 24CM02
	Addr = Addr - 0x30000;
	}
    }
lower_addr = (uint8_t)((0x00FF)&Addr);

if(Mem_Type > 8)
    {
    Addr = Addr>>8;
    upper_addr = (uint8_t)((0x00FF)&Addr);
    }
// Generate the Start Condition
I2C_GenerateSTART(CODEC_I2C, ENABLE);

// Test on I2C2 EV5, Start transmitted successfully and clear it
timeout = I2C2_LONG_TIMEOUT; // Initialize timeout value
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT))
    {
    // If the timeout delay is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;	
    }

// Send Memory device slave Address for write
I2C_Send7bitAddress(CODEC_I2C, memwa, I2C_Direction_Transmitter);

// Test on I2C2 EV6 and clear it
timeout = I2C2_LONG_TIMEOUT; // Initialize timeout value
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
    // If the timeout delay is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
    }

if(Mem_Type > 8)
    {
    // Send I2C2 location address LSB
    I2C_SendData(CODEC_I2C, upper_addr);
    // Test on I2C2 EV8 and clear it
    timeout = I2C2_LONG_TIMEOUT; // Initialize timeout value
    while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // was ..TTED
	{
	// If the timeout delay is exeeded, exit with error code
	if ((timeout--) == 0) return 0xFF;
	}
    }
// Send I2C2 location address LSB
I2C_SendData(CODEC_I2C, lower_addr);

// Test on I2C2 EV8 and clear it
timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // was ..TTED
    {
    /* If the timeout delay is exeeded, exit with error code */
    if ((timeout--) == 0) return 0xFF;
    }

// Send Data
I2C_SendData(CODEC_I2C, Data);

// Test on I2C2 EV8 and clear it 
timeout = I2C2_LONG_TIMEOUT; // Initialize timeout value
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // was ..TTED
    {
    // If the timeout delay is exeeded, exit with error code
    if ((timeout--) == 0) return 0xFF;
    }
// Send I2C2 STOP Condition
I2C_GenerateSTOP(CODEC_I2C, ENABLE);

timeout = I2C2_FLAG_TIMEOUT;
while(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_STOPF))
    {
	if((timeout--) == 0)
	    return 0xFF;
    }

Read_24Cxx(0, Mem_Type);
// If operation is OK, return 0
return 0;
}



uint16_t Read_24Cxx(uint32_t Addr, uint8_t Mem_Type)
{
uint8_t memwa = MEM_DEVICE_WRITE_ADDR;
uint8_t memra = MEM_DEVICE_READ_ADDR;
uint32_t timeout = I2C2_LONG_TIMEOUT;
uint8_t Data = 0;
uint8_t upper_addr,lower_addr;

if(Mem_Type == 17 && Addr > 0xFFFF)
    {
    memra = memra + 8;			// 24LC1025
    memwa = memwa + 8;			// 24LC1025
    Addr = Addr - 0x10000;
    }
if(Mem_Type == 18 && Addr > 0xFFFF)
    {
    memra = memra + 2;			// 24LC1026
    memwa = memwa + 2;			// 24LC1026
    Addr = Addr - 0x10000;
    }
if(Mem_Type == 19)
    {
    if(Addr > 0xFFFF && Addr < 0x20000)
	{
	memwa = memwa + 4;			// 24CM02
	memra = memra + 4;			// 24CM02
	Addr = Addr - 0x10000;
	}
    if(Addr > 0x1FFFF && Addr < 0x30000)
	{
	memwa = memwa + 2;			// 24CM02
	memra = memra + 2;			// 24CM02
	Addr = Addr - 0x20000;
	}
    if(Addr > 0x2FFFF && Addr < 0x40000)
	{
	memwa = memwa + 3;			// 24CM02
	memra = memra + 3;			// 24CM02
	Addr = Addr - 0x30000;
	}
    }

lower_addr = (uint8_t)((0x00FF)&Addr);
if(Mem_Type > 8)
    {
    Addr = Addr>>8;
    upper_addr = (uint8_t)((0x00FF)&Addr);
    }
/* Generate the Start Condition */
I2C_GenerateSTART(CODEC_I2C, ENABLE);

/* Test on I2C2 EV5 and clear it */
timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT))
    {
    /* If the timeout delay is exeeded, exit with error code */
    if ((timeout--) == 0) return 0xF800;
    }

/* Send DCMI selcted device slave Address for read */
I2C_Send7bitAddress(CODEC_I2C, memwa, I2C_Direction_Transmitter);

/* Test on I2C2 EV6 and clear it */
timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
    /* If the timeout delay is exeeded, exit with error code */
    if ((timeout--) == 0) 
	{
	/* Prepare Stop after receiving data */
	I2C_GenerateSTOP(CODEC_I2C, ENABLE);
	return 0xFE00;
	}
    }

if(Mem_Type > 8)
    {
    /* Send I2C2 location address LSB */
    I2C_SendData(CODEC_I2C,upper_addr);

    /* Test on I2C2 EV8 and clear it */
    timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
	/* If the timeout delay is exeeded, exit with error code */
	if ((timeout--) == 0) return 0xFD00;
	}
    }

 /* Send I2C2 location address LSB */
I2C_SendData(CODEC_I2C, lower_addr);

/* Test on I2C2 EV8 and clear it */
timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
    /* If the timeout delay is exeeded, exit with error code */
    if ((timeout--) == 0) return 0xFC00;
    }

/* Generate the Start Condition */
I2C_GenerateSTART(CODEC_I2C, ENABLE);

/* Test on I2C2 EV6 and clear it */
timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT))
     {
    /* If the timeout delay is exeeded, exit with error code */
    if ((timeout--) == 0) return 0xFB00;
    }

/* Send DCMI selcted device slave Address for read */
I2C_Send7bitAddress(CODEC_I2C, memra, I2C_Direction_Receiver);

/* Test on I2C2 EV6 and clear it */
timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
    /* If the timeout delay is exeeded, exit with error code */
    if ((timeout--) == 0) return 0xFA00;
    }

/* Prepare an NACK for the next data received */
I2C_AcknowledgeConfig(CODEC_I2C, DISABLE);

/* Test on I2C2 EV7 and clear it */
timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
    {
    /* If the timeout delay is exeeded, exit with error code */
    if ((timeout--) == 0) return 0xF900;
    }


/* Prepare Stop after receiving data */
 I2C_GenerateSTOP(CODEC_I2C, ENABLE);

/* Receive the Data */
Data = I2C_ReceiveData(CODEC_I2C);

/* return the read data */
return Data;
}


uint8_t Read_24Cxxseq(uint32_t start, uint8_t *buffer, uint16_t length, uint8_t Mem_Type)
{
}

uint8_t Write_24Cxxseq(uint32_t address, uint8_t *buffer, uint16_t length, uint8_t Mem_Type)
{
uint8_t memwa = MEM_DEVICE_WRITE_ADDR;
uint32_t timeout = I2C2_LONG_TIMEOUT;
uint8_t upper_addr,lower_addr;
uint16_t start, i, page, count;
count = 0;

if(Mem_Type == 12)
    page = 64;
if(Mem_Type == 13)
    page = 32;
if(Mem_Type == 14 || Mem_Type == 15)
    page = 64;
if(Mem_Type > 15 && Mem_Type < 19)
    page = 128;
if(Mem_Type == 19)
    page = 256;


if(Mem_Type == 17 && address > 0xFFFF)
    {
    memwa = memwa + 8;			// 24LC1025
    address = address - 0x10000;
    }
if(Mem_Type == 18 && address > 0xFFFF)
    {
    memwa = memwa + 2;			// 24LC1026
    address = address - 0x10000;
    }
if(Mem_Type == 19)
    {
    if(address > 0xFFFF && address < 0x20000)
	{
	memwa = memwa + 4;			// 24CM02
	address = address - 0x10000;
	}
    if(address > 0x1FFFF && address < 0x30000)
	{
	memwa = memwa + 2;			// 24CM02
	address = address - 0x20000;
	}
    if(address > 0x2FFFF && address < 0x40000)
	{
	memwa = memwa + 3;			// 24CM02
	address = address - 0x30000;
	}
    }

while(count <= length)
    {
    start = (uint16_t)address + count;
    lower_addr = (uint8_t)((0x00FF)&start);

    if(Mem_Type > 8)
	{
	start = start>>8;
	upper_addr = (uint8_t)((0x00FF)&start);
	}

    // Generate the Start Condition
    I2C_GenerateSTART(CODEC_I2C, ENABLE);

    // Test on I2C2 EV5, Start transmitted successfully and clear it
    timeout = I2C2_LONG_TIMEOUT; // Initialize timeout value
    while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
	// If the timeout delay is exeeded, exit with error code
	if ((timeout--) == 0) return 0xFF;	
	}

    // Send Memory device slave Address for write
    I2C_Send7bitAddress(CODEC_I2C, memwa, I2C_Direction_Transmitter);

    // Test on I2C2 EV6 and clear it
    timeout = I2C2_LONG_TIMEOUT; // Initialize timeout value
    while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
	// If the timeout delay is exeeded, exit with error code
	if ((timeout--) == 0) return 0xFF;
	}

    if(Mem_Type > 8)
	{
	// Send I2C2 location address LSB
	I2C_SendData(CODEC_I2C, upper_addr);
	// Test on I2C2 EV8 and clear it
	timeout = I2C2_LONG_TIMEOUT; // Initialize timeout value
	while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // was ..TTED
	    {
	    // If the timeout delay is exeeded, exit with error code
	    if ((timeout--) == 0) return 0xFF;
	    }
	}
    // Send I2C2 location address LSB
    I2C_SendData(CODEC_I2C, lower_addr);

    // Test on I2C2 EV8 and clear it
    timeout = I2C2_LONG_TIMEOUT; /* Initialize timeout value */
    while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // was ..TTED
	{
	/* If the timeout delay is exeeded, exit with error code */
	if ((timeout--) == 0) return 0xFF;
	}

	for(i=0; i<page;i++)
	    {
	    // Send Data
	    I2C_SendData(CODEC_I2C, buffer[count]);
	    count++;
	    if(count > length)
		i = 130;
	    // Test on I2C2 EV8 and clear it 
	    timeout = I2C2_LONG_TIMEOUT; // Initialize timeout value
	    while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // was ..TTED
		{
		// If the timeout delay is exeeded, exit with error code
		if ((timeout--) == 0) return 0xFF;
		}
	    }
    // Wait till all data have been physically transferred on the bus
    I2C2_Timeout = I2C2_LONG_TIMEOUT;
    while(!I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BTF))
	{
		if((I2C2_Timeout--) == 0)
			return 0xFF;
	}
    I2C_GenerateSTOP(CODEC_I2C, ENABLE);
    Read_24Cxx(0, Mem_Type);
    }
// If operation is OK, return 0
return 0;
}
