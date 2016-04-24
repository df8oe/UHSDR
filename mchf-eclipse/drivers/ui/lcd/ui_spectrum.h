/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
#ifndef __UI_SPECTRUM_H
#define __UI_SPECTRUM_H
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
**  Licence:      CC BY-NC-SA 3.0                                                **
************************************************************************************/
#include "mchf_board.h"
#include "mchf_types.h"
#include "audio_driver.h"
#include "arm_const_structs.h"

void UiSpectrumInitSpectrumDisplay();
void UiSpectrumClearDisplay();
void UiSpectrumReDrawWaterfall();
void UiSpectrumReDrawScopeDisplay();
void UiSpectrumCreateDrawArea(void);


// Spectrum scope operational constants
//
#define	SPECTRUM_SCOPE_TOP_LIMIT			5	// Top limit of spectrum scope magnitude
//
#define SPECTRUM_SCOPE_AGC_THRESHOLD		2000//400	// AGC "Knee" above which output from spectrum scope FFT  AGC will cause action
#define SPECTRUM_SCOPE_MAX_FFT_VAL			8192//16384 // Value above which input to spectrum scope FFT will cause AGC action
#define SPECTRUM_SCOPE_MIN_GAIN				0.001//0.1	// Minimum gain for spectrum scope FFT AGC loop
#define SPECTRUM_SCOPE_MAX_GAIN				140	// Maximum gain for spectrum scope FFT AGC loop
#define	SPECTRUM_SCOPE_AGC_ATTACK			0.5//0.1	// Attack rate for spectrum scope FFT AGC/gain
#define	SPECTRUM_SCOPE_AGC_DECAY			0.166//0.1	// Decay rate for spectrum scope FFT AGC/gain
//
#define	SPECTRUM_SCOPE_LPF_FACTOR			4	// IIR Factor for spectrum scope low-pass filtering
	// Higher = slower response.  3 = 33% input; 66% feedback, 4 = 25% input, 75% feedback, 5 = 20% input, 80% feedback
#define	SPECTRUM_SCOPE_LPF_FACTOR_SPI		2	// IIR Factor for spectrum scope low-pass filtering using SPI display (update rate is slower, use faster filter)
//
#define PK_AVG_RESCALE_THRESH		3		// This sets the minimum peak-to-average ratio of the spectrum display before it starts to rescale the peak
		// value from the top.  This prevents it from going completely "white" on noise/no-signal conditions.
//
#define	SPECTRUM_SCOPE_RESCALE_ATTACK_RATE	0.1	// Rate at which scaling of spectrum scope adapts to strong signals within its passband
#define	SPECTRUM_SCOPE_RESCALE_DECAY_RATE	0.033	// Rate at which scaling of spectrum scope decays after strong signals disappear from its passband
//
#define SPECTRUM_SCOPE_SPEED_MIN			1	// minimum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_MAX			25	// maximum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_DEFAULT		5
//
#define SPECTRUM_SCOPE_FILTER_MIN			1	// minimum filter setting
#define	SPECTRUM_SCOPE_FILTER_MAX			20	// maximum filter setting
#define SPECTRUM_SCOPE_FILTER_DEFAULT		4	// default filter setting
//
#define	SPECTRUM_SCOPE_AGC_MIN				1	// minimum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_MAX				50	// maximum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_DEFAULT			25	// default spectrum scope AGC rate setting
//
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MIN		10
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MAX		30
#define	SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT		20
//
#define	SPECTRUM_SCOPE_ADJUST_OFFSET	100
//
//
#define	SPECTRUM_AGC_SCALING				25	// scaling factor by which the on-screen Spec. AGC Adj. is divided for adjustment.
//
#define	SCOPE_PREAMP_GAIN					1000//200	// amount of "amplification" in front of the FFT used for the Spectrum Scope and Waterfall used to overcome mathematical "noise floor"
//
#define INIT_SPEC_AGC_LEVEL					-80	// Initial offset for AGC level for spectrum/waterfall display
//
#define SPECTRUM_SCOPE_SPI_HOLDOFF_TIME_TUNE	25	// time, in 100's of second to inhibit spectrum scope update after adjusting tuning while in SPI mode
//
#define	NUMBER_WATERFALL_COLOURS			64		// number of colors in the waterfall table

// ----------------------------------------------------------
// Spectrum draw params
//
// Spectrum display
#define POS_SPECTRUM_IND_X					60
#define POS_SPECTRUM_IND_Y					150
#define POS_SPECTRUM_IND_H					80
#define	POS_SPECTRUM_FREQ_BAR_Y				64	// reducing value moves upwards
#define	POS_SPECTRUM_FILTER_WIDTH_BAR_Y		61
#define POS_SPECTRUM_IND_W					258
#define COL_SPECTRUM_GRAD					0x40

#define	WFALL_MEDIUM_ADDITIONAL	12					// additional vertical height in pixels of "medium" waterfall

// Dark grey colour used for spectrum scope grid
#define Grid                RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD)      // COL_SPECTRUM_GRAD = 0x40

// WARNING:  Because the waterfall uses a "block write" which is, in effect, a "blind" writing of data to the LCD, the size of the graphic
// block *MUST* exactly match the number of pixels within that block.
//
// Furthermore, the "SPECTRUM WIDTH" must match exactly with graphical width of the "X" size of each line to be written or skewing will result!
//
#define SPECTRUM_START_X		POS_SPECTRUM_IND_X
//
// Shift of whole spectrum in vertical direction
#define SPECTRUM_START_Y		(POS_SPECTRUM_IND_Y - 10)
//
// Spectrum hight is bit lower that the whole control
#define SPECTRUM_HEIGHT			(POS_SPECTRUM_IND_H - 10)
//
// How much larger than the NORMAL spectrum display should the BIG Spectrum display be?
#define SPEC_LIGHT_MORE_POINTS 15
//
// Dependent on FFT samples,but should be less than control width!
#define SPECTRUM_WIDTH			256

// Spectrum display
typedef struct SpectrumDisplay
{
    // FFT state
//    arm_rfft_instance_f32           S;  // old, depricated FFT routine, do not use
	arm_cfft_instance_f32           C; // new FFT routine for complex FFT

//	arm_cfft_radix4_instance_f32    S_CFFT; // old, depricated FFT routine, do not use

    // Samples buffer
    //
    float32_t   FFT_Samples[FFT_IQ_BUFF_LEN];
//    float32_t   FFT_Windat[FFT_IQ_BUFF_LEN];
    float32_t   FFT_MagData[FFT_IQ_BUFF_LEN/2];
    q15_t   FFT_BkpData[FFT_IQ_BUFF_LEN];
    q15_t   FFT_DspData[FFT_IQ_BUFF_LEN];       // Rescaled and de-linearized display data
    q15_t   FFT_TempData[FFT_IQ_BUFF_LEN];
    float32_t   FFT_AVGData[FFT_IQ_BUFF_LEN/2];     // IIR low-pass filtered FFT buffer data

    // Current data ptr
    ulong   samp_ptr;

    // Skip flag for FFT processing
    ulong   skip_process;

    // Addresses of vertical grid lines on x axis
    ushort  vert_grid_id[7];

    // Addresses of horizontal grid lines on x axis
    ushort  horz_grid_id[5];

    // State machine current state
    uchar   state;

    // Init done flag
    uchar   enabled;

    // There is no data on screen;
    uint8_t     first_run;

    // Flag to indicate frequency change,
    // we need it to clear spectrum control
    uchar   dial_moved;

    // Variables used in spectrum display AGC
    //
    //
    float mag_calc;     // spectrum display rescale control

    uchar   magnify;    // TRUE if in magnify mode

    float   rescale_rate;   // this holds the rate at which the rescaling happens when the signal appears/disappears
    float   display_offset;                                 // "vertical" offset for spectral scope, gain adjust for waterfall
    float   agc_rate;       // this holds AGC rate for the Spectrum Display
    float   db_scale;       // scaling factor for dB/division
    ushort  wfall_line_update;  // used to set the number of lines per update on the waterfall
    float   wfall_contrast; // used to adjust the contrast of the waterfall display

    ushort  waterfall_colours[NUMBER_WATERFALL_COLOURS+1];  // palette of colors for waterfall data
    float32_t   wfall_temp[FFT_IQ_BUFF_LEN/2];                  // temporary holder for rescaling screen
    ushort  waterfall[SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL +16][FFT_IQ_BUFF_LEN/2];    // circular buffer used for storing waterfall data - remember to increase this if the waterfall is made larger!

    uchar   wfall_line;                                     // pointer to current line of waterfall data
    uchar   wfall_size;                 // vertical size of the waterfall
    uchar   wfall_height;
    uchar   wfall_ystart;


} SpectrumDisplay;

// Spectrum display
extern __IO   SpectrumDisplay      sd;

#endif
