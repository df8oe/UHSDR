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
**  Licence:      CC BY-NC-SA 3.0                                                  **
************************************************************************************/

// Common
#include "mchf_board.h"

#include <stdio.h>
#include <stdlib.h>

#include "ui_lcd_hy28_fonts.h"
#include "ui_lcd_hy28.h"


// Saved fonts
extern sFONT GL_Font8x8;
extern sFONT GL_Font8x12;
extern sFONT GL_Font8x12_bold;
extern sFONT GL_Font12x12;
extern sFONT GL_Font16x24;

static sFONT *fontList[] = {
    &GL_Font8x12_bold,
    &GL_Font16x24,
    &GL_Font12x12,
    &GL_Font8x12,
    &GL_Font8x8,
};


// Transceiver state public structure
extern __IO TransceiverState ts;


int16_t lcd_cs;
GPIO_TypeDef* lcd_cs_pio;

uint16_t display_use_spi;

const uint8_t touchscreentable [] = {0x0c,0x0d,0x0e,0x0f,0x12,0x13,0x14,0x15,0x16,0x18,0x1c,0x1d,0x1e,0x1f,0x22,
0x23,0x24,0x25,0x26,0x27,0x2c,0x2d,0x2e,0x30,0x32,0x34,0x35,0x36,0x3a,0x3c,0x40,0x42,0x44,0x45,0x46,0x47,0x4c,
0x4d,0x4e,0x52,0x54,0x55,0x56,0x5c,0x5d,0x60,0x62,0x64,0x65,0x66,0x67,0x6c,0x6d,0x6e,0x74,0x75,0x76,0x77,0x7c,0x7d,0x80};


static void UiLcdHy28_Delay(ulong delay);
static void UiLcdHy28_Test(void);

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_BacklightInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_BacklightInit(void)
{
   GPIO_InitTypeDef  GPIO_InitStructure;

   // Set as output
   GPIO_InitStructure.GPIO_Pin = LCD_BACKLIGHT;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(LCD_BACKLIGHT_PIO, &GPIO_InitStructure);

   // Backlight off
   LCD_BACKLIGHT_PIO->BSRRH = LCD_BACKLIGHT;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_SpiInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_SpiInit(bool hispeed)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   SPI_InitTypeDef  SPI_InitStructure;

   // Enable the SPI periph
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

   // Common SPI settings
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

   // SPI SCK pin configuration
   GPIO_InitStructure.GPIO_Pin = LCD_SCK;
   GPIO_Init(LCD_SCK_PIO, &GPIO_InitStructure);

   // SPI  MOSI pins configuration
   GPIO_InitStructure.GPIO_Pin =  LCD_MOSI;
   GPIO_Init(LCD_MOSI_PIO, &GPIO_InitStructure);

   // SPI  MISO pins configuration
   GPIO_InitStructure.GPIO_Pin =  LCD_MISO;
   GPIO_Init(LCD_MISO_PIO, &GPIO_InitStructure);

   // Set as alternative
   GPIO_PinAFConfig(LCD_SCK_PIO,  LCD_SCK_SOURCE,  GPIO_AF_SPI2);
   GPIO_PinAFConfig(LCD_MISO_PIO, LCD_MISO_SOURCE, GPIO_AF_SPI2);
   GPIO_PinAFConfig(LCD_MOSI_PIO, LCD_MOSI_SOURCE, GPIO_AF_SPI2);

   // SPI configuration
   SPI_I2S_DeInit(SPI2);
   SPI_InitStructure.SPI_Direction		= SPI_Direction_2Lines_FullDuplex;
   SPI_InitStructure.SPI_DataSize		= SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL			= SPI_CPOL_High;
   SPI_InitStructure.SPI_CPHA			= SPI_CPHA_2Edge;
   SPI_InitStructure.SPI_NSS			= SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler	= hispeed?SPI_BaudRatePrescaler_2:SPI_BaudRatePrescaler_4;   // max speed presc_8 with 50Mhz GPIO, max 4 with 100 Mhz
   SPI_InitStructure.SPI_FirstBit		= SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_Mode			= SPI_Mode_Master;
   SPI_Init(SPI2, &SPI_InitStructure);

   // Enable SPI2
   SPI_Cmd(SPI2, ENABLE);

   // Common misc pins settings
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   // Configure GPIO PIN for Chip select
   GPIO_InitStructure.GPIO_Pin = lcd_cs;
   GPIO_Init(lcd_cs_pio, &GPIO_InitStructure);

   // Configure GPIO PIN for Reset
   GPIO_InitStructure.GPIO_Pin = LCD_RESET;
   GPIO_Init(LCD_RESET_PIO, &GPIO_InitStructure);

   // Deselect : Chip Select high
   GPIO_SetBits(lcd_cs_pio, lcd_cs);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_SpiInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_SpiDeInit()
{
   GPIO_InitTypeDef GPIO_InitStructure;

   SPI_Cmd(SPI2, DISABLE);
   SPI_I2S_DeInit(SPI2);

   // Set as inputs
   GPIO_InitStructure.GPIO_Mode		= GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_PuPd		= GPIO_PuPd_NOPULL;

   // SPI SCK pin configuration
   GPIO_InitStructure.GPIO_Pin		= LCD_SCK;
   GPIO_Init(LCD_SCK_PIO, &GPIO_InitStructure);

   // SPI  MOSI pins configuration
   GPIO_InitStructure.GPIO_Pin		=  LCD_MOSI;
   GPIO_Init(LCD_MOSI_PIO, &GPIO_InitStructure);

   // SPI  MISO pins configuration
   GPIO_InitStructure.GPIO_Pin		=  LCD_MISO;
   GPIO_Init(LCD_MISO_PIO, &GPIO_InitStructure);

   // Configure GPIO PIN for Chip select
   GPIO_InitStructure.GPIO_Pin		= lcd_cs;
   GPIO_Init(lcd_cs_pio, &GPIO_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_ParallelInit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_ParallelInit()
{
   GPIO_InitTypeDef GPIO_InitStructure;

   // Port D usage - data and control
   // SRAM Data lines,  NOE, NE1, A16 and NWE configuration
   GPIO_InitStructure.GPIO_Pin =    LCD_D2 |LCD_D3 |
                           LCD_RD |LCD_WR |
                           LCD_CSA|LCD_D15|
                           LCD_D16|LCD_D17|
                           LCD_RS |LCD_D0 |
                           LCD_D1;

   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOD, &GPIO_InitStructure);

   GPIO_PinAFConfig(LCD_D2_PIO,  LCD_D2_SOURCE,  GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D3_PIO,  LCD_D3_SOURCE,  GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_RD_PIO,  LCD_RD_SOURCE,  GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_WR_PIO,  LCD_WR_SOURCE,  GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_CSA_PIO, LCD_CSA_SOURCE, GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D15_PIO, LCD_D15_SOURCE, GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D16_PIO, LCD_D16_SOURCE, GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D17_PIO, LCD_D17_SOURCE, GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_RS_PIO,  LCD_RS_SOURCE,  GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D0_PIO,  LCD_D0_SOURCE,  GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D1_PIO,  LCD_D1_SOURCE,  GPIO_AF_FSMC);

   // Data port on port E
   GPIO_InitStructure.GPIO_Pin =    LCD_D4 |LCD_D5 |
                           LCD_D6 |LCD_D7 |
                           LCD_D10|LCD_D11|
                           LCD_D12|LCD_D13|
                           LCD_D14;

   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOE, &GPIO_InitStructure);

   GPIO_PinAFConfig(LCD_D4_PIO,  LCD_D4_SOURCE , GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D5_PIO,  LCD_D5_SOURCE , GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D6_PIO,  LCD_D6_SOURCE , GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D7_PIO,  LCD_D7_SOURCE , GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D10_PIO, LCD_D10_SOURCE, GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D11_PIO, LCD_D11_SOURCE, GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D12_PIO, LCD_D12_SOURCE, GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D13_PIO, LCD_D13_SOURCE, GPIO_AF_FSMC);
   GPIO_PinAFConfig(LCD_D14_PIO, LCD_D14_SOURCE, GPIO_AF_FSMC);

   // Configure GPIO PIN for Reset
   GPIO_InitStructure.GPIO_Pin		= LCD_RESET;
   GPIO_InitStructure.GPIO_Mode		= GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
   GPIO_Init(LCD_RESET_PIO, &GPIO_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_Reset
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_Reset()
{
   // Reset
   GPIO_SetBits(LCD_RESET_PIO, LCD_RESET);
   UiLcdHy28_Delay(50000);

   GPIO_ResetBits(LCD_RESET_PIO, LCD_RESET);
   UiLcdHy28_Delay(50000);

   GPIO_SetBits(LCD_RESET_PIO, LCD_RESET);
   UiLcdHy28_Delay(100000);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_FSMCConfig
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_FSMCConfig(void)
{
   FSMC_NORSRAMInitTypeDef        FSMC_NORSRAMInitStructure;
   FSMC_NORSRAMTimingInitTypeDef     p;

   // Enable FSMC clock
   RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

   //-- FSMC Configuration ------------------------------------------------------
   //----------------------- SRAM Bank 3 ----------------------------------------
   // FSMC_Bank1_NORSRAM4 configuration
   p.FSMC_AddressSetupTime       = 6;		// slow external RAM interface to LCD to reduce corruption on slower LCDs
   p.FSMC_AddressHoldTime        = 0;
   p.FSMC_DataSetupTime          = 15;		// slow external RAM interface to LCD to reduce corruption on slower LCDs
   p.FSMC_BusTurnAroundDuration  = 0;
   p.FSMC_CLKDivision            = 0;
   p.FSMC_DataLatency            = 0;
   p.FSMC_AccessMode             = FSMC_AccessMode_A;

   // Color LCD configuration ------------------------------------
   //   LCD configured as follow:
   //     - Data/Address MUX = Disable
   //     - Memory Type = SRAM
   //     - Data Width = 16bit
   //     - Write Operation = Enable
   //     - Extended Mode = Enable
   //     - Asynchronous Wait = Disable

   FSMC_NORSRAMInitStructure.FSMC_Bank			= FSMC_Bank1_NORSRAM1;
   FSMC_NORSRAMInitStructure.FSMC_DataAddressMux	= FSMC_DataAddressMux_Disable;
   FSMC_NORSRAMInitStructure.FSMC_MemoryType		= FSMC_MemoryType_SRAM;
   FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth	= FSMC_MemoryDataWidth_16b;
   FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode	= FSMC_BurstAccessMode_Disable;
   FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait	= FSMC_AsynchronousWait_Disable;
   FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity	= FSMC_WaitSignalPolarity_Low;
   FSMC_NORSRAMInitStructure.FSMC_WrapMode		= FSMC_WrapMode_Disable;
   FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive	= FSMC_WaitSignalActive_BeforeWaitState;
   FSMC_NORSRAMInitStructure.FSMC_WriteOperation	= FSMC_WriteOperation_Enable;
   FSMC_NORSRAMInitStructure.FSMC_WaitSignal		= FSMC_WaitSignal_Disable;
   FSMC_NORSRAMInitStructure.FSMC_ExtendedMode		= FSMC_ExtendedMode_Disable;
   FSMC_NORSRAMInitStructure.FSMC_WriteBurst		= FSMC_WriteBurst_Disable;
   FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct	= &p;
   FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct	= &p;
   FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

   FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_SendByteSpi
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static inline void UiLcdHy28_SendByteSpi(uint8_t byte)
{
   // while ((SPI2>SR & (SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
   // SPI2->DR = Data;
   while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)  == RESET);
   SPI_I2S_SendData(SPI2, byte);

   while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
   SPI_I2S_ReceiveData(SPI2);

}
static inline void UiLcdHy28_SendByteSpiFast(uint8_t byte)
{

   while ((SPI2->SR & (SPI_I2S_FLAG_TXE)) == (uint16_t)RESET);
   SPI2->DR = byte;
   while ((SPI2->SR & (SPI_I2S_FLAG_RXNE)) == (uint16_t)RESET);
   byte = SPI2->DR;
}
static void UiLcdHy28_FinishSpiTransfer()
{
	while (SPI2->SR & SPI_I2S_FLAG_BSY);
	GPIO_SetBits(lcd_cs_pio, lcd_cs);
}
//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_ReadByteSpi
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uint8_t UiLcdHy28_ReadByteSpi(void)
{
   ulong timeout;
   uchar byte = 0;

   /* Loop while DR register in not emplty */
   timeout = 0x1000;
   while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
   {
      if((timeout--) == 0)
         return 0xFF;
   }

   /* Send a Byte through the SPI peripheral */
   SPI_I2S_SendData(SPI2, byte);

   /* Wait to receive a Byte */
   timeout = 0x1000;
   while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
   {
      if((timeout--) == 0)
         return 0xFF;
   }

   /* Return the Byte read from the SPI bus */
   return (uint8_t)SPI_I2S_ReceiveData(SPI2);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_WriteIndexSpi
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_WriteIndexSpi(unsigned char index)
{
   GPIO_ResetBits(lcd_cs_pio, lcd_cs);

   UiLcdHy28_SendByteSpiFast(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0       */
   UiLcdHy28_SendByteSpiFast(0);
   UiLcdHy28_SendByteSpiFast(index);

   UiLcdHy28_FinishSpiTransfer();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_WriteDataSpi
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_WriteDataSpi( unsigned short data)
{
   GPIO_ResetBits(lcd_cs_pio, lcd_cs);

   UiLcdHy28_SendByteSpiFast(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
   UiLcdHy28_SendByteSpiFast((data >>   8));                    /* Write D8..D15                */
   UiLcdHy28_SendByteSpiFast((data & 0xFF));                    /* Write D0..D7                 */

   UiLcdHy28_FinishSpiTransfer();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_WriteDataSpiStart
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static inline void UiLcdHy28_WriteDataSpiStart()
{
   UiLcdHy28_SendByteSpiFast(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_WriteDataOnly
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static inline void UiLcdHy28_WriteDataOnly( unsigned short data)
{
//    if(!GPIO_ReadInputDataBit(TP_IRQ_PIO,TP_IRQ))
//	UiLcdHy28_GetTouchscreenCoordinates(1);		// check touchscreen coordinates

   if(display_use_spi)
   {
      UiLcdHy28_SendByteSpiFast((data >>   8));      /* Write D8..D15                */
      UiLcdHy28_SendByteSpiFast((data & 0xFF));      /* Write D0..D7                 */
   }
   else
      LCD_RAM = data;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_ReadDataSpi
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
unsigned short UiLcdHy28_ReadDataSpi(void)
{
   unsigned short value = 0;
   uchar y,z;

   GPIO_ResetBits(lcd_cs_pio, lcd_cs);

   UiLcdHy28_SendByteSpi(SPI_START | SPI_RD | SPI_DATA);    /* Read: RS = 1, RW = 1         */

   UiLcdHy28_ReadByteSpi();                                /* Dummy read 1                 */

   y   = UiLcdHy28_ReadByteSpi();                      /* Read D8..D15                 */
   value = y;
   value <<= 8;
   z = UiLcdHy28_ReadByteSpi();                      /* Read D0..D7                  */

   value  |= z;

   UiLcdHy28_FinishSpiTransfer();

   return value;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_WriteReg
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_WriteReg( unsigned short LCD_Reg, unsigned short LCD_RegValue)
{
   if(display_use_spi)
    {
     UiLcdHy28_WriteIndexSpi(LCD_Reg);
     UiLcdHy28_WriteDataSpi(LCD_RegValue);
    }
   else
   {
      LCD_REG = LCD_Reg;
      LCD_RAM = LCD_RegValue;
   }
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_ReadReg
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
unsigned short UiLcdHy28_ReadReg( unsigned short LCD_Reg)
{
   if(display_use_spi)
   {
      // Write 16-bit Index (then Read Reg)
      UiLcdHy28_WriteIndexSpi(LCD_Reg);

      // Read 16-bit Reg
      return UiLcdHy28_ReadDataSpi();
   }

   // Write 16-bit Index (then Read Reg)
   LCD_REG = LCD_Reg;

   // Read 16-bit Reg
   return (LCD_RAM);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_SetCursorA
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiLcdHy28_SetCursorA( unsigned short Xpos, unsigned short Ypos )
{
   UiLcdHy28_WriteReg(0x0020, Ypos );
   UiLcdHy28_WriteReg(0x0021, Xpos );
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_WriteRAM_Prepare
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiLcdHy28_WriteRAM_Prepare(void)
{
   if(display_use_spi)
   {
      UiLcdHy28_WriteIndexSpi(0x0022);
      GPIO_ResetBits(lcd_cs_pio, lcd_cs);
      UiLcdHy28_WriteDataSpiStart();
   }
   else
      LCD_REG = 0x22;
}

static inline void UiLcdHy28_WriteRAM_Finish(void)
{
   if(display_use_spi)
   {
      GPIO_SetBits(lcd_cs_pio, lcd_cs);
   }
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_LcdClear
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_LcdClear(ushort Color)
{
   UiLcdHy28_OpenBulkWrite(0,MAX_X,0,MAX_Y);
   UiLcdHy28_BulkWriteColor(Color,MAX_X * MAX_Y);
   UiLcdHy28_CloseBulkWrite();
}

#define PIXELBUFFERSIZE 256
static uint16_t   pixelbuffer[PIXELBUFFERSIZE];
static uint16_t pixelcount = 0;

static inline void UiLcdHy28_BulkPixel_BufferInit() {
	pixelcount = 0;
}

static inline void UiLcdHy28_BulkPixel_BufferFlush() {
	UiLcdHy28_BulkWrite(pixelbuffer,pixelcount);
	pixelcount = 0;
}

inline void UiLcdHy28_BulkPixel_Put(uint16_t pixel) {
	pixelbuffer[pixelcount++] = pixel;
		if (pixelcount == PIXELBUFFERSIZE) {
			UiLcdHy28_BulkPixel_BufferFlush();
		}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_SetPoint
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_SetPoint( unsigned short Xpos, unsigned short Ypos, unsigned short point)
{
   if( Xpos >= MAX_X || Ypos >= MAX_Y )
      return;

   UiLcdHy28_SetCursorA(Xpos,Ypos);
   UiLcdHy28_WriteReg(0x0022,point);
}


//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawColorPoint
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_DrawColorPoint(ushort x, ushort y, ushort color)
{

   UiLcdHy28_SetCursorA(x, y);

   UiLcdHy28_WriteRAM_Prepare();

   UiLcdHy28_WriteDataOnly(color);

   UiLcdHy28_WriteRAM_Finish();
}
//
//

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawStraightLine
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_DrawStraightLine(ushort x, ushort y, ushort Length, uchar Direction,ushort color)
{
   if(Direction == LCD_DIR_VERTICAL)
   {
	  // here we use the "manual" approach instead of using bulk write
	  // since  we need less setup bytes and vertical lines are mostly short
	  // and vertical mode is default, we save some time in most cases.
	  UiLcdHy28_SetCursorA(x, y);
      UiLcdHy28_WriteRAM_Prepare();
      UiLcdHy28_BulkWriteColor(color,Length);
      UiLcdHy28_WriteRAM_Finish();   }
   else
   {
	  UiLcdHy28_OpenBulkWrite(x,Length,y,1);
      UiLcdHy28_BulkWriteColor(color,Length);
      UiLcdHy28_CloseBulkWrite();
   }
}

// FIXME: DrawRectangle should be used
void UiLcdHy28_DrawStraightLineDouble(ushort x, ushort y, ushort Length, uchar Direction,ushort color) {
	UiLcdHy28_DrawStraightLine(x, y, Length, Direction,color);
	if(Direction == LCD_DIR_VERTICAL) {
		UiLcdHy28_DrawStraightLine(x+1, y, Length, Direction,color);
	} else {
		UiLcdHy28_DrawStraightLine(x, y+1, Length, Direction,color);
	}
}

// FIXME: DrawRectangle should be used
void UiLcdHy28_DrawStraightLineTriple(ushort x, ushort y, ushort Length, uchar Direction,ushort color) {
	UiLcdHy28_DrawStraightLine(x, y, Length, Direction,color);
	if(Direction == LCD_DIR_VERTICAL) {
		UiLcdHy28_DrawStraightLine(x+1, y, Length, Direction,color);
		UiLcdHy28_DrawStraightLine(x+2, y, Length, Direction,color);
	} else {
		UiLcdHy28_DrawStraightLineDouble(x, y+1, Length, Direction,color);
		UiLcdHy28_DrawStraightLine(x, y+2, Length, Direction,color);
	}
}
//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawHorizLineWithGrad
//* Object              : draq horizontal line with gradient, most bright in
//* Object              : the middle
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_DrawHorizLineWithGrad(ushort x, ushort y, ushort Length,ushort gradient_start)
{
   uint32_t i = 0,j = 0;
   ushort     k = gradient_start;

   UiLcdHy28_OpenBulkWrite(x,Length,y,1);
   UiLcdHy28_BulkPixel_BufferInit();
   for(i = 0; i < Length; i++)
   {
      UiLcdHy28_BulkPixel_Put(RGB(k,k,k));
       j++;
      if(j == GRADIENT_STEP)
      {
         if(i < (Length/2))
            k += (GRADIENT_STEP/2);
         else
            k -= (GRADIENT_STEP/2);

         j = 0;
      }
   }
   UiLcdHy28_BulkPixel_BufferFlush();
   UiLcdHy28_CloseBulkWrite();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawEmptyRect
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_DrawEmptyRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color)
{

		UiLcdHy28_DrawStraightLine(Xpos, (Ypos),          Width,        LCD_DIR_HORIZONTAL,color);
		UiLcdHy28_DrawStraightLine(Xpos, Ypos,            Height,       LCD_DIR_VERTICAL,color);
		UiLcdHy28_DrawStraightLine((Xpos + Width), Ypos,  (Height + 1), LCD_DIR_VERTICAL,color);
		UiLcdHy28_DrawStraightLine(Xpos, (Ypos + Height), Width,        LCD_DIR_HORIZONTAL,color);

}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawBottomButton
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_DrawBottomButton(ushort Xpos, ushort Ypos, ushort Height, ushort Width,ushort color)
{
   UiLcdHy28_DrawStraightLine(Xpos, (Ypos),        Width, LCD_DIR_HORIZONTAL,color);
   UiLcdHy28_DrawStraightLine(Xpos, Ypos,          Height,LCD_DIR_VERTICAL,  color);
   UiLcdHy28_DrawStraightLine((Xpos + Width), Ypos,Height,LCD_DIR_VERTICAL,  color);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawFullRect
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_DrawFullRect(ushort Xpos, ushort Ypos, ushort Height, ushort Width ,ushort color)
{
		UiLcdHy28_OpenBulkWrite(Xpos, Width, Ypos, Height);
		UiLcdHy28_BulkWriteColor(color,(uint32_t)Height * (uint32_t)Width);
		UiLcdHy28_CloseBulkWrite();
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_OpenBulk
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_OpenBulkWrite(ushort x, ushort width, ushort y, ushort height)
{


   UiLcdHy28_SetCursorA(x, y);	// set starting position of character

	   UiLcdHy28_WriteReg(0x03,  0x1038);    // set GRAM write direction and BGR=1 and turn on cursor auto-increment
	   // Establish CGRAM window for THIS character
	   UiLcdHy28_WriteReg(0x50, y);    // Vertical GRAM Start Address
	   UiLcdHy28_WriteReg(0x51, y + height -1);    // Vertical GRAM End Address	-1
	   UiLcdHy28_WriteReg(0x52, x);    // Horizontal GRAM Start Address
	   UiLcdHy28_WriteReg(0x53, x + width -1);    // Horizontal GRAM End Address  -1
	   //
	   UiLcdHy28_WriteRAM_Prepare();					// prepare for bulk-write to character window


}


//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_BulkWrite
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_BulkWrite(uint16_t* pixel, uint32_t len)
{
		uint32_t i = len;
		for (; i; i--) {
			UiLcdHy28_WriteDataOnly(*(pixel++));
		}
}

void UiLcdHy28_BulkWriteColor(uint16_t Color, uint32_t len)
{
		uint32_t i = len;
		for (; i; i--) {
			UiLcdHy28_WriteDataOnly(Color);
		}
}



//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_CloseBulk
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_CloseBulkWrite(void)
{

   if(display_use_spi)	{		// SPI enabled?

	   GPIO_SetBits(lcd_cs_pio, lcd_cs);	// bulk-write complete!

   }
   //
   UiLcdHy28_WriteReg(0x50, 0x0000);    // Horizontal GRAM Start Address
   UiLcdHy28_WriteReg(0x51, 0x00EF);    // Horizontal GRAM End Address
   UiLcdHy28_WriteReg(0x52, 0x0000);    // Vertical GRAM Start Address
   UiLcdHy28_WriteReg(0x53, 0x013F);    // Vertical GRAM End Address
   //
   UiLcdHy28_WriteReg(0x03,  0x1030);    // set GRAM write direction and BGR=1 and switch increment mode

}



//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawChar
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------



void UiLcdHy28_DrawChar(ushort x, ushort y, char symb,ushort Color, ushort bkColor,const sFONT *cf)
{
	ulong       i,j;
	ushort      a,b;
	const uchar      fw = cf->Width;
	const short   *ch = (const short *)(&cf->table[(symb - 32) * cf->Height]);

	UiLcdHy28_OpenBulkWrite(x,cf->Width,y,cf->Height);
	UiLcdHy28_BulkPixel_BufferInit();
	for(i = 0; i < cf->Height; i++)	{
		// Draw line
		for(j = 0; j < cf->Width; j++)	{
			a = ((ch[i] & ((0x80 << ((fw / 12 ) * 8)) >> j)));
			b = ((ch[i] &  (0x01 << j)));
			//
			UiLcdHy28_BulkPixel_Put(((!a && (fw <= 12)) || (!b && (fw > 12)))?bkColor:Color);
		}
	}
	UiLcdHy28_BulkPixel_BufferFlush();
	// flush all not yet  transferred pixel to display.
	UiLcdHy28_CloseBulkWrite();
}

const sFONT   *UiLcdHy28_Font(uint8_t font) {
  const sFONT   *cf;

  if (font >4) { cf = fontList[0]; }
  else { cf = fontList[font]; }

  return cf;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_PrintText
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_PrintText(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor,uchar font)
{
    uint8_t    TempChar;
    const sFONT   *cf = UiLcdHy28_Font(font);

    if (str != NULL) {
      while ( *str != 0 ) {
        TempChar = *str++;

        UiLcdHy28_DrawChar(Xpos, Ypos, TempChar,Color,bkColor,cf);

        if(Xpos < (MAX_X - cf->Width)) {
          Xpos += cf->Width;

          // Mod the 8x8 font - the shift is too big
          // because of the letters
          if(font == 4) {
            if(*str > 0x39)
              Xpos -= 1;
            else
              Xpos -= 2;
          }
        }
        else if (Ypos < (MAX_Y - cf->Height)) {
          Xpos  = 0;
          Ypos += cf->Height;
        } else {
          Xpos = 0;
          Ypos = 0;
        }
      }
    }
}


uint16_t UiLcdHy28_TextHeight(uint8_t font) {

    const sFONT   *cf = UiLcdHy28_Font(font);
    return cf->Height;
}


uint16_t UiLcdHy28_TextWidth(const char *str, uchar font) {

	uint16_t Xpos = 0;

    const sFONT   *cf = UiLcdHy28_Font(font);
    if (str != NULL) {
      while ( *str != 0 ) {
        Xpos+=cf->Width;
        if(font == 4) {
          if(*str > 0x39)
            Xpos -= 1;
          else
            Xpos -= 2;
        }
        str++;
      }
    }
	return Xpos;
}

void UiLcdHy28_PrintTextRight(uint16_t Xpos, uint16_t Ypos, const char *str,const uint32_t Color, const uint32_t bkColor,uint8_t font)
{

	uint16_t Xwidth = UiLcdHy28_TextWidth(str, font);
	if (Xpos < Xwidth ) {
		 Xpos = 0; // TODO: Overflow is not handled too good, just start at beginning of line and draw over the end.
	} else {
		Xpos -= Xwidth;
	}
	UiLcdHy28_PrintText(Xpos, Ypos, str, Color, bkColor, font);
}

void UiLcdHy28_PrintTextCentered(const uint16_t bbX,const uint16_t bbY,const uint16_t bbW,const char* txt,uint32_t clr_fg,uint32_t clr_bg,uint8_t font)
{
    const uint16_t bbH = UiLcdHy28_TextHeight(0);
    const uint16_t txtW = UiLcdHy28_TextWidth(txt,0);
    const uint16_t bbOffset = txtW>bbW?0:((bbW - txtW)+1)/2;

    // we draw the part of  the box not used by text.
    UiLcdHy28_DrawFullRect(bbX,bbY,bbH,bbOffset,clr_bg);
    UiLcdHy28_PrintText((bbX + bbOffset),bbY,txt,clr_fg,clr_bg,0);

    // if the text is smaller than the box, we need to draw the end part of the
    // box
    if (txtW<bbW) {
      UiLcdHy28_DrawFullRect(bbX+txtW+bbOffset,bbY,bbH,bbW-(bbOffset+txtW),clr_bg);
    }
}


//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_InitA
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
// Note:  Delays in this function doubled to make initialization more reliable
// (e.g. prevent "white screen") - KA7OEI, 20140919
//
uchar UiLcdHy28_InitA(void)
{

   // Read LCD ID
   ts.DeviceCode = UiLcdHy28_ReadReg(0x0000);

   if(ts.DeviceCode == 0x0000 || ts.DeviceCode == 0xffff)
      return 1;

   // HY28A - SPI interface only (ILI9320 controller)
   if(ts.DeviceCode == 0x9320)
   {
      // Start Initial Sequence
      UiLcdHy28_WriteReg(0xE5,0x8000);   // Set the internal vcore voltage
      UiLcdHy28_WriteReg(0x00,  0x0001);    // Start internal OSC.

      // Direction related
      UiLcdHy28_WriteReg(0x01,  0x0100);    // set SS and SM bit

      UiLcdHy28_WriteReg(0x02,  0x0700);    // set 1 line inversion
      UiLcdHy28_WriteReg(0x03,  0x1030);    // set GRAM write direction and BGR=1.
      UiLcdHy28_WriteReg(0x04,  0x0000);    // Resize register
      UiLcdHy28_WriteReg(0x08,  0x0202);    // set the back porch and front porch
      UiLcdHy28_WriteReg(0x09,  0x0000);    // set non-display area refresh cycle ISC[3:0]
      UiLcdHy28_WriteReg(0x0A, 0x0000);    // FMARK function
      UiLcdHy28_WriteReg(0x0C, 0x0000);    // RGB interface setting
      UiLcdHy28_WriteReg(0x0D, 0x0000);    // Frame marker Position
      UiLcdHy28_WriteReg(0x0F, 0x0000);    // RGB interface polarity

      // Power On sequence
      UiLcdHy28_WriteReg(0x10, 0x0000);    // SAP, BT[3:0], AP, DSTB, SLP, STB
      UiLcdHy28_WriteReg(0x11, 0x0000);    // DC1[2:0], DC0[2:0], VC[2:0]
      UiLcdHy28_WriteReg(0x12, 0x0000);    // VREG1OUT voltage
      UiLcdHy28_WriteReg(0x13, 0x0000);    // VDV[4:0] for VCOM amplitude
      UiLcdHy28_Delay(300000);            // Dis-charge capacitor power voltage (300ms)
      UiLcdHy28_WriteReg(0x10, 0x17B0);    // SAP, BT[3:0], AP, DSTB, SLP, STB
      UiLcdHy28_WriteReg(0x11, 0x0137);    // DC1[2:0], DC0[2:0], VC[2:0]
      UiLcdHy28_Delay(100000);             // Delay 100 ms
      UiLcdHy28_WriteReg(0x12, 0x0139);    // VREG1OUT voltage
      UiLcdHy28_Delay(100000);             // Delay 100 ms
      UiLcdHy28_WriteReg(0x13, 0x1d00);    // VDV[4:0] for VCOM amplitude
      UiLcdHy28_WriteReg(0x29, 0x0013);    // VCM[4:0] for VCOMH
      UiLcdHy28_Delay(100000);             // Delay 100 ms
      UiLcdHy28_WriteReg(0x20, 0x0000);    // GRAM horizontal Address
      UiLcdHy28_WriteReg(0x21, 0x0000);    // GRAM Vertical Address

      // Adjust the Gamma Curve
      UiLcdHy28_WriteReg(0x30, 0x0007);
      UiLcdHy28_WriteReg(0x31, 0x0007);
      UiLcdHy28_WriteReg(0x32, 0x0007);
      UiLcdHy28_WriteReg(0x35, 0x0007);
      UiLcdHy28_WriteReg(0x36, 0x0007);
      UiLcdHy28_WriteReg(0x37, 0x0700);
      UiLcdHy28_WriteReg(0x38, 0x0700);
      UiLcdHy28_WriteReg(0x39, 0x0700);
      UiLcdHy28_WriteReg(0x3C, 0x0700);
      UiLcdHy28_WriteReg(0x3D, 0x1F00);

      // Set GRAM area
      UiLcdHy28_WriteReg(0x50, 0x0000);    // Horizontal GRAM Start Address
      UiLcdHy28_WriteReg(0x51, 0x00EF);    // Horizontal GRAM End Address
      UiLcdHy28_WriteReg(0x52, 0x0000);    // Vertical GRAM Start Address
      UiLcdHy28_WriteReg(0x53, 0x013F);    // Vertical GRAM End Address

      // Direction related
      UiLcdHy28_WriteReg(0x60,  0xA700);    // Gate Scan Line
      UiLcdHy28_WriteReg(0x61,  0x0001);    // NDL,VLE, REV

      UiLcdHy28_WriteReg(0x6A, 0x0000);    // set scrolling line

      // Partial Display Control
      UiLcdHy28_WriteReg(0x80, 0x0000);
      UiLcdHy28_WriteReg(0x81, 0x0000);
      UiLcdHy28_WriteReg(0x82, 0x0000);
      UiLcdHy28_WriteReg(0x83, 0x0000);
      UiLcdHy28_WriteReg(0x84, 0x0000);
      UiLcdHy28_WriteReg(0x85, 0x0000);

      // Panel Control
      UiLcdHy28_WriteReg(0x90, 0x0010);
      UiLcdHy28_WriteReg(0x92, 0x0000);
      UiLcdHy28_WriteReg(0x93, 0x0003);
      UiLcdHy28_WriteReg(0x95, 0x0110);
      UiLcdHy28_WriteReg(0x97, 0x0000);
      UiLcdHy28_WriteReg(0x98, 0x0000);

      // Set GRAM write direction
      //UiLcdHy28_WriteReg(0x03, 0x1028);
      UiLcdHy28_WriteReg(0x03, 0x1030);

      // 262K color and display ON
      UiLcdHy28_WriteReg(0x07, 0x0173);

      // delay 50 ms
      UiLcdHy28_Delay(50000);
   }

   // HY28A - Parallel interface only (SPFD5408B controller)
   if(ts.DeviceCode == 0x5408)
   {
      UiLcdHy28_WriteReg(0x0001,0x0000);   // (SS bit 8) - 0x0100 will flip 180 degree
      UiLcdHy28_WriteReg(0x0002,0x0700);   // LCD Driving Waveform Contral
      UiLcdHy28_WriteReg(0x0003,0x1030);   // Entry Mode (AM bit 3)

      UiLcdHy28_WriteReg(0x0004,0x0000);   // Scaling Control register
      UiLcdHy28_WriteReg(0x0008,0x0207);   // Display Control 2
      UiLcdHy28_WriteReg(0x0009,0x0000);   // Display Control 3
      UiLcdHy28_WriteReg(0x000A,0x0000);   // Frame Cycle Control
      UiLcdHy28_WriteReg(0x000C,0x0000);   // External Display Interface Control 1
      UiLcdHy28_WriteReg(0x000D,0x0000);    // Frame Maker Position
      UiLcdHy28_WriteReg(0x000F,0x0000);   // External Display Interface Control 2
      UiLcdHy28_Delay(50000);
      UiLcdHy28_WriteReg(0x0007,0x0101);   // Display Control
      UiLcdHy28_Delay(50000);
      UiLcdHy28_WriteReg(0x0010,0x16B0);    // Power Control 1
      UiLcdHy28_WriteReg(0x0011,0x0001);    // Power Control 2
      UiLcdHy28_WriteReg(0x0017,0x0001);    // Power Control 3
      UiLcdHy28_WriteReg(0x0012,0x0138);    // Power Control 4
      UiLcdHy28_WriteReg(0x0013,0x0800);    // Power Control 5
      UiLcdHy28_WriteReg(0x0029,0x0009);   // NVM read data 2
      UiLcdHy28_WriteReg(0x002a,0x0009);   // NVM read data 3
      UiLcdHy28_WriteReg(0x00a4,0x0000);
      UiLcdHy28_WriteReg(0x0050,0x0000);
      UiLcdHy28_WriteReg(0x0051,0x00EF);
      UiLcdHy28_WriteReg(0x0052,0x0000);
      UiLcdHy28_WriteReg(0x0053,0x013F);

      UiLcdHy28_WriteReg(0x0060,0x2700);   // Driver Output Control (GS bit 15)

      UiLcdHy28_WriteReg(0x0061,0x0003);   // Driver Output Control
      UiLcdHy28_WriteReg(0x006A,0x0000);   // Vertical Scroll Control

      UiLcdHy28_WriteReg(0x0080,0x0000);   // Display Position ?C Partial Display 1
      UiLcdHy28_WriteReg(0x0081,0x0000);   // RAM Address Start ?C Partial Display 1
      UiLcdHy28_WriteReg(0x0082,0x0000);   // RAM address End - Partial Display 1
      UiLcdHy28_WriteReg(0x0083,0x0000);   // Display Position ?C Partial Display 2
      UiLcdHy28_WriteReg(0x0084,0x0000);   // RAM Address Start ?C Partial Display 2
      UiLcdHy28_WriteReg(0x0085,0x0000);   // RAM address End ?C Partail Display2
      UiLcdHy28_WriteReg(0x0090,0x0013);   // Frame Cycle Control
      UiLcdHy28_WriteReg(0x0092,0x0000);    // Panel Interface Control 2
      UiLcdHy28_WriteReg(0x0093,0x0003);   // Panel Interface control 3
      UiLcdHy28_WriteReg(0x0095,0x0110);   // Frame Cycle Control
      UiLcdHy28_WriteReg(0x0007,0x0173);
   }

   // HY28B - Parallel & Serial interface - latest model (ILI9325 & ILI9328 controller)
   if((ts.DeviceCode == 0x9325) || (ts.DeviceCode == 0x9328))
   {
//      UiLcdHy28_WriteReg(0x00e5,0x78f0);	// special testing
      UiLcdHy28_WriteReg(0x0001,0x0000);	// set SS and SM bit
      UiLcdHy28_WriteReg(0x0002,0x0700);	// set 1 line inversion
      UiLcdHy28_WriteReg(0x0004,0x0000);	// resize register
      UiLcdHy28_WriteReg(0x0008,0x0207);	// set the back porch and front porch
      UiLcdHy28_WriteReg(0x0009,0x0000);	// set non-display area refresh cycle
      UiLcdHy28_WriteReg(0x000a,0x0000);	// FMARK function
      UiLcdHy28_WriteReg(0x000c,0x0001);	// RGB interface setting
      UiLcdHy28_WriteReg(0x000d,0x0000);	// frame marker position
      UiLcdHy28_WriteReg(0x000f,0x0000);	// RGB interface polarity

      // Power On sequence
      UiLcdHy28_WriteReg(0x0010,0x0000);	// SAP, BT[3:0], AP, DSTB, SLP, STB
      UiLcdHy28_WriteReg(0x0011,0x0007);	// DC1[2:0], DC0[2:0], VC[2:0]
      UiLcdHy28_WriteReg(0x0012,0x0000);	// VREG1OUT voltage
      UiLcdHy28_WriteReg(0x0013,0x0000);	// VDV[4:0] for VCOM amplitude
      UiLcdHy28_Delay(200000);			// delay 200 ms
      UiLcdHy28_WriteReg(0x0010,0x1590);	// SAP, BT[3:0], AP, DSTB, SLP, STB
      UiLcdHy28_WriteReg(0x0011,0x0227);	// set DC1[2:0], DC0[2:0], VC[2:0]
      UiLcdHy28_Delay(50000);			// delay 50 ms
      UiLcdHy28_WriteReg(0x0012,0x009c);	// internal reference voltage init
      UiLcdHy28_Delay(50000);			// delay 50 ms
      UiLcdHy28_WriteReg(0x0013,0x1900);	// set VDV[4:0] for VCOM amplitude
      UiLcdHy28_WriteReg(0x0029,0x0023);	// VCM[5:0] for VCOMH
      UiLcdHy28_WriteReg(0x002b,0x000d);	// set frame rate: changed from 0e to 0d on 03/28/2016
      UiLcdHy28_Delay(50000);			// delay 50 ms
      UiLcdHy28_WriteReg(0x0020,0x0000);	// GRAM horizontal address
      UiLcdHy28_WriteReg(0x0021,0x0000);	// GRAM vertical address
      UiLcdHy28_WriteReg(0x0050,0x0000);	// horizontal GRAM start address
      UiLcdHy28_WriteReg(0x0051,0x00ef);	// horizontal GRAM end address
      UiLcdHy28_WriteReg(0x0052,0x0000);	// vertical GRAM start address
      UiLcdHy28_WriteReg(0x0053,0x013f);	// vertical GRAM end address
      UiLcdHy28_WriteReg(0x0060,0xa700);	// gate scan line
      UiLcdHy28_WriteReg(0x0061,0x0001);	// NDL, VLE, REV
      UiLcdHy28_WriteReg(0x006a,0x0000);	// set scrolling line
      // partial display control
      UiLcdHy28_WriteReg(0x0080,0x0000);
      UiLcdHy28_WriteReg(0x0081,0x0000);
      UiLcdHy28_WriteReg(0x0082,0x0000);
      UiLcdHy28_WriteReg(0x0083,0x0000);
      UiLcdHy28_WriteReg(0x0084,0x0000);
      UiLcdHy28_WriteReg(0x0085,0x0000);
      // panel control
      UiLcdHy28_WriteReg(0x0090,0x0010);
      UiLcdHy28_WriteReg(0x0092,0x0000);
      // activate display using 262k colours
      UiLcdHy28_WriteReg(0x0007,0x0133);
      }

   // Test LCD
    UiLcdHy28_Test();

    return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_Test
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiLcdHy28_Test(void)
{
   // Backlight on - only when all is drawn
   //LCD_BACKLIGHT_PIO->BSRRL = LCD_BACKLIGHT;

   // Clear all
   //UiLcdHy28_LcdClear(Black);

   // Vertical line
   //UiLcdHy28_DrawStraightLine(20,20,200,LCD_DIR_VERTICAL,White);

   // Horizontal line
   //UiLcdHy28_DrawStraightLine(20,20,200,LCD_DIR_HORIZONTAL,White);

   // Gradient line
   //UiLcdHy28_DrawHorizLineWithGrad(20,20,200,0x80);

   // Draw empty rectangular
   //UiLcdHy28_DrawEmptyRect(40,40,100,100,White);

   // Draw full rectangular
   //UiLcdHy28_DrawFullRect(40,40,100,100,White);

   // Draw button
   //UiLcdHy28_DrawBottomButton(0,224,28,62,Grey);

   // Print text
   //UiLcdHy28_PrintText(0,  0,"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ",White,Blue,0);
   //UiLcdHy28_PrintText(0, 30,"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ",White,Blue,1);
   //UiLcdHy28_PrintText(0,130,"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ",White,Blue,2);
   //UiLcdHy28_PrintText(0,170,"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ",White,Blue,3);
   //UiLcdHy28_PrintText(0,210,"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ",White,Blue,4);
}

/*
 * brief Identifies and initializes to HY28x display family
 *
 * @returns 0 if no display detected, DISPLAY_HY28x_xxx otherwise, see header
 */


uint8_t UiLcdHy28_Init(void)
{
	uint8_t retval = DISPLAY_HY28A_SPI;
   // Backlight
   UiLcdHy28_BacklightInit();

   // Select interface, spi HY28A first
   display_use_spi = 1;

   lcd_cs = LCD_D11;
   lcd_cs_pio = LCD_D11_PIO;

   // Try SPI Init
   UiLcdHy28_SpiInit(false);
   // HY28A works only with less then 32 Mhz, so we do low speed

   // Reset
   UiLcdHy28_Reset();

   // LCD Init
   if(UiLcdHy28_InitA() != 0) {
        // no success, no SPI found
     retval = DISPLAY_HY28B_SPI;

   // SPI disable
   UiLcdHy28_SpiDeInit();

   // Select interface, spi HY28B second
   display_use_spi = 2;

   lcd_cs = LCD_CS;
   lcd_cs_pio = LCD_CS_PIO;

   // Try SPI Init
   UiLcdHy28_SpiInit(HY28BHISPEED);
   // Some HY28B work with 50 Mhz, so we do high speed

   // Reset
   UiLcdHy28_Reset();

   // LCD Init
   }
   if(UiLcdHy28_InitA() != 0) {
      // no success, no SPI found
   // SPI disable
   // Select parallel
   display_use_spi = 0;

   // Parallel init
   UiLcdHy28_ParallelInit();
   UiLcdHy28_FSMCConfig();

   // Reset
   UiLcdHy28_Reset();

   // LCD Init
   retval = UiLcdHy28_InitA() != 0?DISPLAY_NONE:DISPLAY_HY28B_PARALLEL;   // on error here
   }

   return retval;
}


void UiLcdHy28_GetTouchscreenCoordinates(bool mode)
{
    uchar i,x,y;

/*
statemachine stati:
0 = no touchscreen action detected
1 = first touchscreen data fetched
>1 = x times valid data available
0xff = data was already processed by calling function
*/

	if(ts.tp_state < VALID_TP_DATASETS)	// no valid data ready or data ready to process
	    {
	    if(ts.tp_state > 0 && ts.tp_state < VALID_TP_DATASETS)	// first pass finished, get data
		  {
		  GPIO_ResetBits(TP_CS_PIO, TP_CS);
		  UiLcdHy28_SendByteSpi(144);
		  x = UiLcdHy28_ReadByteSpi();
		  UiLcdHy28_SendByteSpi(208);
		  y = UiLcdHy28_ReadByteSpi();
		  GPIO_SetBits(TP_CS_PIO, TP_CS);

		  if(mode)								//do translation with correction table
		    {
		    for(i=0; touchscreentable[i]<= x; i++)
			  ;
		    x = 60-i;
		    for(i=0; touchscreentable[i]<= y; i++)
			  ;
		    y = i--;
		    }
		  if(x == ts.tp_x && y == ts.tp_y)	// got identical data
		    ts.tp_state++;						// touch data valid
		  else
		    {					// set new data
		    ts.tp_x = x;
		    ts.tp_y = y;
		    }
		  }
	    else
		  ts.tp_state = 1;			// do next first data read
	    }
}


void UiLcdHy28_ReadTcData()
{
		  GPIO_ResetBits(TP_CS_PIO, TP_CS);
		  UiLcdHy28_SendByteSpi(144);
		  ts.tp_x = UiLcdHy28_ReadByteSpi();
		  UiLcdHy28_SendByteSpi(208);
		  ts.tp_y = UiLcdHy28_ReadByteSpi();
		  GPIO_SetBits(TP_CS_PIO, TP_CS);
}


#pragma GCC optimize("O0")
//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_Delay
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiLcdHy28_Delay(ulong delay)
{
   ulong    i,k;

    for ( k = 0 ;(k < delay );k++ )
        for ( i = 0 ;(i < US_DELAY );i++ ) { asm(""); }
}
