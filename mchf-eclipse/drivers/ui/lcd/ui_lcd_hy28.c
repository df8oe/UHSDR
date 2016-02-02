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
**  Licence:      For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

// Optimization disable for this file
#pragma GCC optimize "O0"


// Common
#include "mchf_board.h"

#include <stdio.h>
#include <eeprom.h>

#include "ui_lcd_hy28_fonts.h"
#include "ui_lcd_hy28.h"
#include "ui_si570.h"

// For spectrum display struct
#include "audio_driver.h"

// Saved fonts
extern sFONT GL_Font8x8;
extern sFONT GL_Font8x12;
extern sFONT GL_Font8x12_bold;
extern sFONT GL_Font12x12;
extern sFONT GL_Font16x24;

// Transceiver state public structure
__IO TransceiverState ts;

//uchar use_spi = 0;
int16_t lcd_cs;
GPIO_TypeDef* lcd_cs_pio;

// ------------------------------------------------
// Spectrum display
extern __IO   SpectrumDisplay      sd;

extern __IO OscillatorState os;		// oscillator values - including Si570 startup frequency, displayed on splash screen

static void UiLcdHy28_Delay(ulong delay);

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
void UiLcdHy28_SpiInit()
{
   GPIO_InitTypeDef GPIO_InitStructure;
   SPI_InitTypeDef  SPI_InitStructure;

   // Enable the SPI periph
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

   // Common SPI settings
   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

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
   SPI_InitStructure.SPI_Direction       = SPI_Direction_2Lines_FullDuplex;
   SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL             = SPI_CPOL_High;
   SPI_InitStructure.SPI_CPHA             = SPI_CPHA_2Edge;
   SPI_InitStructure.SPI_NSS             = SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;   // max speed presc_8 with 50Mhz GPIO, max 4 with 100 Mhz
   SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_Mode             = SPI_Mode_Master;
   SPI_Init(SPI2, &SPI_InitStructure);

   // Enable SPI2
   SPI_Cmd(SPI2, ENABLE);

   // Common misc pins settings
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

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
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

   // SPI SCK pin configuration
   GPIO_InitStructure.GPIO_Pin = LCD_SCK;
   GPIO_Init(LCD_SCK_PIO, &GPIO_InitStructure);

   // SPI  MOSI pins configuration
   GPIO_InitStructure.GPIO_Pin =  LCD_MOSI;
   GPIO_Init(LCD_MOSI_PIO, &GPIO_InitStructure);

   // SPI  MISO pins configuration
   GPIO_InitStructure.GPIO_Pin =  LCD_MISO;
   GPIO_Init(LCD_MISO_PIO, &GPIO_InitStructure);

   // Configure GPIO PIN for Chip select
   GPIO_InitStructure.GPIO_Pin = lcd_cs;
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
   GPIO_InitStructure.GPIO_Pin = LCD_RESET;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
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
   p.FSMC_AddressSetupTime       = 3;	// Slow external RAM interface to LCD to reduce corruption on some LCDs  10/14 - ka7oei
   p.FSMC_AddressHoldTime          = 0;
   p.FSMC_DataSetupTime          = 9;		// Slow external RAM interface to LCD to reduce corruption on some LCDs  10/14 - ka7oei
   p.FSMC_BusTurnAroundDuration    = 0;
   p.FSMC_CLKDivision             = 0;
   p.FSMC_DataLatency             = 0;
   p.FSMC_AccessMode             = FSMC_AccessMode_A;

   // Color LCD configuration ------------------------------------
   //   LCD configured as follow:
   //     - Data/Address MUX = Disable
   //     - Memory Type = SRAM
   //     - Data Width = 16bit
   //     - Write Operation = Enable
   //     - Extended Mode = Enable
   //     - Asynchronous Wait = Disable

   FSMC_NORSRAMInitStructure.FSMC_Bank                = FSMC_Bank1_NORSRAM1;
   FSMC_NORSRAMInitStructure.FSMC_DataAddressMux          = FSMC_DataAddressMux_Disable;
   FSMC_NORSRAMInitStructure.FSMC_MemoryType             = FSMC_MemoryType_SRAM;
   FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth          = FSMC_MemoryDataWidth_16b;
   FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode          = FSMC_BurstAccessMode_Disable;
   FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait       = FSMC_AsynchronousWait_Disable;
   FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity       = FSMC_WaitSignalPolarity_Low;
   FSMC_NORSRAMInitStructure.FSMC_WrapMode             = FSMC_WrapMode_Disable;
   FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive       = FSMC_WaitSignalActive_BeforeWaitState;
   FSMC_NORSRAMInitStructure.FSMC_WriteOperation          = FSMC_WriteOperation_Enable;
   FSMC_NORSRAMInitStructure.FSMC_WaitSignal             = FSMC_WaitSignal_Disable;
   FSMC_NORSRAMInitStructure.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;
   FSMC_NORSRAMInitStructure.FSMC_WriteBurst             = FSMC_WriteBurst_Disable;
   FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct    = &p;
   FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct       = &p;
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
void UiLcdHy28_SendByteSpi(uint8_t byte)
{
   // while ((SPI2>SR & (SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
   // SPI2->DR = Data;
   while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)  == RESET);
   SPI_I2S_SendData(SPI2, byte);

   while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
   SPI_I2S_ReceiveData(SPI2);

}
void UiLcdHy28_SendByteSpiFast(uint8_t byte)
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
void UiLcdHy28_WriteDataSpiStart(void)
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
void UiLcdHy28_WriteDataOnly( unsigned short data)
{
   if(sd.use_spi)
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
   unsigned short value;
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
   if(GPIO_ReadInputDataBit(TP_IRQ_PIO,TP_IRQ) == 0)	// touchscreen pressed -> read data
	get_touchscreen_coordinates();

   if(sd.use_spi)
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
   if(sd.use_spi)
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
//* Function Name       : UiLcdHy28_LcdClear
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_LcdClear(ushort Color)
{
   ulong i;

   UiLcdHy28_SetCursorA(0,0);
   UiLcdHy28_WriteRAM_Prepare();

   if(sd.use_spi)
   {
      for( i = 0; i < (MAX_X * MAX_Y); i++ )
         UiLcdHy28_WriteDataOnly(Color);

      UiLcdHy28_FinishSpiTransfer();
   }
   else
   {
      for( i = 0; i < (MAX_X * MAX_Y); i++ )
         LCD_RAM = Color;
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

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_WriteRAM_Prepare
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_WriteRAM_Prepare(void)
{
   if(sd.use_spi)
   {
      UiLcdHy28_WriteIndexSpi(0x0022);
      GPIO_ResetBits(lcd_cs_pio, lcd_cs);
      UiLcdHy28_WriteDataSpiStart();
   }
   else
      LCD_REG = 0x22;
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

   if(sd.use_spi)
      GPIO_SetBits(lcd_cs_pio, lcd_cs);

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
   uint32_t i = 0;

   UiLcdHy28_SetCursorA(x, y);

   if(Direction == LCD_DIR_VERTICAL)
   {
      UiLcdHy28_WriteRAM_Prepare();

      for(i = 0; i < Length; i++)
         UiLcdHy28_WriteDataOnly(color);

      if(sd.use_spi)
         GPIO_SetBits(lcd_cs_pio, lcd_cs);
   }
   else
   {
      for(i = 0; i < Length; i++)
      {
         UiLcdHy28_WriteRAM_Prepare();
         UiLcdHy28_WriteDataOnly(color);

         if(sd.use_spi)
            GPIO_SetBits(lcd_cs_pio, lcd_cs);

         x++;
         UiLcdHy28_SetCursorA(x, y);
      }
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

   for(i = 0; i < Length; i++)
   {
      UiLcdHy28_SetCursorA(x, y);

      UiLcdHy28_WriteRAM_Prepare();
      UiLcdHy28_WriteDataOnly(RGB(k,k,k));

      if(sd.use_spi)
         GPIO_SetBits(lcd_cs_pio, lcd_cs);

      j++;
      if(j == GRADIENT_STEP)
      {
         if(i < (Length/2))
            k += (GRADIENT_STEP/2);
         else
            k -= (GRADIENT_STEP/2);

         j = 0;
      }

      //non_os_delay();
      x++;
   }
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
	ulong i, j;

	if(sd.use_spi)	{		// SPI enabled?
		UiLcdHy28_SetCursorA(Xpos, Ypos);	// yes, establish window in which rectangle will be filled
		//
		UiLcdHy28_WriteReg(0x03,  0x1038);    // set GRAM write direction and BGR=1 and turn on cursor auto-increment
		// Establish CGRAM window for THIS rectangle
		UiLcdHy28_WriteReg(0x50, Ypos);    // Vertical GRAM Start Address
		UiLcdHy28_WriteReg(0x51, (Ypos + Height)-1);    // Vertical GRAM End Address
		UiLcdHy28_WriteReg(0x52, Xpos);    // Horizontal GRAM Start Address
		UiLcdHy28_WriteReg(0x53, (Xpos + Width)-1);    // Horizontal GRAM End Address
		//
		j = (ulong)Height * (ulong)Width;		// calculate number of pixels in rectangle
		//
		UiLcdHy28_WriteRAM_Prepare();			// get ready to write pixels
		//
		for(i = 0; i < j; i++)	{				// fill area with pixels of desired color
		      UiLcdHy28_WriteDataOnly(color);
		}

		// Restore normal operation, removing window, setting it back to default (entire display)
		UiLcdHy28_WriteReg(0x50, 0x0000);    // Horizontal GRAM Start Address
		UiLcdHy28_WriteReg(0x51, 0x00EF);    // Horizontal GRAM End Address
		UiLcdHy28_WriteReg(0x52, 0x0000);    // Vertical GRAM Start Address
		UiLcdHy28_WriteReg(0x53, 0x013F);    // Vertical GRAM End Address
		//
		UiLcdHy28_WriteReg(0x03,  0x1030);    // set GRAM write direction and BGR=1 and switch increment mode
	}
	else	{
		while(Width--)	{
			UiLcdHy28_DrawStraightLine(Xpos,Ypos,Height,LCD_DIR_VERTICAL,color);
			Xpos++;
		}
	}
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



//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_CloseBulk
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_CloseBulkWrite(void)
{

   if(sd.use_spi)	{		// SPI enabled?

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
   ushort     x_addr = x;
   ushort      a,b;
   uchar      fw = cf->Width;
   const short   *ch = (const short *)(&cf->table[(symb - 32) * cf->Height]);

   UiLcdHy28_SetCursorA(x_addr,y);	// set starting position of character

   if(sd.use_spi)	{		// SPI enabled?
	   UiLcdHy28_WriteReg(0x03,  0x1038);    // set GRAM write direction and BGR=1 and turn on cursor auto-increment
	   // Establish CGRAM window for THIS character
	   UiLcdHy28_WriteReg(0x50, y);    // Vertical GRAM Start Address
	   UiLcdHy28_WriteReg(0x51, y+(cf->Height)-1);    // Vertical GRAM End Address	-1
	   UiLcdHy28_WriteReg(0x52, x);    // Horizontal GRAM Start Address
	   UiLcdHy28_WriteReg(0x53, x_addr+(cf->Width)-1);    // Horizontal GRAM End Address  -1
	   //
	   UiLcdHy28_WriteRAM_Prepare();					// prepare for bulk-write to character window
	   //
	   for(i = 0; i < cf->Height; i++)	{
		   // Draw line
		   for(j = 0; j < cf->Width; j++)	{
			   a = ((ch[i] & ((0x80 << ((fw / 12 ) * 8)) >> j)));
			   b = ((ch[i] &  (0x01 << j)));
			   //
			   if((!a && (fw <= 12)) || (!b && (fw > 12)))	{	// background color
				   UiLcdHy28_WriteDataOnly(bkColor);
			   }
			   else	{	// foreground color
				   UiLcdHy28_WriteDataOnly(Color);
			   }
		   }
	   }
	   GPIO_SetBits(lcd_cs_pio, lcd_cs);	// bulk-write complete!
	   //
	   UiLcdHy28_WriteReg(0x50, 0x0000);    // Horizontal GRAM Start Address
	   UiLcdHy28_WriteReg(0x51, 0x00EF);    // Horizontal GRAM End Address
	   UiLcdHy28_WriteReg(0x52, 0x0000);    // Vertical GRAM Start Address
	   UiLcdHy28_WriteReg(0x53, 0x013F);    // Vertical GRAM End Address
	   //
	   UiLcdHy28_WriteReg(0x03,  0x1030);    // set GRAM write direction and BGR=1 and switch increment mode
   }

   else	{	// NOT SPI mode
	   for(i = 0; i < cf->Height; i++)	{
		   // Draw line
		   for(j = 0; j < cf->Width; j++)	{
			   UiLcdHy28_WriteRAM_Prepare();
			   a = ((ch[i] & ((0x80 << ((fw / 12 ) * 8)) >> j)));
			   b = ((ch[i] &  (0x01 << j)));

			   if((!a && (fw <= 12)) || (!b && (fw > 12)))
				   UiLcdHy28_WriteDataOnly(bkColor);
			   else
				   UiLcdHy28_WriteDataOnly(Color);

			   x_addr++;
			   UiLcdHy28_SetCursorA(x_addr,y);
		   }

		   y++;
		   x_addr = x;
		   UiLcdHy28_SetCursorA(x_addr,y);
	   }
   }


}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_PrintText
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_PrintText(ushort Xpos, ushort Ypos, char *str,ushort Color, ushort bkColor,uchar font)
{
    uint8_t    TempChar;
    const sFONT   *cf;

    switch(font)
    {
       case 1:
          cf = &GL_Font16x24;
          break;
       case 2:
          cf = &GL_Font12x12;
          break;
       case 3:
          cf = &GL_Font8x12;
          break;
       case 4:
          cf = &GL_Font8x8;
          break;
       default:
          cf = &GL_Font8x12_bold;
          break;
    }

    do{
        TempChar = *str++;

        UiLcdHy28_DrawChar(Xpos, Ypos, TempChar,Color,bkColor,cf);

        if(Xpos < (MAX_X - cf->Width))
        {
            Xpos += cf->Width;

            // Mod the 8x8 font - the shift is too big
            // because of the letters
            if(font == 4)
            {
               if(*str > 0x39)
                  Xpos -= 1;
               else
                  Xpos -= 2;
            }
        }
        else if (Ypos < (MAX_Y - cf->Height))
        {
            Xpos  = 0;
            Ypos += cf->Height;
        }
        else
        {
            Xpos = 0;
            Ypos = 0;
        }

    } while ( *str != 0 );
}


uint16_t UiLcdHy28_TextWidth(char *str, uchar font) {

	const sFONT   *cf;
	uint16_t Xpos = 0;

	switch(font)
	{
	case 1:
		cf = &GL_Font16x24;
		break;
	case 2:
		cf = &GL_Font12x12;
		break;
	case 3:
		cf = &GL_Font8x12;
		break;
	case 4:
		cf = &GL_Font8x8;
		break;
	default:
		cf = &GL_Font8x12_bold;
		break;
	}

	do{
		Xpos+=cf->Width;
		if(font == 4)
		{
			if(*str > 0x39)
				Xpos -= 1;
			else
				Xpos -= 2;
		}
	} while ( *++str != 0 );

	return Xpos;
}

void UiLcdHy28_PrintTextRight(ushort Xpos, ushort Ypos, char *str,ushort Color, ushort bkColor,uchar font)
{

	uint16_t Xwidth = UiLcdHy28_TextWidth(str, font);
	if (Xpos < Xwidth ) {
		 Xpos = 0; // TODO: Overflow is not handled too good, just start at beginning of line and draw over the end.
	} else {
		Xpos -= Xwidth;
	}
	UiLcdHy28_PrintText(Xpos, Ypos, str, Color, bkColor, font);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawSpectrum
//* Object              : repaint spectrum scope control
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_DrawSpectrum(q15_t *fft,ushort color,ushort shift)
{
   ushort       i,k,x,y,y1,len,sh,clr;
   bool      repaint_v_grid = false;

   if(shift)
      sh = (SPECTRUM_WIDTH/2);
   else
      sh = 0;

   // Draw spectrum
   for(x = (SPECTRUM_START_X + sh + 0); x < (POS_SPECTRUM_IND_X + SPECTRUM_WIDTH/2 + sh); x++)
   {
      y = *fft++;

      // Limit vertical
      if(y > (SPECTRUM_HEIGHT - 15))
         y = (SPECTRUM_HEIGHT - 15);

      // Data to y position and length
      y1  = (SPECTRUM_START_Y + SPECTRUM_HEIGHT - 1) - y;
      len = y;

      // Skip noise
      if(y == 0)
         continue;

      // Repaint vertical grid on clear
      // Basically paint over the grid is allowed
      // but during spectrum clear instead of masking
      // grid lines with black - they are repainted
      if(color == Black)
      {
         // Enumerate all saved x positions
         for(k = 0; k < 7; k++)
         {
            // Exit on match
            if(x == sd.vert_grid_id[k])
            {
               // New data for repaint
               x   = sd.vert_grid_id[k];
               clr = ts.scope_grid_colour_active;
               repaint_v_grid = true;
               break;
            }
         }
      }

      UiLcdHy28_SetCursorA(x, y1);
      UiLcdHy28_WriteRAM_Prepare();

      // Draw line
      for(i = 0; i < len; i++)
      {
         // Do not check for horizontal grid when we have vertical masking
         if(!repaint_v_grid)
         {
            clr = color;

            // Are we masking over horizontal grid line ?
            if(color == Black)
            {
               // Enumerate all saved y positions
               for(k = 0; k < 3; k++)
               {
                  if(y1 == sd.horz_grid_id[k])
                  {
                     clr = ts.scope_grid_colour_active;
                     break;
                  }
               }
            }
         }

         if(sd.use_spi)
         {
            // Update GRAM
            UiLcdHy28_WriteDataOnly(clr);
         }
         else
            LCD_RAM = clr;

         // Track absolute position
         y1++;
      }

      if(sd.use_spi)
         lcd_cs_pio->BSRRL = lcd_cs;

      // Reset flag
      if(repaint_v_grid)
         repaint_v_grid = 0;

      // ----------------------------------------------------------
      // ----------------------------------------------------------
   }
}


// This version of "Draw Spectrum" is revised from the original in that it interleaves the erasure with the drawing
// of the spectrum to minimize visible flickering  (KA7OEI, 20140916, adapted from original)
//
// 20141004 NOTE:  This has been somewhat optimized to prevent drawing vertical line segments that would need to be re-drawn:
//  - New lines that were shorter than old ones are NOT erased
//  - Line segments that are to be erased are only erased starting at the position of the new line segment.
//
//  This should reduce the amount of CGRAM access - especially via SPI mode - to a minimum.
//
//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawSpectrum_Interleaved
//* Object              : repaint spectrum scope control
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void    UiLcdHy28_DrawSpectrum_Interleaved(q15_t *fft_old, q15_t *fft_new, ushort color_old, ushort color_new, ushort shift)
{
   ushort       i, k, x, y_old , y_new, y1_old, y1_new, len_old, len_new, sh, clr;
   bool      repaint_v_grid = false;

   if(shift)
      sh = (SPECTRUM_WIDTH/2)-1;   // Shift to fill gap in center
   else
      sh = 1;                  // Shift to fill gap in center

   for(x = (SPECTRUM_START_X + sh + 0); x < (POS_SPECTRUM_IND_X + SPECTRUM_WIDTH/2 + sh); x++)
   {
	  y_old = *fft_old++;
      y_new = *fft_new++;

      // Limit vertical
      if(y_old > (SPECTRUM_HEIGHT - 7))
         y_old = (SPECTRUM_HEIGHT - 7);

      if(y_new > (SPECTRUM_HEIGHT - 7))
         y_new = (SPECTRUM_HEIGHT - 7);

      // Data to y position and length
      y1_old  = (SPECTRUM_START_Y + SPECTRUM_HEIGHT - 1) - y_old;
      len_old = y_old;

      y1_new  = (SPECTRUM_START_Y + SPECTRUM_HEIGHT - 1) - y_new;
      len_new = y_new;

      //
      if(y_old <= y_new)	// is old line going to be overwritten by new line, anyway?
         goto draw_new;		// only draw new line - don't bother erasing old line...

      // Repaint vertical grid on clear
      // Basically paint over the grid is allowed
      // but during spectrum clear instead of masking
      // grid lines with black - they are repainted
      if(color_old == Black)
      {
         // Enumerate all saved x positions
         for(k = 0; k < 7; k++)
         {
            // Exit on match
            if(x == sd.vert_grid_id[k])
            {
               // New data for repaint
               x   = sd.vert_grid_id[k];
               if((sd.magnify) && (k == 3))
           	   	   clr = ts.scope_centre_grid_colour_active;
               else if((ts.iq_freq_mode == FREQ_IQ_CONV_LO_LOW) && (k == 4) && (!sd.magnify))			// place the (spectrum) center line with the selected color based on translate mode
           		   clr = ts.scope_centre_grid_colour_active;
           	   else if((ts.iq_freq_mode == FREQ_IQ_CONV_LO_HIGH) && (k == 2) && (!sd.magnify))
           		   clr = ts.scope_centre_grid_colour_active;
           	   else if ((ts.iq_freq_mode == FREQ_IQ_CONV_MODE_OFF) && (k == 3) && (!sd.magnify))
           		   clr = ts.scope_centre_grid_colour_active;
           	   else
           		   clr = ts.scope_grid_colour_active;
               //
               repaint_v_grid = true;
               break;
            }
         }
      }
      //
      UiLcdHy28_SetCursorA(x, y1_old);
      UiLcdHy28_WriteRAM_Prepare();

      // Draw vertical line, starting only with position of where new line would be!
      for(i = y_new; i < len_old; i++)
      {
         // Do not check for horizontal grid when we have vertical masking
         if(!repaint_v_grid)
         {
            clr = color_old;

            // Are we masking over horizontal grid line ?
            if(color_old == Black)
            {
               // Enumerate all saved y positions
               for(k = 0; k < 3; k++)
               {
                  if(y1_old == sd.horz_grid_id[k])
                  {
                     clr = ts.scope_grid_colour_active;
                     break;
                  }
               }
            }
         }

         if(sd.use_spi)
         {
            // Update GRAM
            UiLcdHy28_WriteDataOnly(clr);
         }
         else
            LCD_RAM = clr;

         // Track absolute position
         y1_old++;
      }

      if(sd.use_spi)
         lcd_cs_pio->BSRRL = lcd_cs;

      // Reset flag
      if(repaint_v_grid)
         repaint_v_grid = 0;

      // ----------------------------------------------------------
      //
      draw_new:
      //
      if(y_new == 0)	// no new line to be drawn?
         continue;
      //
      // Repaint vertical grid on clear
      // Basically paint over the grid is allowed
      // but during spectrum clear instead of masking
      // grid lines with black - they are repainted
      //
      if(color_new == Black)
      {
         // Enumerate all saved x positions
         for(k = 0; k < 7; k++)
         {
            // Exit on match
            if(x == sd.vert_grid_id[k])
            {
               // New data for repaint
               x   = sd.vert_grid_id[k];
               if((sd.magnify) && (k == 3))
           	   	   clr = ts.scope_centre_grid_colour_active;
               else if((ts.iq_freq_mode == FREQ_IQ_CONV_LO_LOW) && (k == 4) && (!sd.magnify))			// place the (spectrum) center line with the selected color based on translate mode
           		   clr = ts.scope_centre_grid_colour_active;
           	   else if((ts.iq_freq_mode == FREQ_IQ_CONV_LO_HIGH) && (k == 2) && (!sd.magnify))
           		   clr = ts.scope_centre_grid_colour_active;
           	   else if ((ts.iq_freq_mode == FREQ_IQ_CONV_MODE_OFF) && (k == 3) && (!sd.magnify))
           		   clr = ts.scope_centre_grid_colour_active;
           	   else
           		   clr = ts.scope_grid_colour_active;

               repaint_v_grid = true;
               break;
            }
         }
      }
      //
      UiLcdHy28_SetCursorA(x, y1_new);
      UiLcdHy28_WriteRAM_Prepare();

      // Draw line
      for(i = 0; i < len_new; i++)
      {
         // Do not check for horizontal grid when we have vertical masking
         if(!repaint_v_grid)
         {
            clr = color_new;

            // Are we masking over horizontal grid line ?
            if(color_new == Black)
            {
               // Enumerate all saved y positions
               for(k = 0; k < 3; k++)
               {
                  if(y1_new == sd.horz_grid_id[k])
                  {
                     clr = ts.scope_grid_colour_active;
                     break;
                  }
               }
            }
         }

         if(sd.use_spi)
         {
            // Update GRAM

            UiLcdHy28_WriteDataOnly(clr);
         }
         else
            LCD_RAM = clr;

         // Track absolute position
         y1_new++;
      }

      if(sd.use_spi)
         lcd_cs_pio->BSRRL = lcd_cs;

      // Reset flag
      if(repaint_v_grid)
         repaint_v_grid = 0;
      //
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
   unsigned short DeviceCode;

   // Read LCD ID
   DeviceCode = UiLcdHy28_ReadReg(0x0000);
   printf("lcd id: 0x%04x\n\r",DeviceCode);

   if(DeviceCode == 0x0000)
      return 1;

   // HY28A - SPI interface only (ILI9320 controller)
   if(DeviceCode == 0x9320)
   {
      printf("doing ILI9320 init\n\r");

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
   if(DeviceCode == 0x5408)
   {
      printf("doing SPFD5408B init\n\r");

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

      UiLcdHy28_WriteReg(0x0080,0x0000);   // Display Position �C Partial Display 1
      UiLcdHy28_WriteReg(0x0081,0x0000);   // RAM Address Start �C Partial Display 1
      UiLcdHy28_WriteReg(0x0082,0x0000);   // RAM address End - Partial Display 1
      UiLcdHy28_WriteReg(0x0083,0x0000);   // Display Position �C Partial Display 2
      UiLcdHy28_WriteReg(0x0084,0x0000);   // RAM Address Start �C Partial Display 2
      UiLcdHy28_WriteReg(0x0085,0x0000);   // RAM address End �C Partail Display2
      UiLcdHy28_WriteReg(0x0090,0x0013);   // Frame Cycle Control
      UiLcdHy28_WriteReg(0x0092,0x0000);    // Panel Interface Control 2
      UiLcdHy28_WriteReg(0x0093,0x0003);   // Panel Interface control 3
      UiLcdHy28_WriteReg(0x0095,0x0110);   // Frame Cycle Control
      UiLcdHy28_WriteReg(0x0007,0x0173);
   }

   // HY28B - Parallel & Serial interface - latest model (ILI9325 controller)
   if((DeviceCode == 0x9325) || (DeviceCode == 0x9328))
   {
      printf("doing ILI9325 init\n\r");

      UiLcdHy28_WriteReg(0x00e7,0x0010);
      UiLcdHy28_WriteReg(0x0000,0x0001);  // start internal osc
      UiLcdHy28_WriteReg(0x0001,0x0000);   // 0x0100 will flip 180 degree
      UiLcdHy28_WriteReg(0x0002,0x0700);    // power on sequence
      UiLcdHy28_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) );
      UiLcdHy28_WriteReg(0x0004,0x0000);
      UiLcdHy28_WriteReg(0x0008,0x0207);
      UiLcdHy28_WriteReg(0x0009,0x0000);
      UiLcdHy28_WriteReg(0x000a,0x0000);    // display setting
      UiLcdHy28_WriteReg(0x000c,0x0001);   // display setting
      UiLcdHy28_WriteReg(0x000d,0x0000);
      UiLcdHy28_WriteReg(0x000f,0x0000);

      // Power On sequence
      UiLcdHy28_WriteReg(0x0010,0x0000);
      UiLcdHy28_WriteReg(0x0011,0x0007);
      UiLcdHy28_WriteReg(0x0012,0x0000);
      UiLcdHy28_WriteReg(0x0013,0x0000);
      UiLcdHy28_Delay(100000);           // delay 100 ms
      UiLcdHy28_WriteReg(0x0010,0x1590);
      UiLcdHy28_WriteReg(0x0011,0x0227);
      UiLcdHy28_Delay(100000);           // delay 100 ms
      UiLcdHy28_WriteReg(0x0012,0x009c);
      UiLcdHy28_Delay(100000);           // delay 100 ms
      UiLcdHy28_WriteReg(0x0013,0x1900);
      UiLcdHy28_WriteReg(0x0029,0x0023);
      UiLcdHy28_WriteReg(0x002b,0x000e);
      UiLcdHy28_Delay(100000);           // delay 100 ms
      UiLcdHy28_WriteReg(0x0020,0x0000);
      UiLcdHy28_WriteReg(0x0021,0x0000);
      UiLcdHy28_Delay(100000);           // delay 100 ms
      UiLcdHy28_WriteReg(0x0030,0x0007);
      UiLcdHy28_WriteReg(0x0031,0x0707);
      UiLcdHy28_WriteReg(0x0032,0x0006);
      UiLcdHy28_WriteReg(0x0035,0x0704);
      UiLcdHy28_WriteReg(0x0036,0x1f04);
      UiLcdHy28_WriteReg(0x0037,0x0004);
      UiLcdHy28_WriteReg(0x0038,0x0000);
      UiLcdHy28_WriteReg(0x0039,0x0706);
      UiLcdHy28_WriteReg(0x003c,0x0701);
      UiLcdHy28_WriteReg(0x003d,0x000f);
      UiLcdHy28_Delay(100000);           // delay 100 ms
      UiLcdHy28_WriteReg(0x0050,0x0000);
      UiLcdHy28_WriteReg(0x0051,0x00ef);
      UiLcdHy28_WriteReg(0x0052,0x0000);
      UiLcdHy28_WriteReg(0x0053,0x013f);
      UiLcdHy28_WriteReg(0x0060,0xa700);
      UiLcdHy28_WriteReg(0x0061,0x0001);
      UiLcdHy28_WriteReg(0x006a,0x0000);
      UiLcdHy28_WriteReg(0x0080,0x0000);
      UiLcdHy28_WriteReg(0x0081,0x0000);
      UiLcdHy28_WriteReg(0x0082,0x0000);
      UiLcdHy28_WriteReg(0x0083,0x0000);
      UiLcdHy28_WriteReg(0x0084,0x0000);
      UiLcdHy28_WriteReg(0x0085,0x0000);

      UiLcdHy28_WriteReg(0x0090,0x0010);
      UiLcdHy28_WriteReg(0x0092,0x0000);
      UiLcdHy28_WriteReg(0x0093,0x0003);
      UiLcdHy28_WriteReg(0x0095,0x0110);
      UiLcdHy28_WriteReg(0x0097,0x0000);
      UiLcdHy28_WriteReg(0x0098,0x0000);

      // display on sequence
      UiLcdHy28_WriteReg(0x0007,0x0133);

      UiLcdHy28_WriteReg(0x0020,0x0000);   // Line first address 0
      UiLcdHy28_WriteReg(0x0021,0x0000);  // Column first site 0
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
void UiLcdHy28_Test(void)
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

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_Init
//* Object              : Search interface and LCD init
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar UiLcdHy28_Init(void)
{
   // Backlight
   UiLcdHy28_BacklightInit();

   // Select interface, spi HY28A first
   sd.use_spi = 1;

   lcd_cs = LCD_D11;
   lcd_cs_pio = LCD_D11_PIO;

   // Try SPI Init
   UiLcdHy28_SpiInit();

   // Reset
   UiLcdHy28_Reset();

   // LCD Init
   if(UiLcdHy28_InitA() == 0)
      return 0;         // success, SPI found

   // SPI disable
   UiLcdHy28_SpiDeInit();

   // Select interface, spi HY28B second
   sd.use_spi = 2;

   lcd_cs = LCD_CS;
   lcd_cs_pio = LCD_CS_PIO;

   // Try SPI Init
   UiLcdHy28_SpiInit();

   // Reset
   UiLcdHy28_Reset();

   // LCD Init
   if(UiLcdHy28_InitA() == 0)
      return 0;         // success, SPI found

   // SPI disable
//   UiLcdHy28_SpiDeInit();

    // SPI enable
//   UiLcdHy28_SpiInit();

   // Select parallel
   sd.use_spi = 0;

   // Parallel init
   UiLcdHy28_ParallelInit();
   UiLcdHy28_FSMCConfig();

   // Reset
   UiLcdHy28_Reset();

   // LCD Init
   UiLcdHy28_InitA();   // on error here ?

   return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_ShowStartUpScreen
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiLcdHy28_ShowStartUpScreen(ulong hold_time)
{
   uint16_t    i;
 //  uint16_t    t, s, u, v;
   char   tx[100];

   // Clear all
   UiLcdHy28_LcdClear(Black);

   non_os_delay();
   // Show first line
   sprintf(tx,"%s",DEVICE_STRING);
   UiLcdHy28_PrintText(0,30,tx,Cyan,Black,1);		// Position with original text size:  78,40
   //

   // Show second line
   sprintf(tx,"%s",AUTHOR_STRING);
   UiLcdHy28_PrintText(36,60,tx,White,Black,0);		// 60,60

   // Show third line
   sprintf(tx,"v %d.%d.%d.%d",TRX4M_VER_MAJOR,TRX4M_VER_MINOR,TRX4M_VER_RELEASE,TRX4M_VER_BUILD);
   UiLcdHy28_PrintText(110,80,tx,Grey3,Black,0);

   // Show fourth line
   #define GITHUB_IDENT "$Id$"
   char ty[8];
   for(i=5;i<12;i++)
    ty[i-5] = GITHUB_IDENT[i];
   ty[7] = 0;
   sprintf(tx,"DF8OE-Github-Commit %s",ty);

//   sprintf(tx,"DF8OE-GitHub-Version %s","0.12.rt.4");
   UiLcdHy28_PrintText(45,100,tx,Yellow,Black,0);

   //
   Read_EEPROM(EEPROM_FREQ_CONV_MODE, &i);	// get setting of frequency translation mode

   if(!(i & 0xff))	{
	   sprintf(tx,"WARNING:  Freq. Translation is OFF!!!");
	   UiLcdHy28_PrintText(16,120,tx,Black,Red3,0);
	   sprintf(tx,"Translation is STRONGLY recommended!!");
	   UiLcdHy28_PrintText(16,135,tx,Black,Red3,0);
   }
   else	{
	   sprintf(tx," Freq. Translate On ");
	   UiLcdHy28_PrintText(80,120,tx,Grey3,Black,0);
   }

   // Display the mode of the display interface
   //
   if(sd.use_spi)
   {
	   if(sd.use_spi == 1)
		   sprintf(tx,"LCD: HY28A SPI Mode");
	   else
		   sprintf(tx,"LCD: HY28B SPI Mode");
   }
   else
	   sprintf(tx,"LCD: Parallel Mode");
   //
   UiLcdHy28_PrintText(88,150,tx,Grey1,Black,0);

   //
   // Display startup frequency of Si570, By DF8OE, 201506
   //
   int vorkomma = (int)(os.fout);
   int nachkomma = (int)roundf((os.fout-vorkomma)*10000);

   sprintf(tx,"%s%u%s%u%s","SI570 startup frequency: ",vorkomma,".",nachkomma," MHz");
   UiLcdHy28_PrintText(15, 165, tx, Grey1, Black, 0);
   //

   if(ts.ee_init_stat != FLASH_COMPLETE)	{	// Show error code if problem with EEPROM init
	   sprintf(tx, "EEPROM Init Error Code:  %d", ts.ee_init_stat);
	   UiLcdHy28_PrintText(60,180,tx,White,Black,0);
   }
   else	{
	   ushort adc2, adc3;
	   adc2 = ADC_GetConversionValue(ADC2);
	   adc3 = ADC_GetConversionValue(ADC3);
	   if((adc2 > MAX_VSWR_MOD_VALUE) && (adc3 > MAX_VSWR_MOD_VALUE))	{
		   sprintf(tx, "SWR Bridge resistor mod NOT completed!");
		   UiLcdHy28_PrintText(8,180,tx,Red3,Black,0);
		   //	sprintf(tx, "Value=%d,%d",adc2, adc3);			// debug
		   //	UiLcdHy28_PrintText(60,170,tx,Red,Black,0);		// debug
	   }
   }


   // Additional Attrib line 1
   sprintf(tx,"%s",ATTRIB_STRING1);
   UiLcdHy28_PrintText(54,195,tx,Grey1,Black,0);

   // Additional Attrib line 2
   sprintf(tx,"%s",ATTRIB_STRING2);
   UiLcdHy28_PrintText(42,210,tx,Grey1,Black,0);

   // Additional Attrib line 3
   sprintf(tx,"%s",ATTRIB_STRING3);
   UiLcdHy28_PrintText(50,225,tx,Grey1,Black,0);



   // Backlight on
   LCD_BACKLIGHT_PIO->BSRRL = LCD_BACKLIGHT;

   // On screen delay - decrease if drivers init takes longer
   for(i = 0; i < hold_time; i++)
      non_os_delay();
}


void get_touchscreen_coordinates(void)
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
      for ( i = 0 ;(i < US_DELAY );i++ );
}
