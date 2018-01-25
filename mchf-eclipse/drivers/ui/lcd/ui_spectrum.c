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
 **  Licence:      GNU GPLv3                                                      **
 ************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ui_spectrum.h"
#include "ui_lcd_hy28.h"
// For spectrum display struct
#include "audio_driver.h"
#include "ui_driver.h"
#include "ui_menu.h"
#include "waterfall_colours.h"
#include "radio_management.h"
#include "rtty.h"
#include "cw_decoder.h"
#include "audio_nr.h"
#include "psk.h"

#if defined(USE_DISP_480_320) || defined(USE_EXPERIMENTAL_MULTIRES)
#define USE_DISP_480_320_SPEC
#endif



typedef struct
{
    const int16_t SCOPE_GRID_VERT_COUNT;
    const int16_t SCOPE_GRID_HORIZ;
} pos_spectrum_display_t;


SpectrumAreas_t slayout;

// full area =      x[const]=58,                y[const]= 128,                          w[const]= 262,                  h[const]=94
// draw area =      x[const]=full_area.x +2,    y[const]=full_area.y + 2,               w[const]=full_area.w - (2 + 2), h[const]=full_area.h - (2 + 2)
// title area =     x[const]=draw_area.x,       y[const]=draw_area.y,                   w[const]=draw_area.w,           h[var]=big?0:16
// scope area =     x[const]=draw_area.x,       y[var]=title_area.y+title_area.h,       w[const]=draw_area.w,           h[var]=scope_disabled?0:(draw_area.h - title_area.h - graticule_area.h)/(wfall_disabled?1:2)
// graticule area = x[const]=draw_area.x,       y[var]=scope_area.y+scope_area.h,       w[const]=draw_area.w,           h[const]=16
// wfall area =     x[const]=draw_area.x,       y[var]=graticule_area.y + graticule.h,  w[const]=draw_area.w,           h[var]=wfall_disabled?0:(draw_area.h - title_area.h - graticule_area.h)/(scope_disabled?1:2)

/*
 * @brief Implements the full calculation of coordinates for a variable sized spectrum display
 * This algorithm can also be used to calculate the layout statically offline (we don't do this yet).
 */
void UiSpectrum_CalculateLayout(const bool is_big, const bool scope_enabled, const bool wfall_enabled, const UiArea_t* full_ptr, const uint16_t padding)
{
	sd.Slayout=&slayout;

    slayout.full.x = full_ptr->x;
    slayout.full.y = full_ptr->y;
    slayout.full.w = full_ptr->w;
    slayout.full.h = full_ptr->h;

    slayout.draw.x = slayout.full.x + padding;
    slayout.draw.y = slayout.full.y + padding;
    slayout.draw.w = slayout.full.w - 2*padding;
    slayout.draw.h = slayout.full.h - 2*padding;

    slayout.title.x = slayout.draw.x;
    slayout.title.y = slayout.draw.y;
    slayout.title.w = slayout.draw.w;
    slayout.title.h = is_big?0:16; // hide title if big

    slayout.graticule.x = slayout.draw.x;
    slayout.graticule.w = slayout.draw.w;
    slayout.graticule.h = 16;

    slayout.scope.x = slayout.draw.x;
    slayout.scope.y = slayout.title.y + slayout.title.h;
    slayout.scope.w = slayout.draw.w;

   // UiSpectrum_SetNewGraticulePosition(ts.graticulePowerupYpos);
    UiSpectrum_ResetSpectrum();

    slayout.scope.h = slayout.graticule.y - slayout.scope.y;

    slayout.wfall.x = slayout.draw.x;
    slayout.wfall.y = slayout.graticule.y + slayout.graticule.h;
    slayout.wfall.w = slayout.draw.w;
    slayout.wfall.h = slayout.draw.y+slayout.draw.h-slayout.wfall.y;
}

/*
//old Danilo's nice calculation
{
    slayout.full.x = full_ptr->x;
    slayout.full.y = full_ptr->y;
    slayout.full.w = full_ptr->w;
    slayout.full.h = full_ptr->h;

    slayout.draw.x = slayout.full.x + padding;
    slayout.draw.y = slayout.full.y + padding;
    slayout.draw.w = slayout.full.w - 2*padding;
    slayout.draw.h = slayout.full.h - 2*padding;

    slayout.title.x = slayout.draw.x;
    slayout.title.y = slayout.draw.y;
    slayout.title.w = slayout.draw.w;
    slayout.title.h = is_big?0:16; // hide title if big

    slayout.graticule.h = 16;

    slayout.scope.x = slayout.draw.x;
    slayout.scope.y = slayout.title.y + slayout.title.h;
    slayout.scope.w = slayout.draw.w;
    slayout.scope.h = scope_enabled?(slayout.draw.h - slayout.title.h - slayout.graticule.h)/(wfall_enabled?2:1) : 0;

    slayout.graticule.x = slayout.draw.x;
    slayout.graticule.y = slayout.scope.y + slayout.scope.h;
    slayout.graticule.w = slayout.draw.w;

    slayout.wfall.x = slayout.draw.x;
    slayout.wfall.y = slayout.graticule.y + slayout.graticule.h;
    slayout.wfall.w = slayout.draw.w;
    slayout.wfall.h = wfall_enabled?(slayout.draw.h - slayout.title.h - slayout.graticule.h)/(scope_enabled?2:1) : 0;
}
 */

//sets graticule position according to control bits (to default for particular case)
void UiSpectrum_ResetSpectrum()
{
	switch(ts.flags1&(FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED))
	{
	case FLAGS1_SCOPE_ENABLED:
		slayout.graticule.y=slayout.draw.y+slayout.draw.h-slayout.graticule.h;
		break;
	case FLAGS1_WFALL_ENABLED:
		slayout.graticule.y=slayout.draw.y+slayout.title.h;
		break;
	case (FLAGS1_SCOPE_ENABLED | FLAGS1_WFALL_ENABLED):
		slayout.graticule.y=UiSprectrum_CheckNewGraticulePos(ts.graticulePowerupYpos);
		break;
	default:
		break;
	}
}
uint16_t UiSprectrum_CheckNewGraticulePos(uint16_t new_y)
{
	if((new_y<sd.Slayout->draw.y+MinimumScopeSize))
	{
		new_y=sd.Slayout->draw.y+MinimumScopeSize;
	}
	if(new_y>(sd.Slayout->draw.y+sd.Slayout->draw.h-MinimumWaterfallSize-slayout.graticule.h))
	{
		new_y=sd.Slayout->draw.y+sd.Slayout->draw.h-MinimumWaterfallSize-slayout.graticule.h;
	}
	return new_y;
}

const pos_spectrum_display_t pos_spectrum_set[] =
{
#ifdef USE_DISP_320_240
        {
#if 0
                .WIDTH = 256,
                .DRAW_Y_TOP = POS_SPECTRUM_DRAW_Y_TOP,
                .NORMAL_HEIGHT = SPECTRUM_HEIGHT,
                .BIG_HEIGHT= (SPECTRUM_HEIGHT + SPEC_LIGHT_MORE_POINTS),
                .DRAW_X_LEFT= (POS_SPECTRUM_IND_X - 2),
                .DRAW_HEIGHT= (94),
                .DRAW_WIDTH= (262),
                .NORMAL_START_Y= (SPECTRUM_START_Y),
                .BIG_START_Y= (SPECTRUM_START_Y - SPEC_LIGHT_MORE_POINTS),
                .GRATICULE_Y = (POS_SPECTRUM_IND_Y + 60),
#endif
                // .GRID_VERT_START = POS_SPECTRUM_GRID_VERT_START,
                // .GRID_HORIZ_START = POS_SPECTRUM_GRID_HORIZ_START,
                // .SCOPE_GRID_VERT = SPECTRUM_SCOPE_GRID_VERT,
                .SCOPE_GRID_VERT_COUNT = SPECTRUM_SCOPE_GRID_VERT_COUNT,
                .SCOPE_GRID_HORIZ = SPECTRUM_SCOPE_GRID_HORIZ,
#if 0
                .IND_Y = POS_SPECTRUM_IND_Y,
                .IND_X = POS_SPECTRUM_IND_X,
                .START_X = POS_SPECTRUM_IND_X, // (SPECTRUM_START_X) seems to be a duplicate; used for waterfall left x
                .IND_W = POS_SPECTRUM_IND_W,
                .FREQ_BAR_Y = POS_SPECTRUM_FREQ_BAR_Y,
                .FREQ_BAR_H = POS_SPECTRUM_FREQ_BAR_H,
                .NORMAL_WATERFALL_START_Y = (SPECTRUM_START_Y + SPECTRUM_SCOPE_TOP_LIMIT),
                .NORMAL_WATERFALL_HEIGHT = (SPECTRUM_HEIGHT - SPECTRUM_SCOPE_TOP_LIMIT),
                .BIG_WATERFALL_START_Y = SPECTRUM_START_Y - WFALL_MEDIUM_ADDITIONAL,
                .BIG_WATERFALL_HEIGHT = SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL,
#endif
        },
#endif
#ifdef USE_DISP_480_320_SPEC
         {
                .SCOPE_GRID_HORIZ = 16, // SPECTRUM_SCOPE_GRID_HORIZ,
                .SCOPE_GRID_VERT_COUNT = 8, // SPECTRUM_SCOPE_GRID_VERT_COUNT,
        },
#endif
};

typedef enum
{
#ifdef USE_DISP_320_240
    RESOLUTION_320_240,
#endif
#ifdef USE_DISP_480_320_SPEC
    RESOLUTION_480_320,
#endif
} disp_resolution_t;

// in single resolution case we can set both of the to const, then the compiler will optimize
// it all memory access to the data away and the code is as performant as with all constant coordinates.
// const pos_spectrum_display_t* pos_spectrum = &pos_spectrum_set[0];
// const disp_resolution_t disp_resolution;
const pos_spectrum_display_t* pos_spectrum = &pos_spectrum_set[0];
disp_resolution_t disp_resolution;



// ------------------------------------------------
// Spectrum display public
SpectrumDisplay  __MCHF_SPECIALMEM       sd;
// this data structure is now located in the Core Connected Memory of the STM32F4
// this is highly hardware specific code. This data structure nicely fills the 64k with roughly 60k.
// If this data structure is being changed,  be aware of the 64k limit. See linker script arm-gcc-link.ld
//
// scaling factors for the various dB/division settings
//
#define DB_SCALING_5                        63.2456     // 5dB/division scaling
#define DB_SCALING_7                        42.1637     // 7.5dB/division scaling
#define DB_SCALING_10                       31.6228     // 10dB/division scaling
#define DB_SCALING_15                       21.0819     // 15dB/division scaling
#define DB_SCALING_20                       15.8114     // 20dB/division scaling
#define DB_SCALING_S1                       52.7046     // 1 S unit (6 dB)/division scaling
#define DB_SCALING_S2                       26.3523     // 2 S unit (12 dB)/division scaling
#define DB_SCALING_S3                       17.5682     // 3 S unit (18 dB)/division scaling
//


typedef struct
{
    float32_t value;
    const char* label;
} scope_scaling_info_t;

static const scope_scaling_info_t scope_scaling_factors[SCOPE_SCALE_NUM+1] =
{
        // scaling factors for the various dB/division settings
        { 0,                        "Waterfall      " }, // just a small trick, the scope will never use scaling index 0
        { DB_SCALING_5,             "SC(5dB/div)    " },
        { DB_SCALING_7,             "SC(7.5dB/div)  " },
        { DB_SCALING_10,            "SC(10dB/div)   " },
        { DB_SCALING_15,            "SC(15dB/div)   " },
        { DB_SCALING_20,            "SC(20dB/div)   " },
        { DB_SCALING_S1,            "SC(1S-Unit/div)" },
        { DB_SCALING_S2,            "SC(2S-Unit/div)" },
        { DB_SCALING_S3,            "SC(3S-Unit/div)" },
        { 0,                        "Dual (10dB/div)" }, // just a small trick, the scope will never use scaling index SCOPE_SCALE_NUM
};

static void     UiSpectrum_DrawFrequencyBar();
static void		UiSpectrum_CalculateDBm();

// FIXME: This is partially application logic and should be moved to UI and/or radio management
// instead of monitoring change, changes should trigger update of spectrum configuration (from pull to push)
static void UiSpectrum_UpdateSpectrumPixelParameters()
{
    static uint16_t old_magnify = 0xFF;
    static bool old_cw_lsb = false;
    static uint8_t old_dmod_mode = 0xFF;
    static uint8_t old_iq_freq_mode = 0xFF;
    static uint16_t old_cw_sidetone_freq = 0;
    static uint8_t old_digital_mode = 0xFF;

    static bool force_update = true;

    if (sd.magnify != old_magnify || force_update)
    {
        old_magnify = sd.magnify;
        sd.hz_per_pixel = IQ_SAMPLE_RATE_F/((1 << sd.magnify) * slayout.scope.w);     // magnify mode is on
        force_update = true;
    }

    if (ts.iq_freq_mode != old_iq_freq_mode  || force_update)
    {
        old_iq_freq_mode = ts.dmod_mode;
        force_update = true;

        if(!sd.magnify)     // is magnify mode on?
        {
            sd.rx_carrier_pos = slayout.scope.w/2 - 0.5 - (AudioDriver_GetTranslateFreq()/sd.hz_per_pixel);
        }
        else        // magnify mode is on
        {
            sd.rx_carrier_pos = slayout.scope.w/2 -0.5;                                // line is always in center in "magnify" mode
        }
    }
    if (ts.cw_lsb != old_cw_lsb || ts.cw_sidetone_freq != old_cw_sidetone_freq || ts.dmod_mode != old_dmod_mode || ts.digital_mode != old_digital_mode || force_update)
    {
        old_cw_lsb = ts.cw_lsb;
        old_cw_sidetone_freq = ts.cw_sidetone_freq;
        old_dmod_mode = ts.dmod_mode;
        old_digital_mode = ts.digital_mode;

        float32_t tx_vfo_offset = ((float32_t)(((int32_t)RadioManagement_GetTXDialFrequency() - (int32_t)RadioManagement_GetRXDialFrequency())/TUNE_MULT))/sd.hz_per_pixel;

        // FIXME: DOES NOT WORK PROPERLY IN SPLIT MODE
        sd.marker_num_prev = sd.marker_num;
        float32_t mode_marker_offset[SPECTRUM_MAX_MARKER];
        switch(ts.dmod_mode)
        {
        case DEMOD_CW:
            mode_marker_offset[0] =(ts.cw_lsb?-1.0:1.0)*((float32_t)ts.cw_sidetone_freq / sd.hz_per_pixel);
            sd.marker_num = 1;
            break;
        case DEMOD_DIGI:
        {
            float32_t mode_marker[SPECTRUM_MAX_MARKER];
            switch(ts.digital_mode)
            {
            case DigitalMode_FreeDV:
            	// 1500 +/- 625Hz
                mode_marker[0] = 875;
                mode_marker[1] = 2125;
                sd.marker_num = 2;
                break;
            case DigitalMode_RTTY:
                mode_marker[0] = 915; // Mark Frequency
                mode_marker[1] = mode_marker[0] + rtty_shifts[rtty_ctrl_config.shift_idx].value;
                sd.marker_num = 2;
                break;
            case DigitalMode_BPSK:
            	mode_marker[0] = 1000;
            	sd.marker_num = 1;
            	break;
            default:
                mode_marker[0] = 0;
                sd.marker_num = 1;
            }

            for (uint16_t idx; idx < sd.marker_num; idx++)
            {
                mode_marker_offset[idx] = (ts.digi_lsb?-1.0:1.0)*(mode_marker[idx] / sd.hz_per_pixel);
            }
        }
        break;
        default:
            mode_marker_offset[0] = 0;
            sd.marker_num = 1;
        }

        for (uint16_t idx; idx < sd.marker_num; idx++)
        {
            sd.marker_offset[idx] = tx_vfo_offset + mode_marker_offset[idx];
            sd.marker_pos[idx] = sd.rx_carrier_pos + sd.marker_offset[idx];
        }
        for (uint16_t idx = sd.marker_num; idx < SPECTRUM_MAX_MARKER; idx++)
        {
        	sd.marker_offset[idx] = 0;
        	sd.marker_pos[idx] = slayout.scope.w; // this is an invalid position out of screen
        }

    }
}

static void UiSpectrum_FFTWindowFunction(char mode)
{
    // Information on these windowing functions may be found on the internet - check the Wikipedia article "Window Function"
    // KA7OEI - 20150602
    const uint16_t  fft_iq_m1_half = (sd.fft_iq_len-1)/2;

    switch(mode)
    {
    case FFT_WINDOW_RECTANGULAR:	// No processing at all
        break;
    case FFT_WINDOW_COSINE:			// Sine window function (a.k.a. "Cosine Window").  Kind of wide...
        for(int i = 0; i < sd.fft_iq_len; i++)
        {
            sd.FFT_Samples[i] = arm_sin_f32((PI * (float32_t)i)/sd.fft_iq_len - 1) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_BARTLETT:		// a.k.a. "Triangular" window - Bartlett (or Fej?r) window is special case where demonimator is "N-1". Somewhat better-behaved than Rectangular
        for(int i = 0; i < sd.fft_iq_len; i++)
        {
            sd.FFT_Samples[i] = (1 - fabs(i - ((float32_t)fft_iq_m1_half))/(float32_t)fft_iq_m1_half) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_WELCH:			// Parabolic window function, fairly wide, comparable to Bartlett
        for(int i = 0; i < sd.fft_iq_len; i++)
        {
            sd.FFT_Samples[i] = (1 - ((i - ((float32_t)fft_iq_m1_half))/(float32_t)fft_iq_m1_half)*((i - ((float32_t)fft_iq_m1_half))/(float32_t)fft_iq_m1_half)) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_HANN:			// Raised Cosine Window (non zero-phase version) - This has the best sidelobe rejection of what is here, but not as narrow as Hamming.
        for(int i = 0; i < sd.fft_iq_len; i++)
        {
            sd.FFT_Samples[i] = 0.5 * (float32_t)((1 - (arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(sd.fft_iq_len-1)))) * sd.FFT_Samples[i]);
        }
        break;
    case FFT_WINDOW_HAMMING:		// Another Raised Cosine window - This is the narrowest with reasonably good sidelobe rejection.
        for(int i = 0; i < sd.fft_iq_len; i++)
        {
            sd.FFT_Samples[i] = (0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(sd.fft_iq_len-1)))) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_BLACKMAN:		// Approx. same "narrowness" as Hamming but not as good sidelobe rejection - probably best for "default" use.
        for(int i = 0; i < sd.fft_iq_len; i++)
        {
            sd.FFT_Samples[i] = (0.42659 - (0.49656*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)sd.fft_iq_len-1)) + (0.076849*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)sd.fft_iq_len-1))) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_NUTTALL:		// Slightly wider than Blackman, comparable sidelobe rejection.
        for(int i = 0; i < sd.fft_iq_len; i++)
        {
            sd.FFT_Samples[i] = (0.355768 - (0.487396*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)sd.fft_iq_len-1)) + (0.144232*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)sd.fft_iq_len-1)) - (0.012604*arm_cos_f32((6*PI*(float32_t)i)/(float32_t)sd.fft_iq_len-1))) * sd.FFT_Samples[i];
        }
        break;
    }

    float32_t gcalc = 1.0/ads.codec_gain_calc;                // Get gain setting of codec and convert to multiplier factor
    arm_scale_f32(sd.FFT_Samples,gcalc, sd.FFT_Samples, sd.fft_iq_len);

}

static void UiSpectrum_SpectrumTopBar_GetText(char* wfbartext)
{
    const char* lefttext;

    if(is_waterfallmode() && is_scopemode())           // dual waterfall
    {
        lefttext = scope_scaling_factors[SCOPE_SCALE_NUM].label; // a small trick, we use the empty location of index SCOPE_SCALE_NUM in the table
    }
    else if(is_waterfallmode())           //waterfall
    {
        lefttext = scope_scaling_factors[0].label; // a small trick, we use the empty location of index 0 in the table
    }
    else                                                // scope
    {
        lefttext = scope_scaling_factors[ts.spectrum_db_scale].label;
    }

    sprintf(wfbartext,"%s < Magnify %2ux >",lefttext, (1<<sd.magnify));
}

/**
 * tells us if x is on a vertical grid line. Called for all slayout.scope.w lines
 */
static bool UiSpectrum_Draw_XposIsOnVertGrid(const uint16_t x)
{
    bool repaint_v_grid = false;

#if 0
    int k;
    // Enumerate all saved x positions
    for(k = 0; k < 7; k++)
    {
        // Exit on match
        if(x == sd.vert_grid_id[k])
        {
            repaint_v_grid = true;
            break;
            // leave loop, found match
        }
        else if (sd.vert_grid_id[k]>x)
        {
            // leave loop, no need to look further, we passed the area where x could have hit a vertical grid line.
            // assume low to high order in sd.vert_grid_id
            break;
        }
    }
#else
    // this should be faster in case of power of 2 vert grid distance than the generic lookup code above
    // since the compiler should detect that module power 2 means just masking the higher bits
    // we have special check if we are on the rightmost pixel, this one is not a vertical line.
    return ((x + 1) % sd.vert_grid == 0) && (x != (slayout.scope.w - 1));
#endif
    return repaint_v_grid;
}

static void UiSpectrum_DrawLine(uint16_t x, uint16_t y_pos_prev, uint16_t y_pos, uint16_t clr)
{

    if(y_pos - y_pos_prev > 1) // && x !=(SPECTRUM_START_X + x_offset))
    { // plot line upwards
        UiLcdHy28_DrawStraightLine(x, y_pos_prev + 1, y_pos -  y_pos_prev,LCD_DIR_VERTICAL, clr);
    }
    else if (y_pos - y_pos_prev < -1) // && x !=(SPECTRUM_START_X + x_offset))
    { // plot line downwards
        UiLcdHy28_DrawStraightLine(x, y_pos, y_pos_prev - y_pos,LCD_DIR_VERTICAL, clr);
    }
    else
    {
        UiLcdHy28_DrawStraightLine(x, y_pos,1,LCD_DIR_VERTICAL, clr);
    }

}

static void UiSpectrum_ScopeStandard_UpdateVerticalDataLine(uint16_t x, uint16_t y_old_pos, uint16_t y_new_pos, uint16_t clr_scope, uint16_t clr_bgr, bool is_carrier_line)
{
    // normal scope
    if(y_old_pos > y_new_pos)
    {
        // is old line going to be overwritten by new line, anyway?
        UiLcdHy28_DrawStraightLine(x, y_new_pos,y_old_pos-y_new_pos, LCD_DIR_VERTICAL, clr_scope);
    }
    else if (y_old_pos < y_new_pos )
    {
        // is old line is longer than the new line?

        // repaint the vertical grid
        bool repaint_v_grid = UiSpectrum_Draw_XposIsOnVertGrid(x - slayout.scope.x);

        // we need to delete by overwriting with background color or grid color if we are on a vertical grid line
        uint16_t clr_bg =
                is_carrier_line?
                        sd.scope_centre_grid_colour_active
                        :
                        ( repaint_v_grid ? sd.scope_grid_colour_active : clr_bgr )
                        ;

        UiLcdHy28_DrawStraightLine(x,y_old_pos,y_new_pos-y_old_pos,LCD_DIR_VERTICAL,clr_bg);

        if (!repaint_v_grid && is_carrier_line == false)
        {
            // now we repaint the deleted points of the horizontal grid lines
            // but only if we are not on a vertical grid line, we already painted that in this case
            for(uint16_t k = 0; k < sd.upper_horiz_gridline && y_old_pos <= sd.horz_grid_id[k]; k++)
            { // we run if pixel positions are inside of deleted area (i.e. not lower than y_old_pos) or if there are no more lines

                if(y_old_pos <= sd.horz_grid_id[k] && sd.horz_grid_id[k] < y_new_pos )
                {
                    UiLcdHy28_DrawStraightLine(x,sd.horz_grid_id[k],1,LCD_DIR_HORIZONTAL, sd.scope_grid_colour_active);
                }
            }
        }
    }
}

static void UiSpectrum_CreateDrawArea()
{
	//Since we have now highlighted spectrum, the grid is to be drawn in UiSpectrum_DrawScope().
	//Here we only calculate positions of grid and write it to appropriate arrays
	//Also the vertical grid array is used for frequency labels in freq ruler
    UiSpectrum_UpdateSpectrumPixelParameters();
    //const bool is_scope_light = (ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE) != 0;
    // get grid colour of all but center line
    UiMenu_MapColors(ts.scope_grid_colour,NULL, &sd.scope_grid_colour_active);
    // Get color of center vertical line of spectrum scope
    UiMenu_MapColors(ts.spectrum_centre_line_colour,NULL, &sd.scope_centre_grid_colour_active);

    // Clear screen where frequency information will be under graticule
    UiLcdHy28_DrawFullRect(slayout.graticule.x, slayout.graticule.y, slayout.graticule.h, slayout.graticule.w, Black);    // Clear screen under spectrum scope by drawing a single, black block (faster with SPI!)

    //sd.wfall_DrawDirection=1;

// was used on 320x240, we may reactivate that at some point for all resolutions
#if 0
    // Frequency bar separator
    UiLcdHy28_DrawHorizLineWithGrad(pos_spectrum->IND_X,(pos_spectrum->IND_Y + pos_spectrum->IND_H - 20),pos_spectrum->IND_W,COL_SPECTRUM_GRAD);

    // Draw control left and right border
    UiLcdHy28_DrawStraightLineDouble((pos_spectrum->DRAW_X_LEFT),
    		(pos_spectrum->IND_Y - 20),
			(pos_spectrum->IND_H + 12),
			LCD_DIR_VERTICAL,
			//									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
			sd.scope_grid_colour_active);

    UiLcdHy28_DrawStraightLineDouble(	(pos_spectrum->IND_X + pos_spectrum->IND_W - 2),
    		(pos_spectrum->IND_Y - 20),
			(pos_spectrum->IND_H + 12),
			LCD_DIR_VERTICAL,
			//									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
			sd.scope_grid_colour_active);
#endif


    if(slayout.title.h != 0)		//don't draw text bar if there is no space allocated for it
    {
    	// Draw top band = grey box in which text is printed
    	for(int i = 0; i < 16; i++)
    	{
    		UiLcdHy28_DrawHorizLineWithGrad(slayout.title.x,slayout.title.y + i, slayout.title.w, COL_SPECTRUM_GRAD);
    	}

    	char bartext[34];

    	// Top band text - middle caption
    	UiSpectrum_SpectrumTopBar_GetText(bartext);

    	UiLcdHy28_PrintTextCentered(
    			slayout.title.x,
    			slayout.title.y + (slayout.title.h - UiLcdHy28_TextHeight(0))/2,
    			slayout.title.w,
				bartext,
				White,
				RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),
				0);
    }

    // Horizontal grid lines
    sd.upper_horiz_gridline = slayout.scope.h / pos_spectrum->SCOPE_GRID_HORIZ;

    // array must be filled from higher to the y coordinates
    // the lookup code for a match counts on this.
    for(int i = 0; i < sd.upper_horiz_gridline; i++)
    {
        // Save y position for grid draw and repaint
        sd.horz_grid_id[i] = (slayout.scope.y + slayout.scope.h  - ((i+1) * pos_spectrum->SCOPE_GRID_HORIZ));
    }

    // Vertical grid lines
    // array must be filled from low to higher x coordinates
    // the lookup code for a match counts on this.
    sd.vert_grid = slayout.scope.w / pos_spectrum->SCOPE_GRID_VERT_COUNT;
    for(int i = 1; i < pos_spectrum->SCOPE_GRID_VERT_COUNT; i++)
    {
        // Save x position for grid draw and repaint
        sd.vert_grid_id[i - 1] = (i*sd.vert_grid - 1);
    }


    if (is_waterfallmode() == true && ts.waterfall.speed == 0)
    {
        // print "disabled" in the middle of the screen if the waterfall or scope was disabled
        UiLcdHy28_PrintTextCentered(
                slayout.wfall.x,
                slayout.wfall.y + (slayout.scope.h - UiLcdHy28_TextHeight(0))/2 ,
                slayout.wfall.w,
                "DISABLED",
                Grey, Black,0);
    }

    if (is_scopemode() == true && ts.scope_speed == 0)
    {
        // print "disabled" in the middle of the screen if the waterfall or scope was disabled
        UiLcdHy28_PrintTextCentered(
                slayout.scope.x,
                slayout.scope.y + (slayout.scope.h - UiLcdHy28_TextHeight(0))/2 ,
                slayout.scope.w,
                "DISABLED",
                Grey, Black,0);
    }

    // Draw Frequency bar text after arrays with coordinates are set
    UiSpectrum_DrawFrequencyBar();
    //show highlighted filter bandwidth on the spectrum
    sd.old_left_filter_border_pos=slayout.scope.x;
    sd.old_right_filter_border_pos=slayout.scope.w+slayout.scope.x;
}

void UiSpectrum_Clear()
{
    UiLcdHy28_DrawFullRect(slayout.full.x, slayout.full.y, slayout.full.h, slayout.full.w, Black);	// Clear screen under spectrum scope by drawing a single, black block (faster with SPI!)
}

// This version of "Draw Scope" is revised from the original in that it interleaves the erasure with the drawing
// of the spectrum to minimize visible flickering  (KA7OEI, 20140916, adapted from original)
//
// 20141004 NOTE:  This has been somewhat optimized to prevent drawing vertical line segments that would need to be re-drawn:
//  - New lines that were shorter than old ones are NOT erased
//  - Line segments that are to be erased are only erased starting at the position of the new line segment.
//
//  This should reduce the amount of CGRAM access - especially via SPI mode - to a minimum.

static void    UiSpectrum_DrawScope(uint16_t *old_pos, float32_t *fft_new)
{

    // before accessing pixel parameters, request update according to configuration
    UiSpectrum_UpdateSpectrumPixelParameters();

    const bool is_scope_light = (ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE) != 0;
    const uint16_t spec_height_limit = sd.scope_size - 1;
    const uint16_t spec_top_y = sd.scope_ystart + sd.scope_size;

    uint32_t clr_scope, clr_scope_normal, clr_scope_fltr, clr_scope_fltrbg;
    uint16_t clr_bg;

    //calculations of bandwidth highlight parameters and colours
    float32_t filter_width_;                          // calculate width of BW highlight in pixels
    float32_t left_filter_border_pos_;				// first pixel of filter

    UiSpectrum_CalculateDisplayFilterBW(&filter_width_,&left_filter_border_pos_);
    uint16_t left_filter_border_pos=left_filter_border_pos_;
    uint16_t right_filter_border_pos=left_filter_border_pos_+filter_width_;

    if (right_filter_border_pos >= slayout.scope.w)
    {
        right_filter_border_pos = slayout.scope.w - 1;
    }

    //mapping the colours of highlighted bandwidth
    //foreground of highlighted bandwidth is one of predefined colours selected fromm array, so simply map it
    //background is the percentage of foreground, so we must disassemly the rgb data(16 bit) into seperate RGB channels,
    //then scale it and assembly to 16 bit
    UiMenu_MapColors(ts.scope_trace_colour, NULL, &clr_scope_normal); //foreground colour
    UiMenu_MapColors(ts.scope_trace_BW_colour, NULL, &clr_scope_fltr);//background colour of highlight
    uint16_t BWHbgr=ts.scope_backgr_BW_colour;
    BWHbgr<<=8;
    BWHbgr/=100;
    uint16_t colR=(clr_scope_fltr>>8)&0xf8;
    uint16_t colG=(clr_scope_fltr>>3)&0xfc;
    uint16_t colB=(clr_scope_fltr<<3)&0xf8;
    colR=(colR*BWHbgr)>>8;
    colG=(colG*BWHbgr)>>8;
    colB=(colB*BWHbgr)>>8;
    clr_scope_fltrbg=RGB(colR,colG,colB);	//background color of the active demodulation filter highlight

    left_filter_border_pos+=slayout.scope.x;		//left boundary of highlighted spectrum in absolute pixels
    right_filter_border_pos+=slayout.scope.x;		//right boundary of highlighted spectrum in absolute pixels

    if((sd.old_left_filter_border_pos!=left_filter_border_pos ) || (sd.old_right_filter_border_pos!=right_filter_border_pos ))
    {
    	//BW changed so we must refresh the highlighted spectrum
    	uint16_t x_start=sd.old_left_filter_border_pos;
    	uint16_t x_end=sd.old_right_filter_border_pos;
    	if(sd.old_left_filter_border_pos>left_filter_border_pos)
    	{
    		x_start=left_filter_border_pos;
    	}
    	if(sd.old_right_filter_border_pos<right_filter_border_pos)
    	{
    		x_end=right_filter_border_pos;
    	}

    	uint16_t xh;
    	for(xh=x_start;xh<=x_end;xh++)
    	{
            if((xh>=left_filter_border_pos)&&(xh<=right_filter_border_pos)) //BW highlight control
            {
            	clr_scope=clr_scope_fltr;
            	clr_bg=clr_scope_fltrbg;
            }
            else
            {
                clr_scope=clr_scope_normal;
                clr_bg=Black;
            }

        	if (is_scope_light)
        	{
        		UiSpectrum_DrawLine(xh, spec_top_y, spec_top_y-spec_height_limit, clr_bg);
        	}
        	else
        	{
        		UiSpectrum_ScopeStandard_UpdateVerticalDataLine(xh, spec_top_y-spec_height_limit, spec_top_y, clr_scope, clr_bg, false);
        	}
        	old_pos[xh-slayout.scope.x]=spec_top_y;
    	}

    	//causing the redraw of all marker lines
    	for (uint16_t idx = 0; idx < SPECTRUM_MAX_MARKER; idx++)
    	{
    		sd.marker_line_pos_prev[idx]=65535;
    	}


    }
    sd.old_left_filter_border_pos=left_filter_border_pos;
    sd.old_right_filter_border_pos=right_filter_border_pos;

    uint16_t marker_line_pos[SPECTRUM_MAX_MARKER];


    for (uint16_t idx = 0; idx < SPECTRUM_MAX_MARKER; idx++)
    {
        marker_line_pos[idx] = slayout.scope.x + sd.marker_pos[idx];

        // this is the tx carrier line, we redraw only if line changes place around,
        // init code must take care to reset prev position to 0xffff in order to get initialization done after clean start

        if (marker_line_pos[idx] != sd.marker_line_pos_prev[idx])
        {
            if (sd.marker_line_pos_prev[idx] < slayout.scope.x + slayout.scope.w)
            {
            	if((sd.marker_line_pos_prev[idx]>=left_filter_border_pos)&&(sd.marker_line_pos_prev[idx]<=right_filter_border_pos)) //BW highlight control
            	{
            		clr_bg=clr_scope_fltrbg;
            	}
            	else
            	{
            		clr_bg=Black;
            	}


                // delete old line if previously inside screen limits

                if(is_scope_light)
                {
                    UiLcdHy28_DrawStraightLine( sd.marker_line_pos_prev[idx],
                            spec_top_y - spec_height_limit,
                            spec_height_limit,
                            LCD_DIR_VERTICAL,
							clr_bg);
                }
                else
                {
                    UiSpectrum_ScopeStandard_UpdateVerticalDataLine(
                            sd.marker_line_pos_prev[idx],
                            spec_top_y - spec_height_limit /* old = max pos */ ,
                            spec_top_y /* new = min pos */,
                            clr_scope, clr_bg,	//TODO: add highlight color here
                            false);

                    // we erase the memory for this location, so that it is fully redrawn
                    if (sd.marker_line_pos_prev[idx] < slayout.scope.x + slayout.scope.w)
                    {
                        old_pos[sd.marker_line_pos_prev[idx] - slayout.scope.x] = spec_top_y;
                    }
                }
            }

            if (marker_line_pos[idx] < slayout.scope.x + slayout.scope.w)
            {

                // draw new line if inside screen limits

                UiLcdHy28_DrawStraightLine( marker_line_pos[idx],
                        spec_top_y - spec_height_limit,
                        spec_height_limit,
                        LCD_DIR_VERTICAL,
                        sd.scope_centre_grid_colour_active);

                if (is_scope_light == false)
                {
                    // we erase the memory for this location, so that it is fully redrawn
                    if (marker_line_pos[idx] < slayout.scope.x + slayout.scope.w)
                    {
                        old_pos[marker_line_pos[idx] - slayout.scope.x] = spec_top_y;
                    }
                }
            }

            // done, remember where line has been drawn
            sd.marker_line_pos_prev[idx] = marker_line_pos[idx];
        }
    }

    uint16_t marker_lines_togo = sd.marker_num;
    for(uint16_t x = slayout.scope.x, idx = 0; idx < slayout.scope.w; x++, idx++)
    {
        if((x>=left_filter_border_pos)&&(x<=right_filter_border_pos)) //BW highlight control
        {
        	clr_scope=clr_scope_fltr;
        	clr_bg=clr_scope_fltrbg;
        }
        else
        {
            clr_scope=clr_scope_normal;
            clr_bg=Black;
        }

        // averaged FFT data scaled to height and (if necessary) limited here to current max height
        uint16_t y_height = (fft_new[idx] < spec_height_limit ? fft_new[idx] : spec_height_limit);

        // Data to y position by subtraction from the lowest spectrum y,
        // since scope y goes from high to low coordinates if y goes from low to high
        // due to screen 0,0 reference being the top left corner.
        uint16_t y_new_pos  = spec_top_y - y_height;

        // we get the old y position value of last scope draw run
        // and remember the new position value for next round in same place
        uint16_t y_old_pos  = old_pos[idx];
        old_pos[idx] = y_new_pos;


        if (is_scope_light)
        {
            static uint16_t      y_new_pos_prev = 0, y_old_pos_prev = 0;
            // ATTENTION: CODE ONLY UPDATES THESE IF IN LIGHT SCOPE MODE !!!
            // pixel values from the previous column left to the current x position
            // these are static so that when we do the right half of the spectrum, we get the previous value from the left side of the screen

            // special case of first x position of spectrum, we don't have a left side neighbor
            if (idx == 0)
            {
                y_old_pos_prev = y_old_pos;
                y_new_pos_prev = y_new_pos;
            }

            // x position is not on vertical center line (the one that indicates the tx carrier frequency)
            // we draw a line if y_new_pos and the last drawn pixel (y_old_pos) are more than 1 pixel apart in the vertical axis
            // makes the spectrum display look more complete . . .
           // uint16_t clr_bg = Black;

            // TODO: we  could find out the lowest marker_line and do not process search before that line
            for (uint16_t marker_idx = 0; marker_lines_togo > 0 && marker_idx < sd.marker_num; marker_idx++)
            {
                if (x == marker_line_pos[marker_idx])
                {
                    clr_bg = sd.scope_centre_grid_colour_active;
                    marker_lines_togo--; // once we marked all, skip further tests;
                    break;
                }
            }
            UiSpectrum_DrawLine(x, y_old_pos_prev, y_old_pos, clr_bg);
            UiSpectrum_DrawLine(x, y_new_pos_prev, y_new_pos, clr_scope);

            // we are done, lets remember this information for next round
            y_new_pos_prev = y_new_pos;
            y_old_pos_prev = y_old_pos;

        }
        else
        {

            bool is_marker_line = false;
            for (uint16_t idx = 0; !(is_marker_line) && idx < sd.marker_num; idx++)
            {
                is_marker_line =  x == marker_line_pos[idx];
            }

            // we just draw our vertical line in a optimized fashion here
            // handles also the grid (re)drawing if necessary
            UiSpectrum_ScopeStandard_UpdateVerticalDataLine(x, y_old_pos, y_new_pos, clr_scope, clr_bg, is_marker_line);
        }
    }
}






/**
 * @brief init data strctures for both "Scope Display" and "Waterfall" Display
 */
static void UiSpectrum_InitSpectrumDisplayData()
{
	//init colour of markers
	UiMenu_MapColors(ts.spectrum_centre_line_colour,NULL, &sd.scope_centre_grid_colour_active);

    // Init publics
    sd.state 		= 0;
    sd.samp_ptr 	= 0;
    sd.enabled		= 0;
    ts.dial_moved	= 0;
    sd.RedrawType   = 0;

    switch(disp_resolution)
    {
#ifdef USE_DISP_320_240
    case RESOLUTION_320_240:
    sd.spec_len = 256;
    sd.fft_iq_len = 512;
    sd.cfft_instance = &arm_cfft_sR_f32_len256;
    break;
#endif
#ifdef USE_DISP_480_320_SPEC
    case RESOLUTION_480_320:
        sd.spec_len = 512;
        sd.fft_iq_len = 1024;
        sd.cfft_instance = &arm_cfft_sR_f32_len512;
        break;
#endif
    }


    sd.agc_rate = ((float32_t)ts.spectrum_agc_rate) / SPECTRUM_AGC_SCALING;	// calculate agc rate
    //
    sd.wfall_line_update = 0;		// init count used for incrementing number of lines for each vertical waterfall screen update
    //
    // load buffer containing waterfall colours
    //

    const uint16_t* wfall_scheme = NULL;

    switch(ts.waterfall.color_scheme)
    {
    case WFALL_HOT_COLD:
        wfall_scheme = &waterfall_cold_hot[0];
        break;
    case WFALL_RAINBOW:
        wfall_scheme = &waterfall_rainbow[0];
        break;
    case WFALL_BLUE:
        wfall_scheme = &waterfall_blue[0];
        break;
    case WFALL_GRAY_INVERSE:
        wfall_scheme = &waterfall_grey_inverse[0];
        break;
    case WFALL_GRAY:
    default:
        wfall_scheme =  &waterfall_grey[0];
        break;
    }

    memcpy(sd.waterfall_colours,wfall_scheme,sizeof(*wfall_scheme)* NUMBER_WATERFALL_COLOURS);

    // erase previous position memory
    // Load "top" color of palette (the 65th) with that to be used for the center grid color
    sd.waterfall_colours[NUMBER_WATERFALL_COLOURS] = sd.scope_centre_grid_colour_active;

    if (is_waterfallmode())
    {
        sd.db_scale = scope_scaling_factors[DB_DIV_10].value;
        // waterfall has fixed 10db scaling
    }
    else
    {
        if (ts.spectrum_db_scale >= SCOPE_SCALE_NUM)
        {
            ts.spectrum_db_scale = DB_DIV_ADJUST_DEFAULT;
        }
        sd.db_scale = scope_scaling_factors[ts.spectrum_db_scale].value;
    }

    // if we later scale the spectrum width from the fft width, we incorporate
    // the required negative gain for the downsampling here.
    // makes spectrum_width float32_t multiplications less per waterfall update

    if (sd.spec_len != slayout.scope.w)
    {
        sd.db_scale *= (float32_t)slayout.scope.w/(float32_t)sd.spec_len;
    }

    sd.scope_ystart = slayout.scope.y;
    sd.scope_size = slayout.scope.h;
    sd.wfall_ystart = slayout.wfall.y;
    sd.wfall_size = slayout.wfall.h;

    // now make sure we fit in
    // please note, this works only if we have enough memory for have the lines
    // otherwise we will reduce size of displayed waterfall
    if(sd.wfall_size * slayout.scope.w > sizeof(sd.waterfall))
    {
        //sd.doubleWaterfallLine = 1;


        if (sd.wfall_size/2 * slayout.scope.w > sizeof(sd.waterfall))
        {
            // we caculate how many lines we can do with the amount of memory
            // and adjust displayed line count accordingly.
            sd.wfall_size = sizeof(sd.waterfall)/(slayout.scope.w);
            // FIXME: Notify user of issue
            // if memory is too small even with doubled
            // lines
        }
        else
        {
            sd.wfall_size /= 2;
        }
    }
 /*   else
    {
        sd.repeatWaterfallLine = 0;
    }*/

    sd.repeatWaterfallLine = slayout.wfall.h/(sd.wfall_size);		//-1 for prewention of doubling lines for equal size

    //no need for this variable because we have slayout.wfall.h
    //sd.wfall_disp_lines = sd.wfall_size * (sd.doubleWaterfallLine==true? 2:1);


    for (uint16_t idx = 0; idx < slayout.scope.w; idx++)
    {
        sd.Old_PosData[idx] = sd.scope_ystart + sd.scope_size;
    }

    sd.wfall_contrast = (float)ts.waterfall.contrast / 100.0;		// calculate scaling for contrast

    for (uint16_t marker_idx = 0; marker_idx < SPECTRUM_MAX_MARKER; marker_idx++)
    {
        sd.marker_line_pos_prev[marker_idx] = 0xffff; // off screen
    }

    sd.enabled		= 1;
}

void UiSpectrum_WaterfallClearData()
{
    // this assume sd.watefall being an array, not a pointer to one!
    memset(sd.waterfall,0, sizeof(sd.waterfall));
    memset(sd.waterfall_frequencies,0,sizeof(sd.waterfall_frequencies));
}


static void UiSpectrum_DrawWaterfall()
{
    sd.wfall_line %= sd.wfall_size; // make sure that the circular buffer is clipped to the size of the display area

    // Contrast:  100 = 1.00 multiply factor:  125 = multiply by 1.25 - "sd.wfall_contrast" already converted to 100=1.00
    arm_scale_f32(sd.FFT_Samples, sd.wfall_contrast, sd.FFT_Samples, sd.spec_len);


    UiSpectrum_UpdateSpectrumPixelParameters(); // before accessing pixel parameters, request update according to configuration


    uint16_t marker_line_pixel_pos[SPECTRUM_MAX_MARKER];
    for (uint16_t idx = 0; idx < sd.marker_num; idx++)
    {
        marker_line_pixel_pos[idx] = sd.marker_pos[idx];
    }

    // After the above manipulation, clip the result to make sure that it is within the range of the palette table
    //for(uint16_t i = 0; i < sd.spec_len; i++)
    uint8_t  * const waterfallline_ptr = &sd.waterfall[sd.wfall_line*slayout.wfall.w];

    for(uint16_t i = 0; i < slayout.wfall.w; i++)
    {
        if(sd.FFT_Samples[i] >= NUMBER_WATERFALL_COLOURS)   // is there an illegal color value?
        {
            sd.FFT_Samples[i] = NUMBER_WATERFALL_COLOURS - 1;   // yes - clip it
        }

        waterfallline_ptr[i] = sd.FFT_Samples[i]; // save the manipulated value in the circular waterfall buffer
    }

    sd.waterfall_frequencies[sd.wfall_line] = sd.FFT_frequency;


    // Draw lines from buffer
    sd.wfall_line++;        // bump to the next line in the circular buffer for next go-around

    uint16_t lptr = sd.wfall_line;      // get current line of "bottom" of waterfall in circular buffer

    sd.wfall_line_update++;                                 // update waterfall line count
    sd.wfall_line_update %= ts.waterfall.vert_step_size;    // clip it to number of lines per iteration

    if(!sd.wfall_line_update)                               // if it's count is zero, it's time to move the waterfall up
    {
    	//if(sd.wfall_DrawDirection==1)
    	//{
    	    // can't use modulo here, doesn't work if we use uint16_t,
    	    // since it 0-1 == 65536 and not -1 (it is an unsigned integer after all)
    	    lptr = lptr?lptr-1 : sd.wfall_size-1;
    	//}

        lptr %= sd.wfall_size;      // do modulus limit of spectrum high

        // set up LCD for bulk write, limited only to area of screen with waterfall display.  This allow data to start from the
        // bottom-left corner and advance to the right and up to the next line automatically without ever needing to address
        // the location of any of the display data - as long as we "blindly" write precisely the correct number of pixels per
        // line and the number of lines.



        //UiLcdHy28_BulkPixel_OpenWrite(slayout.wfall.x, slayout.wfall.w, (sd.wfall_ystart), sd.wfall_disp_lines);
        UiLcdHy28_BulkPixel_OpenWrite(slayout.wfall.x, slayout.wfall.w, slayout.wfall.y, slayout.wfall.h);

        uint16_t spectrum_pixel_buf[slayout.wfall.w];

        const int32_t cur_center_hz = sd.FFT_frequency;

        //uint16_t lcnt = 0;
        //for(uint16_t lcnt = 0;lcnt < sd.wfall_size; lcnt++)                 // set up counter for number of lines defining height of waterfall
        for(uint16_t lcnt = 0;lcnt < slayout.wfall.h;)                 // set up counter for number of lines defining height of waterfall
        {
            uint8_t  * const waterfallline_ptr = &sd.waterfall[lptr*slayout.wfall.w];


            const int32_t line_center_hz = sd.waterfall_frequencies[lptr];

            // if our old_center is lower than cur_center_hz -> find start idx in waterfall_line, end_idx is line end, and pad with black pixels;
            // if our old_center is higher than cur_center_hz -> find start pixel x in end_idx is line end, first pad with black pixels until this point and then use pixel buffer;
            // if identical -> well, no padding.
            const int32_t diff_centers = (line_center_hz - cur_center_hz);
            int32_t offset_pixel = diff_centers/sd.hz_per_pixel;
            uint16_t pixel_start, pixel_count, left_padding_count, right_padding_count;


            // here we actually create a single line pixel by pixel.

            if (offset_pixel >= slayout.wfall.w || offset_pixel <= -slayout.wfall.w)
            {
                offset_pixel = slayout.wfall.w-1;
            }
            if (offset_pixel <= 0)
            {
                                // we have to start -offset_pixel later and then pad with black
                left_padding_count = 0;
                pixel_start = -offset_pixel;
                right_padding_count = -offset_pixel;
                pixel_count = slayout.wfall.w + offset_pixel;
            } else
            {
                // we to start with offset_pixel black padding  and then draw the pixels until we reach spectrum width
                left_padding_count = offset_pixel;
                pixel_start = 0;
                right_padding_count = 0;
                pixel_count = slayout.wfall.w - offset_pixel;
            }


            uint16_t* pixel_buf_ptr = &spectrum_pixel_buf[0];
            // fill from the left border with black pixels
            for(uint16_t i = 0; i < left_padding_count; i++)
            {
                *pixel_buf_ptr++ = Black;
            }

            for(uint16_t idx = pixel_start, i = 0; i < pixel_count; i++,idx++)
            {
                *pixel_buf_ptr++ = sd.waterfall_colours[waterfallline_ptr[idx]];    // write to memory using waterfall color from palette
            }

            // fill to the right border with black pixels
            for(uint16_t i = 0; i < right_padding_count; i++)
            {
                *pixel_buf_ptr++ = Black;
            }

            for (uint16_t idx = 0; idx < sd.marker_num; idx ++)
            {
                // Place center line marker on screen:  Location [64] (the 65th) of the palette is reserved is a special color reserved for this
                if (marker_line_pixel_pos[idx] < slayout.wfall.w)
                {
                    spectrum_pixel_buf[marker_line_pixel_pos[idx]] = sd.waterfall_colours[NUMBER_WATERFALL_COLOURS];
                }
            }


            //UiLcdHy28_BulkPixel_PutBuffer(spectrum_pixel_buf, slayout.wfall.w);

            for(uint8_t doubleLine=0;doubleLine<sd.repeatWaterfallLine+1;doubleLine++)
            {
                UiLcdHy28_BulkPixel_PutBuffer(spectrum_pixel_buf, slayout.wfall.w);
                lcnt++;
                if(lcnt==slayout.wfall.h)	//preventing the window overlap if doubling oversize the display window
                {
                	break;
                }
            }
            lptr = lptr?lptr-1 : sd.wfall_size-1;
            /*
            // point to next/prev line in circular display buffer:
            if(sd.wfall_DrawDirection==1)
            {
                lptr = lptr?lptr-1 : sd.wfall_size-1;
            }
            else
            {
            	lptr++;                         //moving upward (water fountain, "normal" in 320x240)
            }*/
            lptr %= sd.wfall_size;              // clip to display height

        }
        UiLcdHy28_BulkPixel_CloseWrite();                   // we are done updating the display - return to normal full-screen mode
    }

}

static float32_t  UiSpectrum_ScaleFFTValue(const float32_t value, float32_t* min_p)
{
    float32_t sig = sd.display_offset + log10f_fast(value) * sd.db_scale;     // take FFT data, do a log10 and multiply it to scale 10dB (fixed)
    // apply "AGC", vertical "sliding" offset (or brightness for waterfall)

    if (sig < *min_p)
    {
        *min_p = sig;
    }

    return  (sig < 1)? 1 : sig;
}

static void UiSpectrum_ScaleFFT(float32_t dest[], float32_t source[], float32_t* min_p )
{
    // not an in-place algorithm
    assert(dest != source);

    for(uint16_t i = 0; i < (sd.spec_len/2); i++)
    {
        dest[sd.spec_len - i - 1] = UiSpectrum_ScaleFFTValue(source[i + sd.spec_len/2], min_p);
        // take FFT data, do a log10 and multiply it to scale 10dB (fixed)
        // apply "AGC", vertical "sliding" offset (or brightness for waterfall)
    }


    for(uint16_t i = (sd.spec_len/2); i < (sd.spec_len); i++)
    {
        // build right half of spectrum data
        dest[sd.spec_len - i - 1] = UiSpectrum_ScaleFFTValue(source[i - sd.spec_len/2], min_p);
        // take FFT data, do a log10 and multiply it to scale 10dB (fixed)
        // apply "AGC", vertical "sliding" offset (or brightness for waterfall)
    }

}

/**
 * @brief simple algorithm to scale down in place to a fractional scale
 * Please note, that there is no gain correction in this algorithm,
 * We rely on the gain correction taking place elsewhere!
 * required gain correction is (float32_t)to_len/(float32_t)from_len
 */
static void UiSpectrum_ScaleFFT2SpectrumWidth(float32_t samples[], uint16_t from_len, uint16_t to_len)
{

    assert(from_len >= to_len);

    const float32_t full_amount = (float32_t)from_len/(float32_t)to_len;


    // init loop values
    float32_t amount = full_amount;
    uint16_t idx_new = 0;
    uint16_t idx_old = 0;
    float32_t value = 0;


    // iterate over both arrays from index 0 towards (to_len-1) or (from_len-1)
    do
    {
        while (amount >=1)
        {
            value += samples[idx_old];
            idx_old++;
            amount-= 1.0;
        }

        float32_t for_next = (1-amount) * samples[idx_old];

        value += samples[idx_old] - for_next;
        samples[idx_new] = value;
        idx_new++;

        if (idx_new < to_len)
        {
            value  = for_next;
            idx_old++;
            amount = full_amount - (1-amount);
        }
    }
    while(idx_new < to_len);
}

// Spectrum Display code rewritten by C. Turner, KA7OEI, September 2014, May 2015
// Waterfall Display code written by C. Turner, KA7OEI, May 2015 entirely from "scratch"
// - which is to say that I did not borrow any of it
// from anywhere else, aside from keeping some of the general functions found in "Case 1".
/**
 * @briefs implement a staged calculation and drawing of the spectrum scope / waterfall
 * should not be called directly, go through UiSpectrum_Redraw which implements a rate limiter
 * it relies on the audio driver implementing the first stage of data collection.
 */
static void UiSpectrum_RedrawSpectrum()
{
    // Process implemented as state machine
    switch(sd.state)
    {
    case 0:
        sd.reading_ringbuffer = true;
        __DSB();
        // we make sure the interrupt sees this variable value by ensure all memory operations have been done
        // after this point
        arm_copy_f32(&sd.FFT_RingBuffer[sd.samp_ptr],&sd.FFT_Samples[0],sd.fft_iq_len-sd.samp_ptr);
        arm_copy_f32(&sd.FFT_RingBuffer[0],&sd.FFT_Samples[sd.fft_iq_len-sd.samp_ptr],sd.samp_ptr);
        sd.reading_ringbuffer = false;
        sd.state++;
        break;

    // Apply gain to collected IQ samples and then do FFT
    case 1:		// Scale input according to A/D gain and apply Window function
    {
    	//        UiSpectrum_FFTWindowFunction(ts.fft_window_type);		// do windowing function on input data to get less "Bin Leakage" on FFT data
        // fixed window type to Hann Window, because it provides excellent bin leakage behaviour AND
    	// it corresponds very well with the coefficients in the quadratic interpolation algorithm that is used
    	// for the SNAP carrier function
    	UiSpectrum_FFTWindowFunction(FFT_WINDOW_HANN);

        sd.state++;
        break;
    }
    case 2:		// Do FFT and calculate complex magnitude
    {
        arm_cfft_f32(sd.cfft_instance, sd.FFT_Samples,0,1);	// Do FFT
        // Calculate magnitude
        arm_cmplx_mag_f32( sd.FFT_Samples, sd.FFT_MagData ,sd.spec_len);
        // FIXME:

        // just for debugging purposes
        // display the spectral noise reduction bin gain values in the second 64 pixels of the spectrum display
        if((ts.dsp_active & DSP_NR_ENABLE) && NR.gain_display != 0)
        {
        	if(NR.gain_display == 1)
        	{
        	for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
        	{
        		sd.FFT_MagData[(NR_FFT_L / 2 - 1) - bindx] = NR.Hk[bindx] * 150.0;
        	}
        	}
        	else
        	if(NR.gain_display == 2)
        	{
            	for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
            	{
            		sd.FFT_MagData[(NR_FFT_L / 2 - 1) - bindx] = NR2.long_tone_gain[bindx] * 150.0;
            	}
        	}
        	else
        	if(NR.gain_display == 3)
        	{
            	for(int bindx = 0; bindx < NR_FFT_L / 2; bindx++)
            	{
            		sd.FFT_MagData[(NR_FFT_L / 2 - 1) - bindx] = NR.Hk[bindx] * NR2.long_tone_gain[bindx] * 150.0;
            	}
        	}
        	// set all other pixels to a low value
        	for(int bindx = NR_FFT_L / 2; bindx < sd.spec_len; bindx++)
        	{
        		sd.FFT_MagData[bindx] = 10.0;
        	}
        }

        sd.state++;
        break;
    }

    //  Low-pass filter amplitude magnitude data
    case 3:
    {
        float32_t filt_factor = 1/(float)ts.spectrum_filter;		// use stored filter setting inverted to allow multiplication

        if(ts.dial_moved)
        {
            ts.dial_moved = 0;	// Dial moved - reset indicator
            UiSpectrum_DrawFrequencyBar();	// redraw frequency bar on the bottom of the display
        }

        arm_scale_f32(sd.FFT_AVGData, filt_factor, sd.FFT_Samples, sd.spec_len);	// get scaled version of previous data
        arm_sub_f32(sd.FFT_AVGData, sd.FFT_Samples, sd.FFT_AVGData, sd.spec_len);	// subtract scaled information from old, average data
        arm_scale_f32(sd.FFT_MagData, filt_factor, sd.FFT_Samples, sd.spec_len);	// get scaled version of new, input data
        arm_add_f32(sd.FFT_Samples, sd.FFT_AVGData, sd.FFT_AVGData, sd.spec_len);	// add portion new, input data into average

        for(uint32_t i = 0; i < sd.spec_len; i++)	 		// guarantee that the result will always be >= 0
        {
            if(sd.FFT_AVGData[i] < 1)
            {
                sd.FFT_AVGData[i] = 1;
            }
        }

        UiSpectrum_CalculateDBm();

        sd.state++;
        break;
    }

    // De-linearize and normalize display data and do AGC processing
    case 4:
    {
        float32_t	min1=100000;
        // De-linearize data with dB/division
        // Transfer data to the waterfall display circular buffer, putting the bins in frequency-sequential order!
        // TODO: if we would use a different data structure here (e.g. q15), we could speed up collection of enough samples in driver
        // we could let it run as soon as last FFT_Samples read has been done here
        UiSpectrum_ScaleFFT(sd.FFT_Samples,sd.FFT_AVGData,&min1);

        if (sd.spec_len != slayout.scope.w)
        {
            // in place downscaling (!)
            UiSpectrum_ScaleFFT2SpectrumWidth(sd.FFT_Samples,sd.spec_len, slayout.scope.w);
        }

        // Adjust the sliding window so that the lowest signal is always black
        sd.display_offset -= sd.agc_rate*min1/5;

        sd.state++;
        break;
    }
    case 5:	// rescale waterfall horizontally, apply brightness/contrast, process pallate and put vertical line on screen, if enabled.
    {
    	if(sd.RedrawType&Redraw_SCOPE)
    	{
    		UiSpectrum_DrawScope(sd.Old_PosData, sd.FFT_Samples);
    	}

    	if(sd.RedrawType&Redraw_WATERFALL)
    	{
    		UiSpectrum_DrawWaterfall();
    	}

    	if (sd.RedrawType != 0)
    	{
    	    sd.RedrawType=0;
    	    sd.state = 0;
    	}
        break;

    }

    default:
        sd.state = 0;
        break;
    }
}

/**
 * @brief Initialize data and display for spectrum display
 */
void UiSpectrum_Init()
{

#ifdef USE_EXPERIMENTAL_MULTIRES
    if (disp_resolution == RESOLUTION_480_320)
    {
        disp_resolution = RESOLUTION_320_240;
        pos_spectrum = &pos_spectrum_set[0];
    }
    else
    {
        disp_resolution = RESOLUTION_480_320;
        pos_spectrum = &pos_spectrum_set[1];
    }
    UiSpectrum_WaterfallClearData();
#endif


    switch(disp_resolution)
    {
#ifdef USE_DISP_480_320
    case RESOLUTION_480_320:
    {
        const UiArea_t area_480_320 = { .x = 0, .y = 110, .w = 480, .h = 176 };
        UiSpectrum_CalculateLayout(ts.spectrum_size == SPECTRUM_BIG, is_scopemode(), is_waterfallmode(), &area_480_320, 0);
        break;
    }
#endif
#ifdef USE_DISP_320_240
    case RESOLUTION_320_240:
        {
            const UiArea_t area_320_240 = { .x = 58, .y = 128, .w = 260, .h = 94 };
            UiSpectrum_CalculateLayout(ts.spectrum_size == SPECTRUM_BIG, is_scopemode(), is_waterfallmode(), &area_320_240, 2);
            break;
        }
#endif
    }

    UiSpectrum_InitSpectrumDisplayData();
    UiSpectrum_Clear();         // clear display under spectrum scope
    UiSpectrum_CreateDrawArea();
    UiSpectrum_DisplayFilterBW();	// Update on-screen indicator of filter bandwidth
}
/**
 * @brief Calculate parameters for display filter bar. This function is used also for spectrum BW highlight.
 */
void UiSpectrum_CalculateDisplayFilterBW(float32_t* width_pixel_, float32_t* left_filter_border_pos_)
{

    const FilterPathDescriptor* path_p = &FilterPathInfo[ts.filter_path];
    const FilterDescriptor* filter_p = &FilterInfo[path_p->id];
    const float32_t width = filter_p->width;
    const float32_t offset = path_p->offset!=0 ? path_p->offset : width/2;

    UiSpectrum_UpdateSpectrumPixelParameters(); // before accessing pixel parameters, request update according to configuration

    float32_t width_pixel = width/sd.hz_per_pixel;                          // calculate width of line in pixels

    float32_t offset_pixel = offset/sd.hz_per_pixel;                            // calculate filter center frequency offset in pixels

    float32_t left_filter_border_pos;

    if(RadioManagement_UsesBothSidebands(ts.dmod_mode))     // special cases - AM, SAM and FM, which are double-sidebanded
    {
        left_filter_border_pos = sd.rx_carrier_pos - width_pixel;                   // line starts "width" below center
        width_pixel *= 2;                       // the width is double in AM & SAM, above and below center
    }
    else if(RadioManagement_LSBActive(ts.dmod_mode))    // not AM, but LSB:  calculate position of line, compensating for both width and the fact that SSB/CW filters are not centered
    {
        left_filter_border_pos = sd.rx_carrier_pos - (offset_pixel + (width_pixel/2));  // if LSB it will be below zero Hz
    }
    else                // USB mode
    {
        left_filter_border_pos = sd.rx_carrier_pos + (offset_pixel - (width_pixel/2));          // if USB it will be above zero Hz
    }

    if(left_filter_border_pos < 0) // prevents line to leave left border
     {
         width_pixel = width_pixel + left_filter_border_pos;
         left_filter_border_pos = 0;
     }

     if(left_filter_border_pos + width_pixel > slayout.scope.w) // prevents line to leave right border
     {
         width_pixel = (float32_t)slayout.scope.w - left_filter_border_pos;
     }

    *width_pixel_=width_pixel;
    *left_filter_border_pos_=left_filter_border_pos;
}

/**
 * @brief Show a small horizontal line below the spectrum to indicate the rx passband of the currently active filter
 */
void UiSpectrum_DisplayFilterBW()
{

    if(ts.menu_mode == 0)
    {// bail out if in menu mode
        // Update screen indicator - first get the width and center-frequency offset of the currently-selected filter

        float32_t width_pixel;                          // calculate width of line in pixels
        float32_t left_filter_border_pos;
        UiSpectrum_CalculateDisplayFilterBW(&width_pixel,&left_filter_border_pos);

        uint16_t pos_bw_y = slayout.graticule.y + slayout.graticule.h - 2;

        UiLcdHy28_DrawStraightLineDouble(slayout.graticule.x, pos_bw_y, slayout.graticule.w, LCD_DIR_HORIZONTAL, Black);
        uint32_t clr;
        // get color for line
        UiMenu_MapColors(ts.filter_disp_colour,NULL, &clr);
        // draw line
        UiLcdHy28_DrawStraightLineDouble(((float32_t)slayout.graticule.x + roundf(left_filter_border_pos)), pos_bw_y, roundf(width_pixel), LCD_DIR_HORIZONTAL, clr);
    }
}

/**
 * @brief Draw the frequency information on the frequency bar at the bottom of the spectrum scope based on the current frequency
 */
static void UiSpectrum_DrawFrequencyBar()
{

    char    txt[16];

    UiSpectrum_UpdateSpectrumPixelParameters();

    if (ts.spectrum_freqscale_colour != SPEC_BLACK)     // don't bother updating frequency scale if it is black (invisible)!
    {
        float32_t grat = 6.0f / (float32_t)(1 << sd.magnify);

        // This function draws the frequency bar at the bottom of the spectrum scope, putting markers every at every graticule and the full frequency
        // (rounded to the nearest kHz) in the "center".  (by KA7OEI, 20140913)

        // get color for frequency scale
        uint32_t  clr;
        UiMenu_MapColors(ts.spectrum_freqscale_colour,NULL, &clr);

        float32_t freq_calc = (RadioManagement_GetRXDialFrequency() + (ts.dmod_mode == DEMOD_CW?RadioManagement_GetCWDialOffset():0))/TUNE_MULT;      // get current tune frequency in Hz

        if (sd.magnify == 0)
        {
            freq_calc += AudioDriver_GetTranslateFreq();
            // correct for display center not being RX center frequency location
        }
        if(sd.magnify < 3)
        {
            freq_calc = roundf(freq_calc/1000); // round graticule frequency to the nearest kHz
        }
        else if (sd.magnify < 5)
        {
            freq_calc = roundf(freq_calc/100) / 10; // round graticule frequency to the nearest 100Hz
        }
        else if(sd.magnify == 5)
        {
            freq_calc = roundf(freq_calc/50) / 20; // round graticule frequency to the nearest 50Hz
        }


        int16_t centerIdx = -100; // UiSpectrum_GetGridCenterLine(0);

        uint16_t idx2pos[pos_spectrum->SCOPE_GRID_VERT_COUNT+1];

        // remainder of frequency/graticule markings
        for(int i=1;i<pos_spectrum->SCOPE_GRID_VERT_COUNT;i++)
        {
            idx2pos[i]=sd.vert_grid_id[i-1];
        }

        idx2pos[0]=0;
        idx2pos[pos_spectrum->SCOPE_GRID_VERT_COUNT]=slayout.scope.w-1;

        if(sd.magnify > 2)
        {
            idx2pos[pos_spectrum->SCOPE_GRID_VERT_COUNT-1]-=9;
        }

       // FIXME: This code expect 8 vertical lines)
        for (int idx = -4; idx < 5; idx += (sd.magnify < 2) ? 1 : 2 )
        {
            int pos = idx2pos[idx+4];
            const uint8_t graticule_font = 4;
            const uint16_t number_width = UiLcdHy28_TextWidth("    ",graticule_font);
            const uint16_t pos_number_y = (slayout.graticule.y +  (slayout.graticule.h - UiLcdHy28_TextHeight(graticule_font))/2);
            if (idx != centerIdx)
            {
                char *c;
                if(sd.magnify < 3)
                {
                    snprintf(txt,16, "%02lu", ((uint32_t)(freq_calc+(idx*grat)))%100);   // build string for middle-left frequency (1khz precision)
                    c = txt;  // point at 2nd character from the end
                }
                else
                {
                    float32_t disp_freq = freq_calc+(idx*grat);
                    int bignum = disp_freq;
                    int smallnum = roundf((disp_freq-bignum)*100);
                    snprintf(txt,16, " %u.%02u", bignum, smallnum);   // build string for middle-left frequency (10Hz precision)
                    c = &txt[strlen(txt)-4];  // point at 5th character from the end
                }
                if (idx == -4) // left border
                {
                    UiLcdHy28_PrintText( slayout.graticule.x + pos, pos_number_y,c,clr,Black,graticule_font);
                }
                else if (idx == 4) // right border
                {
                    UiLcdHy28_PrintTextRight( slayout.graticule.x + pos, pos_number_y,c,clr,Black,graticule_font);
                }

                else
                {
                    UiLcdHy28_PrintTextCentered(slayout.graticule.x +  pos - number_width/2,pos_number_y, number_width,c,clr,Black,graticule_font);
                }
            }
        }
    }
}



void UiSpectrum_Redraw()
{
    // Only in RX mode and NOT while powering down or in menu mode or if displaying memory information
    if (
            (ts.txrx_mode == TRX_MODE_RX)
            && (ts.menu_mode == false)
            && (ts.powering_down == false)
            && (ts.mem_disp == false)
            && (sd.enabled == true)
            && (ts.lcd_blanking_flag == false)
			&& (ts.SpectrumResize_flag == false)
    )
    {
        if(ts.waterfall.scheduler == 0 && is_waterfallmode())   // is waterfall mode enabled?
        {
            if(ts.waterfall.speed > 0)  // is it time to update the scan, or is this scope to be disabled?
            {
                //ts.waterfall.scheduler = (ts.waterfall.speed)*(sd.doubleWaterfallLine?50:25); // we need to use half the speed if in double line drawing mode
            	ts.waterfall.scheduler = (ts.waterfall.speed)*(25*(sd.repeatWaterfallLine)); // we need to use half the speed if in double line drawing mode
                sd.RedrawType|=Redraw_WATERFALL;
            }
        }

        if(ts.scope_scheduler == 0 && is_scopemode())   // is waterfall mode enabled?
        {
            if(ts.scope_speed > 0)  // is it time to update the scan, or is this scope to be disabled?
            {
                ts.scope_scheduler = (ts.scope_speed)*50;
                sd.RedrawType|=Redraw_SCOPE;
            }
        }
        UiSpectrum_RedrawSpectrum();
    }
}

static void UiSpectrum_DisplayDbm()
{
    // TODO: Move to UI Driver
    bool display_something = false;
    static long oldVal=99999;
    static uint8_t dBmShown=0;
    if( ts.txrx_mode == TRX_MODE_RX)
    {
        long val;
        const char* unit_label;

        switch(ts.display_dbm)
        {
        case DISPLAY_S_METER_DBM:
            display_something = true;
            val = sm.dbm;
            unit_label = "dBm   ";
            break;
        case DISPLAY_S_METER_DBMHZ:
            display_something = true;
            val = sm.dbmhz;
            unit_label = "dBm/Hz";
            break;
        }

        if ((display_something == true) && (val!=oldVal))
        {
            char txt[12];
            snprintf(txt,12,"%4ld      ", val);
            UiLcdHy28_PrintText(POS_DisplayDbm_X,POS_DisplayDbm_Y,txt,White,Blue,0);
            UiLcdHy28_PrintText(POS_DisplayDbm_X+SMALL_FONT_WIDTH * 4,POS_DisplayDbm_Y,unit_label,White,Blue,4);
            oldVal=val;		//this will prevent from useless redrawing the same
            dBmShown=1;		//for indicate that dms are shown and erase function may it clear when needed
        }
    }

    // clear the display since we are not showing dBm or dBm/Hz or we are in TX mode
    if ((display_something == false) && (dBmShown==1))
    {
        UiLcdHy28_DrawFullRect(POS_DisplayDbm_X, POS_DisplayDbm_Y, 15, SMALL_FONT_WIDTH * 10 , Black);
        dBmShown=0;		//just to indicate that dbm is erased
        oldVal=99999;	//some value that will enforce refresh when user enable display dbm
    }
}



//void ui_spectrum_init_cw_snap_display (bool visible)
void UiSpectrum_InitCwSnapDisplay (bool visible)
{
	int color = Green;
	if(!visible)
	{
		color = Black;
		// also erase yellow indicator
        UiLcdHy28_DrawFullRect(CW_SNAP_CARRIER_X-27, CW_SNAP_CARRIER_Y, 6, 57, Black);
	}
	//Horizontal lines of box
	UiLcdHy28_DrawStraightLine(CW_SNAP_CARRIER_X-27,
			CW_SNAP_CARRIER_Y + 6,
            27,
            LCD_DIR_HORIZONTAL,
            color);
	UiLcdHy28_DrawStraightLine(CW_SNAP_CARRIER_X+5,
			CW_SNAP_CARRIER_Y + 6,
            27,
            LCD_DIR_HORIZONTAL,
            color);
	UiLcdHy28_DrawStraightLine(CW_SNAP_CARRIER_X-27,
			CW_SNAP_CARRIER_Y - 1,
            27,
            LCD_DIR_HORIZONTAL,
            color);
	UiLcdHy28_DrawStraightLine(CW_SNAP_CARRIER_X+5,
			CW_SNAP_CARRIER_Y - 1,
            27,
            LCD_DIR_HORIZONTAL,
            color);
	// vertical lines of box
	UiLcdHy28_DrawStraightLine(CW_SNAP_CARRIER_X-27,
			CW_SNAP_CARRIER_Y - 1,
            8,
            LCD_DIR_VERTICAL,
            color);
	UiLcdHy28_DrawStraightLine(CW_SNAP_CARRIER_X+31,
			CW_SNAP_CARRIER_Y - 1,
            8,
            LCD_DIR_VERTICAL,
            color);
}

void UiSpectrum_CwSnapDisplay (float32_t delta)
{
#define max_delta 140.0
#define divider 5.0
#if	defined(STM32F7) || defined(STM32H7)
	static float32_t old_delta = 0.0;
#endif

	static int old_delta_p = 0.0;
	if(delta > max_delta)
	{
		delta = max_delta;
	}
	else if(delta < -max_delta)
	{
		delta = -max_delta;
	}

	// lowpass filtering only for fast processors
#if	defined(STM32F7) || defined(STM32H7)
    delta = 0.3 * delta + 0.7 * old_delta;
#endif


	int delta_p = (int)(0.5 + (delta / divider));

	if(delta_p != old_delta_p)
	{
	    UiLcdHy28_DrawStraightLineDouble( CW_SNAP_CARRIER_X + old_delta_p + 1,
	    		CW_SNAP_CARRIER_Y,
	            6,
	            LCD_DIR_VERTICAL,
	            Black);

	    UiLcdHy28_DrawStraightLineDouble( CW_SNAP_CARRIER_X + delta_p + 1,
	    		CW_SNAP_CARRIER_Y,
	            6,
	            LCD_DIR_VERTICAL,
	            Yellow);
#if	defined(STM32F7) || defined(STM32H7)
	    old_delta = delta;
#endif
		old_delta_p = delta_p;
	}
}


void UiSpectrum_CalculateSnap(float32_t Lbin, float32_t Ubin, int posbin, float32_t bin_BW)
{
	// SNAP is used to estimate the frequency of a carrier and subsequently tune the Rx frequency to that carrier frequency
	// At the moment (January 2018), it is usable in the following demodulation modes:
	// AM & SAM
	// CW -> a morse activity detector (built-in in the CW decoding algorithm) detects whenever a CW signal is present and allows
	//       frequency estimation update ONLY when a carrier is present
	// DIGIMODE -> BPSK
	// DD4WH, Jan 2018
	//
	if(ads.CW_signal || (ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_SAM || (ts.dmod_mode == DEMOD_DIGI && ts.digital_mode == DigitalMode_BPSK)))
		// this is only done, if there has been a pulse from the CW station that exceeds the threshold
		// in the CW decoder section
		// OR if we are in AM/SAM/Digi BPSK mode
	{
		static float32_t freq_old = 10000000.0;
	float32_t help_freq = (float32_t)df.tune_old / ((float32_t)TUNE_MULT);
	// 1. lowpass filter all the relevant bins over 2 to 20 FFTs (?)
	// lowpass filtering already exists in the spectrum/waterfall display driver

// 2. determine bin with maximum value inside these samples

    // look for maximum value and save the bin # for frequency delta calculation
    float32_t maximum = 0.0;
    float32_t maxbin = 1.0;
    float32_t delta1 = 0.0;
    float32_t delta2 = 0.0;
    float32_t delta = 0.0;

    for (int c = (int)Lbin; c <= (int)Ubin; c++)   // search for FFT bin with highest value = carrier and save the no. of the bin in maxbin
    {
        if (maximum < sd.FFT_Samples[c])
        {
            maximum = sd.FFT_Samples[c];
            maxbin = c;
        }
    }

// 3. first frequency carrier offset calculation
    // ok, we have found the maximum, now save first delta frequency
//    delta1 = (maxbin - (float32_t)posbin) * bin_BW;
    delta1 = ((maxbin + 1.0) - (float32_t)posbin) * bin_BW;

// 4. second frequency carrier offset calculation

    if(maxbin < 1.0)
    {
    	maxbin = 1.0;
    }
    float32_t bin1 = sd.FFT_Samples[(int)maxbin-1];
    float32_t bin2 = sd.FFT_Samples[(int)maxbin];
    float32_t bin3 = sd.FFT_Samples[(int)maxbin+1];

    if (bin1+bin2+bin3 == 0.0) bin1= 0.00000001; // prevent divide by 0

    // estimate frequency of carrier by three-point-interpolation of bins around maxbin
    // formula by (Jacobsen & Kootsookos 2007) equation (4) P=1.36 for Hanning window FFT function

//    delta2 = (bin_BW * (1.75 * (bin3 - bin1)) / (bin1 + bin2 + bin3));
    delta2 = (bin_BW * (1.36 * (bin3 - bin1)) / (bin1 + bin2 + bin3));
    if(delta2 > bin_BW) delta2 = 0.0;
    delta = delta1 + delta2;

    if(ts.dmod_mode == DEMOD_CW)
    { // only add offset, if in CW mode, not in AM/SAM etc.
        const float32_t cw_offset = (ts.cw_lsb?1.0:-1.0)*(float32_t)ts.cw_sidetone_freq;
        delta = delta + cw_offset;
    }

    if(ts.dmod_mode == DEMOD_DIGI && ts.digital_mode == DigitalMode_BPSK)
    {
    	// FIXME: has to be substituted by global variable --> centre frequency BPSK
    	if(ts.digi_lsb)
    	{
    		delta = delta + PSK_OFFSET; //
    	}
    	else
    	{
    		delta = delta - PSK_OFFSET; //
    	}
    }
    // these frequency calculations are unused at the moment, they will be used with
    // real snap by button press

    // make 10 frequency measurements and after that take the lowpass filtered frequency to tune to


    help_freq = help_freq + delta;
    // do we need a lowpass filter?
    help_freq = 0.2 * help_freq + 0.8 * freq_old;
    //help_freq = help_freq * ((float32_t)TUNE_MULT);
    ads.snap_carrier_freq = (ulong) (help_freq);
    freq_old = help_freq;

	static uint8_t snap_counter = 0;
#if	defined(STM32F7) || defined(STM32H7)
	const int SNAP_COUNT_MAX = 10;
#else
	const int SNAP_COUNT_MAX = 6;
#endif
    if(sc.snap == true)
    {
    	snap_counter++;
    	if(snap_counter >= SNAP_COUNT_MAX) // take low pass filtered 6 freq measurements
    	{
    		// tune to frequency
            // set frequency of Si570 with 4 * dialfrequency
            df.tune_new = (help_freq * ((float32_t)TUNE_MULT));
    		// reset counter
    		snap_counter = 0;
    		sc.snap = false;
    	}
    }
    // graphical TUNE HELPER display
    UiSpectrum_CwSnapDisplay (delta);

	}
}


static void UiSpectrum_CalculateDBm()
{
    // Variables for dbm display --> void calculate_dBm
    static float32_t m_AttackAvedbm = 0.0;
    static float32_t m_DecayAvedbm = 0.0;
    static float32_t m_AverageMagdbm = 0.0;
    static float32_t m_AttackAvedbmhz = 0.0;
    static float32_t m_DecayAvedbmhz = 0.0;
    static float32_t m_AverageMagdbmhz = 0.0;
    // ALPHA = 1 - e^(-T/Tau)
    static float32_t m_AttackAlpha = 0.5; //0.8647;
    static float32_t m_DecayAlpha  = 0.05; //0.3297;

    //###########################################################################################################################################
    //###########################################################################################################################################
    // dBm/Hz-display DD4WH June, 9th 2016
    // the dBm/Hz display gives an absolute measure of the signal strength of the sum of all signals inside the passband of the filter
    // we take the FFT-magnitude values of the spectrum display FFT for this purpose (which are already calculated for the spectrum display),
    // so the additional processor load and additional RAM usage should be close to zero
    // this code also calculates the basis for the S-Meter (in sm.dbm and sm.dbmhz)
    //
    {
        if( ts.txrx_mode == TRX_MODE_RX)
        {
            const float32_t slope = 19.8; // 19.6; --> empirical values derived from measurements by DL8MBY, 2016/06/30, Thanks!
            const float32_t cons = ts.dbm_constant - 225; // -225; //- 227.0;
            const int buff_len_int = sd.fft_iq_len;
            const float32_t buff_len = buff_len_int;

            // width of a 256 tap FFT bin = 187.5Hz
            // we have to take into account the magnify mode
            // --> recalculation of bin_BW
            // correct bin bandwidth is determined by the Zoom FFT display setting
            const float32_t bin_BW = IQ_SAMPLE_RATE_F * 2.0 / (buff_len * (1 << sd.magnify)) ;

            float32_t width = FilterInfo[FilterPathInfo[ts.filter_path].id].width;
            float32_t offset = FilterPathInfo[ts.filter_path].offset;

            if (offset == 0)
            {
                offset = width/2;
            }

            float32_t lf_freq = offset - width/2;
            float32_t uf_freq = offset + width/2;

            //	determine Lbin and Ubin from ts.dmod_mode and FilterInfo.width
            //	= determine bandwith separately for lower and upper sideband

            float32_t bw_LOWER = 0.0;
            float32_t bw_UPPER = 0.0;

            if (RadioManagement_UsesBothSidebands(ts.dmod_mode) == true)
            {
                bw_UPPER = uf_freq;
                bw_LOWER = -uf_freq;
            }
            else if (RadioManagement_LSBActive(ts.dmod_mode) == true)
            {
                bw_UPPER = -lf_freq;
                bw_LOWER = -uf_freq;
            }
            else if (ts.dmod_mode == DEMOD_DIGI && ts.digital_mode == DigitalMode_BPSK)
            { // this is for experimental SNAP of BPSK carriers
            	bw_LOWER = PSK_OFFSET - PSK_SNAP_RANGE;
            	bw_UPPER = PSK_OFFSET + PSK_SNAP_RANGE;
            }
            else // USB
            {
                bw_UPPER = uf_freq;
                bw_LOWER = lf_freq;
            }


            //  determine posbin (where we receive at the moment) from ts.iq_freq_mode

            // frequency translation off, IF = 0 Hz OR
            // in all magnify cases (2x up to 32x) the posbin is in the centre of the spectrum display

            int bin_offset = 0;


            if(sd.magnify == 0)
            {
                if(ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ)       // we are in RF LO HIGH mode (tuning is below center of screen)
                {
                    bin_offset = - (buff_len_int / 16);
                }
                else if(ts.iq_freq_mode == FREQ_IQ_CONV_M6KHZ)      // we are in RF LO LOW mode (tuning is above center of screen)
                {
                    bin_offset = (buff_len_int / 16);
                }
                else if(ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ)     // we are in RF LO HIGH mode (tuning is below center of screen)
                {
                    bin_offset =  - (buff_len_int / 8);
                }
                else if(ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ)     // we are in RF LO LOW mode (tuning is above center of screen)
                {
                    bin_offset =  (buff_len_int / 8);
                }
            }

            int posbin = buff_len_int / 4 + bin_offset;  // right in the middle!


            // calculate upper and lower limit for determination of signal strength
            // = filter passband is between the lower bin Lbin and the upper bin Ubin
            float32_t Lbin = (float32_t)posbin + roundf(bw_LOWER / bin_BW);
            float32_t Ubin = (float32_t)posbin + roundf(bw_UPPER / bin_BW); // the bin on the upper sideband side

            if(ts.dmod_mode == DEMOD_SAM && ads.sam_sideband == SAM_SIDEBAND_USB) // workaround to make SNAP and carrier offaet display work with sideband-selected SAM
            {
            	Lbin = Lbin - 1.0;
            }

            // take care of filter bandwidths that are larger than the displayed FFT bins
            if(Lbin < 0)
            {
                Lbin = 0;
            }
            //if (Ubin > 255)
            if (Ubin > (sd.spec_len-1))
            {
                //Ubin = 255;
            	Ubin = sd.spec_len-1;
            }

            for(int32_t i = 0; i < (buff_len_int/4); i++)
            {
                sd.FFT_Samples[sd.spec_len - i - 1] = sd.FFT_MagData[i + buff_len_int/4] * SCOPE_PREAMP_GAIN;	// get data
            }
            for(int32_t i = buff_len_int/4; i < (buff_len_int/2); i++)
            {
                sd.FFT_Samples[sd.spec_len - i - 1] = sd.FFT_MagData[i - buff_len_int/4] * SCOPE_PREAMP_GAIN;	// get data
            }

            // here would be the right place to start with the SNAP mode!
            if(cw_decoder_config.snap_enable && (ts.dmod_mode == DEMOD_CW || ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_SAM || (ts.dmod_mode == DEMOD_DIGI && ts.digital_mode == DigitalMode_BPSK)))
            {
            	 UiSpectrum_CalculateSnap(Lbin, Ubin, posbin, bin_BW);

            }

            float32_t sum_db = 0.0;
            // determine the sum of all the bin values in the passband
            for (int c = (int)Lbin; c <= (int)Ubin; c++)   // sum up all the values of all the bins in the passband
            {
                sum_db = sum_db + sd.FFT_Samples[c]; // / (float32_t)(1<<sd.magnify);
            }
            // we have to account for the larger number of bins that are summed up when using higher
            // magnifications
            // for example: if we have 34 bins to sum up for sd.magnify == 1, we sum up 68 bins for sd.magnify == 2

            //            sum_db /= (float32_t)sd.magnify + 1;
            //            cons = cons - 3.0 * (sd.magnify);

            if (sum_db > 0)
            {
                sm.dbm = slope * log10f_fast (sum_db) + cons;
                sm.dbmhz = slope * log10f_fast (sum_db) -  10 * log10f_fast ((float32_t)(((int)Ubin-(int)Lbin) * bin_BW)) + cons;
            }
            else
            {
                sm.dbm = -145.0;
                sm.dbmhz = -145.0;
            }

            // lowpass IIR filter
            // Wheatley 2011: two averagers with two time constants
            // IIR filter with one element analog to 1st order RC filter
            // but uses two different time constants (ALPHA = 1 - e^(-T/Tau)) depending on
            // whether the signal is increasing (attack) or decreasing (decay)
            //
            m_AttackAvedbm = (1.0 - m_AttackAlpha) * m_AttackAvedbm + m_AttackAlpha * sm.dbm;
            m_DecayAvedbm = (1.0 - m_DecayAlpha) * m_DecayAvedbm + m_DecayAlpha * sm.dbm;
            m_AttackAvedbmhz = (1.0 - m_AttackAlpha) * m_AttackAvedbmhz + m_AttackAlpha * sm.dbmhz;
            m_DecayAvedbmhz = (1.0 - m_DecayAlpha) * m_DecayAvedbmhz + m_DecayAlpha * sm.dbmhz;

            if (m_AttackAvedbm > m_DecayAvedbm)
            { // if attack average is larger then it must be an increasing signal
                m_AverageMagdbm = m_AttackAvedbm; // use attack average value for output
                m_DecayAvedbm = m_AttackAvedbm; // set decay average to attack average value for next time
            }
            else
            { // signal is decreasing, so use decay average value
                m_AverageMagdbm = m_DecayAvedbm;
            }

            if (m_AttackAvedbmhz > m_DecayAvedbmhz)
            { // if attack average is larger then it must be an increasing signal
                m_AverageMagdbmhz = m_AttackAvedbmhz; // use attack average value for output
                m_DecayAvedbmhz = m_AttackAvedbmhz; // set decay average to attack average value for next time
            }
            else
            { // signal is decreasing, so use decay average value
                m_AverageMagdbmhz = m_DecayAvedbmhz;
            }

            //        long dbm_Hz = (long) m_AverageMag;
            sm.dbm = m_AverageMagdbm; // write average into variable for S-meter display
            sm.dbmhz = m_AverageMagdbmhz; // write average into variable for S-meter display
        }

        UiSpectrum_DisplayDbm();
    }
}

#ifdef X_USE_DISP_480_320_SPEC
//Waterfall memory pointer allocation.
//It sets memory pointer to Height/2 array located in CCM for f4 devices with low ram amount. For all rest allocates memory by calling malloc.
void UiSpectrum_SetWaterfallMemoryPointer(uint16_t ramsize)
{
	if(ramsize<256)
	{
		sd.waterfall=sd.waterfall_mem;	//CCM memory for devices with low ram amount. Used with each line doubled.
	}
	else
	{
		sd.waterfall=(uint8_t (*)[SPECTRUM_WIDTH]) malloc(WATERFALL_HEIGHT*SPECTRUM_WIDTH);	//malloc returns pointer in normal ram (for F4 devices CCM memory is always 64kB)
	}
}
#endif

