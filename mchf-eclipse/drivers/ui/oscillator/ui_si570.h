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

#ifndef __UI_SI570
#define __UI_SI570

#define 	HIGHER_PRECISION
//#define 	LOWER_PRECISION

// -------------------------------------------------------------------------------------
// Local Oscillator
// ------------------

// The SI570 Min/Max frequencies are 4x the actual tuning frequencies
#define SI570_MIN_FREQ			10000000	// 10=2.5 MHz
#define SI570_MAX_FREQ			160000000	// 160=40 Mhz
//
// These are "hard limit" frequencies below/above which the synthesizer/algorithms must not be adjusted or else the system may crash
#define SI570_HARD_MIN_FREQ		1000000		// 1.0=0.25 MHz
#define SI570_HARD_MAX_FREQ		220000000	// 220=55 MHz

#define	LARGE_STEP_HYSTERESIS	10000		//0.01	// size, in MHz, of hysteresis in large tuning step at output frequency (4x tuning freq)

#define SI570_RECALL			(1<<0)
#define SI570_FREEZE_DCO		(1<<4)
#define SI570_FREEZE_M			(1<<5)
#define SI570_NEW_FREQ			(1<<6)

#define SI570_REG_135			135
#define SI570_REG_137			137

#define FACTORY_FXTAL			114.285

// VCO range
#define	FDCO_MAX 				5670
#define FDCO_MIN 				4850

#define POW_2_28          		268435456.0
// -------------------------------------------------------------------------------------
// Temperature sensor
// ------------------
#define MCP_ADDR        		(0x90)

// MCP registers
#define MCP_TEMP        		(0x00)
#define MCP_CONFIG      		(0x01)
#define MCP_HYSTR       		(0x02)
#define MCP_LIMIT       		(0x03)

// MCP CONFIG register bits
#define MCP_ONE_SHOT    		(7)
#define MCP_ADC_RES     		(5)
#define MCP_FAULT_QUEUE 		(3)
#define MCP_ALERT_POL   		(2)
#define MCP_INT_MODE    		(1)
#define MCP_SHUTDOWN    		(0)
#define R_BIT           		(1)
#define W_BIT           		(0)

#define	MCP_ADC_RES_9			0
#define	MCP_ADC_RES_10			1
#define	MCP_ADC_RES_11			2
#define	MCP_ADC_RES_12			3

#define	MCP_POWER_UP			0
#define	MCP_POWER_DOWN			1

typedef struct OscillatorState
{
    long double			rfreq;
    long double 		rfreq_old;

    float 				fxtal;

    uchar				regs[6];

    float				fout;		// contains startup frequency info of Si570

    unsigned short		si570_address;

    uchar				base_reg;

#ifdef LOWER_PRECISION
    uchar 				init_n1;
    uchar				init_hsdiv;
    ulong 				init_rfreq;
#endif

} OscillatorState;


typedef enum
{
    SI570_OK = 0, // tuning ok
    SI570_TUNE_LIMITED, // tuning to freq close to desired freq, still ok
    SI570_TUNE_IMPOSSIBLE, // did not tune, tune freq unknown
    SI570_I2C_ERROR, // could not talk to Si570, tune freq unknown
    SI570_ERROR_VERIFY, // register do not match, tune freq unknown
    SI570_LARGE_STEP, // did not tune, just checking

} Si570_ResultCodes;

// -------------------------------------------------------------------------------------
// Exports
// ------------------


uchar 	ui_si570_get_configuration(void);
Si570_ResultCodes 	ui_si570_set_frequency(ulong freq, int calib, int temp_factor, uchar test);

uchar 	ui_si570_init_temp_sensor(void);
uchar 	ui_si570_read_temp(int *temp);
void 	ui_si570_conv_temp(uchar *temp, int *dtemp);
void 	ui_si570_calculate_startup_frequency(void);
float   ui_si570_get_startup_frequency();
void 	calc_suf_sub(void);

#endif
