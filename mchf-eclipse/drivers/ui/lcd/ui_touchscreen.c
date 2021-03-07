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
 **  Licence:      GNU GPLv3                                                        **
 ************************************************************************************/

// Common
#include <stdio.h>
#include <stdlib.h>
#include "uhsdr_board.h"
#include "uhsdr_board_config.h"
#include "ui_lcd_hy28.h"
#include "ui_touchscreen.h"

#include "ui_lcd_ra8875.h"
#include "ui_lcd_draw.h"

#include "spi.h"

#ifdef STM32F4
    #define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_64)
#endif

#ifdef STM32F7
    #define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_128)
#endif

#ifdef STM32H7
    #define SPI_PRESCALE_TS_DEFAULT  (SPI_BAUDRATEPRESCALER_32)
    // 16 may be a little bit high for some displays but works with the 480x320 display
#endif

#define TS_SPI_TIMEOUT 100


mchf_touchscreen_t mchf_touchscreen;

/*
 * @brief Called to run the touch detection state machine, results are stored in ts structure
 */
void UiLcdHy28_TouchscreenDetectPress()
{
    if (mchf_touchscreen.present)
    {
        if(!HAL_GPIO_ReadPin(TP_IRQ_PIO,TP_IRQ) && mchf_touchscreen.state != TP_DATASETS_PROCESSED)    // fetch touchscreen data if not already processed
            UiLcdHy28_TouchscreenReadCoordinates();

        if(HAL_GPIO_ReadPin(TP_IRQ_PIO,TP_IRQ) && mchf_touchscreen.state == TP_DATASETS_PROCESSED)     // clear statemachine when data is processed
        {
            mchf_touchscreen.state = TP_DATASETS_NONE;

            mchf_touchscreen.hr_x = mchf_touchscreen.hr_y = 0x7fff;
        }
    }
}
/*
 * @brief tells you that touchscreen coordinates are ready for processing and marks them as processed
 * @returns true if coordinates for processing are available and have been marked as processed, false otherwise
 */
bool UiLcdHy28_TouchscreenHasProcessableCoordinates()
{
    bool retval = false;
    UiLcdHy28_TouchscreenReadCoordinates();

    if(mchf_touchscreen.state >= TP_DATASETS_VALID && mchf_touchscreen.state != TP_DATASETS_PROCESSED)
    //if(mchf_touchscreen.state >= TP_DATASETS_WAIT && mchf_touchscreen.state != TP_DATASETS_PROCESSED)
    {
        mchf_touchscreen.state = TP_DATASETS_NONE;     // tp data processed
         retval = true;
    }
    return retval;
}


static inline void UiLcdHy28_TouchscreenStartSpiTransfer()
{
    // we only have to care about other transfers if the SPI is
    // use by the display as well
    if (UiLcd_LcdSpiUsed())
    {
        UiLcd_BulkPixel_FinishWaitWrite();
    }
    UiLcd_SpiSetPrescaler(SPI_PRESCALE_TS_DEFAULT);
    GPIO_ResetBits(TP_CS_PIO, TP_CS);
}

static inline void UiLcdHy28_TouchscreenFinishSpiTransfer()
{
    UiLcd_SpiFinishTransfer();
    GPIO_SetBits(TP_CS_PIO, TP_CS);
    // we only have to care about other transfers if the SPI is
    // use by the display as well
    if (UiLcd_LcdSpiUsed())
    {
        UiLcd_SpiSetPrescaler(mchf_display.lcd_spi_prescaler);
    }
}


/*
 * @brief Extracts touchscreen touch coordinates, counts how often same position is being read consecutively
 * @param do_translate false -> raw coordinates, true -> mapped coordinates according to calibration data
 */
#define XPT2046_PD_FULL 0x00
#define XPT2046_PD_REF  0x01
#define XPT2046_PD_ADC  0x02
#define XPT2046_PD_NONE 0x03

#define XPT2046_MODE_12BIT 0x00
#define XPT2046_MODE_8BIT  0x08

#define XPT2046_CH_DFR_Y    0x50
#define XPT2046_CH_DFR_X    0x10
#define XPT2046_CONV_START  0x80

#define  XPT2046_COMMAND_LEN 7

static void UiLcdHy28_TouchscreenReadData(uint16_t* x_p,uint16_t* y_p)
{


    static const uint8_t xpt2046_command[XPT2046_COMMAND_LEN] =
    {
            XPT2046_CONV_START|XPT2046_CH_DFR_X|XPT2046_MODE_12BIT|XPT2046_PD_REF,
            0,  XPT2046_CONV_START|XPT2046_CH_DFR_Y|XPT2046_MODE_12BIT|XPT2046_PD_REF,  // the measurement for first command is here, we discard this
            0,  XPT2046_CONV_START|XPT2046_CH_DFR_X|XPT2046_MODE_12BIT|XPT2046_PD_FULL, // y measurement from previous command, next command turns off power
            0, 0                                                                        // x measurement from previous command
    };

    uint8_t xpt_response[XPT2046_COMMAND_LEN];

    UiLcdHy28_TouchscreenStartSpiTransfer();

    HAL_SPI_TransmitReceive(&hspi2, (uint8_t*)xpt2046_command, xpt_response,XPT2046_COMMAND_LEN,TS_SPI_TIMEOUT);

    UiLcdHy28_TouchscreenFinishSpiTransfer();

    *x_p = (xpt_response[5] << 8 | xpt_response[6]) >> 3;
    *y_p = (xpt_response[3] << 8 | xpt_response[4]) >> 3;

}
#define HIRES_TOUCH_MaxDelta 2
#define HIRES_TOUCH_MaxFocus 4

void UiLcdHy28_TouchscreenReadCoordinates()
{

    /*
    statemachine stati:
    TP_DATASETS_NONE = no touchscreen action detected
    TP_DATASETS_WAIT 1 = first touchscreen press
    >1 = x times valid data available
    TP_DATASETS_PROCESSED 0xff = data was already processed by calling function
     */


    if(mchf_touchscreen.state < TP_DATASETS_VALID)  // no valid data ready or data ready to process
    {
        if(mchf_touchscreen.state > TP_DATASETS_NONE && mchf_touchscreen.state < TP_DATASETS_VALID) // first pass finished, get data
        {

            UiLcdHy28_TouchscreenReadData(&mchf_touchscreen.xraw,&mchf_touchscreen.yraw);

            //delta/focus algorithm for filtering the noise from touch panel data
            //based on LM8300/LM8500 datasheet

            //first calculating the delta algorithm

            int16_t TS_dx,TS_dy, TS_predicted_x, TS_predicted_y, NewDeltaX, NewDeltaY;
            TS_dx=mchf_touchscreen.xraw_m1-mchf_touchscreen.xraw_m2;
            TS_dy=mchf_touchscreen.yraw_m1-mchf_touchscreen.yraw_m2;
            TS_predicted_x=mchf_touchscreen.yraw_m1+TS_dx;
            TS_predicted_y=mchf_touchscreen.yraw_m1+TS_dy;

            NewDeltaX=TS_predicted_x-mchf_touchscreen.xraw;
            NewDeltaY=TS_predicted_y-mchf_touchscreen.yraw;

            if(NewDeltaX<0)
                NewDeltaX=-NewDeltaX;

            if(NewDeltaY<0)
                NewDeltaX=-NewDeltaY;

            if((NewDeltaX<=HIRES_TOUCH_MaxDelta) && (NewDeltaY<=HIRES_TOUCH_MaxDelta))
            {

                //ok, the delta algorithm filtered out spikes and the bigger noise
                //now we perform focus algorithm

                NewDeltaX=mchf_touchscreen.focus_xprev-mchf_touchscreen.xraw;
                NewDeltaY=mchf_touchscreen.focus_yprev-mchf_touchscreen.yraw;

                if(NewDeltaX<0)
                    NewDeltaX=-NewDeltaX;

                if(NewDeltaY<0)
                    NewDeltaX=-NewDeltaY;

                if((NewDeltaX<=HIRES_TOUCH_MaxFocus) && (NewDeltaY<=HIRES_TOUCH_MaxFocus))
                {
                    mchf_touchscreen.xraw=mchf_touchscreen.focus_xprev;
                    mchf_touchscreen.yraw=mchf_touchscreen.focus_yprev;
                }
                else
                {
                    mchf_touchscreen.focus_xprev=mchf_touchscreen.xraw;
                    mchf_touchscreen.focus_yprev=mchf_touchscreen.yraw;
                }


                mchf_touchscreen.state=TP_DATASETS_VALID;

                int32_t x,y;

                x=mchf_touchscreen.xraw;
                y=mchf_touchscreen.yraw;

                int32_t xn,yn;
                //transforming the coordinates by calibration coefficients calculated in touchscreen calibration
                //see the UiDriver_TouchscreenCalibration
                //xn=Ax+By+C
                //yn=Dx+Ey+F
                //all coefficients are in format 16.16
                xn=mchf_touchscreen.cal[0]*x+mchf_touchscreen.cal[1]*y+mchf_touchscreen.cal[2];
                yn=mchf_touchscreen.cal[3]*x+mchf_touchscreen.cal[4]*y+mchf_touchscreen.cal[5];

                xn>>=16;
                yn>>=16;

                mchf_touchscreen.hr_x=(int16_t)xn;
                mchf_touchscreen.hr_y=(int16_t)yn;


            }
            else
            {
                mchf_touchscreen.xraw_m2=mchf_touchscreen.xraw_m1;
                mchf_touchscreen.yraw_m2=mchf_touchscreen.yraw_m1;
                mchf_touchscreen.xraw_m1=mchf_touchscreen.xraw;
                mchf_touchscreen.yraw_m1=mchf_touchscreen.yraw;
                mchf_touchscreen.state = TP_DATASETS_WAIT;
            }
        }
        else
        {
            mchf_touchscreen.state = TP_DATASETS_WAIT;      // restart machine
        }
    }

}

bool UiLcdHy28_TouchscreenPresenceDetection()
{
    bool retval = false;
    uint16_t x = 0xffff, y = 0xffff;

    UiLcdHy28_TouchscreenReadData(&x,&y);
    UiLcdHy28_TouchscreenReadData(&x,&y);

    mchf_touchscreen.state = TP_DATASETS_PROCESSED;

    if(x != 0xffff && y != 0xffff && x != 0 && y != 0)
    {// touchscreen data valid?
        retval = true;                      // yes - touchscreen present!
    }
    return retval;
}

void UiLcdHy28_TouchscreenInit(uint8_t mirror)
{
    mchf_touchscreen.xraw = 0;
    mchf_touchscreen.yraw = 0;

    mchf_touchscreen.hr_x = 0x7FFF;                        // invalid position
    mchf_touchscreen.hr_y = 0x7FFF;                        // invalid position
    mchf_touchscreen.present = UiLcdHy28_TouchscreenPresenceDetection();
}


