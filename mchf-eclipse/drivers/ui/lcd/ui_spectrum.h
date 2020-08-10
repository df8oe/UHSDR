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
**  Licence:      GNU GPLv3                                                      **
************************************************************************************/

#include "uhsdr_board.h"
#include "uhsdr_types.h"
#include "audio_driver.h"
#include "arm_const_structs.h"
#include "ui_lcd_layouts.h"

typedef struct
{
    UiArea_t full;
    UiArea_t draw;
    UiArea_t title;
    UiArea_t scope;
    UiArea_t graticule;
    UiArea_t wfall;
} SpectrumAreas_t;


void UiSpectrum_Init(void);
void UiSpectrum_Clear(void);
void UiSpectrum_Redraw(void);
void UiSpectrum_WaterfallClearData(void);
void UiSpectrum_CalculateDisplayFilterBW(float32_t* width_pixel_, float32_t* left_filter_border_pos_);
void UiSpectrum_DisplayFilterBW(void);

void UiSpectrum_InitCwSnapDisplay (bool visible);
void UiSpectrum_ResetSpectrum(void);
uint16_t UiSprectrum_CheckNewGraticulePos(uint16_t new_y);

// Settings for dB/division for spectrum display
enum
{
    DB_DIV_UNUSED = 0,
    DB_DIV_5,
    DB_DIV_7,
    DB_DIV_10,
    DB_DIV_15,
    DB_DIV_20,
    S_1_DIV,
    S_2_DIV,
    S_3_DIV,
    SCOPE_SCALE_NUM
};

#define DB_DIV_ADJUST_MIN                   DB_DIV_5
#define DB_DIV_ADJUST_MAX                   S_3_DIV
#define DB_DIV_ADJUST_DEFAULT               DB_DIV_10


// Enumeration to select which waterfall palette to use
enum
{
    WFALL_GRAY = 0,
    WFALL_HOT_COLD,
    WFALL_RAINBOW,
    WFALL_BLUE,
    WFALL_GRAY_INVERSE,
    WFALL_COLOR_NUM
};
//
#define WATERFALL_COLOR_MIN                 0
#define WATERFALL_COLOR_MAX                 (WFALL_COLOR_NUM-1)
#define WATERFALL_COLOR_DEFAULT             WFALL_GRAY
//
#define WATERFALL_STEP_SIZE_MIN             1
#define WATERFALL_STEP_SIZE_MAX             5
#define WATERFALL_STEP_SIZE_DEFAULT         2
//
#define WATERFALL_OFFSET_MIN                60
#define WATERFALL_OFFSET_MAX                140
#define WATERFALL_OFFSET_DEFAULT            100
//
#define WATERFALL_CONTRAST_MIN              10
#define WATERFALL_CONTRAST_MAX              225
#define WATERFALL_CONTRAST_DEFAULT          120
//
#define WATERFALL_SPEED_MIN                 0
#define WATERFALL_SPEED_MAX                 30
#define WATERFALL_SPEED_DEFAULT 10
//
#define WATERFALL_NOSIG_ADJUST_MIN          10
#define WATERFALL_NOSIG_ADJUST_MAX          30
#define WATERFALL_NOSIG_ADJUST_DEFAULT      20

//
#define WATERFALL_SPEED_WARN        1//5
#define WATERFALL_SPEED_WARN1       3//9
//
// Constants for waterfall size settings
//
enum
{
    SPECTRUM_NORMAL=0,
    SPECTRUM_BIG
};

enum
{
    FFT_WINDOW_RECTANGULAR=0,
    FFT_WINDOW_COSINE,
    FFT_WINDOW_BARTLETT,
    FFT_WINDOW_WELCH,
    FFT_WINDOW_HANN,
    FFT_WINDOW_HAMMING,
    FFT_WINDOW_BLACKMAN,
    FFT_WINDOW_NUTTALL,
    FFT_WINDOW_MAX
};

enum
{
	Redraw_SCOPE=1,
	Redraw_WATERFALL=2
};


//#define FFT_WINDOW_DEFAULT                  FFT_WINDOW_BLACKMAN

#define SPECTRUM_SIZE_DEFAULT               SPECTRUM_NORMAL


// Spectrum scope operational constants

#define SPECTRUM_SCOPE_SPEED_MIN			0	// minimum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_MAX			30	// maximum spectrum scope speed
#define SPECTRUM_SCOPE_SPEED_DEFAULT		5
//
#define SPECTRUM_FILTER_MIN			1	// minimum filter setting
#define	SPECTRUM_FILTER_MAX			20	// maximum filter setting
#define SPECTRUM_FILTER_DEFAULT		4	// default filter setting
//
#define	SPECTRUM_SCOPE_AGC_MIN				1	// minimum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_MAX				50	// maximum spectrum scope AGC rate setting
#define	SPECTRUM_SCOPE_AGC_DEFAULT			25	// default spectrum scope AGC rate setting
//
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MIN		10
#define SPECTRUM_SCOPE_NOSIG_ADJUST_MAX		30
#define	SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT		20


#define	SPECTRUM_AGC_SCALING				25	// scaling factor by which the on-screen Spec. AGC Adj. is divided for adjustment.

#define	SCOPE_PREAMP_GAIN					1000//200	// amount of "amplification" in front of the FFT used for the Spectrum Scope and Waterfall used to overcome mathematical "noise floor"

#define INIT_SPEC_AGC_LEVEL					-80	// Initial offset for AGC level for spectrum/waterfall display

#define	NUMBER_WATERFALL_COLOURS			64		// number of colors in the waterfall table


#ifdef USE_DISP_320_240
    #define WATERFALL_MAX_LINES WATERFALL_MAX_SIZE
#endif

#ifdef USE_DISP_480_320
    #ifdef STM32F7
        #define WATERFALL_MAX_LINES WATERFALL_HEIGHT
    #endif
    #ifdef STM32F4
        #define WATERFALL_MAX_LINES (WATERFALL_HEIGHT/2)
    #endif
#endif

// FIXME: This is a temporary hack
// this needs to be as long at the longest scope width (in case of multiple resolutions)
// list highest resolution first

#ifdef USE_DISP_480_320
    #define SPECTRUM_WIDTH_MAX 480
#elif defined(USE_DISP_320_240)
    #define SPECTRUM_WIDTH_MAX 480
#endif

// Spectrum display
typedef struct SpectrumDisplay
{
    // Samples buffer
    float32_t   FFT_RingBuffer[FFT_IQ_BUFF_LEN];
    float32_t   FFT_Samples[FFT_IQ_BUFF_LEN];
    float32_t   FFT_MagData[SPEC_BUFF_LEN];
    float32_t   FFT_AVGData[SPEC_BUFF_LEN];     // IIR low-pass filtered FFT buffer data
    uint32_t    FFT_frequency; // center frequency of stored FFT
    // scope pixel data
    uint16_t    Old_PosData[SPECTRUM_WIDTH_MAX];

    // Current data ptr
    uint32_t   samp_ptr;
    volatile bool    reading_ringbuffer;
    // if the user level code wants to read the ring buffer
    // simply set this, and the audio driver will stop writing
    // to the buffer.  This means we loose some samples here but this is not a problem I would think.
    // if we don't want or should do this,
    // we have to add a little extra space to the ringbuffer (one audio driver sample block), which gives sufficient
    // space to add data to while we are reading.


    // Addresses of vertical grid lines on x axis
    ushort  vert_grid_id[SPECTRUM_SCOPE_GRID_VERT_COUNT-1];
    uint16_t vert_grid; // distance between vert grid lines

    // Addresses of horizontal grid lines on x axis
    ushort  horz_grid_id[16];
    uint8_t upper_horiz_gridline;  // how many grid lines to use

    // State machine current state
    uchar   state;

    // Init done flag
    uchar   enabled;

    // Variables used in spectrum display AGC
    uint8_t   magnify;          // 2^magnify == zoom factor, max is 5

    uint16_t    spec_len;
    uint16_t    fft_iq_len;
    const arm_cfft_instance_f32 * cfft_instance;

    float   display_offset;     // "vertical" offset for spectral scope, gain adjust for waterfall
    float   agc_rate;           // this holds AGC rate for the Spectrum Display
    float   db_scale;           // scaling factor for dB/division

    ushort  wfall_line_update;  // used to set the number of lines per update on the waterfall
    float   wfall_contrast;     // used to adjust the contrast of the waterfall display

    uint16_t waterfall_colours[NUMBER_WATERFALL_COLOURS+1];  // palette of colors for waterfall data
    // uint8_t (*waterfall)[SPECTRUM_WIDTH];	//pointer to waterfall memory
    uint8_t repeatWaterfallLine;				//line repeating count for waterfall size grater than number of data lines in waterfall array
    // uint8_t  waterfall[WATERFALL_MAX_LINES*SPECTRUM_WIDTH];    // circular buffer used for storing waterfall data - remember to increase this if the waterfall is made larger!
    uint8_t  waterfall[(WATERFALL_HEIGHT+10)*256];    // circular buffer used for storing waterfall data - remember to increase this if the waterfall is made larger!
    uint32_t waterfall_frequencies[(WATERFALL_HEIGHT+10)]; // we reserve hopefully enough frequency stores here. We store for each line in waterfall the center frequency of it.
    //uint8_t wfall_DrawDirection;	//0=upward (water fountain), 1=downward (real waterfall)
    uint32_t wfall_line;        // pointer to current line of waterfall data
    uint32_t wfall_size;        // vertical size of the waterfall data (number of stored fft results)
    //uint16_t wfall_disp_lines;        // vertical size of the waterfall on display
    uint32_t wfall_ystart;

    uint32_t scope_size;
    uint32_t scope_ystart;

    float32_t hz_per_pixel;        // how many Hertz is one pixel in the spectrum
    float32_t rx_carrier_pos;      // where is the current receiving frequency carrier (in pixels)
    float32_t marker_offset[SPECTRUM_MAX_MARKER];   // how is the current transmitting frequency carrier offset from rx carrier (in pixels)

    float32_t marker_pos[SPECTRUM_MAX_MARKER];      // where is the current transmitting frequency carrier (in pixels)
    uint16_t  marker_line_pos_prev[SPECTRUM_MAX_MARKER]; // previous x-axis location of carrier line in screen coordinates, 0xffff indicates off screen
    uint16_t  marker_num; // how many marker lines we have right now
    uint16_t  marker_num_prev; // how many marker lines we had last time

    uint32_t   scope_centre_grid_colour_active;    // active colour of the spectrum scope center grid line
    uint32_t   scope_grid_colour_active;   // active color of spectrum scope grid;

    uint16_t old_left_filter_border_pos;	//previous BW highlight left border
    uint16_t old_right_filter_border_pos;	//previous BW highlight right border
    uint8_t RedrawType;
    SpectrumAreas_t* Slayout;

} SpectrumDisplay;

// Spectrum display
extern SpectrumDisplay      sd;

#define MinimumScopeSize 24
#define MinimumWaterfallSize 16


#endif
