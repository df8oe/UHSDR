/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/*************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:      uhsdr_structs.h                                                **
**  Description:    Collect globally used structs and types.                       **
**  Created by:     m-chichikalov (rv9yw)                                          **
**  Licence:		GNU GPLv3                                                      **
**                                                                                 **
**  Last Modified:  2018-14-12: added display_t struct & display_interafce_e enum. **
**                                                                                 **
************************************************************************************/

#ifndef __MCHF_STRUCTS_H
#define __MCHF_STRUCTS_H

#include "uhsdr_types.h"

/*
 * \brief Described direction of line or something that have parameter such direction.
 */
typedef enum
{
	VERTICAL = 0,
	HORIZONTAL,
} direction_e;

/*
 * \brief Described the display interface.
 */
typedef enum
{
	SPI = 0,
	PARALLEL,
} display_interafce_e;

/*
 * \brief Described the bandwidth of interface.
 */
typedef enum
{
	DATA_8_BIT = 0,
	DATA_16_BIT,
	DATA_32_BIT,
} IO_bitwidth_e;

typedef struct
{
	void*  GPIO_port;
	ushort GPIO_pin;
} dio_t;

// forward declaration
struct UHSDR_INTERFACE;
typedef struct UHSDR_INTERFACE UHSDR_INTERFACE_t;

typedef struct IO_
{
	int32_t ( *Init)( UHSDR_INTERFACE_t* uhsdr_IF );
	int32_t ( *DeInit)( UHSDR_INTERFACE_t* uhsdr_IF );
	int32_t ( *RequestTransaction)( UHSDR_INTERFACE_t* uhsdr_IF, uint32_t TimeOut );
	void ( *CloseTransaction)( UHSDR_INTERFACE_t* uhsdr_IF );
	void ( *Receive_Polling)( UHSDR_INTERFACE_t* uhsdr_IF, uint8_t* Buffer, uint16_t Size, uint32_t Settings );
	void ( *Send_Polling)( UHSDR_INTERFACE_t* uhsdr_IF, uint8_t* Source, uint16_t Size, uint32_t Settings );
	void ( *Receive_DMA)( UHSDR_INTERFACE_t* uhsdr_IF, uint8_t* Source, uint16_t Size, uint32_t Settings );
	void ( *Send_DMA)( UHSDR_INTERFACE_t* uhsdr_IF, uint8_t* Source, uint16_t Size, uint32_t Settings );
	// going remove later this one below
	void (*SendReceive_Polling)( UHSDR_INTERFACE_t* uhsdr_IF, uint8_t* Source, uint8_t* Destination, uint16_t Size, uint32_t Settings );
} IO_t;

// FIXME maybe reorganize as UNION to be specific for each interface?
typedef struct HW_INTERFACE
{
    void* handler;
    volatile bool lock;
    uint32_t time_out;
    UHSDR_INTERFACE_t* parent;
    void (*Init) ( void );
    void (*DeInit) ( void );
} HW_INTERFACE_t;

typedef struct HW_SPI_SPECIFIC
{
    dio_t cs;
} HW_SPI_SPECIFIC_t;

typedef struct HW_FMC_SPECIFIC
{
} HW_FMC_SPECIFIC_t;

typedef struct HW_I2C_SPECIFIC
{
} HW_I2C_SPECIFIC_t;

typedef struct HW_I2S_SPECIFIC
{
} HW_I2S_SPECIFIC_t;

typedef struct UHSDR_INTERFACE
{
    const IO_t* IO;
    HW_INTERFACE_t* hw;
    union
    {
        HW_SPI_SPECIFIC_t spi;
        HW_FMC_SPECIFIC_t fmc;
        HW_I2C_SPECIFIC_t i2c;
        HW_I2S_SPECIFIC_t i2s;
    } extra_parameter;
} UHSDR_INTERFACE_t;

/*
 * \brief display_t contains pointers to functions of driver and driver specific data.
 */
// Forward declaration
struct display_;
typedef struct display_ display_t;

typedef struct display_
{
//	uchar  display_type;           // existence/identification of display type
	const char* name;                        // pointer to name string
	display_interafce_e display_inteface;    // LCD interface
	ushort device_code;                      // LCD ident code

	void   ( *StartTransaction)( display_t* Display );
	void   ( *PrepareGRAM)( display_t* Display, ushort Xpos, ushort Width, ushort Ypos, ushort Height );
	void   ( *WriteBuffer)( display_t* Display, ushort* Buffer, uint Len );
	void   ( *CloseTransaction)( display_t* Display );

	// Display specific functions implementation, if not needed leave them as NULL.
	void   ( *DrawColorPoint)( display_t* Display, ushort Xpos, ushort Ypos, ushort Color );
	void   ( *DrawFullRect)( display_t* Display, ushort Xpos, ushort Ypos, ushort Height, ushort Width, ushort Color );
	void   ( *DrawStraightLine)( display_t* Display, ushort Xpos, ushort Ypos, ushort Length, uchar Direction, ushort Color);

	// private functions, used only by drivers, for parallel I/F they set needed address into FMC.
	void   ( *PrepareData)( display_t* Display );
	void   ( *PrepareCmd)( display_t* Display );

	ushort MAX_X;
	ushort MAX_Y;

	dio_t spi_dc;     // data/command DIO
	dio_t lcd_reset;  // reset LCD DIO

	volatile bool semaphore; // display mutex, protects buffers.
	UHSDR_INTERFACE_t IF;
} display_t;


#endif // __MCHF_STRUCTS_H
