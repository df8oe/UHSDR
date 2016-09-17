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
 **  Licence:      CC BY-NC-SA 3.0                                                **
 ************************************************************************************/
#include <stdio.h>
#include "ui_spectrum.h"
#include "ui_lcd_hy28.h"
// For spectrum display struct
#include "audio_driver.h"
#include "ui_driver.h"
#include "ui_menu.h"
#include "ui_rotary.h" // dial frequency df
#include "waterfall_colours.h"
// ------------------------------------------------
// Spectrum display public
SpectrumDisplay  __attribute__ ((section (".ccm")))       sd;
// this data structure is now located in the Core Connected Memory of the STM32F4
// this is highly hardware specific code. This data structure nicely fills the 64k with roughly 60k.
// If this data structure is being changed,  be aware of the 64k limit. See linker script arm-gcc-link.ld

// Variables for dbm display --> void calculate_dBm
// float32_t dbm = 0.0;
//float32_t dbm_old = 0.0;
float32_t m_AttackAvedbm = 0.0;
float32_t m_DecayAvedbm = 0.0;
float32_t m_AverageMagdbm = 0.0;
float32_t m_AttackAvedbmhz = 0.0;
float32_t m_DecayAvedbmhz = 0.0;
float32_t m_AverageMagdbmhz = 0.0;
// ALPHA = 1 - e^(-T/Tau), T = 0.02s (because dbm routine is called every 20ms!)
// Tau     ALPHA
//  10ms   0.8647
//  30ms   0.4866
//  50ms   0.3297
// 100ms   0.1812
// 500ms   0.0391
float32_t m_AttackAlpha = 0.8647;
float32_t m_DecayAlpha  = 0.3297;

static void 	UiDriverFFTWindowFunction(char mode);
static void     UiSpectrum_FrequencyBarText(void);
static void		calculate_dBm(void);

static void UiDriverFFTWindowFunction(char mode)
{
    ulong i;
    float32_t gcalc;
    gcalc = 1/ads.codec_gain_calc;				// Get gain setting of codec and convert to multiplier factor
    float32_t s;

    // Information on these windowing functions may be found on the internet - check the Wikipedia article "Window Function"
    // KA7OEI - 20150602

    switch(mode)
    {
    case FFT_WINDOW_RECTANGULAR:	// No processing at all - copy from "Samples" buffer to "Windat" buffer
//			arm_copy_f32((float32_t *)sd.FFT_Windat, (float32_t *)sd.FFT_Samples,FFT_IQ_BUFF_LEN);	// use FFT data as-is
        break;
    case FFT_WINDOW_COSINE:			// Sine window function (a.k.a. "Cosine Window").  Kind of wide...
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = arm_sin_f32((PI * (float32_t)i)/FFT_IQ_BUFF_LEN - 1) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_BARTLETT:		// a.k.a. "Triangular" window - Bartlett (or Fej?r) window is special case where demonimator is "N-1". Somewhat better-behaved than Rectangular
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (1 - fabs(i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_WELCH:			// Parabolic window function, fairly wide, comparable to Bartlett
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (1 - ((i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF)*((i - ((float32_t)FFT_IQ_BUFF_M1_HALF))/(float32_t)FFT_IQ_BUFF_M1_HALF)) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_HANN:			// Raised Cosine Window (non zero-phase version) - This has the best sidelobe rejection of what is here, but not as narrow as Hamming.
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = 0.5 * (float32_t)((1 - (arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_HAMMING:		// Another Raised Cosine window - This is the narrowest with reasonably good sidelobe rejection.
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (float32_t)((0.53836 - (0.46164 * arm_cos_f32(PI*2 * (float32_t)i / (float32_t)(FFT_IQ_BUFF_LEN-1)))) * sd.FFT_Samples[i]);
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_BLACKMAN:		// Approx. same "narrowness" as Hamming but not as good sidelobe rejection - probably best for "default" use.
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (0.42659 - (0.49656*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.076849*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    case FFT_WINDOW_NUTTALL:		// Slightly wider than Blackman, comparable sidelobe rejection.
        for(i = 0; i < FFT_IQ_BUFF_LEN; i++)
        {
            s = (0.355768 - (0.487396*arm_cos_f32((2*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) + (0.144232*arm_cos_f32((4*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1)) - (0.012604*arm_cos_f32((6*PI*(float32_t)i)/(float32_t)FFT_IQ_BUFF_LEN-1))) * sd.FFT_Samples[i];
            sd.FFT_Samples[i] = s * gcalc;
        }
        break;
    }
    //
    // used for debugging
//		char txt[32];
//		sprintf(txt, " %d    ", (int)(c1));
//		UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 20),txt,White,Grid,0);
}
//
//
//
//


static int8_t UiSpectrum_GetGridCenterLine(int8_t reference) {
    int8_t c = 0;
    switch (ts.iq_freq_mode)
    {
    case FREQ_IQ_CONV_P12KHZ:
        c = -2;
        break;
    case FREQ_IQ_CONV_P6KHZ:
        c = -1;
        break;
    case FREQ_IQ_CONV_M6KHZ:
        c = 1;
        break;
    case FREQ_IQ_CONV_M12KHZ:
        c = 2;
        break;
    }

    if(sd.magnify)
  	  c = 0;

    return c + reference;
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiSpectrumCreateDrawArea
//* Object              : draw the spectrum scope control
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiSpectrumCreateDrawArea(void)
{
    ulong i;
    uint32_t clr;

    //
    // get grid colour of all but center line
    //
    UiMenu_MapColors(ts.scope_grid_colour,NULL, &ts.scope_grid_colour_active);
    if(ts.scope_grid_colour == SPEC_GREY)
    {
        ts.scope_grid_colour_active = Grid;
    }
    else
    {
        UiMenu_MapColors(ts.scope_grid_colour,NULL, &ts.scope_grid_colour_active);
    }
    //
    //
    // Get color of center vertical line of spectrum scope
    //
    if(ts.spectrum_centre_line_colour == SPEC_GREY)
    {
        ts.scope_centre_grid_colour_active = Grid;
    }
    else
    {
        UiMenu_MapColors(ts.spectrum_centre_line_colour,NULL, &ts.scope_centre_grid_colour_active);
    }

    // Clear screen where frequency information will be under graticule
    //
    UiLcdHy28_PrintText(POS_SPECTRUM_IND_X - 2, POS_SPECTRUM_IND_Y + 60, "                                 ", Black, Black, 0);

    // Frequency bar separator
    UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y + POS_SPECTRUM_IND_H - 20),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);

    // Draw Frequency bar text
    sd.dial_moved = 1; // TODO: HACK: always print frequency bar under spectrum display
    UiSpectrum_FrequencyBarText();

    // draw centre line indicating the receive frequency

    ts.c_line = UiSpectrum_GetGridCenterLine(4);

    UiLcdHy28_DrawStraightLine (POS_SPECTRUM_IND_X + 32*ts.c_line + 1, (POS_SPECTRUM_IND_Y - 4 - SPEC_LIGHT_MORE_POINTS), (POS_SPECTRUM_IND_H - 15) + SPEC_LIGHT_MORE_POINTS, LCD_DIR_VERTICAL, ts.scope_centre_grid_colour_active);

    if(!ts.spectrum_size)		//don't draw text bar when size is BIG
    {
        // Draw top band = grey box in which text is printed
        for(i = 0; i < 16; i++)
        {
            UiLcdHy28_DrawHorizLineWithGrad(POS_SPECTRUM_IND_X,(POS_SPECTRUM_IND_Y - 20 + i),POS_SPECTRUM_IND_W,COL_SPECTRUM_GRAD);
        }

		char bartext[34];

        // Top band text - middle caption
        UiGet_Wfscope_Bar_Text(bartext);
        UiLcdHy28_PrintTextCentered(
                    POS_SPECTRUM_IND_X,
                    (POS_SPECTRUM_IND_Y - 18),
                    POS_SPECTRUM_IND_W,
                    bartext,
                    White,
                    RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);

        // Draw control left and right border
        for(i = 0; i < 2; i++)
        {
            UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X - 2 + i),
                    (POS_SPECTRUM_IND_Y - 20),
                    (POS_SPECTRUM_IND_H + 12),
                    LCD_DIR_VERTICAL,
                    //									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
                    ts.scope_grid_colour_active);

            UiLcdHy28_DrawStraightLine(	(POS_SPECTRUM_IND_X + POS_SPECTRUM_IND_W - 2 + i),
                    (POS_SPECTRUM_IND_Y - 20),
                    (POS_SPECTRUM_IND_H + 12),
                    LCD_DIR_VERTICAL,
                    //									RGB(COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD,COL_SPECTRUM_GRAD));
                    ts.scope_grid_colour_active);
        }

    }
    /////////////////////////////// was here /////////////////////////////////////
    // Is (scope enabled AND NOT scope light) or Waterfall?
    if (!((ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE) || (ts.flags1 & FLAGS1_WFALL_SCOPE_TOGGLE)))
    { // if spectrum display light enabled, bail out here!


        // Horizontal grid lines
        char upperline, y_add;
        if(ts.spectrum_size)		//set range big/normal
        {
            upperline = 6;
            y_add = 32;
        }
        else
        {
            upperline = 4;
            y_add = 0;
        }

        for(i = 1; i < upperline; i++)
        {
            // Save y position for repaint
            sd.horz_grid_id[i - 1] = (POS_SPECTRUM_IND_Y - 5 - y_add + i*16);

            // Draw
            UiLcdHy28_DrawStraightLine(	POS_SPECTRUM_IND_X,
                    sd.horz_grid_id[i - 1],
                    POS_SPECTRUM_IND_W,
                    LCD_DIR_HORIZONTAL,
                    ts.scope_grid_colour_active);
        }

//        ts.c_line = UiSpectrum_GetGridCenterLine(4);
        // Vertical grid lines
        for(i = 1; i < 8; i++)
        {

            if (i == ts.c_line)
            {
                clr = ts.scope_centre_grid_colour_active;
            }
            else
            {
                clr = ts.scope_grid_colour_active;								// normal color if other lines
            }
            // Save x position for repaint
            sd.vert_grid_id[i - 1] = (POS_SPECTRUM_IND_X + 32*i + 1);

            // Draw
            UiLcdHy28_DrawStraightLine(	sd.vert_grid_id[i - 1],
                    (POS_SPECTRUM_IND_Y -  4 - y_add/2),
                    (POS_SPECTRUM_IND_H - 15 + y_add/2),
                    LCD_DIR_VERTICAL,
                    clr);

        }

        if (((ts.flags1 & FLAGS1_WFALL_SCOPE_TOGGLE) && (!ts.waterfall_speed)) || (!(ts.flags1 & FLAGS1_WFALL_SCOPE_TOGGLE) && (!ts.scope_speed)))
        {
            // print "disabled" in the middle of the screen if the waterfall or scope was disabled
            UiLcdHy28_PrintText(			(POS_SPECTRUM_IND_X + 72),
                    (POS_SPECTRUM_IND_Y + 18),
                    "   DISABLED   ",
                    Grey,
                    RGB((COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2),(COL_SPECTRUM_GRAD*2)),0);
        }
    }
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverClearSpectrumDisplay
//* Object              : Clears the spectrum display
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiSpectrumClearDisplay(void)
{
    UiLcdHy28_DrawFullRect(POS_SPECTRUM_IND_X - 2, (POS_SPECTRUM_IND_Y - 22), 94, 264, Black);	// Clear screen under spectrum scope by drawing a single, black block (faster with SPI!)
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

static inline bool UiSpectrum_Draw_IsVgrid(const uint16_t x, const uint16_t color_new, uint16_t* clr_ptr, const uint16_t x_center_line)
{
    bool repaint_v_grid = false;

    if (x == x_center_line)
    {
        *clr_ptr = ts.scope_centre_grid_colour_active;
        repaint_v_grid = true;
    }
    else
    {
        int k;
        // Enumerate all saved x positions
        for(k = 0; k < 7; k++)
        {
            // Exit on match
            if(x == sd.vert_grid_id[k])
            {
                *clr_ptr = ts.scope_grid_colour_active;
                repaint_v_grid = true;
                break;
                // leave loop, found match
            }
        }
    }
    return repaint_v_grid;
}

static uint16_t UiSpectrum_Draw_GetCenterLineX()
{
    static uint16_t idx;

    static const uint16_t  center[FREQ_IQ_CONV_MODE_MAX+1] = { 3,2,4,1,5 };
    // list the idx for the different modes (which are numbered from 0)
    // the list static const in order to have it in flash
    // it would be faster in ram but this is not necessary

    if(sd.magnify)
    {
        idx = 3;
    }
    else
    {
        idx = center[ts.iq_freq_mode];
    }
    return sd.vert_grid_id[idx];
}


void    UiSpectrumDrawSpectrum(q15_t *fft_old, q15_t *fft_new, const ushort color_old, const ushort color_new, const ushort shift)
{

    int spec_height = SPECTRUM_HEIGHT; //x
    int spec_start_y = SPECTRUM_START_Y;

    //    if ((ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE) && ts.spectrum_size == SPECTRUM_BIG)
    if (ts.spectrum_size == SPECTRUM_BIG)
    {
        spec_height = spec_height + SPEC_LIGHT_MORE_POINTS;
        spec_start_y = spec_start_y - SPEC_LIGHT_MORE_POINTS;
    }
    static uint16_t pixel_buf[SPECTRUM_HEIGHT+SPEC_LIGHT_MORE_POINTS];

    uint16_t      i, k, x, y_old , y_new, y1_old, y1_new, len_old, sh, clr;
	uint16_t 	  y1_new_minus = 0;
	uint16_t	  y1_old_minus = 0;
    uint16_t 	  idx = 0;
    bool      repaint_v_grid = false;
    clr = color_new;

    uint16_t x_center_line = UiSpectrum_Draw_GetCenterLineX();


    if(shift)
//        sh = (SPECTRUM_WIDTH/2)-1;   // Shift to fill gap in center
    	sh = (SPECTRUM_WIDTH/2) + 1;   // Shift to fill gap in center
    else
        sh = 1;                  // Shift to fill gap in center

    if (sd.first_run>0)
    {
        idx = 0;
        for(x = (SPECTRUM_START_X + sh + 0); x < (POS_SPECTRUM_IND_X + SPECTRUM_WIDTH/2 + sh); x++)
        {
            y_new = fft_new[idx++];

            if(y_new > (spec_height - 7))
                y_new = (spec_height - 7);
            y1_new  = (spec_start_y + spec_height - 1) - y_new;
            if (!(ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE))
            {
                UiLcdHy28_DrawStraightLine(x,y1_new,y_new,LCD_DIR_VERTICAL,color_new);
            }
            //			else
            //	UiLcdHy28_DrawColorPoint (x, y1_new, color_new);
//            y1_new_minus = y1_new;
//            y1_old_minus = y1_new;
        }
        sd.first_run--;
    }
    else
    {

        idx = 0;

        for(x = (SPECTRUM_START_X + sh + 0); x < (POS_SPECTRUM_IND_X + SPECTRUM_WIDTH/2 + sh); x++)
        {
            if (ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE)

            {
                if ((idx > 1) && (idx < 126)) //
                {
                    // moving window - weighted average of 5 points of the spectrum to smooth spectrum in the frequency domain
                    // weights:  x: 50% , x-1/x+1: 36%, x+2/x-2: 14%
                    y_old = fft_old[idx] *0.5 + fft_old[idx-1]*0.18 + fft_old[idx-2]*0.07 + fft_old[idx+1]*0.18 + fft_old[idx+2]*0.07;
                    y_new = fft_new[idx] *0.5 + fft_new[idx-1]*0.18 + fft_new[idx-2]*0.07 + fft_new[idx+1]*0.18 + fft_new[idx+2]*0.07;
                }
                else
                {
                    y_old = fft_old[idx];
                    y_new = fft_new[idx];
                }

                idx++;

            }
            else
            {
                y_old = *fft_old++;
                y_new = *fft_new++;
            }

            // Limit vertical
            if(y_old > (spec_height - 7))
                y_old = (spec_height - 7);

            if(y_new > (spec_height - 7))
                y_new = (spec_height - 7);

            // Data to y position and length
            y1_old  = (spec_start_y + spec_height - 1) - y_old;
            len_old = y_old;

            y1_new  = (spec_start_y + spec_height - 1) - y_new;

            //            if (y1_old != y1_new && (ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE) && x != (POS_SPECTRUM_IND_X + 32*ts.c_line + 1))

            if (x == SPECTRUM_START_X + 1) // special case of first x position of spectrum
            {
            	y1_old_minus = y1_old;
            	y1_new_minus = y1_new;
            }

        	if (x == SPECTRUM_START_X + (SPECTRUM_WIDTH/2) + 1) // special case of first line of right part of spectrum
        	{
        		y1_old_minus = (spec_start_y + spec_height - 1) - sd.FFT_BkpData[255];
        		y1_new_minus = (spec_start_y + spec_height - 1) - sd.FFT_DspData[255];
        	}

            if ((ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE) && x != (POS_SPECTRUM_IND_X + 32*ts.c_line + 1))
            {
                // x position is not on vertical centre line (the one that indicates the receive frequency)

            	// here I would like to draw a line if y1_new and the last drawn pixel (y1_new_minus) are more than 1 pixel apart in the vertical axis
            	// makes the spectrum display look more complete . . .
            	//


            	if(y1_old - y1_old_minus > 1) // && x !=(SPECTRUM_START_X + sh))
            	 { // plot line upwards
            		UiLcdHy28_DrawStraightLine(x,y1_old_minus + 1,y1_old - y1_old_minus,LCD_DIR_VERTICAL,color_old);
            	 }
            	else if (y1_old - y1_old_minus < -1) // && x !=(SPECTRUM_START_X + sh))
            	 { // plot line downwards
            		UiLcdHy28_DrawStraightLine(x,y1_old,y1_old_minus-y1_old,LCD_DIR_VERTICAL,color_old);
            	 }
            	else
            	 {
            		  UiLcdHy28_DrawColorPoint (x, y1_old, color_old);
            	 }

            	if(y1_new - y1_new_minus > 1) // && x !=(SPECTRUM_START_X + sh))
            	 { // plot line upwards
                     UiLcdHy28_DrawStraightLine(x,y1_new_minus + 1,y1_new - y1_new_minus,LCD_DIR_VERTICAL,color_new);

            	 }
            	else if (y1_new - y1_new_minus < -1) // && x !=(SPECTRUM_START_X + sh))
            	 { // plot line downwards
                     UiLcdHy28_DrawStraightLine(x,y1_new,y1_new_minus - y1_new,LCD_DIR_VERTICAL,color_new);

            	 }
            	 else
            	 {
            		  UiLcdHy28_DrawColorPoint (x, y1_new, color_new);
            	 }

            }
            y1_new_minus = y1_new;
            y1_old_minus = y1_old;


            if (!(ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE))
            {
                if(y_old <= y_new)
                {
                    // is old line going to be overwritten by new line, anyway?
                    // ----------------------------------------------------------
                    //
                    if (y_old != y_new)
                    {
                        UiLcdHy28_DrawStraightLine(x,y1_new,y_new-y_old,LCD_DIR_VERTICAL,color_new);
                    }
                }
                else
                {

                    // Repaint vertical grid on clear
                    // Basically paint over the grid is allowed
                    // but during spectrum clear instead of masking
                    // grid lines with black - they are repainted
                    // TODO: This code is  always executed, since this function is always called with color_old == Black
                    repaint_v_grid = UiSpectrum_Draw_IsVgrid(x, color_new, &clr, x_center_line);
                    //
                    UiLcdHy28_OpenBulkWrite(x, 1, y1_old, len_old);

                    idx = 0;
                    // Draw vertical line, starting only with position of where new line would be!
                    for(i = y_new; i < len_old; i++)
                    {
                        // Do not check for horizontal grid when we have vertical masking
                        if(!repaint_v_grid)
                        {
                            clr = color_old;

                            // Are we trying to paint over horizontal grid line ?
                            // prevent that by changing this points color to the grid color
                            // TODO: This code is  always executed, since this function is always called with color_old == Black
                            // This code does not make sense to me: it should check if the CURRENT y value (stored in i) is a horizontal
                            // grid, not the y1_old.
                            // Enumerate all saved y positions
                            char upperline;
                            if(ts.spectrum_size)		//set range big/normal for redraw
                                upperline = 5;
                            else
                                upperline = 3;

                            for(k = 0; k < upperline; k++)
                            {
                                if(y1_old == sd.horz_grid_id[k])
                                {
                                    clr = ts.scope_grid_colour_active;
                                    break;
                                }
                            }

                        }

                        pixel_buf[idx++] = clr;
                        // Track absolute position
                        y1_old++;
                    }
                    UiLcdHy28_BulkWrite(pixel_buf,len_old-y_new);
                    UiLcdHy28_CloseBulkWrite();
                    // Reset flag
                    if(repaint_v_grid)
                        repaint_v_grid = 0;
                    UiLcdHy28_DrawStraightLine(	sd.vert_grid_id[ts.c_line - 1],
                                                (POS_SPECTRUM_IND_Y -  4),
                                                (POS_SPECTRUM_IND_H - 15),
                                                LCD_DIR_VERTICAL,
//									RGB((COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD),(COL_SPECTRUM_GRAD)));
                                                ts.scope_centre_grid_colour_active);

                }
            }
        }
    }
}

static inline const uint32_t FftIdx2BufMap(const uint32_t idx)
{
    return (FFT_IQ_BUFF_LEN/4 + idx)%(FFT_IQ_BUFF_LEN/2);
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverInitSpectrumDisplay - for both "Spectrum Display" and "Waterfall" Display
//* Object              : FFT init
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiSpectrum_InitSpectrumDisplay()
{
    ulong i;
//	arm_status	a;

    // Init publics
    sd.state 		= 0;
    sd.samp_ptr 	= 0;
    sd.skip_process = 0;
    sd.enabled		= 0;
    sd.dial_moved	= 0;
    //
    sd.rescale_rate = (float)ts.scope_rescale_rate;	// load rescale rate
    sd.rescale_rate = 1/sd.rescale_rate;				// actual rate is inverse of this setting
    //
    sd.agc_rate = (float)ts.scope_agc_rate;	// calculate agc rate
    sd.agc_rate = sd.agc_rate/SPECTRUM_AGC_SCALING;
    //
    sd.mag_calc = 1;				// initial setting of spectrum display scaling factor
    //
    sd.wfall_line_update = 0;		// init count used for incrementing number of lines for each vertical waterfall screen update
    //
    // load buffer containing waterfall colours
    //
    for(i = 0; i < NUMBER_WATERFALL_COLOURS; i++)
    {
        switch(ts.waterfall_color_scheme)
        {
        case WFALL_HOT_COLD:
            sd.waterfall_colours[i] = waterfall_cold_hot[i];
            break;
        case WFALL_RAINBOW:
            sd.waterfall_colours[i] = waterfall_rainbow[i];
            break;
        case WFALL_BLUE:
            sd.waterfall_colours[i] = waterfall_blue[i];
            break;
        case WFALL_GRAY_INVERSE:
            sd.waterfall_colours[i] = waterfall_grey_inverse[i];
            break;
        case WFALL_GRAY:
        default:
            sd.waterfall_colours[i] = waterfall_grey[i];
            break;
        }
    }
    //
    //
    // Load "top" color of palette (the 65th) with that to be used for the center grid color
    //
    sd.waterfall_colours[NUMBER_WATERFALL_COLOURS] = (ushort)ts.scope_centre_grid_colour_active;
    //
    //
    /*
    	//
    	// Load waterfall data with "splash" showing palette
    	//
    	j = 0;					// init count of lines on display
    	k = sd.wfall_line;		// start with line currently displayed in buffer
    	while(j < SPECTRUM_HEIGHT)	{		// loop number of times of buffer
    		for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	{		// do this all of the way across, horizonally
    			sd.waterfall[k][i] = (SPECTRUM_HEIGHT - j) % SPECTRUM_HEIGHT;	// place the color of the palette, indexed to vertical position
    		}
    		j++;		// update line count
    		k++;		// update position within circular buffer - which also is used to calculate color
    		k %= SPECTRUM_HEIGHT;	// limit to modulus count of circular buffer size
    	}
    	//
    */
    //
    switch(ts.spectrum_db_scale)
    {
    case	DB_DIV_5:
        sd.db_scale = DB_SCALING_5;
        break;
    case	DB_DIV_7:
        sd.db_scale = DB_SCALING_7;
        break;
    case	DB_DIV_15:
        sd.db_scale = DB_SCALING_15;
        break;
    case	DB_DIV_20:
        sd.db_scale = DB_SCALING_20;
        break;
    case S_1_DIV:
        sd.db_scale = DB_SCALING_S1;
        break;
    case S_2_DIV:
        sd.db_scale = DB_SCALING_S2;
        break;
    case S_3_DIV:
        sd.db_scale = DB_SCALING_S3;
        break;
    case	DB_DIV_10:
    default:
        sd.db_scale = DB_SCALING_10;
        break;
    }
    //
    if(ts.spectrum_size == SPECTRUM_NORMAL)	 						// waterfall the same size as spectrum scope
    {
        sd.wfall_height = SPECTRUM_HEIGHT - SPECTRUM_SCOPE_TOP_LIMIT;
        sd.wfall_ystart = SPECTRUM_START_Y + SPECTRUM_SCOPE_TOP_LIMIT;
        sd.wfall_size = SPECTRUM_HEIGHT - SPECTRUM_SCOPE_TOP_LIMIT;
    }																	// waterfall larger, covering the word "Waterfall Display"
    else if(ts.spectrum_size == SPECTRUM_BIG)
    {
        sd.wfall_height = SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL;
        sd.wfall_ystart = SPECTRUM_START_Y - WFALL_MEDIUM_ADDITIONAL;
        sd.wfall_size = SPECTRUM_HEIGHT + WFALL_MEDIUM_ADDITIONAL;
    }
    //
    //
    sd.wfall_contrast = (float)ts.waterfall_contrast;		// calculate scaling for contrast
    sd.wfall_contrast /= 100;

    // Init FFT structures
//	a = arm_rfft_init_f32((arm_rfft_instance_f32 *)&sd.S,(arm_cfft_radix4_instance_f32 *)&sd.S_CFFT,FFT_IQ_BUFF_LEN,FFT_QUADRATURE_PROC,1);
//	arm_rfft_fast_init_f32((arm_rfft_fast_instance_f32 *)&sd.S_fast,FFT_IQ_BUFF_LEN);
//	const static arm_cfft_instance_f32 *S;
//	C = &arm_cfft_sR_f32_len256;

//	if(a != ARM_MATH_SUCCESS)
//	{
//		return;
//	}

    // Ready
    sd.enabled		= 1;
    sd.first_run 	= 2;
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverReDrawSpectrumDisplay
//* Object              : state machine implementation
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
// Spectrum Display code rewritten by C. Turner, KA7OEI, September 2014, May 2015
//
void UiSpectrumReDrawScopeDisplay()
{
    int spec_height = SPECTRUM_HEIGHT;
    if ((ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE) && ts.spectrum_size == SPECTRUM_BIG)
    {
        spec_height = spec_height + SPEC_LIGHT_MORE_POINTS;
    }
    ulong i, spec_width;
    uint32_t	max_ptr;	// throw-away pointer for ARM maxval and minval functions
//	float32_t	gcalc;
    //

    // Only in RX mode and NOT while powering down or in menu mode or if displaying memory information
    if((ts.txrx_mode != TRX_MODE_RX) || (ts.powering_down) || (ts.menu_mode) || (ts.mem_disp) || (ts.boot_halt_flag))
        return;

    if((ts.spectrum_scope_scheduler) || (!ts.scope_speed))	// is it time to update the scan, or is this scope to be disabled?
        return;
    else
        ts.spectrum_scope_scheduler = (ts.scope_speed-1)*2;

    // No spectrum display in DIGI modes
    //if(ts.dmod_mode == DEMOD_DIGI)
    //	return;

    // Nothing to do here otherwise, or if scope is to be held off while other parts of the display are to be updated or the LCD is being blanked
    if((!sd.enabled) || (ts.lcd_blanking_flag))
        return;

    // The state machine will rest
    // in between states
//	sd.skip_process++;
//	if(sd.skip_process < 1000)
//		return;

//	sd.skip_process = 0;

//	gcalc = 1/ads.codec_gain_calc;				// Get gain setting of codec and convert to multiplier factor

    // Process implemented as state machine
    switch(sd.state)
    {
    //
    // Apply gain to collected IQ samples and then do FFT
    //
    case 1:
    {
        // with the new FFT lib arm_cfft we need to put the input and output samples into one buffer, therefore
        // UiDriverFFTWindowFunction was changed
        // new arm_cfft lib does not seem to need SCOPE_PREAMP_GAIN any more! Why?
        // gain application of 1/ads.codec_gain_calc is now done in UiDriverFFTWindowFunction to save RAM
        //
//			arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)(gcalc * SCOPE_PREAMP_GAIN), (float32_t *)sd.FFT_Windat, FFT_IQ_BUFF_LEN);	// scale input according to A/D gain
//			arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)(gcalc), (float32_t *)sd.FFT_Windat, FFT_IQ_BUFF_LEN);	// scale input according to A/D gain
//			arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)(gcalc), (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN);	// scale input according to A/D gain
        //
        UiDriverFFTWindowFunction(ts.fft_window_type);		// do windowing function on input data to get less "Bin Leakage" on FFT data
        //
//			arm_rfft_f32((arm_rfft_instance_f32 *)&sd.S,(float32_t *)(sd.FFT_Windat),(float32_t *)(sd.FFT_Samples));	// Do FFT
//			arm_rfft_fast_f32((arm_rfft_fast_instance_f32 *)&sd.S_fast,(float32_t *)(sd.FFT_Windat),(float32_t *)(sd.FFT_Samples),0);	// Do FFT
        //        arm_cfft_f32(&arm_cfft_sR_f32_len256,(float32_t *)(sd.FFT_Samples),0,1);	// Do complex FFT with new lib (faster! sexier! more accurate!?)
        arm_cfft_f32(&arm_cfft_sR_f32_len256,(float32_t *)(sd.FFT_Samples),0,1);	// Do complex FFT with new lib (faster! sexier! more accurate!?)

        //
        sd.state++;
        break;
    }
    //
    // Do magnitude processing and gain control (AGC) on input of FFT
    //
    case 2:
    {
        //
        // Save old display data - we will use later to mask pixel on the control
        //
        arm_copy_q15((q15_t *)sd.FFT_DspData, (q15_t *)sd.FFT_BkpData, FFT_IQ_BUFF_LEN/2);
        //
        // Calculate magnitude
        //
        arm_cmplx_mag_f32((float32_t *)(sd.FFT_Samples),(float32_t *)(sd.FFT_MagData),(FFT_IQ_BUFF_LEN/2));
        //
        sd.state++;
        break;
    }
    //
    //  Low-pass filter amplitude magnitude data
    //
    case 3:
    {
        uint32_t i;
        float32_t		filt_factor;
        //
        filt_factor = (float)ts.spectrum_filter;		// use stored filter setting
        filt_factor = 1/filt_factor;		// invert filter factor to allow multiplication
        //
        if(sd.dial_moved)
        {
            sd.dial_moved = 0;	// Dial moved - reset indicator
            //
            UiSpectrum_FrequencyBarText();	// redraw frequency bar on the bottom of the display
            //
        }
        // as I understand this, this calculates an IIR filter first order
        // AVGData = filt_factor * Sample[t] + (1 - filt_factor) * Sample [t - 1]
        //
        arm_scale_f32((float32_t *)sd.FFT_AVGData, (float32_t)filt_factor, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);	// get scaled version of previous data
        arm_sub_f32((float32_t *)sd.FFT_AVGData, (float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// subtract scaled information from old, average data
        arm_scale_f32((float32_t *)sd.FFT_MagData, (float32_t)filt_factor, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);	// get scaled version of new, input data
        arm_add_f32((float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_AVGData, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// add portion new, input data into average
        //
        for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)
        {
            //		// guarantee that the result will always be >= 0
            if(sd.FFT_AVGData[i] < 1)
                sd.FFT_AVGData[i] = 1;
        }
		calculate_dBm();
        sd.state++;

        break;
    }
    //
    // De-linearize and normalize display data and do AGC processing
    //
    case 4:
    {
        q15_t	max1, max2, max3, min1, min2, min3;
        q15_t	mean1, mean2, mean3;
        float32_t	sig;
        //
        // De-linearize data with dB/division
        // AND flip data round ! = mirror values from right to left and vice versa (had to be done because of the new FFT lib) DDD4WH april 2016
        for(i = 0; i < (FFT_IQ_BUFF_LEN/2); i++)
        {
            sig = log10(sd.FFT_AVGData[i]) * sd.db_scale;		// take FFT data, do a log10 and multiply it to scale it to get desired dB/divistion
            sig += sd.display_offset;							// apply "AGC", vertical "sliding" offset (or brightness for waterfall)
            if(sig > 1)											// is the value greater than 1?
//					sd.FFT_DspData[i] = (q15_t)sig;					// it was a useful value - save it
                sd.FFT_DspData[FFT_IQ_BUFF_LEN/2 - i - 1] = (q15_t)sig;					// it was a useful value - save it
            else
                sd.FFT_DspData[FFT_IQ_BUFF_LEN/2 - i - 1] = 1;							// not greater than 1 - assign it to a base value of 1 for sanity's sake
        }
        //
        arm_copy_q15((q15_t *)sd.FFT_DspData, (q15_t *)sd.FFT_TempData, FFT_IQ_BUFF_LEN/2);
        //
        // Find peak and average to vertically adjust display
        //
        if(sd.magnify != 0)	 	// are we in magnify mode?  If so, find max/mean of only those portions of the spectrum magnified - which are NOT in the proper order, dammit!
        {
            //
            if(!ts.iq_freq_mode)	 	// yes, are we NOT in translate mode?
            {
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*3/8], FFT_IQ_BUFF_LEN/8, &max1, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*3/8], FFT_IQ_BUFF_LEN/8, &min1, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*3/8], FFT_IQ_BUFF_LEN/8, &mean1);				// find mean value in center portion
                //
                arm_max_q15((q15_t *)sd.FFT_TempData, FFT_IQ_BUFF_LEN/8, &max2, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)sd.FFT_TempData, FFT_IQ_BUFF_LEN/8, &min2, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)sd.FFT_TempData, FFT_IQ_BUFF_LEN/8, &mean2);				// find mean value in center portion
                //
                if(max2 > max1)
                    max1 = max2;
                //
                if(mean2 > mean1)
                    mean1 = mean2;
                //
                if(min2 < min1)
                    min1 = min2;
            }
            else if(ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ)	 	// we are in RF LO HIGH mode (tuning is below center of screen)
            {
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*5/16], FFT_IQ_BUFF_LEN/8, &max1, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*5/16], FFT_IQ_BUFF_LEN/8, &min1, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*5/16], FFT_IQ_BUFF_LEN/8, &mean1);				// find mean value in center portion
                //
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*7/16], FFT_IQ_BUFF_LEN/16, &max2, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*7/16], FFT_IQ_BUFF_LEN/16, &min2, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*7/16], FFT_IQ_BUFF_LEN/16, &mean2);				// find mean value in center portion
                //
                if(max2 > max1)
                    max1 = max2;
                //
                if(min2 < min1)
                    min1 = min2;
                //
                if(mean2 > mean1)
                    mean1 = mean2;
                //
                arm_max_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &max3, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &min3, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &mean3);				// find mean value in center portion
                //
                if(max3 > max1)
                    max1 = max3;
                //
                if(min3 < min1)
                    min1 = min3;
                //
                if(mean3 > mean1)
                    mean1 = mean3;
            }
            else if(ts.iq_freq_mode == FREQ_IQ_CONV_M6KHZ)	 	// we are in RF LO LOW mode (tuning is above center of screen)
            {
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*7/16], FFT_IQ_BUFF_LEN/16, &max1, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*7/16], FFT_IQ_BUFF_LEN/16, &min1, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*7/16], FFT_IQ_BUFF_LEN/16, &mean1);				// find mean value in center portion
                //
                arm_max_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &max2, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &min2, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &mean2);				// find mean value in center portion
                //
                if(max2 > max1)
                    max1 = max2;
                //
                if(min2 < min1)
                    min1 = min2;
                //
                if(mean2 > mean1)
                    mean1 = mean2;
                //
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/16], FFT_IQ_BUFF_LEN/8, &max3, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/16], FFT_IQ_BUFF_LEN/8, &min3, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/16], FFT_IQ_BUFF_LEN/8, &mean3);				// find mean value in center portion
                //
                if(max3 > max1)
                    max1 = max3;
                //
                if(min2 < min1)
                    min1 = min2;
                //
                if(mean3 > mean1)
                    mean1 = mean3;
            }
            else if(ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ)	 	// we are in RF LO HIGH mode (tuning is below center of screen)		// aaaaaaaaaaaaaaaaaaaaa
            {
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/4], FFT_IQ_BUFF_LEN/8, &max1, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/4], FFT_IQ_BUFF_LEN/8, &min1, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/4], FFT_IQ_BUFF_LEN/8, &mean1);				// find mean value in center portion
                //
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*3/8], FFT_IQ_BUFF_LEN/16, &max2, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*3/8], FFT_IQ_BUFF_LEN/16, &min2, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN*3/8], FFT_IQ_BUFF_LEN/16, &mean2);				// find mean value in center portion
                //
                if(max2 > max1)
                    max1 = max2;
                //
                if(min2 < min1)
                    min1 = min2;
                //
                if(mean2 > mean1)
                    mean1 = mean2;
                //
                arm_max_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &max3, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &min3, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &mean3);				// find mean value in center portion
                //
                if(max3 > max1)
                    max1 = max3;
                //
                if(min3 < min1)
                    min1 = min3;
                //
                if(mean3 > mean1)
                    mean1 = mean3;
            }
            else if(ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ)	 	// we are in RF LO LOW mode (tuning is above center of screen)
            {
                arm_max_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &max1, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &min1, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[0], FFT_IQ_BUFF_LEN/16, &mean1);				// find mean value in center portion
                //
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/16], FFT_IQ_BUFF_LEN/16, &max2, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/16], FFT_IQ_BUFF_LEN/16, &min2, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/16], FFT_IQ_BUFF_LEN/16, &mean2);				// find mean value in center portion
                //
                if(max2 > max1)
                    max1 = max2;
                //
                if(min2 < min1)
                    min1 = min2;
                //
                if(mean2 > mean1)
                    mean1 = mean2;
                //
                arm_max_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/8], FFT_IQ_BUFF_LEN/8, &max3, &max_ptr);		// find maximum element in center portion
                arm_min_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/8], FFT_IQ_BUFF_LEN/8, &min3, &max_ptr);		// find minimum element in center portion
                arm_mean_q15((q15_t *)&sd.FFT_TempData[FFT_IQ_BUFF_LEN/8], FFT_IQ_BUFF_LEN/8, &mean3);				// find mean value in center portion
                //
                if(max3 > max1)
                    max1 = max3;
                //
                if(min2 < min1)
                    min1 = min2;
                //
                if(mean3 > mean1)
                    mean1 = mean3;
            }
        }
        else
        {
            spec_width = FFT_IQ_BUFF_LEN/2;
            arm_max_q15((q15_t *)sd.FFT_TempData, spec_width, &max1, &max_ptr);		// find maximum element
            arm_min_q15((q15_t *)sd.FFT_TempData, spec_width, &min1, &max_ptr);		// find minimum element
            arm_mean_q15((q15_t *)sd.FFT_TempData, spec_width, &mean1);				// find mean value
        }

        //
        // Vertically adjust spectrum scope so that the strongest signals are adjusted to the top
        //
        if(max1 > spec_height)  	// is result higher than display
        {
            sd.display_offset -= sd.agc_rate;	// yes, adjust downwards quickly
//				if(max1 > spec_height+(spec_height/2))			// is it WAY above top of screen?
//					sd.display_offset -= sd.agc_rate*3;	// yes, adjust downwards REALLY quickly
        }
        //
        // Prevent "empty" spectrum display from filling with "noise" by checking the peak/average of what was found
        //
        else if(((max1*10/mean1) <= (q15_t)ts.spectrum_scope_nosig_adjust) && (max1 < spec_height+(spec_height/2)))	 	// was "average" signal ratio below set threshold and average is not insanely strong??
        {
            if((min1 > 2) && (max1 > 2))	 		// prevent the adjustment from going downwards, "into the weeds"
            {
                sd.display_offset -= sd.agc_rate;	// yes, adjust downwards
                if(sd.display_offset < (-(spec_height + SPECTRUM_SCOPE_ADJUST_OFFSET)))
                    sd.display_offset = (-(spec_height + SPECTRUM_SCOPE_ADJUST_OFFSET));
            }
        }
        else
            sd.display_offset += (sd.agc_rate/3);	// no, adjust upwards more slowly
        //
        //
        if((min1 <= 2) && (max1 <= 2))	 	// ARGH - We must already be in the weeds, below the bottom - let's adjust upwards quickly to get it back onto the display!
        {
            sd.display_offset += sd.agc_rate*10;
        }
        //
        // used for debugging
//				char txt[32];
//				sprintf(txt, " %d,%d,%d,%d ", (int)(max1*100/mean1), (int)(min1), (int)(max1),(int)mean1);
//				sprintf(txt, " %d,%d,%d,%d ", (int)sd.display_offset*100, (int)min1*100,(int)max1*100,(int)spec_height);
//				UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 20),txt,White,Grid,0);

        //
        //
        ushort ptr;
        //
        // Now, re-arrange the spectrum for the various magnify modes so that they display properly!
        //
        // we can calculate any position in the spectrum by using the
        // following thinking
        // the spectrum is 256 entries wide == FFT_IQ_BUFF_LEN/2
        // the begin of the spectrum (== -24khz) is in the middle of the buffer
        // i.e. 0 == FFT_IQ_BUFF_LEN/2/2 == 128
        // that means (FFT_IQ_BUFF_LEN/4 + idx)%FFT_IQ_BUFF_LEN/2 == (128+idx)%256
        // gives us the index in the buffer.
        // we use this  knowledge to simplify the magnification code
        // compiler can heavily optimize this since we  all these values being power of 2 value
//        if(sd.magnify != 0)	 	// is magnify mode on?
        if(0)	 	// FIXME
        { // we don�t need all this any more, because the new Zoom FFT takes care of that, DD4WH, Aug 15th, 2016
            uint32_t end_range;
            switch(ts.iq_freq_mode)
            {
                break;
            case FREQ_IQ_CONV_P6KHZ:	// frequency translate mode is in "RF LO HIGH" mode - tune below center of screen
                ptr = FFT_IQ_BUFF_LEN/16 ; // FFT_IQ_BUFF_LEN/8 + FFT_IQ_BUFF_LEN/16  = -12khz + 6khz = -6khz <-> + 18khz
                break;
            case FREQ_IQ_CONV_M6KHZ: // frequency translate mode is in "RF LO HIGH" mode - tune below center of screen
                ptr = 3* FFT_IQ_BUFF_LEN/16 ; // FFT_IQ_BUFF_LEN/8 - FFT_IQ_BUFF_LEN/16  = -12khz - 6khz = -18khz <-> + 6khz
                break;
            case FREQ_IQ_CONV_P12KHZ: // frequency translate mode is in "RF LO HIGH" mode - tune below center of screen
                ptr = 0; // FFT_IQ_BUFF_LEN/8 + FFT_IQ_BUFF_LEN/8  = -12khz + 12khz = 0khz <-> + 24khz
                break;
            case FREQ_IQ_CONV_M12KHZ:	// frequency translate mode is in "RF LO HIGH" mode - tune below center of screen
                ptr = FFT_IQ_BUFF_LEN/4 ; // FFT_IQ_BUFF_LEN/8 - FFT_IQ_BUFF_LEN/8  = -12khz - 12khz = -24khz <-> 0khz
                break;
            default:	// yes - frequency translate mode is off
                ptr = FFT_IQ_BUFF_LEN/8; // FFT_IQ_BUFF_LEN/8  = -12khz = -12khz <-> + 12khz
            }
            end_range = ptr+FFT_IQ_BUFF_LEN/4; // exclusive

            // this would be the right place to interpolate between two FFT values in sd.FFT_TempData in order
            // to make the mcHF user think that in magnify mode we have 256 different FFT values for the spectrum display
            // when IN FACT we only have 128 values and interpolate between them to expand to 256 pixels, DD4WH June 2016
            // when i = 2 * ptr, take same value
            // in all other cases:
            /*
             *   sd.FFT_DspData[FftIdx2BufMap(i++)] = sd.FFT_TempData[FftIdx2BufMap(ptr)];  each entry from fft is used twice
             *   sd.FFT_DspData[FftIdx2BufMap(i++)] = (sd.FFT_TempData[FftIdx2BufMap(ptr)] + sd.FFT_TempData[FftIdx2BufMap(ptr + 1)]) / 2 ;  each entry from fft is used twice
             *
             *
             * */
            for(i=0; ptr < end_range; ptr++)	 	// expand data to fill entire screen - get lower half
            {
                sd.FFT_DspData[FftIdx2BufMap(i++)] = sd.FFT_TempData[FftIdx2BufMap(ptr)]; /* each entry from fft is used twice */
                if (i != ptr * 2) {
                	sd.FFT_DspData[FftIdx2BufMap(i++)] = (sd.FFT_TempData[FftIdx2BufMap(ptr)] + sd.FFT_TempData[FftIdx2BufMap(ptr + 1)]) / 2 ;
                	// interpolated value of FFT [ptr] and FFT [ptr + 1]
                }
                else {
                		sd.FFT_DspData[FftIdx2BufMap(i++)] = sd.FFT_TempData[FftIdx2BufMap(ptr)]; /* same entry from fft */
                }
            }
        }
        else
            arm_copy_q15((q15_t *)sd.FFT_DspData, (q15_t *)sd.FFT_TempData, FFT_IQ_BUFF_LEN/2);
        //
        // After the above manipulation, clip the result to make sure that it is within the range of the palette table
        //
        for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)
        {
            if(sd.FFT_DspData[i] >= spec_height)	// is there an illegal height value?
                sd.FFT_DspData[i] = spec_height - 1;	// yes - clip it

        }
        //
        //
        sd.state++;
        break;

    }
    //
    //  update LCD control
    //
    case 5:
    {
        uint32_t	clr;
        UiMenu_MapColors(ts.scope_trace_colour,NULL, &clr);
        // Left part of screen(mask and update in one operation to minimize flicker)
/*        UiSpectrumDrawSpectrum((q15_t *)(sd.FFT_BkpData + FFT_IQ_BUFF_LEN/4), (q15_t *)(sd.FFT_DspData + FFT_IQ_BUFF_LEN/4), Black, clr,0);
        // Right part of the screen (mask and update) left part of screen is stored in the first quarter [0...127]
        UiSpectrumDrawSpectrum((q15_t *)(sd.FFT_BkpData), (q15_t *)(sd.FFT_DspData), Black, clr,1);
*/
        // Left part of screen(mask and update in one operation to minimize flicker)
        UiSpectrumDrawSpectrum((q15_t *)(sd.FFT_BkpData + FFT_IQ_BUFF_LEN/4), (q15_t *)(sd.FFT_DspData + FFT_IQ_BUFF_LEN/4), Black, clr,0);
        // Right part of the screen (mask and update) left part of screen is stored in the first quarter [0...127]
        UiSpectrumDrawSpectrum((q15_t *)(sd.FFT_BkpData), (q15_t *)(sd.FFT_DspData), Black, clr,1);



        sd.state = 0;   // Stage 0 - collection of data by the Audio driver
        break;
    }
    default:
        sd.state = 0;
        break;
    }
}
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverReDrawWaterfallDisplay
//* Object              : state machine implementation
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
// Waterfall Display code written by C. Turner, KA7OEI, May 2015 entirely from "scratch" - which is to say that I did not borrow any of it
// from anywhere else, aside from keeping some of the general functions found in "Case 1".
//
void UiSpectrumReDrawWaterfall()
{
    ulong i, spec_width;
    uint32_t	max_ptr;	// throw-away pointer for ARM maxval AND minval functions
//	float32_t	gcalc;
    //

    // Only in RX mode and NOT while powering down or in menu mode or if displaying memory information
    if((ts.txrx_mode != TRX_MODE_RX) || (ts.powering_down) || (ts.menu_mode) || (ts.mem_disp) || (ts.boot_halt_flag))
        return;

    if((ts.spectrum_scope_scheduler) || (!ts.waterfall_speed))	// is it time to update the scan, or is this scope to be disabled?
        return;
    else
        ts.spectrum_scope_scheduler = (ts.waterfall_speed-1)*2;


    // No spectrum display in DIGI modes
    //if(ts.dmod_mode == DEMOD_DIGI)
    //	return;

    // Nothing to do here otherwise, or if scope is to be held off while other parts of the display are to be updated or the LCD is being blanked
    if((!sd.enabled) || (ts.lcd_blanking_flag))
        return;

    // The state machine will rest
    // in between states
//	sd.skip_process++;
//	if(sd.skip_process < 1000)
//		return;

//	sd.skip_process = 0;

//	gcalc = 1/ads.codec_gain_calc;				// Get gain setting of codec and convert to multiplier factor

    // Process implemented as state machine
    switch(sd.state)
    {
    //
    // Apply gain to collected IQ samples and then do FFT
    //
    case 1:		// Scale input according to A/D gain and apply Window function
    {
        // with the new FFT lib arm_cfft we need to put the input and output samples into one buffer, therefore
        // UiDriverFFTWindowFunction was changed
        // new arm_cfft lib does not seem to need SCOPE_PREAMP_GAIN any more! Why?
//			arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)(gcalc * SCOPE_PREAMP_GAIN), (float32_t *)sd.FFT_Windat, FFT_IQ_BUFF_LEN);	// scale input according to A/D gain
//			arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)(gcalc), (float32_t *)sd.FFT_Windat, FFT_IQ_BUFF_LEN);	// scale input according to A/D gain
        //
        UiDriverFFTWindowFunction(ts.fft_window_type);		// do windowing function on input data to get less "Bin Leakage" on FFT data
        //
        sd.state++;
        break;
    }
    case 2:		// Do FFT and calculate complex magnitude
    {
//			arm_rfft_f32((arm_rfft_instance_f32 *)&sd.S,(float32_t *)(sd.FFT_Windat),(float32_t *)(sd.FFT_Samples));	// Do FFT
//			arm_rfft_fast_f32((arm_rfft_fast_instance_f32 *)&sd.S,(float32_t *)(sd.FFT_Windat),(float32_t *)(sd.FFT_Samples),0);	// Do FFT
        arm_cfft_f32(&arm_cfft_sR_f32_len256,(float32_t *)(sd.FFT_Samples),0,1);	// Do FFT

        //
        // Calculate magnitude
        //
        arm_cmplx_mag_f32((float32_t *)(sd.FFT_Samples),(float32_t *)(sd.FFT_MagData),(FFT_IQ_BUFF_LEN/2));
        //
        sd.state++;
        break;
    }
    //
    //  Low-pass filter amplitude magnitude data
    //
    case 3:
    {
        uint32_t i;
        float32_t		filt_factor;
        //
        filt_factor = (float)ts.spectrum_filter;		// use stored filter setting
        filt_factor = 1/filt_factor;		// invert filter factor to allow multiplication
        //
        if(sd.dial_moved)
        {
            sd.dial_moved = 0;	// Dial moved - reset indicator
            //
            UiSpectrum_FrequencyBarText();	// redraw frequency bar on the bottom of the display
            //
        }
        arm_scale_f32((float32_t *)sd.FFT_AVGData, (float32_t)filt_factor, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);	// get scaled version of previous data
        arm_sub_f32((float32_t *)sd.FFT_AVGData, (float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// subtract scaled information from old, average data
        arm_scale_f32((float32_t *)sd.FFT_MagData, (float32_t)filt_factor, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);	// get scaled version of new, input data
        arm_add_f32((float32_t *)sd.FFT_Samples, (float32_t *)sd.FFT_AVGData, (float32_t *)sd.FFT_AVGData, FFT_IQ_BUFF_LEN/2);	// add portion new, input data into average
        //
        for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	 		//		// guarantee that the result will always be >= 0
        {
            if(sd.FFT_AVGData[i] < 1)
                sd.FFT_AVGData[i] = 1;
        }

		calculate_dBm();
		sd.state++;


        break;
    }
    //
    // De-linearize and normalize display data and do AGC processing
    //
    case 4:
    {
        float32_t	max, min, mean, offset;
        float32_t	sig;
        //
        // De-linearize data with dB/division
        //
        for(i = 0; i < (FFT_IQ_BUFF_LEN/2); i++)
        {
            sig = log10(sd.FFT_AVGData[i]) * DB_SCALING_10;		// take FFT data, do a log10 and multiply it to scale 10dB (fixed)
            sig += sd.display_offset;							// apply "AGC", vertical "sliding" offset (or brightness for waterfall)
            if(sig > 1)											// is the value greater than 1?
                sd.FFT_DspData[i] = (q15_t)sig;					// it was a useful value - save it
            else
                sd.FFT_DspData[i] = 1;							// not greater than 1 - assign it to a base value of 1 for sanity's sake
        }
        //
        // Transfer data to the waterfall display circular buffer, putting the bins in frequency-sequential order!
        //

        for(i = 0; i < (FFT_IQ_BUFF_LEN/2); i++)
        {
            if(i < (SPECTRUM_WIDTH/2))	 		// build left half of spectrum data
            {
//					sd.FFT_Samples[i] = sd.FFT_DspData[i + FFT_IQ_BUFF_LEN/4];	// get data
                sd.FFT_Samples[(FFT_IQ_BUFF_LEN/2) - i - 1] = sd.FFT_DspData[i + FFT_IQ_BUFF_LEN/4];	// get data
            }
            else	 							// build right half of spectrum data
            {
//					sd.FFT_Samples[i] = sd.FFT_DspData[i - FFT_IQ_BUFF_LEN/4];	// get data
                sd.FFT_Samples[(FFT_IQ_BUFF_LEN/2) - i - 1] = sd.FFT_DspData[i - FFT_IQ_BUFF_LEN/4];	// get data
            }
        }

        //
        // Find peak and average to vertically adjust display
        //
        uint16_t samp_idx;
        if(sd.magnify)	 	// are we in magnify mode?
        {
            spec_width = FFT_IQ_BUFF_LEN/4;	// yes - define new spectrum width

            switch(ts.iq_freq_mode)
            {
            case FREQ_IQ_CONV_P6KHZ:	 	// we are in RF LO HIGH mode (tuning is below center of screen)
                samp_idx = FFT_IQ_BUFF_LEN/16;
                break;
            case FREQ_IQ_CONV_M6KHZ:	 	// we are in RF LO LOW mode (tuning is above center of screen)
                samp_idx = FFT_IQ_BUFF_LEN*3/16;
            case FREQ_IQ_CONV_P12KHZ:	 	// we are in RF LO HIGH mode (tuning is below center of screen)		// aaaaaaaaaaaaaaaaaaaaaaaaa
                samp_idx = 0;
                break;
            case FREQ_IQ_CONV_M12KHZ:	 	// we are in RF LO LOW mode (tuning is above center of screen)
                samp_idx = FFT_IQ_BUFF_LEN/4;
                break;
            default:
                samp_idx = FFT_IQ_BUFF_LEN/8;
            }
        }
        else
        {
            spec_width = FFT_IQ_BUFF_LEN/2;
            samp_idx = 0;
        }
        arm_max_f32(&sd.FFT_Samples[samp_idx], spec_width, &max, &max_ptr);        // find maximum element in center portion
        arm_min_f32(&sd.FFT_Samples[samp_idx], spec_width, &min, &max_ptr);        // find minimum element in center portion
        arm_mean_f32(&sd.FFT_Samples[samp_idx], spec_width, &mean);                // find mean value in center portion

        //
        // Calculate "brightness" offset for amplitude value
        //
        offset = (float)ts.waterfall_offset;
        offset -= 100;
        //
        //
        // Vertically adjust spectrum scope so that the strongest signals are adjusted to the top
        //
        if((max - offset) >= NUMBER_WATERFALL_COLOURS - 1)	 	// is result higher than display brightness
        {
            sd.display_offset -= sd.agc_rate;	// yes, adjust downwards quickly
//				if(max1 > SPECTRUM_HEIGHT+(SPECTRUM_HEIGHT/2))			// is it WAY above top of screen?
//					sd.display_offset -= sd.agc_rate*3;	// yes, adjust downwards REALLY quickly
        }
        //
        // Prevent "empty" spectrum display from filling with "noise" by checking the peak/average of what was found
        //
        else if(((max*10/mean) <= (q15_t)ts.waterfall_nosig_adjust) && (max < SPECTRUM_HEIGHT+(SPECTRUM_HEIGHT/2)))	 	// was "average" signal ratio below set threshold and average is not insanely strong??
        {
            if((min > 2) && (max > 2))	 		// prevent the adjustment from going downwards, "into the weeds"
            {
                sd.display_offset -= sd.agc_rate;	// yes, adjust downwards
                if(sd.display_offset < (-(SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET)))
                    sd.display_offset = (-(SPECTRUM_HEIGHT + SPECTRUM_SCOPE_ADJUST_OFFSET));
            }
        }
        else
            sd.display_offset += (sd.agc_rate/3);	// no, adjust upwards more slowly
        //
        //
        if((min <= 2) && (max <= 2))	 	// ARGH - We must already be in the weeds, below the bottom - let's adjust upwards quickly to get it back onto the display!
        {
            sd.display_offset += sd.agc_rate*10;
        }
        //
        // used for debugging
        //	char txt[32];
        //	sprintf(txt, " %d,%d,%d ", (int)sd.display_offset*100, (int)min*100,(int)max*100);
        //	UiLcdHy28_PrintText    ((POS_RIT_IND_X + 1), (POS_RIT_IND_Y + 20),txt,White,Grid,0);
        //
        // Copy to holder for the waterfall buffer
        sd.state++;
        break;
    }
    case 5:	// rescale waterfall horizontally, apply brightness/contrast, process pallate and put vertical line on screen, if enabled.
    {
        //
        sd.wfall_line %= sd.wfall_size;	// make sure that the circular buffer is clipped to the size of the display area
        //
        //
        // Contrast:  100 = 1.00 multiply factor:  125 = multiply by 1.25 - "sd.wfall_contrast" already converted to 100=1.00
        //
        arm_scale_f32((float32_t *)sd.FFT_Samples, (float32_t)sd.wfall_contrast, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);
        //
//        ushort ptr;
        uint16_t center_pixel_pos;
        // determine the pixel location for center line

//        uint16_t magnify_offset;
        // only used if magnify is on

        switch (ts.iq_freq_mode) {
        case FREQ_IQ_CONV_P6KHZ:
            center_pixel_pos = FFT_IQ_BUFF_LEN*3/16;
//            magnify_offset = FFT_IQ_BUFF_LEN/16;
            break;
        case FREQ_IQ_CONV_M6KHZ:
            center_pixel_pos = FFT_IQ_BUFF_LEN*5/16;
//            magnify_offset = FFT_IQ_BUFF_LEN*3/16;
            break;
        case FREQ_IQ_CONV_P12KHZ:
            center_pixel_pos = FFT_IQ_BUFF_LEN*2/16;
//            magnify_offset = 0;
            break;
        case FREQ_IQ_CONV_M12KHZ:
            center_pixel_pos = FFT_IQ_BUFF_LEN*6/16;
//            magnify_offset = FFT_IQ_BUFF_LEN/4;
            break;
        default:
            center_pixel_pos = FFT_IQ_BUFF_LEN*4/16;
//            magnify_offset = FFT_IQ_BUFF_LEN/8;
            break;
        }

        if (sd.magnify)
        {
            center_pixel_pos = FFT_IQ_BUFF_LEN*4/16;
            // position of center is always in the middle if
            // in magnify mode, so we fix that position here

/*			// we don�t need the following anymore, because the new Zoom FFT already
  	  	  	// takes care of that, DD4WH, Aug 15th, 2016
            for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)	 	// expand data to fill entire screen - get lower half
            {
                ptr = (i/2) + magnify_offset;
                if(ptr < FFT_IQ_BUFF_LEN/2)
                {
                    sd.wfall_temp[i] = sd.FFT_Samples[ptr];
                }
            }
            arm_copy_f32((float32_t *)sd.wfall_temp, (float32_t *)sd.FFT_Samples, FFT_IQ_BUFF_LEN/2);		// copy the rescaled/shifted data into the main buffer
*/
        }

        // After the above manipulation, clip the result to make sure that it is within the range of the palette table
        for(i = 0; i < FFT_IQ_BUFF_LEN/2; i++)
        {
            if(sd.FFT_Samples[i] >= NUMBER_WATERFALL_COLOURS)	// is there an illegal color value?
            {
                sd.FFT_Samples[i] = NUMBER_WATERFALL_COLOURS - 1;	// yes - clip it
            }

            sd.waterfall[sd.wfall_line][i] = (ushort)sd.FFT_Samples[i];	// save the manipulated value in the circular waterfall buffer
        }

        // Place center line marker on screen:  Location [64] (the 65th) of the palette is reserved is a special color reserved for this
        sd.waterfall[sd.wfall_line][center_pixel_pos] = NUMBER_WATERFALL_COLOURS;

        sd.wfall_line++;		// bump to the next line in the circular buffer for next go-around

        sd.state++;
        break;
    }
    //
    //  update LCD control
    //
    case 6:
    {
        uchar lptr = sd.wfall_line;		// get current line of "bottom" of waterfall in circular buffer

        sd.wfall_line_update++;									// update waterfall line count
        sd.wfall_line_update %= ts.waterfall_vert_step_size;	// clip it to number of lines per iteration

        if(!sd.wfall_line_update)	 							// if it's count is zero, it's time to move the waterfall up
        {
            //
            lptr %= sd.wfall_size;		// do modulus limit of spectrum high
            //
            // set up LCD for bulk write, limited only to area of screen with waterfall display.  This allow data to start from the
            // bottom-left corner and advance to the right and up to the next line automatically without ever needing to address
            // the location of any of the display data - as long as we "blindly" write precisely the correct number of pixels per
            // line and the number of lines.
            //
            UiLcdHy28_OpenBulkWrite(SPECTRUM_START_X, SPECTRUM_WIDTH, (sd.wfall_ystart + 1), sd.wfall_height);
            //
            uint16_t spectrumLine[2][SPECTRUM_WIDTH];
            uchar lcnt = 0;                 // initialize count of number of lines of display

            for(lcnt = 0;lcnt < sd.wfall_size; lcnt++)	 				// set up counter for number of lines defining height of waterfall
            {
                for(i = 0; i < (SPECTRUM_WIDTH); i++)	 	// scan to copy one line of spectral data - "unroll" to optimize for ARM processor
                {
                    spectrumLine[lcnt%2][i] = sd.waterfall_colours[sd.waterfall[lptr][i]];	// write to memory using waterfall color from palette
                }

                UiLcdHy28_BulkWrite(spectrumLine[lcnt%2],SPECTRUM_WIDTH);

                lptr++;									// point to next line in circular display buffer
                lptr %= sd.wfall_size;				// clip to display height
            }
            //
            UiLcdHy28_CloseBulkWrite();					// we are done updating the display - return to normal full-screen mode
        }
        sd.state = 0;	// Stage 0 - collection of data by the Audio driver
        break;
    }
    default:
        sd.state = 0;
        break;
    }
}
//
//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiInitSpectrumScopeWaterfall
//* Object              : Does all steps for clearing screen and initializing spectrum scope and waterfall
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiSpectrumInitSpectrumDisplay()
{
    if(ts.boot_halt_flag)			// do not build spectrum display/waterfall if we are loading EEPROM defaults!
        return;

    UiSpectrumClearDisplay();			// clear display under spectrum scope
    UiSpectrumCreateDrawArea();
    UiSpectrum_InitSpectrumDisplay();
    UiDriverDisplayFilterBW();	// Update on-screen indicator of filter bandwidth
}

//
//*----------------------------------------------------------------------------
//* Function Name       : UiDrawSpectrumScopeFrequencyBarText
//* Object              : Draw the frequency information on the frequency bar at the bottom of the spectrum scope based on the current frequency
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void UiSpectrum_FrequencyBarText()
{
    float   freq_calc;
    ulong   i, clr;
    char    txt[16], *c;
    float   grat;
    int centerIdx;

    if(ts.spectrum_freqscale_colour == SPEC_BLACK)     // don't bother updating frequency scale if it is black (invisible)!
        return;

	grat = (float)6 / (1 << sd.magnify);

    //
    // This function draws the frequency bar at the bottom of the spectrum scope, putting markers every at every graticule and the full frequency
    // (rounded to the nearest kHz) in the "center".  (by KA7OEI, 20140913)
    //
    // get color for frequency scale
    //
    UiMenu_MapColors(ts.spectrum_freqscale_colour,NULL, &clr);


    freq_calc = (float)(df.tune_new/TUNE_MULT);      // get current frequency in Hz

    if(!sd.magnify)         // if magnify is off, way *may* have the graticule shifted.  (If it is on, it is NEVER shifted from center.)
    {
        freq_calc += audio_driver_xlate_freq();
    }

    if(sd.magnify < 3)
  	{
  	  freq_calc = roundf(freq_calc/1000); // round graticule frequency to the nearest kHz
	}
    if(sd.magnify > 2 && sd.magnify < 5)
  	{
  	  freq_calc = roundf(freq_calc/100) / 10; // round graticule frequency to the nearest 100Hz
	}
    if(sd.magnify == 5)
  	{
  	  freq_calc = roundf(freq_calc/50) / 20; // round graticule frequency to the nearest 50Hz
	}

    centerIdx = UiSpectrum_GetGridCenterLine(0);

    {
        // remainder of frequency/graticule markings
        const static int idx2pos[] = {0,26,58,90,122,154,186,218, 242};
        const static int centerIdx2pos[] = {62,94,130,160,192};

		if(sd.magnify < 3)
		{
      	  snprintf(txt,16, "  %lu  ", (ulong)(freq_calc+(centerIdx*grat))); // build string for center frequency precision 1khz
      	}
      	else
		{
		  float disp_freq = freq_calc+(centerIdx*grat);
		  int bignum = (int)disp_freq;
		  int smallnum = (int)roundf((disp_freq-bignum)*100);
      	  snprintf(txt,16, "  %u.%02u  ", bignum,smallnum); // build string for center frequency precision 100Hz/10Hz
      	}

        i = centerIdx2pos[centerIdx+2] -((strlen(txt)-2)*4);    // calculate position of center frequency text
        UiLcdHy28_PrintText((POS_SPECTRUM_IND_X + i),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),txt,clr,Black,4);


        int idx;
        for (idx = -4; idx < 5; idx++)
        {
            int pos = idx2pos[idx+4];
            if (idx != centerIdx)
            {
			  if(sd.magnify < 3)
			  {
                snprintf(txt,16, " %lu ", (ulong)(freq_calc+(idx*grat)));   // build string for middle-left frequency (1khz precision)
            	c = &txt[strlen(txt)-3];  // point at 2nd character from the end
      		  }
      		  else
			  {
				float disp_freq = freq_calc+(idx*grat);
				int bignum = (int)disp_freq;
				int smallnum = (int)roundf((disp_freq-bignum)*100);
          		snprintf(txt,16, " %u.%02u ", bignum, smallnum);   // build string for middle-left frequency (10Hz precision)
            	c = &txt[strlen(txt)-5];  // point at 5th character from the end
			  }

			  if((idx == 3 || idx == 4) && (sd.magnify > 2))
			  {
				  pos = pos - 9;
			  }
              UiLcdHy28_PrintText((POS_SPECTRUM_IND_X +  pos),(POS_SPECTRUM_IND_Y + POS_SPECTRUM_FREQ_BAR_Y),c,clr,Black,4);
            }
      		if(sd.magnify > 2)
      		{
      		idx++;
      		}
        }
    }

}


static void calculate_dBm(void)
{
        //###########################################################################################################################################
        //###########################################################################################################################################
        // dBm/Hz-display DD4WH June, 9th 2016
        // this will be renewed every 20ms
        // the dBm/Hz display gives an absolute measure of the signal strength of the sum of all signals inside the passband of the filter
        // we take the FFT-magnitude values of the spectrum display FFT for this purpose (which are already calculated for the spectrum display),
        // so the additional processor load and additional RAM usage should be close to zero
		// this code also calculates the basis for the S-Meter (in sm.dbm and sm.dbmhz), if not in old school S-Meter mode
        //

		// FIXME: since implementation of the Zoom FFT, the dBm display does only show correct values when in 1x magnify mode!


        if(ts.sysclock > ts.dBm_count + 19 && ts.txrx_mode == TRX_MODE_RX && (ts.s_meter == 1 || ts.s_meter == 2 || ts.display_dbm != 0))
        {
        char txt[12];
        ulong i;
        float32_t slope = 19.8; // 19.6; --> empirical values derived from measurements by DL8MBY, 2016/06/30, Thanks!
        float32_t cons = -225; //- 227.0;
        float32_t  Lbin, Ubin;
        float32_t bw_LSB = 0.0;
        float32_t bw_USB = 0.0;
        float64_t sum_db = 0.0;
        int posbin = 0;
        float32_t buff_len = (float32_t) FFT_IQ_BUFF_LEN;
        float32_t width;

        float32_t bin_BW = (float32_t) (48000.0 * 2.0 / buff_len);
        // width of a 256 tap FFT bin = 187.5Hz
        // we have to take into account the magnify mode
        // --> recalculation of bin_BW
        bin_BW = bin_BW / (1 << sd.magnify); // correct bin bandwidth is determined by the Zoom FFT display setting

        int buff_len_int = FFT_IQ_BUFF_LEN;

        //	determine posbin (where we receive at the moment) from ts.iq_freq_mode

        if(!ts.iq_freq_mode)	 	// frequency translation off, IF = 0 Hz
      	  {
          posbin = buff_len_int / 4; // right in the middle!
      	  } // frequency translation ON
        else if(ts.iq_freq_mode == FREQ_IQ_CONV_P6KHZ)	 	// we are in RF LO HIGH mode (tuning is below center of screen)
      	  {
          posbin = (buff_len_int / 4) - (buff_len_int / 16);
      	  }
        else if(ts.iq_freq_mode == FREQ_IQ_CONV_M6KHZ)	 	// we are in RF LO LOW mode (tuning is above center of screen)
      	  {
          posbin = (buff_len_int / 4) + (buff_len_int / 16);
      	  }
        else if(ts.iq_freq_mode == FREQ_IQ_CONV_P12KHZ)	 	// we are in RF LO HIGH mode (tuning is below center of screen)
      	  {
          posbin = (buff_len_int / 4) - (buff_len_int / 8);
      	  }
        else if(ts.iq_freq_mode == FREQ_IQ_CONV_M12KHZ)	 	// we are in RF LO LOW mode (tuning is above center of screen)
      	  {
          posbin = (buff_len_int / 4) + (buff_len_int / 8);
      	  }

        if(sd.magnify != 0)
        	// in all magnify cases (2x up to 32x) the posbin is in the centre of the spectrum display
        {
        	posbin = buff_len_int / 4;
        }

        width = (float32_t)FilterInfo[FilterPathInfo[ts.filter_path].id].width;

        //	determine Lbin and Ubin from ts.dmod_mode and FilterInfo.width
        //	= determine bandwith separately for lower and upper sideband

        if (ts.dmod_mode == DEMOD_LSB)
      	  {
          bw_USB = 0.0;
          bw_LSB = width;
      	  }

        if (ts.dmod_mode == DEMOD_USB)
      	  {
          bw_LSB = 0.0;
          bw_USB = width;
      	  }

        if (ts.dmod_mode == DEMOD_CW)
      	  {
      	  //
          if(ts.cw_lsb)
            {
            bw_USB = 0.0;
            bw_LSB = width;
            }
          else
            {
            bw_LSB = 0.0;
            bw_USB = width;
            }
      	  }

        if (ts.dmod_mode == DEMOD_SAM || ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_FM)
      	  {
          bw_LSB = width;
          bw_USB = width;
      	  }

        // calculate upper and lower limit for determination of signal strength
        // = filter passband is between the lower bin Lbin and the upper bin Ubin
        Lbin = (float32_t)posbin - round(bw_LSB / bin_BW);
        Ubin = (float32_t)posbin + round(bw_USB / bin_BW); // the bin on the upper sideband side

        // take care of filter bandwidths that are larger than the displayed FFT bins
        if(Lbin < 0)
        {
        	Lbin = 0;
        }
        if (Ubin > 255)
        {
        	Ubin = 255;
        }

        i=0;
        for(i = 0; i < (buff_len_int/2); i++)
      	  {
          if(i < (buff_len_int/4))	 		// build left half of magnitude data
            {
            sd.FFT_Samples[FFT_IQ_BUFF_LEN/2 - i - 1] = sd.FFT_MagData[i + buff_len_int/4]*SCOPE_PREAMP_GAIN;	// get data
            }
          else	 							// build right half of magnitude data
            {
            sd.FFT_Samples[FFT_IQ_BUFF_LEN/2 - i - 1] = sd.FFT_MagData[i - buff_len_int/4]*SCOPE_PREAMP_GAIN;	// get data
            }
      	  }

          // determine the sum of all the bin values in the passband
        int c;
        for (c = (int)Lbin; c <= (int)Ubin; c++)   // sum up all the values of all the bins in the passband
          {
          sum_db = sum_db + sd.FFT_Samples[c];
          }

        if (sum_db > 0)
        {
    		sm.dbm = slope * log10 (sum_db) + cons;
    		sm.dbmhz = slope * log10 (sum_db) -  10 * log10 ((float32_t)(((int)Ubin-(int)Lbin) * bin_BW)) + cons;
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
        // m_AttackAlpha = 0.8647; //  ALPHA = 1 - e^(-T/Tau), T = 0.02s (because dbm routine is called every 20ms!)
		// Tau = 10ms = 0.01s attack time
        // m_DecayAlpha = 0.0392; // 500ms decay time
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

        if (ts.display_dbm == 1)
        {
            snprintf(txt,12,"%4ld dBm   ", (long)m_AverageMagdbm);
            UiLcdHy28_PrintTextCentered(162,64,41,txt,White,Blue,0);
        }
        else if (ts.display_dbm == 2)
        {
            snprintf(txt,12,"%4ld dBm/Hz", (long)m_AverageMagdbmhz);
            UiLcdHy28_PrintTextCentered(162,64,41,txt,White,Blue,0);
        }

//            snprintf(txt,12,"%4ld bins", (long)(Ubin-Lbin));
            // TODO: make coordinates constant variables

        ts.dBm_count = ts.sysclock;				// reset timer
        }
        if (ts.display_dbm == 0)
        {
        	UiLcdHy28_DrawFullRect(162, 63, 15, 144 , Black);
        }
}


// function builds text which is displayed above waterfall/scope area
void UiGet_Wfscope_Bar_Text(char* wfbartext)
{
  char* lefttext;
  char* righttext;

  if(ts.flags1 & FLAGS1_WFALL_SCOPE_TOGGLE)			//waterfall
  {
	lefttext = "WATERFALL      ";
  }
  else												// scope
  {
    switch(ts.spectrum_db_scale)	 	// convert variable to setting
  	{
  	  case DB_DIV_5:
        lefttext = "SC(5dB/div)    ";
        break;
  	  case DB_DIV_7:
        lefttext = "SC(7.5dB/div)  ";
        break;
  	  case DB_DIV_15:
        lefttext = "SC(15dB/div)   ";
        break;
  	  case DB_DIV_20:
        lefttext = "SC(20dB/div)   ";
        break;
  	  case S_1_DIV:
        lefttext = "SC(1S-Unit/div)";
        break;
  	  case S_2_DIV:
        lefttext = "SC(2S-Unit/div)";
        break;
  	  case S_3_DIV:
        lefttext = "SC(3S-Unit/div)";
        break;
  	  case DB_DIV_10:
  	  default:
        lefttext = "SC(10dB/div)   ";
        break;
  	}
  }

  switch(sd.magnify)
  {
	case 1:
	  righttext = " < Magnify x2 > ";
	  break;
	case 2:
	  righttext = " < Magnify x4 > ";
	  break;
	case 3:
	  righttext = " < Magnify x8 > ";
	  break;
	case 4:
	  righttext = " < Magnify x16 >";
	  break;
	case 5:
	  righttext = " < Magnify x32 >";
	  break;
	case 0:
	default:
	  righttext = " < Magnify x1 > ";
	  break;
  }

sprintf(wfbartext,"%s%s",lefttext,righttext);
}
