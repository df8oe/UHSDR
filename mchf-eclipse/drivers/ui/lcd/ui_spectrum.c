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
#include "ui_spectrum.h"
#include "mchf_board.h"
#include "ui_lcd_hy28.h"
// For spectrum display struct
#include "audio_driver.h"

// ------------------------------------------------
// Spectrum display
extern __IO   SpectrumDisplay      sd;





// This version of "Draw Spectrum" is revised from the original in that it interleaves the erasure with the drawing
// of the spectrum to minimize visible flickering  (KA7OEI, 20140916, adapted from original)
//
// 20141004 NOTE:  This has been somewhat optimized to prevent drawing vertical line segments that would need to be re-drawn:
//  - New lines that were shorter than old ones are NOT erased
//  - Line segments that are to be erased are only erased starting at the position of the new line segment.
//
//  This should reduce the amount of CGRAM access - especially via SPI mode - to a minimum.
//

static inline bool UiLcdHy28_DrawSpectrum_IsVgrid(const uint16_t x, const uint16_t color_new, uint16_t* clr_ptr, const uint16_t x_center_line) {
	bool repaint_v_grid = false;

	if (x == x_center_line) {
		*clr_ptr = ts.scope_centre_grid_colour_active;
		repaint_v_grid = true;
	} else {
		int k;
		// Enumerate all saved x positions
		for(k = 0; k < 7; k++)
		{
			// Exit on match
			if(x == sd.vert_grid_id[k]) {
				*clr_ptr = ts.scope_grid_colour_active;
				repaint_v_grid = true;
				break;
				// leave loop, found match
			}
		}
	}
	return repaint_v_grid;
}

static uint16_t UiLcdHy28_DrawSpectrum_GetCenterLineX() {
	static uint16_t idx;

	static const uint16_t  center[FREQ_IQ_CONV_MODE_MAX+1] = { 3,2,4,1,5 };
	// list the idx for the different modes (which are numbered from 0)
	// the list static const in order to have it in flash
	// it would be faster in ram but this is not necessary

	if(sd.magnify) {
		idx = 3;
	} else {
		idx = center[ts.iq_freq_mode];
	}
	return sd.vert_grid_id[idx];
}


//*----------------------------------------------------------------------------
//* Function Name       : UiLcdHy28_DrawSpectrum_Interleaved
//* Object              : repaint spectrum scope control
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void    UiLcdHy28_DrawSpectrum_Interleaved(q15_t *fft_old, q15_t *fft_new, const ushort color_old, const ushort color_new, const ushort shift)
{
	static uint16_t pixel_buf[SPECTRUM_HEIGHT];

	uint16_t      i, k, x, y_old , y_new, y1_old, y1_new, len_old, len_new, sh, clr;
	uint16_t idx = 0;
	bool      repaint_v_grid = false;
	clr = color_new;

	uint16_t x_center_line = UiLcdHy28_DrawSpectrum_GetCenterLineX();


	if(shift)
		sh = (SPECTRUM_WIDTH/2)-1;   // Shift to fill gap in center
	else
		sh = 1;                  // Shift to fill gap in center

	if (sd.first_run>0) {
		int idx = 0;
		for(x = (SPECTRUM_START_X + sh + 0); x < (POS_SPECTRUM_IND_X + SPECTRUM_WIDTH/2 + sh); x++) {
			y_new = fft_new[idx++];

			if(y_new > (SPECTRUM_HEIGHT - 7))
				y_new = (SPECTRUM_HEIGHT - 7);
			y1_new  = (SPECTRUM_START_Y + SPECTRUM_HEIGHT - 1) - y_new;
			UiLcdHy28_DrawStraightLine(x,y1_new,y_new,LCD_DIR_VERTICAL,color_new);

		}
		sd.first_run--;
	} else {

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


			if(y_old <= y_new) {
				// is old line going to be overwritten by new line, anyway?
				// ----------------------------------------------------------
				//
				if (y_old != y_new) {
					UiLcdHy28_DrawStraightLine(x,y1_new,y_new-y_old,LCD_DIR_VERTICAL,color_new);
				}
			} else {

				// Repaint vertical grid on clear
				// Basically paint over the grid is allowed
				// but during spectrum clear instead of masking
				// grid lines with black - they are repainted
				// TODO: This code is  always executed, since this function is always called with color_old == Black
				repaint_v_grid = UiLcdHy28_DrawSpectrum_IsVgrid(x, color_new, &clr, x_center_line);
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
						for(k = 0; k < 3; k++)
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

			}
		}
	}
}

