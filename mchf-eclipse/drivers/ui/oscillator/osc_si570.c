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

// Common
#include "uhsdr_board.h"
#include "codec.h"
//
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "uhsdr_hw_i2c.h"
#include "osc_si570.h"

// -------------------------------------------------------------------------------------
// Local Oscillator
// ------------------
/* There are two supported alternative choices for the Si570:
     USE_SI570_CMOS     frequencies up to 160/220 Mhz, used in most boards
     USE_SI570_CGRADE   frequencies up to 280 Mhz, used in Lapwing
*/
#ifndef RF_BRD_LAPWING
    #define USE_SI570_CMOS
#else
    #define USE_SI570_CGRADE
#endif

// The SI570 Min/Max frequencies id spec sheet
// The "hard limits" frequencies below/above which the synthesizer cannot be adjusted or else the system may crash
// Lower limits apply to all cases

#define SI570_MIN_FREQ          10000000    //10.0=2.5 MHz
#define SI570_HARD_MIN_FREQ     3500000     // 3.5=0.875 MHz

#ifdef USE_SI570_CGRADE
    #define SI570_MAX_FREQ          280000000   // 280=70 Mhz
    #define SI570_HARD_MAX_FREQ     280000000   // 280=70 MHz
#endif

#ifdef USE_SI570_CMOS
    #define SI570_MAX_FREQ          160000000   // 160=40 Mhz
    #define SI570_HARD_MAX_FREQ     220000000   // 220=55 MHz
#endif

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


typedef struct {
    uint8_t hsdiv;
    uint8_t n1;
    float64_t fdco;
    float64_t rfreq;
    float64_t freq;
} Si570_FreqConfig;

typedef struct OscillatorState
{
    Si570_FreqConfig    cur_config;
    Si570_FreqConfig    next_config;

    float64_t               fxtal;      // base fxtal value
    float64_t               fxtal_ppm;  // Frequency Correction of fxtal_calc
    float64_t               fxtal_calc; // ppm corrected fxtal value

    uint8_t             cur_regs[6];

    bool                next_is_small;

    float               fout;       // contains startup frequency info of Si570

    unsigned short      si570_address;

    uint8_t             base_reg;

    bool                present; // is a working Si570 present?
} OscillatorState;


#define SMOOTH_DELTA (0.0035)
// Datasheet says 0.0035  == 3500PPM but there have been issues if we get close to that value.
// to play it safe, we make the delta range a little smaller.
// if you want to play with it, tune to the end of the 10m band, set 100 khz step width and dial around
// sooner or later jumps with delta close to 0.0035 (actually a calculated delta of 0.00334) cause a "crash" of Si570

static const uchar	hs_div[6]	= {11, 9, 7, 6, 5, 4};
static const float	fdco_max	= FDCO_MAX;
static const float	fdco_min	= FDCO_MIN;

OscillatorState os;

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

static uint16_t Si570_ReadRegisters(uint8_t* regs)
{
    return UhsdrHw_I2C_ReadBlock(SI570_I2C, os.si570_address, os.base_reg, 1, regs, 6);
}

/*
 * @brief reads Si570 registers and verifies match with local copy of settings
 * @returns SI570_OK if matching, SI570_I2C_ERROR if I2C is not working, SI570_ERROR otherwise
 */
static Oscillator_ResultCodes_t Si570_VerifyFrequencyRegisters()
{
    Oscillator_ResultCodes_t retval = OSC_OK;
    uchar	regs[6];

    // Read all regs
    if (Si570_ReadRegisters(&regs[0]))
    {
            retval = OSC_COMM_ERROR;
    }

    if (retval == OSC_OK)
    {
        // Not working - need fix
        // memset(regs, 0, 6);
        // trx4m_hw_i2c_ReadData(os.si570_address, 7, regs, 5);

        if(memcmp(regs, (uchar*)os.cur_regs, 6) != 0)
        {
            retval = OSC_ERROR_VERIFY;
        }
    }

    return retval;
}

static uint16_t Si570_SetRegisterBits(uint8_t si570_address, uint8_t regaddr ,uint8_t* reg_ptr,uint8_t val){
    uint16_t retval = UhsdrHw_I2C_ReadRegister(SI570_I2C, si570_address, regaddr, 1, reg_ptr);
    if (retval == 0)
    {
        retval = UhsdrHw_I2C_WriteRegister(SI570_I2C, si570_address, regaddr, 1, (*reg_ptr|val));
    }
    return retval;
}
static uint16_t Si570_ClearRegisterBits(uint8_t si570_address, uint8_t regaddr ,uint8_t* reg_ptr,uint8_t val){
    uint16_t retval = UhsdrHw_I2C_ReadRegister(SI570_I2C, si570_address, regaddr, 1, reg_ptr);
    if (retval == 0)
    {
        retval = UhsdrHw_I2C_WriteRegister(SI570_I2C, si570_address, regaddr, 1, (*reg_ptr & ~val));
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
static Oscillator_ResultCodes_t Si570_SmallFrequencyChange()
{
    uint16_t ret;
    Oscillator_ResultCodes_t retval = OSC_OK;
    uchar reg_135;

    // Read current
    ret = Si570_SetRegisterBits(os.si570_address, SI570_REG_135, &reg_135, SI570_FREEZE_M);
    if (ret == 0)
    {
        // Write as block, registers 7-12
        ret = UhsdrHw_I2C_WriteBlock(SI570_I2C, os.si570_address, os.base_reg, 1, (uchar*)os.cur_regs, 6);
        if (ret == 0)
        {
            retval = Si570_VerifyFrequencyRegisters();
        }
        else
        {
            retval = OSC_COMM_ERROR;
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
static Oscillator_ResultCodes_t Si570_LargeFrequencyChange()
{
    uint16_t ret;
    uint8_t reg_135, reg_137;
    Oscillator_ResultCodes_t retval = OSC_COMM_ERROR;

    if (Si570_SetRegisterBits(os.si570_address, SI570_REG_137, &reg_137, SI570_FREEZE_DCO) == 0)
    {
        // Write as block, registers 7-12
        if(UhsdrHw_I2C_WriteBlock(SI570_I2C, os.si570_address, os.base_reg, 1, (uchar*)os.cur_regs, 6) == 0)
        {
            retval = Si570_VerifyFrequencyRegisters();
        }
    }

    // no matter what happened, try to unfreeze the Si570
    ret = Si570_ClearRegisterBits(os.si570_address, SI570_REG_137, &reg_137, SI570_FREEZE_DCO);

    if (ret == 0 && retval == OSC_OK)
    {
        if (Si570_SetRegisterBits(os.si570_address, SI570_REG_135, &reg_135, SI570_NEW_FREQ) == 0)
        {
            // Wait for action completed
            do
            {
                ret = UhsdrHw_I2C_ReadRegister(SI570_I2C, os.si570_address, SI570_REG_135, 1, &reg_135);
            } while(ret == 0 && (reg_135 & SI570_NEW_FREQ));
        }
    }
    return ret!=0?OSC_COMM_ERROR:retval;
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

static float64_t Si570_GetFDCOForFreq(float64_t new_freq, uint8_t n1, uint8_t hsdiv){
    return (new_freq * (float64_t)(n1 * hsdiv));
}

static bool Si570_FindSmoothRFreqForFreq(const Si570_FreqConfig* cur_config, Si570_FreqConfig* new_config) {
    float64_t fdco = Si570_GetFDCOForFreq(new_config->freq, cur_config->n1, cur_config->hsdiv);
    bool retval = false;
    float64_t fdiff = (fdco - cur_config->fdco)/cur_config->fdco;

    if (fdiff < 0.0)
    {
        fdiff = -fdiff;
    }

    if (fdiff <= SMOOTH_DELTA && Si570_FDCO_InRange(fdco))
    {
        new_config->rfreq = fdco / (float64_t)os.fxtal_calc;
        new_config->fdco = cur_config->fdco; // since we do only a small step, our fdco remains the same, so that we can keep an eye on the +/-3500ppm  rule
        new_config->n1 = cur_config->n1;
        new_config->hsdiv = cur_config->hsdiv;

        retval = true;
    }
    return retval;
}


static bool Si570_FindConfigForFreq(Si570_FreqConfig* config) {
    uchar   i;
    uint16_t  divider_max, curr_div;
    bool retval = false;

    bool    n1_found = false;
    uint8_t   n1 = 1; // this is to shut up gcc 4.9.x regarding uninitalized variable
    uint8_t   hsdiv;
    float64_t fdco;

    divider_max = (ushort)floorf(fdco_max / config->freq);
    curr_div    = (ushort)ceilf (fdco_min / config->freq);

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
        fdco = Si570_GetFDCOForFreq(config->freq,n1,hsdiv);
        if (Si570_FDCO_InRange(fdco)) {
            config->n1 =n1;
            config->hsdiv = hsdiv;
            config->fdco = fdco;
            config->rfreq = fdco / os.fxtal_calc;

            retval = true;
        }
    }
    return retval;
}

static Oscillator_ResultCodes_t Si570_ConfigToRegs(Si570_FreqConfig* config, uint8_t regs[6]) {

    uint32_t   frac_bits;
    uint16_t whole;

    uint8_t n1_regVal = config->n1 - 1;
    uint8_t hsdiv_regVal = config->hsdiv - 4;
    uint8_t i;
    // the written value is n1 - 1, hsdiv -4 according to the datasheet
    Oscillator_ResultCodes_t retval = OSC_OK;

    for(i = 0; i < 6; i++)
    {
        regs[i] = 0;
    }

    regs[0] = (hsdiv_regVal << 5);

    regs[0] = Si570_SetBits(regs[0], 0xE0, (n1_regVal >> 2));
    regs[1] = (n1_regVal & 3) << 6;

    whole = floorf(config->rfreq);
    frac_bits = floorf((config->rfreq - whole) * POW_2_28);

    for(i = 5; i >= 3; i--)
    {
        regs[i] = frac_bits & 0xFF;
        frac_bits = frac_bits >> 8;
    }

    regs[2] = Si570_SetBits(regs[2], 0xF0, (frac_bits & 0xF));
    regs[2] = Si570_SetBits(regs[2], 0x0F, (whole & 0xF) << 4);
    regs[1] = Si570_SetBits(regs[1], 0xC0, (whole >> 4) & 0x3F);

    return retval;
}


static Oscillator_ResultCodes_t Si570_WriteRegs(bool is_small) {

    Oscillator_ResultCodes_t retval = OSC_OK;

    if(is_small)
    {
        retval = Si570_SmallFrequencyChange();
    }
    else
    {
        retval = Si570_LargeFrequencyChange();
    }
    if(retval == OSC_OK)
    {

        // Verify second time - we might be transmitting, so
        // it is absolutely unacceptable to be on startup
        // SI570 frequency if any I2C error or chip reset occurs!
        retval = Si570_VerifyFrequencyRegisters();
    }
    return retval;
}


static Oscillator_ResultCodes_t Si570_PrepareChangeFrequency(float64_t new_freq)
{
    Oscillator_ResultCodes_t retval = OSC_OK;
    Si570_FreqConfig* next_config_ptr = &os.next_config;
    Si570_FreqConfig* cur_config_ptr = &os.cur_config;

    next_config_ptr->freq = new_freq;

    os.next_is_small = Si570_FindSmoothRFreqForFreq(cur_config_ptr,next_config_ptr);

    if (os.next_is_small == false && Si570_FindConfigForFreq(next_config_ptr) == false)
    {
        retval = OSC_TUNE_IMPOSSIBLE;
    }
    else
    {
        retval = Si570_ConfigToRegs(next_config_ptr,os.cur_regs);
    }
    return retval;

}

/**
 * @returns true if the next prepared step will be a large one, requiring sound muting etc. Requires a call to Si570_PrepareNextFrequency to have correct information
 */
static bool Si570_IsNextStepLarge()
{
    return os.next_is_small == false;
}

/**
 * @brief execute the prepared frequency change. May be called multiple times in case of I2C issues
 * @returns SI570_OK or SI570_I2C_ERROR if I2C communication failed (which can happen at very high I2C speeds).  SI570_ERROR_VERIFY should never happen anymore.
 */
static Oscillator_ResultCodes_t Si570_ChangeToNextFrequency()
{
    Oscillator_ResultCodes_t retval = OSC_OK;
    Si570_FreqConfig* next_config_ptr = &os.next_config;
    Si570_FreqConfig* cur_config_ptr = &os.cur_config;

    retval = Si570_WriteRegs(os.next_is_small);

    // TODO: remove this handling, since it was almost certainly caused
    // by a wrong interpretation of the data sheet regarding small steps.
    if (retval == OSC_ERROR_VERIFY && os.next_is_small == true)
    {
        //
        // sometimes the small change simply does not work
        // for unknown reasons, so we execute a large step
        // instead to recover.
        retval = Si570_WriteRegs(false);
    }

    // If everything is fine, get on with remembering our current configuration
    if (retval == OSC_OK) {
        Si570_CopyConfig(next_config_ptr,cur_config_ptr);
    }
    else
    {
        Si570_ClearConfig(cur_config_ptr);
    }
    return retval;
}



//
// by DF8OE
//
// startupfrequency-subroutine
static void Si570_CalcSufHelper()
{
    uchar si_regs[6];
    int hs_div;
    int n1;
    float rsfreq;
    Si570_ReadRegisters(si_regs);
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



float   Si570_GetStartupFrequency()
{
    return os.fout;
}

uint8_t   Si570_GetI2CAddress()
{
    return os.si570_address;
}

/**
 * @brief Sets a new PPM value AND corrects the internally used xtal frequency accordingly
 * @param ppm ppm value
 */
static void Si570_SetPPM(float32_t ppm)
{
    os.fxtal_ppm = ppm;
    os.fxtal_calc = os.fxtal + (os.fxtal / (float64_t)1000000.0) * os.fxtal_ppm;
    if (Si570_PrepareChangeFrequency(os.cur_config.freq) == OSC_OK)
    {
        Si570_ChangeToNextFrequency();
    }
}

static uint8_t Si570_ResetConfiguration()
{
    uint8_t retval = 0;

    short   res;

    ulong   rfreq_frac;
    ulong   rfreq_int;

    uchar   hsdiv_curr;
    uchar   n1_curr;

    // Reset publics
    os.fxtal        = FACTORY_FXTAL;
    os.present = false;

    res = UhsdrHw_I2C_WriteRegister(SI570_I2C, os.si570_address, SI570_REG_135, 1, SI570_RECALL);
    if(res != 0)
    {
        retval = 1;
    }
    else
    {
        uint8_t   ret;
        int i = 0;
        do
        {
            res = UhsdrHw_I2C_ReadRegister(SI570_I2C, os.si570_address, SI570_REG_135, 1, &ret);
            if(res != 0)
            {
                retval = 2;
                break;
            }

            i++;

            if(i == 30)
            {
            	retval = 3;
            	break;
            }
        }  while(ret & SI570_RECALL);

        if (retval == 0 && Si570_ReadRegisters(&(os.cur_regs[0])) != 0)
        {
        	retval = 4;
        }
        else
        {
        	hsdiv_curr = ((os.cur_regs[0] & 0xE0) >> 5) + 4;

        	n1_curr = 1 + ((os.cur_regs[0] & 0x1F) << 2) + ((os.cur_regs[1] & 0xC0) >> 6);


        	rfreq_int = (os.cur_regs[1] & 0x3F);
        	rfreq_int = (rfreq_int << 4) + ((os.cur_regs[2] & 0xF0) >> 4);

        	rfreq_frac = (os.cur_regs[2] & 0x0F);
        	rfreq_frac = (rfreq_frac << 8) + os.cur_regs[3];
        	rfreq_frac = (rfreq_frac << 8) + os.cur_regs[4];
        	rfreq_frac = (rfreq_frac << 8) + os.cur_regs[5];

        	float64_t rfreq = rfreq_int + (float64_t)rfreq_frac / POW_2_28;
        	os.fxtal = ((float64_t)os.fout * (float64_t)(n1_curr *hsdiv_curr)) / rfreq;

        	Si570_SetPPM(os.fxtal_ppm);

        	os.cur_config.rfreq = rfreq;
        	os.cur_config.n1 = n1_curr;
        	os.cur_config.hsdiv = hsdiv_curr;
        	os.cur_config.fdco = Si570_GetFDCOForFreq(os.fout,n1_curr,hsdiv_curr);

        	os.present = true;
        }
    }
    return retval;
}

static Oscillator_ResultCodes_t Si570_PrepareNextFrequency(ulong freq, int temp_factor);

/**
 * @brief Checks if all oscillator resources are available for switching frequency
 * It basically checks if the I2C is currently in use
 * This function must be called before changing the oscillator in interrupts
 * otherwise deadlocks may happen
 * @return true if it safe to call oscillator functions in an interrupt
 */
bool Si570_ReadyForIrqCall()
{
    return (SI570_I2C->Lock == HAL_UNLOCKED);
}

static bool Oscillator_IsPresent()
{
    return os.present;
}

static uint32_t Si570_getMinFrequency()
{
#ifdef RF_BRD_LAPWING
    return 1240000000L;
#else
    return SI570_HARD_MIN_FREQ/4;
#endif
}

static uint32_t Si570_getMaxFrequency()
{
#ifdef RF_BRD_LAPWING
    return 1300000000L;
#else
    return SI570_HARD_MAX_FREQ/4;
#endif
}

static uint32_t Si570_translateExt2Osc(uint32_t freq)
{
#ifdef RF_BRD_LAPWING
    return freq - 1124500000L;
#else
    return freq * 4;
    // frequency multiplied with 4 since we drive a johnson counter for phased clock generation
#endif
}
const OscillatorInterface_t osc_si570 =
{
		.init = Si570_Init,
		.isPresent = Oscillator_IsPresent,
		.setPPM = Si570_SetPPM,
		.prepareNextFrequency = Si570_PrepareNextFrequency,
		.changeToNextFrequency = Si570_ChangeToNextFrequency,
		.isNextStepLarge = Si570_IsNextStepLarge,
		.readyForIrqCall = Si570_ReadyForIrqCall,
		.name = "Si570",
		.type = OSC_SI570,
		.getMinFrequency = Si570_getMinFrequency,
		.getMaxFrequency = Si570_getMaxFrequency,
};


void Si570_Init()
{

	os.base_reg = 13;	// first test with regs 13+ for 7ppm SI570
	uchar dummy;

	// test for hardware address of SI570
	os.si570_address = (0x55 << 1);
	if(UhsdrHw_I2C_ReadRegister(SI570_I2C,os.si570_address, (os.base_reg),1, &dummy) != 0)
	{
		os.si570_address = (0x50 << 1);
	}


	if (UhsdrHw_I2C_DeviceReady(SI570_I2C,os.si570_address) == HAL_OK)
	{
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
		for (int i = 0; suf_table[i] != 0; i++)
		{
			float test = os.fout - suf_table[i];
			if (test < 0)
			{
				test = -test;
			}
			if (test < 0.2)
			{
				os.fout = suf_table[i];
				break;
			}
		}

		// we wait a little and then reset again
		HAL_Delay(40);
		Si570_ResetConfiguration();
	}
	osc = os.present?&osc_si570:NULL;
}

/**
 * @brief prepares all necessary information for the next frequency change
 * @param freq frequency in Hz to which the LO should be tuned.
 * @param calib the calibration correction value for the real vs. data sheet frequency of the Si570.
 * @param temp_factor the SoftTCXO code calculates a temperature correct value which is used to make a virtual tcxo out of the Si570.
 *
 * @returns SI570_TUNE_IMPOSSIBLE if tuning to the desired frequency is not possible at all (multiple reasons), SI570_OK if it is possible and within spec, SI570_TUNE_LIMITED if possible but out of spec
 */
static Oscillator_ResultCodes_t Si570_PrepareNextFrequency(ulong freq, int temp_factor)
{
    Oscillator_ResultCodes_t retval = OSC_TUNE_IMPOSSIBLE;

    if (osc->isPresent() == true) {
        float64_t  freq_calc, temp_scale;

        freq_calc = Si570_translateExt2Osc(freq);

        temp_scale = ((float64_t)temp_factor)/14000000.0;
        // calculate scaling factor for the temperature correction (referenced to 14.000 MHz)

        freq_calc *= (1 + temp_scale);  // rescale by temperature correction factor

        // when tuning frequency is outside SI570 hard limits, don't do it
        if (freq_calc <= SI570_HARD_MAX_FREQ && freq_calc >= SI570_HARD_MIN_FREQ)
        {
            // tuning inside known working spec
            retval = Si570_PrepareChangeFrequency(freq_calc/((float64_t)1000000.0));
            if ((freq_calc > SI570_MAX_FREQ  || freq_calc < SI570_MIN_FREQ))
            {
                // outside official spec but known to work
                if (retval == OSC_OK)
                {
                    retval = OSC_TUNE_LIMITED;
                }
            }
        }
    }
    return retval;
}
