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

// Common
#include "mchf_board.h"
#include "codec.h"
//
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "mchf_hw_i2c.h"
#include "ui_si570.h"

// -------------------------------------------------------------------------------------
// Local Oscillator
// ------------------

// The SI570 Min/Max frequencies are 4x the actual tuning frequencies
#define SI570_MIN_FREQ          10000000    // 10=2.5 MHz
#define SI570_MAX_FREQ          160000000   // 160=40 Mhz
//
// These are "hard limit" frequencies below/above which the synthesizer cannot be adjusted or else the system may crash
#define SI570_HARD_MIN_FREQ     3500000     // 3.5=  0.875 MHz
#define SI570_HARD_MAX_FREQ     220000000   // 220=55 MHz

#define SI570_RECALL            (1<<0)
#define SI570_FREEZE_DCO        (1<<4)
#define SI570_FREEZE_M          (1<<5)
#define SI570_NEW_FREQ          (1<<6)

#define SI570_REG_135           135
#define SI570_REG_137           137

#define FACTORY_FXTAL           114.285

// VCO range
#define FDCO_MAX                5670
#define FDCO_MIN                4850

#define POW_2_28                268435456.0
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


#define SMOOTH_DELTA (0.0030)
// Datasheet says 0.0035  == 3500PPM but there have been issues if we get close to that value.
// to play it safe, we make the delta range a little smaller.
// if you want to play with it, tune to the end of the 10m band, set 100 khz step width and dial around
// sooner or later jumps with delta close to 0.0035 (actually a calculated delta of 0.00334) cause a "crash" of Si570

static const uchar	hs_div[6]	= {11, 9, 7, 6, 5, 4};
static const float	fdco_max	= FDCO_MAX;
static const float	fdco_min	= FDCO_MIN;

__IO OscillatorState os;

/*
 * @brief Returns startup frequency value of Si570, call only after init of Si570
 *
 * @returns Startup frequency in Mhz
 */
//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_setbits
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar Si570_SetBits(unsigned char original, unsigned char reset_mask, unsigned char new_val)
{
    return ((original & reset_mask) | new_val);
}

/*
 * @brief reads Si570 registers and verifies match with local copy of settings
 * @returns SI570_OK if matching, SI570_I2C_ERROR if I2C is not working, SI570_ERROR otherwise
 */
static Si570_ResultCodes Si570_VerifyFrequencyRegisters()
{
    Si570_ResultCodes retval = SI570_OK;
    uchar	i, res;
    uchar	regs[10];

    // Read all regs
    for(i = 0; i < 6; i++)
    {
        res = mchf_hw_i2c_ReadRegister(os.si570_address, (os.base_reg + i), &regs[i]);
        if(res != 0)
        {
            retval = SI570_I2C_ERROR;
            break;
        }
    }

    if (retval == SI570_OK)
    {
        // Not working - need fix
        // memset(regs, 0, 6);
        // trx4m_hw_i2c_ReadData(os.si570_address, 7, regs, 5);

        if(memcmp(regs, (uchar*)os.regs, 6) != 0)
        {
            retval = SI570_ERROR_VERIFY;
        }
    }

    return retval;
}

static uint16_t Si570_SetRegisterBits(uint8_t si570_address, uint8_t regaddr ,uint8_t* reg_ptr,uint8_t val){
    uint16_t retval = mchf_hw_i2c_ReadRegister(si570_address, regaddr, reg_ptr);
    if (retval == 0)
    {
        retval = mchf_hw_i2c_WriteRegister(si570_address, regaddr, (*reg_ptr|val));
    }
    return retval;
}
static uint16_t Si570_ClearRegisterBits(uint8_t si570_address, uint8_t regaddr ,uint8_t* reg_ptr,uint8_t val){
    uint16_t retval = mchf_hw_i2c_ReadRegister(si570_address, regaddr, reg_ptr);
    if (retval == 0)
    {
        retval = mchf_hw_i2c_WriteRegister(si570_address, regaddr, (*reg_ptr & ~val));
    }
    return retval;
}



//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_small_frequency_change
//* Object              : small frequency changes handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static Si570_ResultCodes Si570_SmallFrequencyChange()
{
    uint16_t ret;
    Si570_ResultCodes retval = SI570_OK;
    uchar reg_135;

    // Read current
    ret = Si570_SetRegisterBits(os.si570_address, SI570_REG_135, &reg_135, SI570_FREEZE_M);
    if (ret == 0)
    {
        // Write as block, registers 7-12
        ret = mchf_hw_i2c_WriteBlock(os.si570_address, os.base_reg, (uchar*)os.regs, 6);
        if (ret == 0)
        {
            retval = Si570_VerifyFrequencyRegisters();
        }
        else
        {
            retval = SI570_I2C_ERROR;
        }
    }
    Si570_ClearRegisterBits(os.si570_address, SI570_REG_135, &reg_135, SI570_FREEZE_M);
    return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_large_frequency_change
//* Object              : large frequency changes handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static Si570_ResultCodes Si570_LargeFrequencyChange()
{
    uint16_t ret;
    uint8_t reg_135, reg_137;
    Si570_ResultCodes retval = SI570_I2C_ERROR;

    if (Si570_SetRegisterBits(os.si570_address, SI570_REG_137, &reg_137, SI570_FREEZE_DCO) == 0)
    {
        // Write as block, registers 7-12
        if(mchf_hw_i2c_WriteBlock(os.si570_address, os.base_reg, (uchar*)os.regs, 6) == 0)
        {
            retval = Si570_VerifyFrequencyRegisters();
        }
    }

    // no matter what happened, try to unfreeze the Si570
    ret = Si570_ClearRegisterBits(os.si570_address, SI570_REG_137, &reg_137, SI570_FREEZE_DCO);

    if (ret == 0 && retval == SI570_OK)
    {
        if (Si570_SetRegisterBits(os.si570_address, SI570_REG_135, &reg_135, SI570_NEW_FREQ) == 0)
        {
            // Wait for action completed
            do
            {
                ret = mchf_hw_i2c_ReadRegister(os.si570_address, SI570_REG_135, &reg_135);
            } while(ret == 0 && (reg_135 & SI570_NEW_FREQ));
        }
    }
    return ret!=0?SI570_I2C_ERROR:retval;
}




static void Si570_ClearConfig(Si570_FreqConfig *in) {
    memset(in,0,sizeof(Si570_FreqConfig));
}


static void Si570_CopyConfig(Si570_FreqConfig *in, Si570_FreqConfig* out) {
    memcpy(out,in,sizeof(Si570_FreqConfig));
}

static float64_t Si570_FDCO_InRange(float64_t fdco){
    return (fdco >= fdco_min && fdco <= fdco_max);
}

static float64_t Si570_GetFDCOForFreq(float new_freq, uint8_t n1, uint8_t hsdiv){
    return ((float64_t)new_freq * (float64_t)(n1 * hsdiv));
}

static bool Si570_FindSmoothRFreqForFreq(float new_freq,const Si570_FreqConfig* cur_config, Si570_FreqConfig* new_config) {
    float64_t fdco = Si570_GetFDCOForFreq(new_freq, cur_config->n1, cur_config->hsdiv);
    bool retval = false;
    float64_t fdiff = (fdco - cur_config->fdco)/cur_config->fdco;

    if (fdiff < 0.0)
    {
        fdiff = -fdiff;
    }

    if (fdiff <= SMOOTH_DELTA && Si570_FDCO_InRange(fdco))
    {
        new_config->rfreq = fdco / (float64_t)os.fxtal;
        new_config->fdco = fdco;
        new_config->n1 = cur_config->n1;
        new_config->hsdiv = cur_config->hsdiv;

        retval = true;
    }
    return retval;
}


static bool Si570_FindConfigForFreq(float new_freq,Si570_FreqConfig* config) {
    uchar   i;
    uint16_t  divider_max, curr_div;
    bool retval = false;

    bool    n1_found = false;
    uint8_t   n1 = 1; // this is to shut up gcc 4.9.x regarding uninitalized variable
    uint8_t   hsdiv;
    float64_t fdco;

    divider_max = (ushort)floorf(fdco_max / new_freq);
    curr_div    = (ushort)ceilf (fdco_min / new_freq);

    // for each available divisor hsdiv we calculate the n1 range
    // and see if an acceptable n1 (1,all even numbers between 2..128)
    // is available for the given divisor.
    // this requires at most 12 float division
    // for frequencies in the range from 3,45 to 120Mhz at most
    for(i = 0; i < 6 && n1_found == false; i++)
    {
        hsdiv = hs_div[i];
        uint8_t n1_cand_min = ceilf((float)curr_div / (float)hsdiv);
        uint8_t n1_cand_max = floorf((float)divider_max / (float)hsdiv);

        if ((n1_cand_max >= 1) && (n1_cand_min <= 128) ) {
            if (n1_cand_min <= 1)
            {
                n1 = 1;
                n1_found = true;
            } else {
                n1 = ((n1_cand_min+1)& ~1);
                // get the closest even number (towards higher numbers)
                if (n1 <= n1_cand_max) {
                    n1_found = true;
                }
            }
        }
    }
    if (n1_found) {
        fdco = Si570_GetFDCOForFreq(new_freq,n1,hsdiv);
        if (Si570_FDCO_InRange(fdco)) {
            config->n1 =n1;
            config->hsdiv = hsdiv;
            config->fdco = fdco;
            config->rfreq = fdco / os.fxtal;

            retval = true;
        }
    }
    return retval;
}

static Si570_ResultCodes Si570_WriteConfig(Si570_FreqConfig* config, bool is_small) {

    uint32_t   frac_bits;
    uint16_t whole;

    uint8_t n1_regVal = config->n1 - 1;
    uint8_t hsdiv_regVal = config->hsdiv - 4;
    uint8_t i;
    // the written value is n1 - 1, hsdiv -4 according to the datasheet
    Si570_ResultCodes retval = SI570_OK;

    for(i = 0; i < 6; i++)
    {
        os.regs[i] = 0;
    }

    os.regs[0] = (hsdiv_regVal << 5);

    os.regs[0] = Si570_SetBits(os.regs[0], 0xE0, (n1_regVal >> 2));
    os.regs[1] = (n1_regVal & 3) << 6;

    whole = floorf(config->rfreq);
    frac_bits = floorf((config->rfreq - whole) * POW_2_28);

    for(i = 5; i >= 3; i--)
    {
        os.regs[i] = frac_bits & 0xFF;
        frac_bits = frac_bits >> 8;
    }

    os.regs[2] = Si570_SetBits(os.regs[2], 0xF0, (frac_bits & 0xF));
    os.regs[2] = Si570_SetBits(os.regs[2], 0x0F, (whole & 0xF) << 4);
    os.regs[1] = Si570_SetBits(os.regs[1], 0xC0, (whole >> 4) & 0x3F);

    if(is_small)
    {
        retval = Si570_SmallFrequencyChange();
    }
    else
    {
        retval = Si570_LargeFrequencyChange();
    }
    if(retval == SI570_OK)
    {

        // Verify second time - we might be transmitting, so
        // it is absolutely unacceptable to be on startup
        // SI570 frequency if any I2C error or chip reset occurs!
        retval = Si570_VerifyFrequencyRegisters();
    }
    return retval;
}


//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_change_frequency
//* Object              :
//* Input Parameters    : input frequency (float), test: 0 = tune, 1 = calculate, but do not actually tune to see if a large tuning step will occur
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static Si570_ResultCodes Si570_ChangeFrequency(float new_freq, uchar test)
{
    Si570_ResultCodes retval = SI570_OK;
    Si570_FreqConfig new_config;
    Si570_FreqConfig* cur_config_ptr = (Si570_FreqConfig*)&os.cur_config;

    bool    is_small = Si570_FindSmoothRFreqForFreq(new_freq,cur_config_ptr,&new_config);

    if (test == false ) {
        if (is_small == false) {
            if (!Si570_FindConfigForFreq(new_freq,&new_config))
            {
                CriticalError(102);
            }
        }
        retval = Si570_WriteConfig(&new_config,is_small);
        if (retval == SI570_ERROR_VERIFY && is_small == true)
        {
            // sometimes the small change simply does not work
            // for unknown reasons, so we execute a large step
            // instead to recover.
            // TODO: Maybe this should be handled on  the application layer
            // as this introduces some noise without muting
            // The frequencies are not random, i.e. it is possible to reproduce the issue
            // e.g. going from higher frequencies towards 33.901 Mhz in 100 khz steps
            // shows this problem (from a 1 mhz distance, e.g. 34.901 Mhz).
            // but it also happens at much smaller step width.
            retval = Si570_WriteConfig(&new_config,false);
        }
        if (retval == SI570_OK) {
            Si570_CopyConfig(&new_config,cur_config_ptr);
        }
        else
        {
            Si570_ClearConfig(cur_config_ptr);
        }
    }
    else
    {
        if (is_small == false) {
            retval = SI570_LARGE_STEP;
        }
    }
    return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_set_frequency
//* Object              :
//* Input Parameters    : freq = 32 bit frequency in HZ, temp_factor = temperature calibration factor ref to 14.000 MHz, test: 0= set freq, 1= calculate, but do not change freq
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void Si570_ConvExternalTemp(uchar *temp, int *dtemp)
{
    ushort  ts;
    int     t = 0, d = 0;

    if(dtemp == NULL)
        return;

    ts = *temp << 8;
    temp++;
    ts |= *temp;

    // Full part
    t = (ts >> 8) & 0xFF;

    // Decimal part
    if(ts & 0x80) d += 5000;
    if(ts & 0x40) d += 2500;
    if(ts & 0x20) d += 1250;
    if(ts & 0x10) d += 625;

    // Return int temperature (sign ok after this ?)
    *dtemp = (t * 10000) + d;
}


//
// by DF8OE
//
// startupfrequency-subroutine
static void Si570_CalcSufHelper()
{
    uchar si_regs[7];
    int hs_div;
    int n1;
    float rsfreq;
    mchf_hw_i2c_ReadData(os.si570_address, (os.base_reg), si_regs, 6);
    // calculate startup frequency
    rsfreq = (float)((si_regs[5] + (si_regs[4] * 0x100) + (si_regs[3] * 0x10000) + (double)((double)si_regs[2] * (double)0x1000000) + (double)((double)(si_regs[1] & 0x3F) * (double)0x100000000)) / (double)POW_2_28);
    hs_div = (si_regs[0] & 0xE0) / 32 + 4;
    n1 = (si_regs[1] & 0xC0) / 64 + (si_regs[0] & 0x1F) *4 + 1;
    if (n1 %2 != 0 && n1 != 1)
    {
        n1++;
    }
    os.fout = roundf((1142850 * rsfreq) / (hs_div * n1)) / 10000;
}


void Si570_CalculateStartupFrequency()
{
    if(os.fout < 5)
    {
        os.base_reg = 13;	// first test with regs 13+ for 7ppm SI570
        uchar dummy;

        // test for hardware address of SI570
        os.si570_address = (0x50 << 1);
        if(mchf_hw_i2c_ReadRegister(os.si570_address, (os.base_reg), &dummy) != 0)
        {
            os.si570_address = (0x55 << 1);
            mchf_hw_i2c_reset();
        }

        non_os_delay();
        // make sure everything is cleared and in initial state
        Si570_ResetConfiguration();
        Si570_CalcSufHelper();

        if(os.fout > 39.2 && os.fout < 39.3)
        {
            // its a 20 or 50 ppm device, use regs 7+
            os.base_reg = 7;
            Si570_CalcSufHelper();
        }

        // all known startup frequencies
        static const float suf_table[] =
        {
                10,
                10.356,
                14.05,
                14.1,
                15,
                16.0915,
                22.5792,
                34.285,
                56.32,
                63,
                76.8,
                100,
                122,
                125,
                156.25,
                0
        };
        // test if startup frequency is known
        int i;
        for (i = 0; suf_table[i] != 0; i++)
        {
            float test = os.fout - suf_table[i];
            if (test < 0)
                test = -test;
            if (test < 0.2)
            {
                os.fout = suf_table[i];
                break;
            }
        }
    }
}


float   Si570_GetStartupFrequency()
{
    return os.fout;
}

uint8_t   Si570_GeTI2CAddress()
{
    return os.si570_address;
}


uchar Si570_ResetConfiguration()
{
    int i;
    uchar   ret;
    short   res;

    ulong   rfreq_frac;
    ulong   rfreq_int;

    uchar   hsdiv_curr;
    uchar   n1_curr;

    // Reset publics
    os.fxtal        = FACTORY_FXTAL;
    os.present = false;

    res = mchf_hw_i2c_WriteRegister(os.si570_address, SI570_REG_135, SI570_RECALL);
    if(res != 0)
    {
        return 1;
    }

    i = 0;
    do
    {
        res = mchf_hw_i2c_ReadRegister(os.si570_address, SI570_REG_135, &ret);
        if(res != 0)
        {
            return 2;
        }

        i++;

        if(i == 30)
        {
            return 3;
        }
    }  while(ret & SI570_RECALL);


    for(i = 0; i < 6; i++)
    {
        res = mchf_hw_i2c_ReadRegister(os.si570_address, (os.base_reg + i), (uchar*)&(os.regs[i]));
        if(res != 0)
        {
            return 4;
        }
    }

    hsdiv_curr = ((os.regs[0] & 0xE0) >> 5) + 4;

    n1_curr = 1 + ((os.regs[0] & 0x1F) << 2) + ((os.regs[1] & 0xC0) >> 6);


    rfreq_int = (os.regs[1] & 0x3F);
    rfreq_int = (rfreq_int << 4) + ((os.regs[2] & 0xF0) >> 4);

    rfreq_frac = (os.regs[2] & 0x0F);
    rfreq_frac = (rfreq_frac << 8) + os.regs[3];
    rfreq_frac = (rfreq_frac << 8) + os.regs[4];
    rfreq_frac = (rfreq_frac << 8) + os.regs[5];

    float64_t rfreq = rfreq_int + (float64_t)rfreq_frac / POW_2_28;
    os.fxtal = (os.fout * n1_curr * hsdiv_curr) / rfreq;

    os.cur_config.rfreq = rfreq;
    os.cur_config.n1 = n1_curr;
    os.cur_config.hsdiv = hsdiv_curr;
    os.cur_config.fdco = Si570_GetFDCOForFreq(os.fout,n1_curr,hsdiv_curr);

    os.present = true;
    return 0;
}


Si570_ResultCodes Si570_SetFrequency(ulong freq, int calib, int temp_factor, uchar test)
{
    Si570_ResultCodes retval = SI570_TUNE_IMPOSSIBLE;

    if (Si570_IsPresent() == true) {
        float		freq_calc, freq_scale, temp_scale, temp;

        freq_scale = (float)freq;		// get frequency
        freq_calc = freq_scale;		// copy frequency
        freq_scale /= 14000000;		// get scaling factor since our calibrations are referenced to 14.000 MHz

        temp = (float)calib;			// get calibration factor
        temp *= (freq_scale);		// scale calibration for operating frequency but double magnitude of calibration factor

        freq_calc -= temp;				// subtract calibration factor

        temp_scale = (float)temp_factor;	// get temperature factor
        temp_scale /= 14000000;		// calculate scaling factor for the temperature correction (referenced to 14.000 MHz)

        freq_calc *= (1 + temp_scale);	// rescale by temperature correction factor

        // new DF8OE disabler of system crash when tuning frequency is outside SI570 hard limits
        if (freq_calc <= SI570_HARD_MAX_FREQ && freq_calc >= SI570_HARD_MIN_FREQ)
        {
            // tuning inside known working spec
            retval = Si570_ChangeFrequency((float64_t)freq_calc/1000000.0, test);
            if ((freq_calc > SI570_MAX_FREQ  || freq_calc < SI570_MIN_FREQ) && *(__IO uint32_t*)(SRAM2_BASE+5) != 0x29)
            {
                // outside official spec but known to work
                if (retval == SI570_OK)
                {
                    retval = SI570_TUNE_LIMITED;
                }
            }
        }
    }
    return retval;
}


uchar Si570_InitExternalTempSensor()
{
    uchar config, res;

    // Read config reg
    res = mchf_hw_i2c_ReadRegister(MCP_ADDR, MCP_CONFIG, &config);
    if(res != 0)
        return 1;

    // Modify resolution
    config &= ~(3 << MCP_ADC_RES);
    config |= (MCP_ADC_RES_12 << MCP_ADC_RES);

    // Modify power mode
    config &= ~(1 << MCP_SHUTDOWN);
    config |= (MCP_POWER_UP << MCP_SHUTDOWN);

    // Write config reg
    res = mchf_hw_i2c_WriteRegister(MCP_ADDR, MCP_CONFIG, config);
    if(res != 0)
        return 2;

    return 0;
}




uchar Si570_ReadExternalTempSensor(int *temp)
{
    uchar	res;
    uchar	data[10];

    if(data == NULL)
        return 1;

    // Read temperature
    res = mchf_hw_i2c_ReadData(MCP_ADDR, MCP_TEMP, data, 2);
    if(res != 0)
        return 2;

    // Convert to decimal
    Si570_ConvExternalTemp(data, temp);

    return 0;
}


