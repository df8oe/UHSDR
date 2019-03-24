/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

//
// SI570 frequency shift table
//       0-100 degrees C range
//       shots from 20m band (56Mhz)
//
// This frequency compensation table referenced to 14.000 MHz by KA7OEI, 10/14 using
// a GPS reference in a temperature-controlled environment, extrapolated below 17C and above
// 70C.
//
// Temperature normalized to 43C, a temperature achieved very soon after power-up
// with both the Si570 and the temperature sensor thermally bonded.
//
// Note:  The exact frequency/temperature dependencies will likely vary from unit to unit
// for each Si570, but the values below appear to approximately follow typical AT-cut
// temperature-frequency curves.
//
#include "uhsdr_board.h"
#include "soft_tcxo.h"
#include "radio_management.h"
#include "uhsdr_hw_i2c.h"

LoTcxo lo;

// -------------------------------------------------------------------------------------
// Temperature sensor
// ------------------
#define MCP_ADDR                (0x90)

// MCP registers
#define MCP_TEMP                (0x00)
#define MCP_CONFIG              (0x01)
#define MCP_HYSTR               (0x02)
#define MCP_LIMIT               (0x03)

// MCP CONFIG register bits
#define MCP_ONE_SHOT            (7)
#define MCP_ADC_RES             (5)
#define MCP_FAULT_QUEUE         (3)
#define MCP_ALERT_POL           (2)
#define MCP_INT_MODE            (1)
#define MCP_SHUTDOWN            (0)
#define R_BIT                   (1)
#define W_BIT                   (0)

#define MCP_ADC_RES_9           0
#define MCP_ADC_RES_10          1
#define MCP_ADC_RES_11          2
#define MCP_ADC_RES_12          3

#define MCP_POWER_UP            0
#define MCP_POWER_DOWN          1

static int32_t MCP9801_ConvExternalTemp(uint8_t *temp)
{
    int32_t  ts = 0;

    ts = temp[1];

    ts += (int8_t)temp[0] << 8;
    ts *= 10000;
    ts /= 256;

    return ts;
}


/*
 * @brief configures the MCP9801 external temperature sensor for monitoring Si570 temperature
 * @returns 0 if detected and working, 1 if not detected on I2C, 2 for other errors
 */
static uint8_t MCP9801_InitExternalTempSensor()
{
    uint8_t config, retval = 0;
    uint16_t res;

    // Read config reg
    res = UhsdrHw_I2C_ReadRegister(MCP_I2C, MCP_ADDR, MCP_CONFIG, 1, &config);
    if(res != 0)
    {
        retval = 1;
    }
    else
    {
        // Modify resolution
        config &= ~(3 << MCP_ADC_RES);
        config |= (MCP_ADC_RES_12 << MCP_ADC_RES);

        // Modify power mode
        config &= ~(1 << MCP_SHUTDOWN);
        config |= (MCP_POWER_UP << MCP_SHUTDOWN);

        // Write config reg
        res = UhsdrHw_I2C_WriteRegister(MCP_I2C, MCP_ADDR, MCP_CONFIG, 1, config);
        if(res != 0)
        {
            retval = 2;
        }
    }
    return retval;
}


static uint8_t MCP9801_ReadExternalTempSensor(int32_t *temp)
{
    uint8_t	data[2];
    uint8_t retval = 0;


    // Read temperature
    if(temp != NULL && UhsdrHw_I2C_ReadBlock(MCP_I2C, MCP_ADDR, MCP_TEMP, 1, data, 2) == 0)
    {
        *temp = MCP9801_ConvExternalTemp(data);
    }
    else
    {
        retval = 2;
    }
    return retval;
}


#define TCXO_TBL_SIZE 100
const short tcxo_table_20m[TCXO_TBL_SIZE] =
{
    -165    //   0 C
    -162,   //   1 C
    -160,   //   2 C
    -157,   //   3 C
    -155,   //   4 C
    -152,   //   5 C
    -150,   //   6 C
    -148,   //   7 C
    -144,   //   8 C
    -142,   //   9 C
    -136,   //  10 C
    -132,   //  11 C
    -130,   //  12 C
    -129,   //  13 C
    -126,   //  14 C
    -121,   //  15 C
    -116,   //  16 C
    -112,   //  17 C
    -108,   //  18 C
    -103,   //  19 C
    -99,    //  20 C
    -95,    //  21 C
    -90,    //  22 C
    -86,    //  23 C
    -81,    //  24 C
    -75,    //  25 C
    -70,    //  26 C
    -66,    //  27 C
    -61,    //  28 C
    -57,    //  29 C
    -53,    //  30 C
    -46,    //  31 C
    -42,    //  32 C
    -38,    //  33 C
    -32,    //  34 C
    -28,    //  35 C
    -25,    //  36 C
    -20,    //  37 C
    -17,    //  38 C
    -13,    //  39 C
    -9, //  40 C
    -5, //  41 C
    -2, //  42 C
    1,  //  43 C
    4,  //  44 C
    7,  //  45 C
    10, //  46 C
    12, //  47 C
    15, //  48 C
    16, //  49 C
    18, //  50 C
    20, //  51 C
    21, //  52 C
    22, //  53 C
    23, //  54 C
    23, //  55 C
    24, //  56 C
    23, //  57 C
    23, //  58 C
    22, //  59 C
    21, //  60 C
    20, //  61 C
    18, //  62 C
    16, //  63 C
    14, //  64 C
    11, //  65 C
    7,  //  66 C
    3,  //  67 C
    -1, //  68 C
    -5, //  69 C
    -11,//  70 C
    -17,    //  71 C
    -24,    //  72 C
    -31,    //  73 C
    -38,    //  74 C
    -45,    //  75 C
    -52,    //  76 C
    -58,    //  77 C
    -67,    //  78 C
    -71,    //  79 C
    -77,    //  80 C
    -83,    //  81 C
    -89,    //  82 C
    -95,    //  83 C
    -102,   //  84 C
    -108,   //  85 C
    -115,   //  86 C
    -121,   //  87 C
    -127,   //  88 C
    -134,   //  89 C
    -141,   //  90 C
    -147,   //  91 C
    -153,   //  92 C
    -159,   //  93 C
    -166,   //  94 C
    -177,   //  95 C
    -184,   //  96 C
    -190,   //  97 C
    -197,   //  98 C
    -203    //  99 C
};

void SoftTcxo_Init()
{

    lo.comp                 = 0;

    // Temp sensor setup
    lo.sensor_present = MCP9801_InitExternalTempSensor() == 0;
}


/*
 * @brief measure local oscillator temperature and calculates compensation value
 * @return true if the temperature value has been measured
 */
bool SoftTcxo_HandleLoTemperatureDrift()
{
    int32_t temp = 0;
    int     comp, comp_p;
    float   dtemp, remain, t_index;
    uchar   tblp;

    uint8_t temp_mode = RadioManagement_TcxoGetMode();

    bool retval = false;

    // No need to process if no chip avail or tcxo is disabled
    if((lo.sensor_present == true) && RadioManagement_TcxoIsEnabled())
    {
        {
            // Get current temperature
            if(MCP9801_ReadExternalTempSensor(&temp) == 0)
            {
                // Get temperature from sensor with its maximum precision
                dtemp = (float)temp;    // get temperature
                dtemp /= 10000;         // convert to decimal degrees
                remain = truncf(dtemp); // get integer portion of temperature
                remain = dtemp - remain;    // get fractional portion

                // Compensate only if enabled
                if((temp_mode == TCXO_ON))
                {
                    // Temperature to unsigned table pointer
                    t_index  = (uchar)((temp%1000000)/100000);
                    t_index *= 10;
                    t_index += (uchar)((temp%100000)/10000);

                    // Check for overflow - keep within the lookup table
                    if((t_index < 0) || (t_index > 150))        // the temperature sensor function "wraps around" below zero
                    {
                        t_index = 0;                        // point at the bottom of the temperature table
                        dtemp = 0;                          // zero out fractional calculations
                        remain = 0;
                    }
                    else if(t_index > TCXO_TBL_SIZE - 2)                   // High temperature - limit to maximum
                    {
                        t_index = TCXO_TBL_SIZE - 2;                       // Point to (near) top of table
                        dtemp = 0;                          // zero out fractional calculations
                        remain = 0;
                    }

                    tblp = (uchar)t_index;                      // convert to index

                    comp = tcxo_table_20m[tblp];                // get the first entry in the table
                    comp_p = tcxo_table_20m[tblp + 1];          // get the next entry in the table to determine fraction of frequency step

                    comp_p = comp_p - comp; //                  // get frequency difference between the two steps

                    dtemp = (float)comp_p;  // change it to float for the calculation
                    dtemp *= remain;        // get proportion of temperature difference between the two steps using the fraction

                    comp += (int)dtemp;     // add the compensation value to the lower of the two frequency steps

                    // Change needed ?
                    if(lo.comp != comp)         // is it there a difference?
                    {
                        // Update frequency, without reflecting it on the LCD
                        df.temp_factor = comp;
                        df.temp_factor_changed = true;
                        lo.comp = comp;
                    }
                }
                // Refresh UI
                retval = true;
                lo.temp = temp;
            }
        }
    }
    return retval;
}
