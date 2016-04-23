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

const uchar	hs_div[6]	= {11, 9, 7, 6, 5, 4};
const float	fdco_max	= FDCO_MAX;
const float	fdco_min	= FDCO_MIN;

// unsigned short si570_address;

// All publics as struct, so eventually could be malloc-ed and in CCM for faster access!!
__IO OscillatorState os;

/*
 * @brief Returns startup frequency value of Si570, call only after init of Si570
 *
 * @returns Startup frequency in Mhz
 */
float   ui_si570_get_startup_frequency() {
  return os.fout;
}
//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_setbits
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_setbits(unsigned char original, unsigned char reset_mask, unsigned char new_val)
{
	return ((original & reset_mask) | new_val);
}

/*
 * @brief reads Si570 registers and verifies match with local copy of settings
 * @returns SI570_OK if matching, SI570_I2C_ERROR if I2C is not working, SI570_ERROR otherwise
 */
static Si570_ResultCodes ui_si570_verify_frequency(void)
{
    Si570_ResultCodes retval = SI570_OK;
    uchar	i, res;
    uchar	regs[10];

    // Read all regs
    for(i = 0; i < 6; i++)
    {
        res = mchf_hw_i2c_ReadRegister(os.si570_address, (os.base_reg + i), &regs[i]);
        if(res != 0) {
            retval = SI570_I2C_ERROR;
            break;
        }
    }

    if (retval == SI570_OK) {
        // Not working - need fix
        // memset(regs, 0, 6);
        // trx4m_hw_i2c_ReadData(os.si570_address, 7, regs, 5);

        if(memcmp(regs, (uchar*)os.regs, 6) != 0) {
            retval = SI570_ERROR_VERIFY;
        }
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
static Si570_ResultCodes ui_si570_small_frequency_change(void)
{
    uint16_t ret;
    Si570_ResultCodes retval = SI570_OK;
    uchar reg_135;

    // Read current
    ret = mchf_hw_i2c_ReadRegister(os.si570_address, SI570_REG_135, &reg_135);

    if (ret == 0) {
        // Write to freeze M bit
        ret = mchf_hw_i2c_WriteRegister(os.si570_address, SI570_REG_135, (reg_135|SI570_FREEZE_M));
        if (ret == 0) {

            // Write as block, registers 7-12
            ret = mchf_hw_i2c_WriteBlock(os.si570_address, os.base_reg, (uchar*)os.regs, 6);
            if (ret == 0) {
                retval = ui_si570_verify_frequency();
            } else {
                retval = SI570_I2C_ERROR;
            }
        }
    }
    mchf_hw_i2c_WriteRegister(os.si570_address, SI570_REG_135, (reg_135 & ~SI570_FREEZE_M));

    return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_large_frequency_change
//* Object              : large frequency changes handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static Si570_ResultCodes ui_si570_large_frequency_change(void)
{
    uint16_t ret;
    uint8_t reg_135, reg_137;
    bool unfreeze = false;
    Si570_ResultCodes retval = SI570_OK;

    // Read the current state of Register 137
    ret = mchf_hw_i2c_ReadRegister(os.si570_address, SI570_REG_137, &reg_137);
    if (ret == 0) {
        // Set the Freeze DCO bit
        ret = mchf_hw_i2c_WriteRegister(os.si570_address, SI570_REG_137, (reg_137|SI570_FREEZE_DCO));
        if (ret == 0) {
            // Write as block, registers 7-12
            ret = mchf_hw_i2c_WriteBlock(os.si570_address, os.base_reg, (uchar*)os.regs, 6);
            if(ret)
            {
                unfreeze = 1;
            } else {
                retval = ui_si570_verify_frequency();
                if(retval != SI570_OK)
                {
                    unfreeze = 1;
                } else {
                    // Clear the Freeze DCO bit
                    ret = mchf_hw_i2c_WriteRegister(os.si570_address, SI570_REG_137, (reg_137 & ~SI570_FREEZE_DCO));
                    if (ret == 0) {
                        // Read current
                        ret = mchf_hw_i2c_ReadRegister(os.si570_address, SI570_REG_135, &reg_135);
                        if (ret == 0) {
                            // Set the NewFreq bit
                            ret = mchf_hw_i2c_WriteRegister(os.si570_address, SI570_REG_135, (reg_135|SI570_NEW_FREQ));
                            if (ret == 0) {
                                // Wait for action completed
                                reg_135 = SI570_NEW_FREQ;
                                while(ret == 0 && (reg_135 & SI570_NEW_FREQ))
                                {
                                    ret = mchf_hw_i2c_ReadRegister(os.si570_address, SI570_REG_135, &reg_135);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if(unfreeze) {
        mchf_hw_i2c_WriteRegister(os.si570_address, SI570_REG_137, (reg_137 & ~SI570_FREEZE_DCO));
    }
    return ret!=0?SI570_I2C_ERROR:retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_is_large_change
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static bool ui_si570_is_large_change(void)
{
    long double	delta_rfreq;
    bool retval = false;

    if(os.rfreq_old != os.rfreq) {
        if(os.rfreq_old < os.rfreq) {
            delta_rfreq = os.rfreq - os.rfreq_old;
        } else {
            delta_rfreq = os.rfreq_old - os.rfreq;
        }

        if((delta_rfreq / os.rfreq_old) > 0.0035) {
            retval = true;
        }
    }
    return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_get_configuration
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar ui_si570_get_configuration(void)
{
	int	i;
	uchar	ret;
	short	res;

	ulong	rfreq_frac;
	ulong	rfreq_int;

	uchar	hsdiv_curr;
	uchar	n1_curr;
	// uchar	sig[10];

	// Reset publics
	os.rfreq_old	= 0.0;
	os.fxtal		= FACTORY_FXTAL;

	res = mchf_hw_i2c_WriteRegister(os.si570_address, SI570_REG_135, SI570_RECALL);
	if(res != 0)
	{
		return 1;
	}

	ret = SI570_RECALL;
	i = 0;
	while(ret & SI570_RECALL)
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
	}

	for(i = 0; i < 6; i++)
	{
		res = mchf_hw_i2c_ReadRegister(os.si570_address, (os.base_reg + i), (uchar*)&(os.regs[i]));
		if(res != 0)
		{
			return 4;
		}
	}

	hsdiv_curr = ((os.regs[0] & 0xE0) >> 5) + 4;

#ifdef LOWER_PRECISION
	os.init_hsdiv = hsdiv_curr;
#endif

	n1_curr = ((os.regs[0] & 0x1F) << 2) + ((os.regs[1] & 0xC0) >> 6);
	if(n1_curr == 0)
		n1_curr = 1;
	else if((n1_curr & 1) != 0)
		n1_curr = n1_curr + 1;

#ifdef LOWER_PRECISION
	os.init_n1 = n1_curr;
#endif

	rfreq_int =	(os.regs[1] & 0x3F);
	rfreq_int =	(rfreq_int << 4) + ((os.regs[2] & 0xF0) >> 4);

	rfreq_frac = (os.regs[2] & 0x0F);
	rfreq_frac = (rfreq_frac << 8) + os.regs[3];
	rfreq_frac = (rfreq_frac << 8) + os.regs[4];
	rfreq_frac = (rfreq_frac << 8) + os.regs[5];

#ifdef LOWER_PRECISION
	os.init_rfreq = (os.regs[1] & 0x3F);
	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[2]);
	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[3]);
	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[4]);
	os.init_rfreq = (os.init_rfreq << 6) + (os.regs[5] >> 2);
#endif

	os.rfreq = rfreq_int + rfreq_frac / POW_2_28;
	os.fxtal = (os.fout * n1_curr * hsdiv_curr) / os.rfreq;
	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_change_frequency
//* Object              :
//* Input Parameters    : input frequency (float), test: 0 = tune, 1 = calculate, but do not actually tune to see if a large tuning step will occur
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static Si570_ResultCodes ui_si570_change_frequency(float new_freq, uchar test)
{
    uchar	i;
    ushort	divider_max, curr_div, whole;

    float	curr_n1;
    float	n1_tmp;

    ulong	frac_bits;
    bool    is_large = false;
    Si570_ResultCodes retval = SI570_OK;

#ifdef LOWER_PRECISION
    float	ratio = 0;
    ulong	final_rfreq_long;
#endif

    uchar	n1;
    uchar	hsdiv;

    divider_max = (ushort)floorf(fdco_max / new_freq);
    curr_div	= (ushort)ceilf (fdco_min / new_freq);

    bool found;
    for (found = false;(curr_div <= divider_max) && !found; curr_div++)
    {
        for(i = 0; i < 6; i++)
        {
            hsdiv = hs_div[i];
            curr_n1 = (float)(curr_div) / (float)(hsdiv);

            n1_tmp = floorf(curr_n1);
            n1_tmp = curr_n1 - n1_tmp;

            if(n1_tmp == 0.0)
            {
                n1 = (uchar)curr_n1;
                if((n1 == 1) || ((n1 & 1) == 0))
                {
                    found = true;
                    break;
                }
            }
        }
    }

    if (!found) {
        CriticalError(102);
    }
    else
    {
        // New RFREQ calculation
        os.rfreq = ((long double)new_freq * (long double)(n1 * hsdiv)) / os.fxtal;
        is_large = ui_si570_is_large_change();
        // check to see if this tuning will result in a "large" tuning step, without setting the frequency yet;
        if (!test) {
#ifdef LOWER_PRECISION
            ratio = new_freq / fout0;
            ratio = ratio * (((float)n1)/((float)os.init_n1));
            ratio = ratio * (((float)hsdiv)/((float)os.init_hsdiv));
            final_rfreq_long = ratio * os.init_rfreq;
#endif

            for(i = 0; i < 6; i++) {
                os.regs[i] = 0;
            }

            hsdiv = hsdiv - 4;
            os.regs[0] = (hsdiv << 5);

            if(n1 == 1) {
                n1 = 0;
            } else if((n1 & 1) == 0) {
                n1 = n1 - 1;
            }

            os.regs[0] = ui_si570_setbits(os.regs[0], 0xE0, (n1 >> 2));
            os.regs[1] = (n1 & 3) << 6;

#ifdef LOWER_PRECISION
            os.regs[1] = os.regs[1] | (final_rfreq_long >> 30);
            os.regs[2] = final_rfreq_long >> 22;
            os.regs[3] = final_rfreq_long >> 14;
            os.regs[4] = final_rfreq_long >> 6;
            os.regs[5] = final_rfreq_long << 2;
#endif

#ifdef HIGHER_PRECISION
            whole = floorf(os.rfreq);
            frac_bits = floorf((os.rfreq - whole) * POW_2_28);

            for(i = 5; i >= 3; i--)
            {
                os.regs[i] = frac_bits & 0xFF;
                frac_bits = frac_bits >> 8;
            }

            os.regs[2] = ui_si570_setbits(os.regs[2], 0xF0, (frac_bits & 0xF));
            os.regs[2] = ui_si570_setbits(os.regs[2], 0x0F, (whole & 0xF) << 4);
            os.regs[1] = ui_si570_setbits(os.regs[1], 0xC0, (whole >> 4) & 0x3F);
#endif


            //
            if(is_large)
            {
                retval = ui_si570_large_frequency_change();
            }
            else
            {
                retval = ui_si570_small_frequency_change();
                if (retval != SI570_OK) {
                    // Maybe large change was needed, let's try it
                    retval = ui_si570_large_frequency_change();
                }
            }
            if(retval == SI570_OK) {

                // Verify second time - we might be transmitting, so
                // it is absolutely unacceptable to be on startup
                // SI570 frequency if any I2C error or chip reset occurs!
                retval = ui_si570_verify_frequency();
                if(retval == SI570_OK) {
                    os.rfreq_old = os.rfreq;
                }
            }
        } else {
            if (is_large) {
                retval = SI570_LARGE_STEP;
            }
        }
    }
    if (retval == SI570_ERROR_VERIFY) {
        // make sure to generate a large step
        // in order to recover on next tune attempt
        os.rfreq_old = 0.0;
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
Si570_ResultCodes ui_si570_set_frequency(ulong freq, int calib, int temp_factor, uchar test)
{
	Si570_ResultCodes retval = SI570_OK;
	long double d;
	float		si_freq, freq_calc, freq_scale, temp_scale, temp;

	freq_scale = (float)freq;		// get frequency
	freq_calc = freq_scale;		// copy frequency
	freq_scale /= 14000000;		// get scaling factor since our calibrations are referenced to 14.000 MHz
	//
	temp = (float)calib;			// get calibration factor
	temp *= (freq_scale);		// scale calibration for operating frequency but double magnitude of calibration factor
	freq_calc -= temp;				// subtract calibration factor
	//
	//
	temp_scale = (float)temp_factor;	// get temperature factor
	temp_scale /= 14000000;		// calculate scaling factor for the temperature correction (referenced to 14.000 MHz)
	//
	freq_calc *= (1 + temp_scale);	// rescale by temperature correction factor
	//
	freq = (ulong)freq_calc;


	d = freq;								// convert to float
	d = d / 1000000.0;						// Si570 set value = decimal MHz
	si_freq = d;							// convert to float

	// new DF8OE disabler of system crash when tuning frequency is outside SI570 hard limits
	if (si_freq <= SI570_MAX_FREQ / 1000000 && si_freq >= SI570_MIN_FREQ / 1000000)	{
		// tuning inside specs, tuning ok, color white
		retval = ui_si570_change_frequency(si_freq, test);
	}
	else if (si_freq <= SI570_HARD_MAX_FREQ / 1000000 && si_freq >= SI570_HARD_MIN_FREQ / 1000000)	{
		// tuning outside specs, inside hard limits --> tuning with gaps, color yellow
		ui_si570_change_frequency(si_freq, test);
		retval = SI570_TUNE_LIMITED;
	}
	else	{
		// tuning outside hard limits, tuning bad, color red
		if (si_freq > SI570_MAX_FREQ / 1000000)
			si_freq = SI570_MAX_FREQ / 1000000;
		if (si_freq < SI570_MIN_FREQ / 1000000)
			si_freq = SI570_MIN_FREQ / 1000000;
		ui_si570_change_frequency(si_freq, test);
		retval = SI570_TUNE_IMPOSSIBLE;
	}

	return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_init_temp_sensor
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar ui_si570_init_temp_sensor(void)
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

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_read_temp
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar ui_si570_read_temp(int *temp)
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
	ui_si570_conv_temp(data, temp);

	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_conv_temp
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_si570_conv_temp(uchar *temp, int *dtemp)
{
	ushort	ts;
	int		t = 0, d = 0;

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
//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_calc_startupfrequency
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void ui_si570_calculate_startup_frequency(void)
{
	if(os.fout >= 5)
		return;
	os.base_reg = 13;	// first test with regs 13+ for 7ppm SI570
	uchar dummy;

	// test for hardware address of SI570
	os.si570_address = (0x50 << 1);
	if(mchf_hw_i2c_ReadRegister(os.si570_address, (os.base_reg), &dummy) != 0)
	{
		os.si570_address = (0x55 << 1);
		mchf_hw_i2c_reset();
	}

	calc_suf_sub();

	if(os.fout > 39.2 && os.fout < 39.3)
	{	
		// its a 20 or 50 ppm device, use regs 7+
		os.base_reg = 7;
		calc_suf_sub();
	}

	// all known startup frequencies
	float suf_table[] = {
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

// startupfrequency-subroutine
void calc_suf_sub(void)
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
