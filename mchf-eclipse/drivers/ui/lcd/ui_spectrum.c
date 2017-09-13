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

static const scope_scaling_info_t scope_scaling_factors[SCOPE_SCALE_NUM] =
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
        { DB_SCALING_S3,            "SC(3S-Unit/div)" }
};



static void     UiSpectrum_DrawFrequencyBar();
static void		UiSpectrum_CalculateDBm();
void ui_spectrum_init_cw_snap_display (uint8_t visible);

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
        sd.pixel_per_hz = IQ_SAMPLE_RATE_F/((1 << sd.magnify) * SPECTRUM_WIDTH);     // magnify mode is on
        force_update = true;
    }

    if (ts.iq_freq_mode != old_iq_freq_mode  || force_update)
    {
        old_iq_freq_mode = ts.dmod_mode;
        force_update = true;

        if(!sd.magnify)     // is magnify mode on?
        {
            sd.rx_carrier_pos = SPECTRUM_WIDTH/2 - 0.5 - (AudioDriver_GetTranslateFreq()/sd.pixel_per_hz);
        }
        else        // magnify mode is on
        {
            sd.rx_carrier_pos = SPECTRUM_WIDTH/2 -0.5;                                // line is always in center in "magnify" mode
        }
    }
    if (ts.cw_lsb != old_cw_lsb || ts.cw_sidetone_freq != old_cw_sidetone_freq || ts.dmod_mode != old_dmod_mode || ts.digital_mode != old_digital_mode || force_update)
    {
        old_cw_lsb = ts.cw_lsb;
        old_cw_sidetone_freq = ts.cw_sidetone_freq;
        old_dmod_mode = ts.dmod_mode;
        old_digital_mode = ts.digital_mode;

        float32_t tx_vfo_offset = ((float32_t)(((int32_t)RadioManagement_GetTXDialFrequency() - (int32_t)RadioManagement_GetRXDialFrequency())/TUNE_MULT))/sd.pixel_per_hz;

        // FIXME: DOES NOT WORK PROPERLY IN SPLIT MODE
        sd.marker_num_prev = sd.marker_num;
        float32_t mode_marker_offset[SPECTRUM_MAX_MARKER];
        switch(ts.dmod_mode)
        {
        case DEMOD_CW:
            mode_marker_offset[0] =(ts.cw_lsb?-1.0:1.0)*((float32_t)ts.cw_sidetone_freq / sd.pixel_per_hz);
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
                mode_marker[0] = 2125;
                sd.marker_num = 2;
                break;
            case DigitalMode_RTTY:
                mode_marker[0] = 915; // Mark Frequency
                mode_marker[1] = mode_marker[0] + rtty_shifts[rtty_ctrl_config.shift_idx].value;
                sd.marker_num = 2;
                break;
            default:
                mode_marker[0] = 0;
                sd.marker_num = 1;
            }

            for (uint16_t idx; idx < sd.marker_num; idx++)
            {
                mode_marker_offset[idx] = (ts.digi_lsb?-1.0:1.0)*(mode_marker[idx] / sd.pixel_per_hz);
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
        	sd.marker_pos[idx] = SPECTRUM_WIDTH; // this is an invalid position out of screen
        }

    }
}

static void UiSpectrum_FFTWindowFunction(char mode)
{
    // Information on these windowing functions may be found on the internet - check the Wikipedia article "Window Function"
    // KA7OEI - 20150602

    switch(mode)
    {
    case FFT_WINDOW_RECTANGULAR:	// No processing at all
        break;
    case FFT_WINDOW_COSINE:			// Sine window function (a.k.a. "Cosine Window").  Kind of wide...
        for(int i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            sd.FFT_Samples[i] = arm_sin_f32((PI * (float32_t)i)/FFT_IQ_BUFF_LEN - 1) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_BARTLETT:		// a.k.a. "Triangular" window - Bartlett (or Fej?r) window is special case where demonimator is "N-1". Somewhat better-behaved than Rectangular
        for(int i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            sd.FFT_Samples[i] = (1 - fabs(i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_WELCH:			// Parabolic window function, fairly wide, comparable to Bartlett
        for(int i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            sd.FFT_Samples[i] = (1 - ((i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF)*((i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF)) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_HANN:			// Raised Cosine Window (non zero-phase version) - This has the best sidelobe rejection of what is here, but not as narrow as Hamming.
        for(int i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            sd.FFT_Samples[i] = 0.5 * (float32_t)((1 - (arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
        }
        break;
    case FFT_WINDOW_HAMMING:		// Another Raised Cosine window - This is the narrowest with reasonably good sidelobe rejection.
        for(int i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            sd.FFT_Samples[i] = (0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_BLACKMAN:		// Approx. same "narrowness" as Hamming but not as good sidelobe rejection - probably best for "default" use.
        for(int i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            sd.FFT_Samples[i] = (0.42659 - (0.49656*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.076849*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];
        }
        break;
    case FFT_WINDOW_NUTTALL:		// Slightly wider than Blackman, comparable sidelobe rejection.
        for(int i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            sd.FFT_Samples[i] = (0.355768 - (0.487396*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.144232*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) - (0.012604*arm_cos_f32((6*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];
        }
        break;
    }

    float32_t gcalc = 1.0/ads.codec_gain_calc;                // Get gain setting of codec and convert to multiplier factor
    arm_scale_f32(sd.FFT_Samples,gcalc, sd.FFT_Samples, FFT_IQ_BUFF_LEN);

}

static void UiSpectrum_SpectrumTopBar_GetText(char* wfbartext)
{
    const char* lefttext;

    if(is_waterfallmode())           //waterfall
    {
        lefttext = scope_scaling_factors[0].label; // a small trick, we use the empty location of index 0 in the table
    }
    else                                                // scope
    {
        lefttext = scope_scaling_factors[ts.spectrum_db_scale].label;
    }

    sprintf(wfbartext,"%s < Magnify %2ux >",lefttext, (1<<sd.magnify));
}


static void UiSpectrum_CreateDrawArea()
{
    uint32_t clr;

    // get grid colour of all but center line
    UiMenu_MapColors(ts.scope_grid_colour,NULL, &sd.scope_grid_colour_active);
    // Get color of center vertical line of spectrum scope
    UiMenu_MapColors(ts.spectrum_centre_line_colour,NULL, &sd.scope_centre_grid_colour_active);

    UiSpectrum_UpdateSpectrumPixelParameters();

    // Clear screen where frequency information will be under graticule
    UiLcdHy28_PrintText(POS_SPECTRUM_IND_X - 2, POS_SPECTRUM_IND_Y + 60, "                                 ", Black, Black, 0);

    // Frequency bar separator
    UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 20),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

    // Draw Frequency bar text
    UiSpectrum_DrawFrequencyBar();

    if(ts.spectrum_size == SPECTRUM_NORMAL)		//don't draw text bar when size is BIG
    {
        // Draw top band = grey box in which text is printed
        for(int i = 0; i < 16; i++)
        {
            UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y - 20 + i),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);
        }

        char bartext[34];

        // Top band text - middle caption
        UiSpectrum_SpectrumTopBar_GetText(bartext);
        UiLcdHy28_PrintTextCentered(
                POS_SPECTRUM_IND_X,
                (POS_SPECTRUM_IND_Y - 18),
                POS_SPECTRUM_IND_W,
                bartext,
                White,
                RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);
    }
    // Draw control left and right border
    UiLcdHy28_DrawStraightLineDouble((POS_SPECTRUM_IND_X - 2),
            (POS_SPECTRUM_IND_Y - 20),
            (POS_SPECTRUM_IND_H + 12),
            LCD_DIR_VERTICAL,
            //									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
            sd.scope_grid_colour_active);

    UiLcdHy28_DrawStraightLineDouble(	(POS_SPECTRUM_IND_X + POS_SPECTRUM_IND_W - 2),
            (POS_SPECTRUM_IND_Y - 20),
            (POS_SPECTRUM_IND_H + 12),
            LCD_DIR_VERTICAL,
            //									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
            sd.scope_grid_colour_active);

    // Is (scope enabled AND NOT scope light)
    // draw grid
    if ( is_waterfallmode() == false && (ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE) == false)
    {
        // Horizontal grid lines
        uint8_t y_add;
        if(ts.spectrum_size)		//set range big/normal
        {
            sd.upper_horiz_gridline = 5;
            y_add = SPECTRUM_SCOPE_GRID_HORIZ;
            // 16 lines is the actual added space, which is one more segment,
            // but in order to generate also a upper closing bar, we add two grid lines
        }
        else
        {
            sd.upper_horiz_gridline = 3;
            y_add = 0;
        }

        // array must be filled from higher to the y coordinates
        // the lookup code for a match counts on this.
        for(int i = 0; i < sd.upper_horiz_gridline; i++)
        {
            // Save y position for repaint
            sd.horz_grid_id[i] = (POS_SPECTRUM_GRID_HORIZ_START - i * SPECTRUM_SCOPE_GRID_HORIZ);

            // Draw
            UiLcdHy28_DrawStraightLine(	POS_SPECTRUM_IND_X,
                    sd.horz_grid_id[i],
                    SPECTRUM_WIDTH,
                    LCD_DIR_HORIZONTAL,
                    sd.scope_grid_colour_active);
        }

        // Vertical grid lines
        // array must be filled from low to higher x coordinates
        // the lookup code for a match counts on this.
        for(int i = 1; i < 8; i++)
        {
            clr = sd.scope_grid_colour_active;
            // Save x position for repaint
            sd.vert_grid_id[i - 1] = (POS_SPECTRUM_GRID_VERT_START + i*SPECTRUM_SCOPE_GRID_VERT);

            // Draw
            UiLcdHy28_DrawStraightLine(	sd.vert_grid_id[i - 1],
                    (POS_SPECTRUM_IND_Y -  4 - y_add),
                    (POS_SPECTRUM_IND_H - 16 + y_add),
                    LCD_DIR_VERTICAL,
                    clr);
        }
    }

    if ((is_waterfallmode() && (!ts.waterfall.speed)) || (is_waterfallmode() == false && (!ts.scope_speed)))
    {
        // print "disabled" in the middle of the screen if the waterfall or scope was disabled
        UiLcdHy28_PrintText(
                (POS_SPECTRUM_IND_X + 72),
                (POS_SPECTRUM_IND_Y + 18),
                "   DISABLED   ",
                Grey,
                RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);
    }
    if(cw_decoder_config.snap_enable && ts.dmod_mode == DEMOD_CW)
    {
    	ui_spectrum_init_cw_snap_display(1);
    }
}

void UiSpectrum_Clear()
{
    UiLcdHy28_DrawFullRect(POS_SPECTRUM_IND_X - 2, (POS_SPECTRUM_IND_Y - 22), 94, 264, Black);	// Clear screen under spectrum scope by drawing a single, black block (faster with SPI!)
}

/**
 * tells us if x is on a vertical grid line. Called for all SPECTRUM_WIDTH lines
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
    return ((x - POS_SPECTRUM_GRID_VERT_START) % SPECTRUM_SCOPE_GRID_VERT == 0) && (x != (POS_SPECTRUM_GRID_VERT_START + SPECTRUM_WIDTH)) ;
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


static void UiSpectrum_ScopeStandard_UpdateVerticalDataLine(uint16_t x, uint16_t y_old_pos, uint16_t y_new_pos, uint16_t clr_scope, bool is_carrier_line)
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
        bool repaint_v_grid = UiSpectrum_Draw_XposIsOnVertGrid(x);

        // we need to delete by overwriting with background color or grid color if we are on a vertical grid line
        uint16_t clr_bg =
                is_carrier_line?
                        sd.scope_centre_grid_colour_active
                        :
                        ( repaint_v_grid ? sd.scope_grid_colour_active : Black )
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
    const uint16_t spec_height_limit = sd.scope_size - 7;
    const uint16_t spec_top_y = sd.scope_ystart + sd.scope_size;

    uint32_t clr_scope;
    UiMenu_MapColors(ts.scope_trace_colour, NULL, &clr_scope);

    uint16_t marker_line_pos[SPECTRUM_MAX_MARKER];

    for (uint16_t idx = 0; idx < SPECTRUM_MAX_MARKER; idx++)
    {
        marker_line_pos[idx] = SPECTRUM_START_X + sd.marker_pos[idx];

        // this is the tx carrier line, we redraw only if line changes place around,
        // init code must take care to reset prev position to 0xffff in order to get initialization done after clean start

        if (marker_line_pos[idx] != sd.marker_line_pos_prev[idx])
        {
            if (sd.marker_line_pos_prev[idx] < SPECTRUM_START_X + SPECTRUM_WIDTH)
            {
                // delete old line if previously inside screen limits

                if(is_scope_light)
                {
                    UiLcdHy28_DrawStraightLine( sd.marker_line_pos_prev[idx],
                            spec_top_y - spec_height_limit,
                            spec_height_limit,
                            LCD_DIR_VERTICAL,
                            Black);
                }
                else
                {
                    UiSpectrum_ScopeStandard_UpdateVerticalDataLine(
                            sd.marker_line_pos_prev[idx],
                            spec_top_y - spec_height_limit /* old = max pos */ ,
                            spec_top_y /* new = min pos */,
                            clr_scope,
                            false);

                    // we erase the memory for this location, so that it is fully redrawn
                    if (sd.marker_line_pos_prev[idx] < SPECTRUM_START_X + SPECTRUM_WIDTH)
                    {
                        old_pos[sd.marker_line_pos_prev[idx] - SPECTRUM_START_X] = spec_top_y;
                    }
                }
            }

            if (marker_line_pos[idx] < SPECTRUM_START_X + SPECTRUM_WIDTH)
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
                    if (marker_line_pos[idx] < SPECTRUM_START_X + SPECTRUM_WIDTH)
                    {
                        old_pos[marker_line_pos[idx] - SPECTRUM_START_X] = spec_top_y;
                    }
                }
            }

            // done, remember where line has been drawn
            sd.marker_line_pos_prev[idx] = marker_line_pos[idx];
        }
    }

    uint16_t marker_lines_togo = sd.marker_num;
    for(uint16_t x = SPECTRUM_START_X, idx = 0; idx < SPECTRUM_WIDTH; x++, idx++)
    {
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
            uint16_t clr_bg = Black;

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
            UiSpectrum_ScopeStandard_UpdateVerticalDataLine(x, y_old_pos, y_new_pos, clr_scope, is_marker_line);
        }
    }
}






/**
 * @brief init data strctures for both "Scope Display" and "Waterfall" Display
 */
static void UiSpectrum_InitSpectrumDisplayData()
{
    // Init publics
    sd.state 		= 0;
    sd.samp_ptr 	= 0;
    sd.enabled		= 0;
    ts.dial_moved	= 0;

    sd.agc_rate = ((float32_t)ts.scope_agc_rate) / SPECTRUM_AGC_SCALING;	// calculate agc rate
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

    /*
    	//
    	// Load waterfall data with "splash" showing palette
    	//
    	j = 0;					// init count of lines on display
    	k = sd.wfall_line;		// start with line currently displayed in buffer
    	while(j < SPECTRUM_HEIGHT)	{		// loop number of times of buffer
    		for(i = 0; i < SPEC_BUFF_LEN; i++)	{		// do this all of the way across, horizonally
    			sd.waterfall[k][i] = (SPECTRUM_HEIGHT - j) % SPECTRUM_HEIGHT;	// place the color of the palette, indexed to vertical position
    		}
    		j++;		// update line count
    		k++;		// update position within circular buffer - which also is used to calculate color
    		k %= SPECTRUM_HEIGHT;	// limit to modulus count of circular buffer size
    	}
    	//
     */


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

    if(ts.spectrum_size == SPECTRUM_NORMAL)	 						// waterfall the same size as spectrum scope
    {
        sd.wfall_ystart = SPECTRUM_START_Y + SPECTRUM_SCOPE_TOP_LIMIT;
        sd.wfall_size = SPECTRUM_HEIGHT - SPECTRUM_SCOPE_TOP_LIMIT;

        sd.scope_ystart = SPECTRUM_START_Y;
        sd.scope_size = SPECTRUM_HEIGHT;
    }																	// waterfall larger, covering the word "Waterfall Display"
    else if(ts.spectrum_size == SPECTRUM_BIG)
    {
        sd.wfall_ystart = SPECTRUM_START_Y - WFALL_MEDIUM_ADDITIONAL;
        sd.wfall_size = SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL;

        sd.scope_ystart = SPECTRUM_START_Y - SPEC_LIGHT_MORE_POINTS;
        sd.scope_size = SPECTRUM_HEIGHT + SPEC_LIGHT_MORE_POINTS;
    }

    for (uint16_t idx = 0; idx < SPECTRUM_WIDTH; idx++)
    {
        sd.Old_PosData[idx] = sd.scope_ystart + sd.scope_size;
    }

    sd.wfall_contrast = (float)ts.waterfall.contrast / 100.0;		// calculate scaling for contrast

    for (uint16_t marker_idx = 0; marker_idx < SPECTRUM_MAX_MARKER; marker_idx++)
    {
        sd.marker_line_pos_prev[marker_idx] = 0xffff; // off screen
    }
    // Ready
    sd.enabled		= 1;
}

void UiSpectrum_WaterfallClearData()
{
    for(int i = 0; i < WATERFALL_MAX_SIZE; i++)   // clear old wf lines if changing magnify
    {
        for(int j = 0; j < SPECTRUM_WIDTH; j++)
        {
            sd.waterfall[i][j] = 0;
        }
    }
}


static void UiSpectrum_DrawWaterfall()
{
    sd.wfall_line %= sd.wfall_size; // make sure that the circular buffer is clipped to the size of the display area

    // Contrast:  100 = 1.00 multiply factor:  125 = multiply by 1.25 - "sd.wfall_contrast" already converted to 100=1.00
    arm_scale_f32(sd.FFT_Samples, sd.wfall_contrast, sd.FFT_Samples, SPEC_BUFF_LEN);


    UiSpectrum_UpdateSpectrumPixelParameters(); // before accessing pixel parameters, request update according to configuration


    uint16_t marker_line_pixel_pos[SPECTRUM_MAX_MARKER];
    for (uint16_t idx = 0; idx < sd.marker_num; idx++)
    {
        marker_line_pixel_pos[idx] = sd.marker_pos[idx];
    }

    // After the above manipulation, clip the result to make sure that it is within the range of the palette table
    for(uint16_t i = 0; i < SPEC_BUFF_LEN; i++)
    {
        if(sd.FFT_Samples[i] >= NUMBER_WATERFALL_COLOURS)   // is there an illegal color value?
        {
            sd.FFT_Samples[i] = NUMBER_WATERFALL_COLOURS - 1;   // yes - clip it
        }

        sd.waterfall[sd.wfall_line][i] = sd.FFT_Samples[i]; // save the manipulated value in the circular waterfall buffer
    }

    for (uint16_t idx = 0; idx < sd.marker_num; idx ++)
    {
        // Place center line marker on screen:  Location [64] (the 65th) of the palette is reserved is a special color reserved for this
        if (marker_line_pixel_pos[idx] < SPECTRUM_WIDTH)
        {
            sd.waterfall[sd.wfall_line][marker_line_pixel_pos[idx]] = NUMBER_WATERFALL_COLOURS;
        }
    }

    sd.wfall_line++;        // bump to the next line in the circular buffer for next go-around

    uint16_t lptr = sd.wfall_line;      // get current line of "bottom" of waterfall in circular buffer

    sd.wfall_line_update++;                                 // update waterfall line count
    sd.wfall_line_update %= ts.waterfall.vert_step_size;    // clip it to number of lines per iteration

    if(!sd.wfall_line_update)                               // if it's count is zero, it's time to move the waterfall up
    {

        lptr %= sd.wfall_size;      // do modulus limit of spectrum high

        // set up LCD for bulk write, limited only to area of screen with waterfall display.  This allow data to start from the
        // bottom-left corner and advance to the right and up to the next line automatically without ever needing to address
        // the location of any of the display data - as long as we "blindly" write precisely the correct number of pixels per
        // line and the number of lines.

        UiLcdHy28_BulkPixel_OpenWrite(SPECTRUM_START_X, SPECTRUM_WIDTH, (sd.wfall_ystart + 1), sd.wfall_size);

        uint16_t spectrum_pixel_buf[SPECTRUM_WIDTH];

        for(uint16_t lcnt = 0;lcnt < sd.wfall_size; lcnt++)                 // set up counter for number of lines defining height of waterfall
        {
            for(uint16_t i = 0; i < SPECTRUM_WIDTH; i++)      // scan to copy one line of spectral data - "unroll" to optimize for ARM processor
            {
                spectrum_pixel_buf[i] = sd.waterfall_colours[sd.waterfall[lptr][i]];    // write to memory using waterfall color from palette
            }

            UiLcdHy28_BulkPixel_PutBuffer(spectrum_pixel_buf, SPECTRUM_WIDTH);

            lptr++;                                 // point to next line in circular display buffer
            lptr %= sd.wfall_size;              // clip to display height
        }
        UiLcdHy28_BulkPixel_CloseWrite();                   // we are done updating the display - return to normal full-screen mode
    }
}

static float32_t  UiSpectrum_ScaleFFT(const float32_t value, float32_t* min_p)
{
    float32_t sig = sd.display_offset + log10f_fast(value) * sd.db_scale;     // take FFT data, do a log10 and multiply it to scale 10dB (fixed)
    // apply "AGC", vertical "sliding" offset (or brightness for waterfall)

    if (sig < *min_p)
    {
        *min_p = sig;
    }

    return  (sig < 1)? 1 : sig;
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

    // Apply gain to collected IQ samples and then do FFT
    case 1:		// Scale input according to A/D gain and apply Window function
    {
        // with the new FFT lib arm_cfft we need to put the input and output samples into one buffer, therefore
        // UiDriverFFTWindowFunction was changed
        UiSpectrum_FFTWindowFunction(ts.fft_window_type);		// do windowing function on input data to get less "Bin Leakage" on FFT data

        sd.state++;
        break;
    }
    case 2:		// Do FFT and calculate complex magnitude
    {
        arm_cfft_f32(&arm_cfft_sR_f32_len256, sd.FFT_Samples,0,1);	// Do FFT

        // Calculate magnitude
        arm_cmplx_mag_f32( sd.FFT_Samples, sd.FFT_MagData ,SPEC_BUFF_LEN);

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

        arm_scale_f32(sd.FFT_AVGData, filt_factor, sd.FFT_Samples, SPEC_BUFF_LEN);	// get scaled version of previous data
        arm_sub_f32(sd.FFT_AVGData, sd.FFT_Samples, sd.FFT_AVGData, SPEC_BUFF_LEN);	// subtract scaled information from old, average data
        arm_scale_f32(sd.FFT_MagData, filt_factor, sd.FFT_Samples, SPEC_BUFF_LEN);	// get scaled version of new, input data
        arm_add_f32(sd.FFT_Samples, sd.FFT_AVGData, sd.FFT_AVGData, SPEC_BUFF_LEN);	// add portion new, input data into average

        for(uint32_t i = 0; i < SPEC_BUFF_LEN; i++)	 		// guarantee that the result will always be >= 0
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
        for(uint16_t i = 0; i < (SPEC_BUFF_LEN/2); i++)
        {
            sd.FFT_Samples[SPEC_BUFF_LEN - i - 1] = UiSpectrum_ScaleFFT(sd.FFT_AVGData[i + SPEC_BUFF_LEN/2], &min1);
            // take FFT data, do a log10 and multiply it to scale 10dB (fixed)
            // apply "AGC", vertical "sliding" offset (or brightness for waterfall)
        }

        for(uint16_t i = (SPEC_BUFF_LEN/2); i < (SPEC_BUFF_LEN); i++)
        {
            // build right half of spectrum data
            sd.FFT_Samples[SPEC_BUFF_LEN - i - 1] = UiSpectrum_ScaleFFT(sd.FFT_AVGData[i - SPEC_BUFF_LEN/2], &min1);
            // take FFT data, do a log10 and multiply it to scale 10dB (fixed)
            // apply "AGC", vertical "sliding" offset (or brightness for waterfall)
        }

        // Adjust the sliding window so that the lowest signal is always black
        sd.display_offset -= sd.agc_rate*min1/5;

        sd.state++;
        break;
    }
    case 5:	// rescale waterfall horizontally, apply brightness/contrast, process pallate and put vertical line on screen, if enabled.
    {
        if(is_waterfallmode())
        {
            UiSpectrum_DrawWaterfall();
        }
        else
        {
            UiSpectrum_DrawScope(sd.Old_PosData, sd.FFT_Samples);
        }
        sd.state = 0;
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
    UiSpectrum_Clear();			// clear display under spectrum scope
    UiSpectrum_CreateDrawArea();
    UiSpectrum_InitSpectrumDisplayData();
    UiSpectrum_DisplayFilterBW();	// Update on-screen indicator of filter bandwidth
}

/**
 * @brief Show a small horizontal line below the spectrum to indicate the rx passband of the currently active filter
 */
void UiSpectrum_DisplayFilterBW()
{

    if(ts.menu_mode == 0)
    {// bail out if in menu mode
        // Update screen indicator - first get the width and center-frequency offset of the currently-selected filter

        const FilterPathDescriptor* path_p = &FilterPathInfo[ts.filter_path];
        const FilterDescriptor* filter_p = &FilterInfo[path_p->id];
        const float32_t width = filter_p->width;
        const float32_t offset = path_p->offset!=0 ? path_p->offset : width/2;

        UiSpectrum_UpdateSpectrumPixelParameters(); // before accessing pixel parameters, request update according to configuration


        float32_t width_pixel = width/sd.pixel_per_hz;                          // calculate width of line in pixels

        float32_t offset_pixel = offset/sd.pixel_per_hz;                            // calculate filter center frequency offset in pixels

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

        //  erase old line by clearing whole area
        UiLcdHy28_DrawStraightLineDouble((POS_SPECTRUM_IND_X), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y), SPECTRUM_WIDTH, LCD_DIR_HORIZONTAL, Black);

        if(left_filter_border_pos < 0) // prevents line to leave left border
        {
            width_pixel = width_pixel + left_filter_border_pos;
            left_filter_border_pos = 0;
        }

        if(left_filter_border_pos + width_pixel > SPECTRUM_WIDTH) // prevents line to leave right border
        {
            width_pixel = (float32_t)SPECTRUM_WIDTH - left_filter_border_pos;
        }

        uint32_t clr;
        // get color for line
        UiMenu_MapColors(ts.filter_disp_colour,NULL, &clr);
        // draw line
        UiLcdHy28_DrawStraightLineDouble(((float32_t)POS_SPECTRUM_IND_X + roundf(left_filter_border_pos)), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y), roundf(width_pixel), LCD_DIR_HORIZONTAL, clr);
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

        // remainder of frequency/graticule markings
        const static int idx2pos[2][9] = {{0,26,58,90,122,154,186,218, 242},{0,26,58,90,122,154,186,209, 229} };

        for (int idx = -4; idx < 5; idx += (sd.magnify < 2) ? 1 : 2 )
        {
            int pos = idx2pos[sd.magnify < 2? 0 : 1][idx+4];

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
                    snprintf(txt,16, " %u.%02u ", bignum, smallnum);   // build string for middle-left frequency (10Hz precision)
                    c = &txt[strlen(txt)-5];  // point at 5th character from the end
                }
                UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +  pos),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
            }
        }
    }
}

void UiSpectrum_Redraw()
{
    // Only in RX mode and NOT while powering down or in menu mode or if displaying memory information
    if (
            (ts.spectrum_scheduler == 0)
            && (ts.txrx_mode == TRX_MODE_RX)
            && (ts.menu_mode == false)
            && (ts.powering_down == false)
            && (ts.mem_disp == false)
            && (sd.enabled == true)
            && (ts.lcd_blanking_flag == false)
    )
    {
        if(is_waterfallmode())   // is waterfall mode enabled?
        {
            if(ts.waterfall.speed > 0)  // is it time to update the scan, or is this scope to be disabled?
            {
                ts.spectrum_scheduler = (ts.waterfall.speed-1)*2;

                UiSpectrum_RedrawSpectrum();   // yes - call waterfall update instead
            }
        }
        else
        {
            if(ts.scope_speed > 0)  // is it time to update the scan, or is this scope to be disabled?
            {
                ts.spectrum_scheduler = (ts.scope_speed-1)*2;
                UiSpectrum_RedrawSpectrum();;    // Spectrum Display enabled - do that!
            }
        }
    }
}

static void UiSpectrum_DisplayDbm()
{
    // TODO: Move to UI Driver
    bool display_something = false;
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

        if (display_something == true) {
            char txt[12];
            snprintf(txt,12,"%4ld      ", val);
            UiLcdHy28_PrintText(161,64,txt,White,Blue,0);
            UiLcdHy28_PrintText(161+SMALL_FONT_WIDTH * 4,64,unit_label,White,Blue,4);
        }
    }

    // clear the display since we are not showing dBm or dBm/Hz or we are in TX mode
    if (display_something == false)
    {
        UiLcdHy28_DrawFullRect(161, 64, 15, SMALL_FONT_WIDTH * 10 , Black);
    }
}


#define CW_snap_carrier_X	27 // central position of variable freq marker
#define CW_snap_carrier_Y	122 // position of variable freq marker

void ui_spectrum_init_cw_snap_display (uint8_t visible)
{
	int color = Green;
	if(!visible)
	{
		color = Black;
		// also erase yellow indicator
        UiLcdHy28_DrawFullRect(0, CW_snap_carrier_Y, 6, 57, Black);
	}
	//Horizontal lines of box
	UiLcdHy28_DrawStraightLine(0,
    		CW_snap_carrier_Y + 6,
            27,
            LCD_DIR_HORIZONTAL,
            color);
	UiLcdHy28_DrawStraightLine(32,
    		CW_snap_carrier_Y + 6,
            27,
            LCD_DIR_HORIZONTAL,
            color);
	UiLcdHy28_DrawStraightLine(0,
    		CW_snap_carrier_Y - 1,
            27,
            LCD_DIR_HORIZONTAL,
            color);
	UiLcdHy28_DrawStraightLine(32,
    		CW_snap_carrier_Y - 1,
            27,
            LCD_DIR_HORIZONTAL,
            color);
	// vertical lines of box
	UiLcdHy28_DrawStraightLine(0,
    		CW_snap_carrier_Y - 1,
            8,
            LCD_DIR_VERTICAL,
            color);
	UiLcdHy28_DrawStraightLine(58,
    		CW_snap_carrier_Y - 1,
            8,
            LCD_DIR_VERTICAL,
            color);
#if 0
    UiLcdHy28_DrawStraightLineDouble(((float32_t)POS_SPECTRUM_IND_X + roundf(left_filter_border_pos)), (POS_SPECTRUM_IND_Y + POS_SPECTRUM_FILTER_WIDTH_BAR_Y), roundf(width_pixel), LCD_DIR_HORIZONTAL, clr);
    UiLcdHy28_DrawStraightLine( marker_line_pos[idx],
            spec_top_y - spec_height_limit,
            spec_height_limit,
            LCD_DIR_VERTICAL,
            sd.scope_centre_grid_colour_active);
#endif
}

void ui_spectrum_cw_snap_display (float32_t delta)
{
	#define max_delta 140.0
	#define divider 5.0
	//    static float32_t old_delta = 0.0;
	// delta is the offset from the carrier freq
	// now we have to account for CW sidetones etc.

//	delta = RadioManagement_GetTXDialFrequency()/TUNE_MULT;
//	delta = RadioManagement_GetRXDialFrequency()/TUNE_MULT;
    if(ts.cw_lsb)
    {
    	delta = delta + (float32_t)(ts.cw_sidetone_freq);
    }
    else
    {
    	delta = delta - (float32_t)(ts.cw_sidetone_freq);
    }

    static int old_delta_p = 0.0;
    if(delta > max_delta)
    {
    	delta = max_delta;
    }
    else if(delta < -max_delta)
    {
    	delta = -max_delta;
    }
//    delta = 0.1 * delta + 0.9 * old_delta;

    int delta_p = (int)(0.5 + (delta / divider));

    if(delta_p != old_delta_p)
    {
	UiLcdHy28_DrawStraightLineDouble( CW_snap_carrier_X + old_delta_p + 1,
    		CW_snap_carrier_Y,
            6,
            LCD_DIR_VERTICAL,
            Black);

	UiLcdHy28_DrawStraightLineDouble( CW_snap_carrier_X + delta_p + 1,
    		CW_snap_carrier_Y,
            6,
            LCD_DIR_VERTICAL,
            Yellow);
//	old_delta = delta;
	old_delta_p = delta_p;
    }
}


void UiSpectrum_Calculate_snap(float32_t Lbin, float32_t Ubin, int posbin, float32_t bin_BW)
{
	if(ads.CW_signal) // this is only done, if there has been a pulse from the CW station that exceeds the threshold
		// in the CW decoder section
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
    if(delta > 300.0)
    {
    	delta = 0.0;
    }
    help_freq = help_freq + delta;
    help_freq = 0.2 * help_freq + 0.8 * freq_old;
    //help_freq = help_freq * ((float32_t)TUNE_MULT);
    ads.snap_carrier_freq = (ulong) (help_freq);
    freq_old = help_freq;
    ui_spectrum_cw_snap_display (delta);
    // this estimated carrier freq is then printed in the small
    // frequency display by UiDriver_MainHandler

/*
		5. display delta + Rx-frequency in small freq display (like SAM freq)
		6. new display
	 * */
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
    // this code also calculates the basis for the S-Meter (in sm.dbm and sm.dbmhz), if not in old school S-Meter mode
    //
    {
        if( ts.txrx_mode == TRX_MODE_RX && ((ts.s_meter != DISPLAY_S_METER_STD) || (ts.display_dbm != DISPLAY_S_METER_STD )))
        {
            const float32_t slope = 19.8; // 19.6; --> empirical values derived from measurements by DL8MBY, 2016/06/30, Thanks!
            const float32_t cons = ts.dbm_constant - 225; // -225; //- 227.0;
            const int buff_len_int = FFT_IQ_BUFF_LEN;
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

            // take care of filter bandwidths that are larger than the displayed FFT bins
            if(Lbin < 0)
            {
                Lbin = 0;
            }
            if (Ubin > 255)
            {
                Ubin = 255;
            }

            for(int32_t i = 0; i < (buff_len_int/4); i++)
            {
                sd.FFT_Samples[SPEC_BUFF_LEN - i - 1] = sd.FFT_MagData[i + buff_len_int/4] * SCOPE_PREAMP_GAIN;	// get data
            }
            for(int32_t i = buff_len_int/4; i < (buff_len_int/2); i++)
            {
                sd.FFT_Samples[SPEC_BUFF_LEN - i - 1] = sd.FFT_MagData[i - buff_len_int/4] * SCOPE_PREAMP_GAIN;	// get data
            }

            // here would be the right place to start with the SNAP mode!
            if(cw_decoder_config.snap_enable && ts.dmod_mode == DEMOD_CW)
            {
            	 UiSpectrum_Calculate_snap(Lbin, Ubin, posbin, bin_BW);

            }

            float32_t sum_db = 0.0;
            // determine the sum of all the bin values in the passband
            for (int c = Lbin; c <= (int)Ubin; c++)   // sum up all the values of all the bins in the passband
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



