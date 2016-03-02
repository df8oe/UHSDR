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


void UiSpectrumInitSpectrumDisplay();
void UiSpectrumInitWaterfallDisplay();
void UiSpectrumClearDisplay();
void UiSpectrumReDrawWaterfall();
void UiSpectrumReDrawSpectrumDisplay();


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
#define	SPECTRUM_SCOPE_FILTER_MAX			10	// maximum filter setting
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
// Dependent on FFT samples,but should be less than control width!
#define SPECTRUM_WIDTH			256



#endif
