/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                              C Turner - KA7OEI 2014                             **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:    	ui_menu.c                                                      **
**  Description:    main user interface configuration/adjustment menu system       **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/
// Common
//
#include <src/mchf_version.h>
#include "mchf_board.h"
#include "ui_menu.h"
#include "ui_menu_internal.h"
#include "ui_configuration.h"
#include "config_storage.h"
#include "serial_eeprom.h"
#include "ui_spectrum.h"
#include "rtc.h"

#include <stdio.h>
#include <stdlib.h>

#include "arm_math.h"
#include "math.h"
#include "codec.h"
#include "radio_management.h"
#include "soft_tcxo.h"

void float2fixedstr(char* buf, int maxchar, float32_t f, uint16_t digitsBefore, uint16_t digitsAfter)
{
    char formatstr[16],numberstr[32];

    int32_t mult_digits = pow10(digitsAfter);

    snprintf(formatstr,16,(digitsBefore + digitsBefore > 0)?"%%%dd":"%%d",digitsBefore + digitsAfter);

    snprintf(numberstr, 31, formatstr, (int32_t)(f * mult_digits));

    int32_t num_len = strnlen(numberstr,32);

    int idx = 0;
    for (idx= 0; idx < num_len - digitsAfter && idx < maxchar; idx++)
    {
        buf[idx] = numberstr[idx];
    }

    if (idx == 0 || buf[idx-1] == '-')
    {
        buf[idx++] = '0';
    }
    else if (buf[idx-1] == ' ')
    {
        buf[idx-1] = '0';
    }

    buf[idx++] = '.';

    for (int i= num_len - digitsAfter; i < num_len && i < maxchar; i++)
    {
        char c = numberstr[i];
        if (c == ' ')
        {
            c = '0';
        }
        buf[idx++] = c;
    }
    buf[idx] = '\0';
}


// LCD
#include "ui_lcd_hy28.h" // for colors!

#include "mchf_hw_i2c.h"
#include "mchf_rtc.h"

// Codec control
#include "codec.h"
#include "softdds.h"

#include "audio_driver.h"
#include "audio_filter.h"
#include "audio_management.h"
#include "ui_driver.h"
#include "cat_driver.h"

// CW generation
#include "cw_gen.h"


// returns true if the value was changed in its value!
bool __attribute__ ((noinline)) UiDriverMenuItemChangeUInt8(int var, uint8_t mode, volatile uint8_t* val_ptr,uint8_t val_min,uint8_t val_max, uint8_t val_default, uint8_t increment)
{
    uint8_t old_val = *val_ptr;

    if(var >= 1)	 	// setting increase?
    {
        ts.menu_var_changed = 1;	// indicate that a change has occurred
        if (*val_ptr < val_max)
        {
            (*val_ptr)+= increment;
        }
    }
    else if(var <= -1)	 	// setting decrease?
    {
        ts.menu_var_changed = 1;
        if (*val_ptr > val_min)
        {
            (*val_ptr)-= increment;
        }

    }
    if(*val_ptr < val_min)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_min;
    }
    if(*val_ptr > val_max)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_max;
    }
    if(mode == MENU_PROCESS_VALUE_SETDEFAULT)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_default;
    }

    return old_val != *val_ptr;
}

bool __attribute__ ((noinline)) UiDriverMenuItemChangeUInt32(int var, uint32_t mode, volatile uint32_t* val_ptr,uint32_t val_min,uint32_t val_max, uint32_t val_default, uint32_t increment)
{
    uint32_t old_val = *val_ptr;
    if(var >= 1)	 	// setting increase?
    {
        ts.menu_var_changed = 1;	// indicate that a change has occurred
        if (*val_ptr < val_max)
        {
            (*val_ptr)+= increment;
        }
    }
    else if(var <= -1)	 	// setting decrease?
    {
        ts.menu_var_changed = 1;
        if (*val_ptr > val_min)
        {
            (*val_ptr)-= increment;
        }

    }
    if(*val_ptr < val_min)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_min;
    }
    if(*val_ptr > val_max)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_max;
    }
    if(mode == MENU_PROCESS_VALUE_SETDEFAULT)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_default;
    }
    return old_val != *val_ptr;
}

bool __attribute__ ((noinline)) UiDriverMenuItemChangeInt(int var, uint32_t mode, volatile int* val_ptr,int val_min,int val_max, int val_default, uint32_t increment)
{
    uint32_t old_val = *val_ptr;
    if(var >= 1)	 	// setting increase?
    {
        ts.menu_var_changed = 1;	// indicate that a change has occurred
        if (*val_ptr < val_max)
        {
            (*val_ptr)+= increment;
        }
    }
    else if(var <= -1)	 	// setting decrease?
    {
        ts.menu_var_changed = 1;
        if (*val_ptr > val_min)
        {
            (*val_ptr)-= increment;
        }

    }
    if(*val_ptr < val_min)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_min;
    }
    if(*val_ptr > val_max)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_max;
    }
    if(mode == MENU_PROCESS_VALUE_SETDEFAULT)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_default;
    }
    return old_val != *val_ptr;
}

bool inline UiDriverMenuItemChangeInt32(int var, uint32_t mode, volatile int32_t* val_ptr,int val_min,int val_max, int val_default, uint32_t increment)
{
    return UiDriverMenuItemChangeInt(var, mode, (int*)val_ptr,val_min,val_max, val_default, increment);
}


bool __attribute__ ((noinline)) UiDriverMenuItemChangeInt16(int var, uint32_t mode, volatile int16_t* val_ptr,int16_t val_min,int16_t val_max, int16_t val_default, uint16_t increment)
{
    uint32_t old_val = *val_ptr;
    if(var >= 1)	 	// setting increase?
    {
        ts.menu_var_changed = 1;	// indicate that a change has occurred
        if (*val_ptr < val_max)
        {
            (*val_ptr)+= increment;
        }
    }
    else if(var <= -1)	 	// setting decrease?
    {
        ts.menu_var_changed = 1;
        if (*val_ptr > val_min)
        {
            (*val_ptr)-= increment;
        }

    }
    if(*val_ptr < val_min)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_min;
    }
    if(*val_ptr > val_max)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_max;
    }
    if(mode == MENU_PROCESS_VALUE_SETDEFAULT)
    {
        ts.menu_var_changed = 1;
        *val_ptr = val_default;
    }
    return old_val != *val_ptr;
}

bool __attribute__ ((noinline)) UiDriverMenuItemChangeOnOff(int var, uint8_t mode, volatile uint8_t* val_ptr, uint8_t val_default)
{
    // we have to align the values to true and false, since sometimes other values are passed for true (use of temp_var)
    // but this does not work properly.

    *val_ptr = (*val_ptr)?1:0;

    return UiDriverMenuItemChangeUInt8(var, mode, val_ptr,
                                       0,
                                       1,
                                       val_default,
                                       1
                                      );

}

// always sets 1 or 0 as result, no matter what is passed as "true" value. Only 0 is recognized as false/
bool __attribute__ ((noinline)) UiDriverMenuItemChangeDisableOnOff(int var, uint8_t mode, volatile uint8_t* val_ptr, uint8_t val_default, char* options, uint32_t* clr_ptr)
{
    bool res = UiDriverMenuItemChangeOnOff(var, mode, val_ptr, val_default);
    strcpy(options, *val_ptr?"OFF":" ON");
    if (*val_ptr)
    {
        *clr_ptr = Orange;
    }

    return res;
}

bool __attribute__ ((noinline)) UiDriverMenuItemChangeEnableOnOff(int var, uint8_t mode, volatile uint8_t* val_ptr, uint8_t val_default, char* options, uint32_t* clr_ptr)
{
    bool res = UiDriverMenuItemChangeOnOff(var, mode, val_ptr, val_default);
    strcpy(options, *val_ptr?" ON":"OFF");
    if (!*val_ptr)
    {
        *clr_ptr = Orange;
    }

    return res;
}


bool __attribute__ ((noinline)) UiMenu_ChangeFilterPathMemory(int var, uint8_t mode, char* options, uint32_t* clr_ptr, uint16_t filter_mode,uint8_t memory_idx)
{
    uint32_t temp_var = ts.filter_path_mem[filter_mode][memory_idx];
    uint16_t old_fp = temp_var;
    // for now just a single location CW for testing
    bool tchange = UiDriverMenuItemChangeUInt32(var, mode, &temp_var,
                   0,
                   AUDIO_FILTER_PATH_NUM,
                   0,
                   1);
    if(tchange)     // did something change?
    {
        uint16_t fp = AudioFilter_NextApplicableFilterPath(PATH_ALL_APPLICABLE|PATH_DONT_STORE | (temp_var< old_fp?PATH_DOWN:PATH_UP),filter_mode,old_fp);
        if (fp >= old_fp && temp_var < old_fp)
        {
            // wrap around -> we need to insert "0"
            fp = 0;
        }
        ts.filter_path_mem[filter_mode][memory_idx] = fp;
    }
    if (ts.filter_path_mem[filter_mode][memory_idx] > 0)
    {
        const char *filter_names[2];
        AudioFilter_GetNamesOfFilterPath(ts.filter_path_mem[filter_mode][memory_idx],filter_names);
        sprintf(options, "   %s/%s", filter_names[0],filter_names[1]);
    }
    else
    {
        sprintf(options, "      UNUSED");
    }
    return tchange;
}

void UiMenu_HandleDemodModeDisable(int var, uint8_t mode, char* options, uint32_t* clr_ptr, uint16_t demod_mode_disable)
{
    uint8_t mode_disable = (ts.demod_mode_disable & demod_mode_disable) > 0;
    UiDriverMenuItemChangeDisableOnOff(var, mode, &mode_disable,0,options,clr_ptr);
    if(mode_disable == true)
    {
        ts.demod_mode_disable |= demod_mode_disable;
    }
    else
    {
        ts.demod_mode_disable &= ~demod_mode_disable;
    }
}

void UiMenu_HandleIQAdjust(int var, uint8_t mode, char* options, uint32_t* clr_ptr, volatile iq_balance_data_t* val_ptr, const uint16_t txrx_mode, int32_t min, int32_t max, iq_trans_idx_t valid_for)
{
    bool tchange = false;

    // used to decide whether the current transceiver state enables us to change the value
    // later also mode is used but this is for the menu internal operation

    // txrx_mode valid_t        Cond
    // RX        IQ_TRANS_OFF   RadioManagement_GetRealFreqTranslationMode(txrx_mode, ts.dmod_mode, ts.iq_freq_mode) == FREQ_IQ_CONV_MODE_OFF && (is_ssb(ts.dmod_mode) || ts.dmod_mode == DEMOD_CW) // if freq translate is off, ssb and cw are fine
    // TX        IQ_TRANS_OFF   RadioManagement_GetRealFreqTranslationMode(txrx_mode, ts.dmod_mode, ts.iq_freq_mode) == FREQ_IQ_CONV_MODE_OFF && (is_ssb(ts.dmod_mode) || ts.dmod_mode == DEMOD_CW) // we transmit in CW always in non-translated mode
    // RX        IQ_TRANS_ON    RadioManagement_GetRealFreqTranslationMode(txrx_mode, ts.dmod_mode, ts.iq_freq_mode) != FREQ_IQ_CONV_MODE_OFF && (is_ssb(ts.dmod_mode) || ts.dmod_mode == DEMOD_CW)
    // TX        IQ_TRANS_ON    RadioManagement_GetRealFreqTranslationMode(txrx_mode, ts.dmod_mode, ts.iq_freq_mode) != FREQ_IQ_CONV_MODE_OFF && (is_ssb(ts.dmod_mode) || ts.dmod_mode == DEMOD_CW)

    iq_trans_idx_t current_trans_idx =  RadioManagement_GetRealFreqTranslationMode(ts.txrx_mode, ts.dmod_mode, ts.iq_freq_mode) == FREQ_IQ_CONV_MODE_OFF? IQ_TRANS_OFF : IQ_TRANS_ON;

    bool trans_mode_match = current_trans_idx == valid_for && (is_ssb(ts.dmod_mode) || ts.dmod_mode == DEMOD_CW);

    if(trans_mode_match && (ts.txrx_mode == txrx_mode))       // only allow adjustment if in right mode
    {
        tchange = UiDriverMenuItemChangeInt32(var, mode, &val_ptr->value[valid_for],
                min,
                max,
                min,
                1);
        if(tchange)
        {
            AudioManagement_CalcIqPhaseGainAdjust(ts.tune_freq/TUNE_MULT);
        }
    }
    else        // Orange if not in correct mode
    {
        *clr_ptr = Orange;
    }
    if (val_ptr->value[valid_for] == IQ_BALANCE_OFF)
    {
        snprintf(options,32, " OFF");
    }
    else
    {
        snprintf(options,32, "%4d", (int)val_ptr->value[valid_for]);
    }
}

void UiMenu_HandleIQAdjustGain(int var, uint8_t mode, char* options, uint32_t* clr_ptr, volatile iq_balance_data_t* val_ptr, const uint16_t txrx_mode, iq_trans_idx_t valid_for)
{
    UiMenu_HandleIQAdjust(var, mode, options, clr_ptr, val_ptr, txrx_mode, MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE, valid_for);
}

void UiMenu_HandleIQAdjustPhase(int var, uint8_t mode, char* options, uint32_t* clr_ptr, volatile iq_balance_data_t* val_ptr, const uint16_t txrx_mode, iq_trans_idx_t valid_for)
{
    UiMenu_HandleIQAdjust(var, mode, options, clr_ptr, val_ptr, txrx_mode, MIN_IQ_PHASE_BALANCE, MAX_IQ_PHASE_BALANCE, valid_for);
}

const ColorNameValue MchfColor_Id2ValueName[SPEC_MAX_COLOUR] =
{
        { White,    "Whit"},
        { Grey,     "Grey"},
        { Blue,     "Blue"},
        { Red,      "Red1"},
        { Red2,     "Red2"},
        { Red3,     "Red2"},
        { Magenta,  "Mgnt"},
        { Green,    "Gren"},
        { Cyan,     "Cyan"},
        { Yellow,   "Yelw"},
        { Orange,   "Oran"},
        { Cream,    "Crea"},
        { Black,    "Blck"},
        { Grey1,    "Gry1"},
        { Grey2,    "Gry2"},
        { Grey3,    "Gry3"},
        { Grey4,    "Gry4"},
        { Grey5,    "Gry5"},
        { Grey6,    "Gry6"},
};



void __attribute__ ((noinline)) UiMenu_MapColors(uint32_t color ,char* options,volatile uint32_t* clr_ptr)
{
    const ColorNameValue* cnv;
    if (color < SPEC_MAX_COLOUR)
    {
        cnv = &MchfColor_Id2ValueName[color];
    }
    else
    {
     cnv = &MchfColor_Id2ValueName[SPEC_GREY];
    }
    if (options != NULL)
    {
        strcpy(options,cnv->name);
    }
    *clr_ptr = cnv->value;
}
void __attribute__ ((noinline)) UiDriverMenuMapStrings(char* output, uint32_t value ,const uint32_t string_max, const char** strings)
{
    strcpy(output,(value <= string_max)?strings[value]:"UNDEFINED");
}



static const char* display_types[] = {
     "No Display",
     "HY28A SPI",
     "HY28B SPI",
     "HY28A/B Para."
 };

/**
 * @returns: information for requested item as string. Do not write to this string.
 */
const char* UiMenu_GetSystemInfo(uint32_t* m_clr_ptr, int info_item)
{
    static char out[48];
    const char* outs = NULL;
    *m_clr_ptr = White;

    out[0] = 0;

    switch (info_item)
    {
    case INFO_DISPLAY:
    {
        outs = display_types[ts.display->display_type];
        break;
    }
    case INFO_DISPLAY_CTRL:
    {
        // const char* disp_com = ts.display_type==3?"parallel":"SPI";
        // snprintf(out,32,"ILI%04x %s",ts.DeviceCode,disp_com);
        snprintf(out,32,"ILI%04x",ts.display->DeviceCode);
        break;
    }
    case INFO_SI570:
    {
        if (Si570_IsPresent()) {
            float suf = Si570_GetStartupFrequency();
            int vorkomma = (int)(suf);
            int nachkomma = (int) roundf((suf-vorkomma)*10000);
            snprintf(out,32,"%xh / %u.%04u MHz",(Si570_GetI2CAddress() >> 1),vorkomma,nachkomma);
        }
        else
        {
            outs = "Not found!";
            *m_clr_ptr = Red;
        }
    }
    break;
    case INFO_TP:
        outs = (ts.tp->present == 0)?"n/a":"XPT2046";
        break;
    case INFO_RFMOD:
        outs = (ts.rfmod_present == 0)?"n/a":"present";
        break;
    case INFO_VHFUHFMOD:
        outs = (ts.vhfuhfmod_present == 0)?"n/a":"present";
        break;
    case INFO_FLASH:
            snprintf(out,32,"%d",(STM32_GetFlashSize()));
            break;
    case INFO_CPU:
            snprintf(out,32,"%xh",(STM32_GetSignature()));
            break;
    case INFO_RAM:
            snprintf(out,32,"%d",(ts.ramsize));
            break;
    case INFO_EEPROM:
    {
        const char* label = "";
        switch(ts.ser_eeprom_in_use)
         {
         case SER_EEPROM_IN_USE_I2C:
             label = " [used]";
             *m_clr_ptr = Green;
             break; // in use & ok
         case SER_EEPROM_IN_USE_ERROR: // not ok
             label = " [error]";
             *m_clr_ptr = Red;
             break;
         case SER_EEPROM_IN_USE_TOO_SMALL: // too small
             label = " [too small]";
             *m_clr_ptr = Red;
			 break;
		 default:
             label = " [not used]";
             *m_clr_ptr = Red;
         }

        const char* i2c_size_unit = "K";
        uint i2c_size = 0;
        
		if(ts.ser_eeprom_type >= 0 && ts.ser_eeprom_type < 20)
		  {
      	  i2c_size = SerialEEPROM_eepromTypeDescs[ts.ser_eeprom_type].size / 1024;
		  }

        // in case we have no or very small eeprom (less than 1K)
        if (i2c_size == 0)
        {
            i2c_size = SerialEEPROM_eepromTypeDescs[ts.ser_eeprom_type].size;
            i2c_size_unit = "B";
        }
        snprintf(out,48,"%s/%u%s%s",SerialEEPROM_eepromTypeDescs[ts.ser_eeprom_type].name, i2c_size, i2c_size_unit, label);
    }
    break;
    case INFO_BL_VERSION:
    {
        const uint8_t* begin = (uint8_t*)0x8000000;
        outs = "Unknown BL";

        // We search for string "Version: " in bootloader memory
        for(int i=0; i < 32768-8; i++)
        {
            if( begin[i] == 'V' && begin[i+1] == 'e' && begin[i+2] == 'r'
                    && begin[i+3] == 's' && begin[i+4] == 'i' && begin[i+5] == 'o'
                    && begin[i+6] == 'n' && begin[i+7] == ':' && begin[i+8] == ' ')
            {
                snprintf(out,32, "%s", &begin[i+9]);
                outs = out;
                break;
            }
            else if (begin[i] == 'M' && begin[i+1] == '0' && begin[i+2] == 'N'
                    && begin[i+3] == 'K' && begin[i+4] == 'A' && begin[i+5] == ' '
                    && begin[i+6] == '2' && begin[i+11] == 0xb5)
            {
                outs = "M0NKA 0.0.0.9";
                break;
            }
            else if (begin[i] == 'M' && begin[i+1] == '0' && begin[i+2] == 'N'
                    && begin[i+3] == 'K' && begin[i+4] == 'A' && begin[i+5] == ' '
                    && begin[i+6] == '2' && begin[i+11] == 0xd1)
            {
                outs = "M0NKA 0.0.0.14";
                break;
            }
        }
    }
    break;
    case INFO_FW_VERSION:
    {
        snprintf(out,32, "%s", TRX4M_VERSION+4);
    }
    break;
    case INFO_BUILD:
    {
        snprintf(out,32, "%s", TRX4M_BUILD_DAT+4);
    }
    break;
    case INFO_RTC:
    {
        snprintf(out,32, "%s", ts.rtc_present?"Yes":"N/A");
    }
    break;
    case INFO_VBAT:
    {
        snprintf(out,32, "%s", ts.vbat_present?"Yes":"N/A");
    }
    break;
    case INFO_LICENCE:
    {
        snprintf(out,32, "%s", TRX4M_LICENCE);
    }
    break;
    default:
        outs = "NO INFO";
    }
    if (outs == NULL) {
        outs = out;
    }
    return outs;
}



bool __attribute__ ((noinline)) UiDriverMenuBandPowerAdjust(int var, uint8_t mode, uint8_t band_mode, uint8_t pa_level, char* options, uint32_t* clr_ptr)
{
    volatile uint8_t* adj_ptr;
    adj_ptr = &ts.pwr_adj[pa_level == PA_LEVEL_FULL?ADJ_FULL_POWER:ADJ_5W][band_mode];

    bool tchange = false;
    if((band_mode == RadioManagement_GetBand(df.tune_old/TUNE_MULT)) && (ts.power_level == pa_level))
    {
        tchange = UiDriverMenuItemChangeUInt8(var, mode, adj_ptr,
                                              TX_POWER_FACTOR_MIN,
                                              RadioManagement_IsPowerFactorReduce(df.tune_old/TUNE_MULT)?TX_POWER_FACTOR_MAX:TX_POWER_FACTOR_MAX/4,
                                              TX_POWER_FACTOR_MIN,
                                              1
                                             );

        if(tchange)	 		// did something change?
        {
            RadioManagement_SetBandPowerFactor(ts.band);	// yes, update the power factor
            if(!ts.iq_freq_mode)	// Is translate mode *NOT* active?
            {
                Codec_TxSidetoneSetgain(ts.txrx_mode);				// adjust the sidetone gain
            }
        }
    }
    else	// not enabled
        *clr_ptr = Orange;
    //
    sprintf(options, "  %u", *adj_ptr);
    return tchange;
}

bool __attribute__ ((noinline))  UiDriverMenuBandRevCouplingAdjust(int var, uint8_t mode, uint8_t filter_band, char* options, uint32_t* clr_ptr)
{
    bool tchange = false;
    volatile uint8_t *adj_ptr = &swrm.coupling_calc[filter_band];
    if(ts.filter_band == filter_band)	 	// is this band selected?
    {
        tchange = UiDriverMenuItemChangeUInt8(var, mode, adj_ptr,
                                              SWR_COUPLING_MIN,
                                              SWR_COUPLING_MAX,
                                              SWR_COUPLING_DEFAULT,
                                              1
                                             );
    }
    if((ts.txrx_mode != TRX_MODE_TX) || (ts.filter_band != filter_band))	// Orange if not in TX mode or NOT on this band
        *clr_ptr = Orange;
    sprintf(options, "  %u", *adj_ptr);
    return tchange;
}


//
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverUpdateMenuLines
//* Object              :
//* Input Parameters    : index:  Line to display  mode:  ,
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//

void UiMenu_UpdateItem(uint16_t select, uint16_t mode, int pos, int var, char* options, const char** txt_ptr_ptr, uint32_t* clr_ptr)
{

    const char* txt_ptr = NULL;
    uint32_t clr = *clr_ptr;

    // case statement local variables defined for convenience here
    bool var_change = false;
    uint8_t temp_var_u8; // used to temporarily represent some configuration values as uint8_t value


    if(mode == MENU_PROCESS_VALUE_CHANGE)
    {
        if(select == CONFIG_XVTR_FREQUENCY_OFFSET)  // signal if we are in XVTR FREQUENCY OFFSET adjust mode for alternate frequency steps
            ts.xvtr_adjust_flag = 1;
        else                                // NOT in transverter mode
        {
            if(ts.xvtr_adjust_flag)         // had transverter frequency mode been active?
            {
                ts.xvtr_adjust_flag = 0;    // yes - disable flag
                UiDriver_ChangeTuningStep(0);   // force to valid frequency step size for normal tuning
                UiDriver_ChangeTuningStep(1);
            }
        }
    }

    switch(select)          //  DSP_NR_STRENGTH_MAX
    {
    case MENU_DSP_NR_STRENGTH:  // DSP Noise reduction strength

        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp_nr_strength,
                                              0,
                                              DSP_NR_STRENGTH_MAX,
                                              DSP_NR_STRENGTH_DEFAULT,
                                              1
                                             );
        if(var_change)
        {
            // did it change?
            if(ts.dsp_active & DSP_NR_ENABLE)   // only change if DSP active
            {
                AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
            }
        }
        //
        if(!(ts.dsp_active & DSP_NR_ENABLE))    // make red if DSP not active
        {
            clr = Orange;
        }
        else
        {
            if(ts.dsp_nr_strength >= DSP_STRENGTH_RED)
                clr = Red;
            else if(ts.dsp_nr_strength >= DSP_STRENGTH_ORANGE)
                clr = Orange;
            else if(ts.dsp_nr_strength >= DSP_STRENGTH_YELLOW)
                clr = Yellow;
        }
        //
        snprintf(options,32, "  %u", ts.dsp_nr_strength);
        break;
    case MENU_AM_DISABLE: // AM mode enable/disable
        UiMenu_HandleDemodModeDisable(var, mode, options, &clr, DEMOD_AM_DISABLE);
        break;
    case MENU_DIGI_DISABLE: // AM mode enable/disable
        UiMenu_HandleDemodModeDisable(var, mode, options, &clr, DEMOD_DIGI_DISABLE);
        break;
    case MENU_CW_DISABLE: // AM mode enable/disable
        UiMenu_HandleDemodModeDisable(var, mode, options, &clr, DEMOD_CW_DISABLE);
        break;
    case MENU_DEMOD_SAM:    // Enable demodulation mode SAM
        temp_var_u8 = (ts.flags1 & FLAGS1_SAM_ENABLE)? 1 : 0;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if (temp_var_u8)
                ts.flags1 |= FLAGS1_SAM_ENABLE;
            else
                ts.flags1 &= ~FLAGS1_SAM_ENABLE;
        }
        break;
    case MENU_SSB_AUTO_MODE_SELECT:     // Enable/Disable auto LSB/USB select
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.lsb_usb_auto_select,
                                              0,
                                              AUTO_LSB_USB_MAX,
                                              AUTO_LSB_USB_OFF,
                                              1
                                             );
        switch(ts.lsb_usb_auto_select)
        {
        case AUTO_LSB_USB_ON:       // LSB on bands < 10 MHz
            txt_ptr = "     ON";        // yes
            break;
        case AUTO_LSB_USB_60M:  // USB on 60 meters?
            txt_ptr = "USB 60M";        // yes
            break;
        default:
        txt_ptr = "    OFF";        // no (obviously!)
        }
        break;
    case MENU_FM_MODE_ENABLE:   // Enable/Disable FM
        if(ts.iq_freq_mode)
        {
            temp_var_u8 = ts.flags2 & FLAGS2_FM_MODE_ENABLE;
            var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
            if(var_change)
            {
                if(temp_var_u8) // band up/down swap is to be enabled
                    ts.flags2 |= FLAGS2_FM_MODE_ENABLE;     // FM is enabled
                else            // band up/down swap is to be disabled
                    ts.flags2 &= ~FLAGS2_FM_MODE_ENABLE;        // FM is disabled
            }

        }
        else        // translate mode is off - NO FM!!!
        {
            strcpy(options, "OFF");     // Say that it is OFF!
            clr = Red;
        }
        break;
    case MENU_FM_GEN_SUBAUDIBLE_TONE:   // Selection of subaudible tone for FM transmission
        UiDriverMenuItemChangeUInt32(var, mode, &ts.fm_subaudible_tone_gen_select,
                                     0,
                                     NUM_SUBAUDIBLE_TONES,
                                     FM_SUBAUDIBLE_TONE_OFF,
                                     1
                                    );

        if(ts.fm_subaudible_tone_gen_select)        // tone select not zero (tone activated
        {
            int a,b;
            AudioManagement_CalcSubaudibleGenFreq();        // calculate frequency word
            a = (int)(ads.fm_subaudible_tone_gen_freq * 10);        // convert to integer, Hz*10
            b = a;
            a /= 10;        // remove 10ths of Hz
            a *= 10;        // "a" now has Hz*100 with 10ths removed
            b -= a;         // "b" now has 10ths of Hz
            a /= 10;        // "a" is back to units of Hz
            snprintf(options,32, "  %d.%dHz", a, b);
        }
        else                                // tone is off
        {
            snprintf(options,32, "     OFF");       // make it dislay "off"
            ads.fm_subaudible_tone_word = 0;    // set word to 0 to turn it off
        }
        //
        if(ts.dmod_mode != DEMOD_FM)    // make orange if we are NOT in FM mode
            clr = Orange;
        else if(ads.fm_subaudible_tone_det_freq > 200)      // yellow for tones above 200 Hz as they are more audible
            clr = Yellow;
        break;
    //
    case MENU_FM_DET_SUBAUDIBLE_TONE:   // Selection of subaudible tone for FM reception
        UiDriverMenuItemChangeUInt32(var, mode, &ts.fm_subaudible_tone_det_select,
                                     0,
                                     NUM_SUBAUDIBLE_TONES,
                                     FM_SUBAUDIBLE_TONE_OFF,
                                     1
                                    );
        //
        if(ts.fm_subaudible_tone_det_select)        // tone select not zero (tone activated
        {
            int a,b;
            AudioManagement_CalcSubaudibleDetFreq();        // calculate frequency word
            a = (int)(ads.fm_subaudible_tone_det_freq * 10);        // convert to integer, Hz*10
            b = a;
            a /= 10;        // remove 10ths of Hz
            a *= 10;        // "a" now has Hz*100 with 10ths removed
            b -= a;         // "b" now has 10ths of Hz
            a /= 10;        // "a" is back to units of Hz
            snprintf(options,32, "  %d.%dHz", a, b);
        }
        else                                // tone is off
        {
            snprintf(options,32, "     OFF");       // make it dislay "off"
            ads.fm_subaudible_tone_word = 0;    // set word to 0 to turn it off
        }
        //
        if(ts.dmod_mode != DEMOD_FM)    // make orange if we are NOT in FM
            clr = Orange;
        else if(ads.fm_subaudible_tone_det_freq > 200)      // yellow for tones above 200 Hz as they are more audible
            clr = Yellow;
        break;
    //
    case MENU_FM_TONE_BURST_MODE:
        UiDriverMenuItemChangeUInt8(var, mode, &ts.fm_tone_burst_mode,
                                    0,
                                    FM_TONE_BURST_MAX,
                                    FM_TONE_BURST_OFF,
                                    1
                                   );
        switch(ts.fm_tone_burst_mode) {
        case FM_TONE_BURST_1750_MODE:           // if it was 1750 Hz mode, load parameters
            ads.fm_tone_burst_active = 0;                               // make sure it is turned off
            txt_ptr = "1750 Hz";
            ads.fm_tone_burst_word = FM_TONE_BURST_1750;
            break;
        case FM_TONE_BURST_2135_MODE:       // if it was 2135 Hz mode, load information
            ads.fm_tone_burst_active = 0;                               // make sure it is turned off
            txt_ptr = "2135 Hz";
            ads.fm_tone_burst_word = FM_TONE_BURST_2135;
            break;
        default:                                                    // anything else, turn it off
            txt_ptr = "    OFF";
            ads.fm_tone_burst_word = FM_TONE_BURST_OFF;
            ads.fm_tone_burst_active = 0;
        }

        if(ts.dmod_mode != DEMOD_FM)    // make orange if we are NOT in FM
        {
            clr = Orange;
        }
        break;
    //
    /*  case MENU_FM_RX_BANDWIDTH:
            fchange = UiDriverMenuItemChangeUInt8(var, mode, &ts.fm_rx_bandwidth,
                            0,
                            FM_RX_BANDWIDTH_MAX,
                            FM_BANDWIDTH_DEFAULT,
                            1
                            );
                    //
            if(ts.fm_rx_bandwidth == FM_RX_BANDWIDTH_7K2)   {       // if it is 7.2 kHz FM RX bandwidth
                strcpy(options, "7.5kHz");
            }
            else if(ts.fm_rx_bandwidth == FM_RX_BANDWIDTH_12K)  {   // if it was 12 kHz bandwidth
                strcpy(options, "12 kHz");
            }
    //      else if(ts.fm_rx_bandwidth == FM_RX_BANDWIDTH_15K)  {   // if it was 15 kHz bandwidth
    //          strcpy(options, "15 kHz");
    //      }
            else    {                <       // it was anything else (10 kHz - hope!)
                strcpy(options, "10 kHz");
            }
            //
            if(fchange) {           // was the bandwidth changed?
                AudioFilter_InitRxHilbertFIR();
    //          AudioFilter_CalcRxPhaseAdj();           // yes - update the filters!
                UiDriverChangeFilterDisplay();  // update display of filter bandwidth (numerical) on screen only
            }
            //
            if(ts.dmod_mode != DEMOD_FM)    // make orange if we are NOT in FM
                clr = Orange;
            break;
*/  //
    case MENU_FM_DEV_MODE:  // Select +/- 2.5 or 5 kHz deviation on RX and TX
        if(ts.iq_freq_mode)
        {
            temp_var_u8 = ts.flags2 & FLAGS2_FM_MODE_DEVIATION_5KHZ;
            var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
            if(var_change)
            {
                if(temp_var_u8) // band up/down swap is to be enabled
                    ts.flags2 |= FLAGS2_FM_MODE_DEVIATION_5KHZ;     // set 5 kHz mode
                else            // band up/down swap is to be disabled
                    ts.flags2 &= ~FLAGS2_FM_MODE_DEVIATION_5KHZ;        // set 2.5 kHz mode
            }

            if(ts.flags2 & FLAGS2_FM_MODE_DEVIATION_5KHZ)               // Check state of bit indication 2.5/5 kHz
            {
                txt_ptr = "+-5k (Wide)";        // Bit is set - 5 kHz
            }
            else
            {
                txt_ptr = "+-2k5 (Nar)";        // Not set - 2.5 kHz
            }
        }
        else        // translate mode is off - NO FM!!!
        {
            txt_ptr = "  OFF";      // Say that it is OFF!
            clr = Red;
        }
        break;
    case MENU_AGC_MODE: // AGC mode
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.agc_mode,
                                              0,
                                              AGC_MAX_MODE,
                                              AGC_DEFAULT,
                                              1
                                             );
        switch(ts.agc_mode) {
        case AGC_SLOW:
            txt_ptr = " SLOW";
            break;
        case AGC_MED:
            txt_ptr = "  MED";
            break;
        case AGC_FAST:
            txt_ptr = "  FAST";
            break;
        case AGC_OFF:
            txt_ptr = "MANUAL";
            clr = Red;
            break;
        case AGC_CUSTOM:
            txt_ptr = "CUSTOM";
            break;
        }

        if(var_change)
        {
            // now set the AGC
            AudioManagement_CalcAGCDecay(); // initialize AGC decay ("hang time") values
        }
        if(ts.txrx_mode == TRX_MODE_TX) // Orange if in TX mode
        {
            clr = Orange;
        }
        break;

    case MENU_RF_GAIN_ADJ:      // RF gain control adjust
        var_change = UiDriverMenuItemChangeInt(var, mode, &ts.rf_gain,
                                            0,
                                            MAX_RF_GAIN,
                                            DEFAULT_RF_GAIN,
                                            1
                                           );
        if(var_change)
        {
            AudioManagement_CalcRFGain();
        }

        if(ts.rf_gain < 20)
        {
            clr = Red;
        }
        else if(ts.rf_gain < 30)
        {
            clr = Orange;
        }
        else if(ts.rf_gain < 40)
        {
            clr = Yellow;
        }
        else
        {
            clr = White;
        }

        if(var_change)      // did RFGain get changed?
        {
            UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
        }
        snprintf(options, 32, "  %d", ts.rf_gain);
        break;

    case MENU_AGC_WDSP_MODE: // AGC mode
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.agc_wdsp_mode,
                                              0, //
                                              5,
                                              2,
                                              1
                                             );
        switch(ts.agc_wdsp_mode) {
        case 0:
            txt_ptr = "very LONG";
            break;
        case 1:
            txt_ptr = "     LONG";
            break;
        case 2:
            txt_ptr = "     SLOW";
            break;
        case 3:
            txt_ptr = "      MED";
            break;
        case 4:
            txt_ptr = "      FAST";
            break;
        case 5:
            txt_ptr = "     OFF ";
            clr = Red;
            break;
        }

        if(var_change)
        {
            // now set the AGC parameters
            ts.agc_wdsp_switch_mode = 1; // set flag to 1 for parameter change
            AudioDriver_SetupAGC();
            UiMenu_RenderMenu(MENU_RENDER_ONLY);
        }
        if(ts.txrx_mode == TRX_MODE_TX) // Orange if in TX mode
        {
            clr = Orange;
        }
        break;

    case MENU_AGC_WDSP_SLOPE:      //
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.agc_wdsp_slope,
                                            0,
                                            200,
                                            40,
                                            10
                                           );
        if(var_change)
        {
            AudioDriver_SetupAGC();
        }
        snprintf(options, 32, "  %ddB", ts.agc_wdsp_slope / 10);
        break;

    case MENU_AGC_WDSP_THRESH:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &ts.agc_wdsp_thresh,
                                            -20,
                                            120,
                                            40,
                                            1
                                           );
        if(var_change)
        {
            AudioDriver_SetupAGC();
        }
        snprintf(options, 32, "  %ddB", ts.agc_wdsp_thresh);
        break;

    case MENU_AGC_WDSP_HANG_THRESH:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &ts.agc_wdsp_hang_thresh,
                                            -20,
                                            120,
                                            40,
                                            1
                                           );
        if(var_change)
        {
            AudioDriver_SetupAGC();
        }
        snprintf(options, 32, "  %ddB", ts.agc_wdsp_hang_thresh);
        break;

    case MENU_AGC_WDSP_TAU_DECAY:      //
       var_change = UiDriverMenuItemChangeInt(var, mode, &ts.agc_wdsp_tau_decay[ts.agc_wdsp_mode],
                                           100,
                                           5000,
                                           1000,
                                           100
                                          );
       if(var_change)
       {
           AudioDriver_SetupAGC();
       }
       snprintf(options, 32, "  %ums", ts.agc_wdsp_tau_decay[ts.agc_wdsp_mode]);
       break;

    case MENU_AGC_WDSP_TAU_HANG_DECAY:      //
       var_change = UiDriverMenuItemChangeInt(var, mode, &ts.agc_wdsp_tau_hang_decay,
                                           100,
                                           5000,
                                           1000,
                                           100
                                          );
       if(var_change)
       {
           AudioDriver_SetupAGC();
       }
       snprintf(options, 32, "  %ums", ts.agc_wdsp_tau_hang_decay);
       break;

     case MENU_DBM_CALIBRATE:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &ts.dbm_constant,
                                            -100,
                                            100,
                                            0,
                                            1
                                           );
        snprintf(options, 32, "  %ddB", ts.dbm_constant);
        break;

    case MENU_AGC_WDSP_HANG_TIME:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &ts.agc_wdsp_hang_time,
                                            10,
                                            5000,
                                            250,
                                            10
                                           );
        if(var_change)
        {
            AudioDriver_SetupAGC();
        }
        snprintf(options, 32, "  %dms", ts.agc_wdsp_hang_time);
        break;

        case MENU_AGC_WDSP_SWITCH:     // Enable/Disable wdsp AGC
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.agc_wdsp,
                                                  0,
                                                  1,
                                                  0,
                                                  1
                                                 );
            switch(ts.agc_wdsp)
            {
            case 1:       //
                txt_ptr = "    WDSP AGC";        //
                if(ts.s_meter == 0) // old school S-Meter does not work with WDSP AGC, so we switch to dBm S-Meter in that case!
                    {
                        ts.s_meter = 1;
                    }
                break;
            default:
                txt_ptr = "Standard AGC";        //
            }
            break;

            case MENU_AGC_WDSP_HANG_ENABLE:     //
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.agc_wdsp_hang_enable,
                                                  0,
                                                  1,
                                                  0,
                                                  1
                                                 );
            switch(ts.agc_wdsp_hang_enable)
            {
            case 1:       //
                txt_ptr = "  ON";        //
                break;
            case 0:
            txt_ptr = " OFF";        //
            }
            break;

            case MENU_SAM_PLL_LOCKING_RANGE:      //
                var_change = UiDriverMenuItemChangeInt(var, mode, &ads.pll_fmax_int,
                                                    50,
                                                    8000,
                                                    500,
                                                    10
                                                   );
                if(var_change)
                {
                    AudioDriver_SetSamPllParameters();
                }
                snprintf(options, 32, "  %d", ads.pll_fmax_int);
                break;

            case MENU_SAM_PLL_STEP_RESPONSE:      //
                var_change = UiDriverMenuItemChangeInt(var, mode, &ads.zeta_int,
                                                    1,
                                                    100,
                                                    65,
                                                    1
                                                   );
                if(var_change)
                {
                    AudioDriver_SetSamPllParameters();


                }
                snprintf(options, 32, "  %d", ads.zeta_int);
                break;

            case MENU_SAM_PLL_BANDWIDTH:      //
                var_change = UiDriverMenuItemChangeInt(var, mode, &ads.omegaN_int,
                                                    25,
                                                    1000,
                                                    250,
                                                    5
                                                   );
                if(var_change)
                {
                    AudioDriver_SetSamPllParameters();

                }
                snprintf(options, 32, "  %d", ads.omegaN_int);
                break;

            case MENU_SAM_FADE_LEVELER:     // Enable/Disable fade leveler for SAM
                var_change = UiDriverMenuItemChangeUInt8(var, mode, &ads.fade_leveler,
                                                      0,
                                                      1,
                                                      0,
                                                      1
                                                     );
                switch(ads.fade_leveler)
                {
                case 1:       //
                    txt_ptr = "     ON";        //
                    break;
                default:
                txt_ptr = "    OFF";        //
                }
                break;

            /*    case MENU_SAM_SIDEBAND:  //
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ads.sam_sideband,
                                              0,
                                              2,
                                              0,
                                              1
                                             );
        if(var_change)
        {
            UiDriver_ShowMode();
        }
        //
        if(ads.sam_sideband == 1 || ads.sam_sideband == 2)
        {
            clr = Red;
        }

        switch(ads.sam_sideband)
        {
        case 0:
            txt_ptr = "BOTH";
            break;
        case 1:
            txt_ptr = " LSB";
            break;
        case 2:
            txt_ptr = " USB";
            break;
        }
        break;

    case CONFIG_SAM_PLL_TAUR:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &ads.tauR_int,
                                            1,
                                            1000,
                                            20,
                                            1
                                           );
        if(var_change)
        {
            set_SAM_PLL_parameters();

        }
        snprintf(options, 32, "  %d", ads.tauR_int);
        break;

    case CONFIG_SAM_PLL_TAUI:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &ads.tauI_int,
                                            1,
                                            1000,
                                            140,
                                            1
                                           );
        if(var_change)
        {
        }
        snprintf(options, 32, "  %d", ads.tauI_int);
        break;
*/

        // RX Codec gain adjust
    case MENU_CUSTOM_AGC:       // Custom AGC adjust
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.agc_custom_decay,
                                              0,
                                              AGC_CUSTOM_MAX,
                                              AGC_CUSTOM_DEFAULT,
                                              1
                                             );
        if(var_change)
        {
            if(ts.agc_custom_decay > AGC_CUSTOM_MAX)
                ts.agc_custom_decay = AGC_CUSTOM_MAX;
            // now set the custom AGC - if in custom mode
            if(ts.agc_mode == AGC_CUSTOM)
            {
                float tcalc;
                tcalc = (float)ts.agc_custom_decay; // use temp var "tcalc" as audio function
                tcalc += 30;            // can be called mid-calculation!
                tcalc /= 10;
                tcalc *= -1;
                tcalc = powf(10, tcalc);
                ads.agc_decay = tcalc;
            }
        }

        if((ts.txrx_mode == TRX_MODE_TX) || (ts.agc_mode != AGC_CUSTOM))    // Orange if in TX mode
        {
            clr = Orange;
        }
        else if(ts.agc_custom_decay <= AGC_CUSTOM_FAST_WARNING)             // Display in red if setting may be too fast
        {
            clr = Red;
        }
        snprintf(options,32, "  %d", ts.agc_custom_decay);
        break;
    // A/D Codec Gain/Mode setting/adjust
    case MENU_CODEC_GAIN_MODE:
        UiDriverMenuItemChangeUInt8(var, mode, &ts.rf_codec_gain,
                                    0,
                                    MAX_RF_CODEC_GAIN_VAL,
                                    DEFAULT_RF_CODEC_GAIN_VAL,
                                    1
                                   );

        if(ts.rf_codec_gain == 9)
            strcpy(options, " AUTO");
        else        // if anything other than "Auto" give a warning in RED
        {
            snprintf(options,32,"> %u <", ts.rf_codec_gain);
            clr = Red;
        }
        break;
    case MENU_NOISE_BLANKER_SETTING:
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.nb_setting,
                                              0,
                                              MAX_NB_SETTING,
                                              0,
                                              1
                                             );

        if(ts.nb_setting >= NB_WARNING3_SETTING)
        {
            clr = Red;      // above this value, make it red
        }
        else if(ts.nb_setting >= NB_WARNING2_SETTING)
        {
            clr = Orange;       // above this value, make it orange
        }
        else if(ts.nb_setting >= NB_WARNING1_SETTING)
        {
            clr = Yellow;       // above this value, make it yellow
        }
        snprintf(options,32,"   %u", ts.nb_setting);

        break;
    case MENU_RX_FREQ_CONV:     // Enable/Disable receive frequency conversion
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.iq_freq_mode,
                                              0,
                                              FREQ_IQ_CONV_MODE_MAX,
                                              FREQ_IQ_CONV_MODE_DEFAULT,
                                              1
                                             );
        switch(ts.iq_freq_mode)
        {
        case FREQ_IQ_CONV_MODE_OFF:
            txt_ptr = ">> OFF! <<";
            clr = Red3;
            break;
        case FREQ_IQ_CONV_P6KHZ:
            txt_ptr ="RX  +6kHz";
            break;
        case FREQ_IQ_CONV_M6KHZ:
            txt_ptr = "RX  -6kHz";
            break;
        case FREQ_IQ_CONV_P12KHZ:
            txt_ptr = "RX +12kHz";
            break;
        case FREQ_IQ_CONV_M12KHZ:
            txt_ptr = "RX -12kHz";
            break;
        }
        if(var_change)      // update parameters if changed
        {
            UiDriver_FrequencyUpdateLOandDisplay(true); // update frequency display without checking encoder, unconditionally updating synthesizer
        }
        break;
    case MENU_MIC_LINE_MODE:    // Mic/Line mode
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.tx_audio_source,
                                              0,
                                              TX_AUDIO_MAX_ITEMS,
                                              TX_AUDIO_MIC,
                                              1
                                             );
        switch(ts.tx_audio_source)
        {
        case TX_AUDIO_MIC:
            txt_ptr = "    MIC";
            break;
        case TX_AUDIO_LINEIN_L:
            txt_ptr = " LINE-L";
            break;
        case TX_AUDIO_LINEIN_R:
            txt_ptr = " LINE-R";
            break;
        case TX_AUDIO_DIG:
            txt_ptr = " DIGITAL";
            break;
        case TX_AUDIO_DIGIQ:
            txt_ptr = " DIG I/Q";
            break;
        }
        if(var_change)          // if there was a change, do update of on-screen information
        {
            if(ts.dmod_mode != DEMOD_CW)
            {
                UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
            }
        }

        if((!(ts.flags1 & FLAGS1_CAT_MODE_ACTIVE)) && (ts.tx_audio_source == TX_AUDIO_DIG || ts.tx_audio_source == TX_AUDIO_DIGIQ) )
        {
            // RED if CAT is not enabled and  digital input is selected
            clr = Red;
        }
        break;
    case MENU_MIC_GAIN: // Mic Gain setting

        if(ts.tx_audio_source == TX_AUDIO_MIC)      // Allow adjustment only if in MIC mode
        {
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.tx_gain[TX_AUDIO_MIC],
                                                  MIC_GAIN_MIN,
                                                  MIC_GAIN_MAX,
                                                  MIC_GAIN_DEFAULT,
                                                  1
                                                 );
        }
        if(var_change)
        {
            Codec_SwitchMicTxRxMode(ts.txrx_mode);

            if(ts.dmod_mode != DEMOD_CW)
            {
                UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
            }
        }

        if(ts.tx_audio_source != TX_AUDIO_MIC)      // Orange if not in MIC-IN mode
        {
            clr = Orange;
        }

        snprintf(options,32, "   %u", ts.tx_gain[TX_AUDIO_MIC]);
        break;
    case MENU_LINE_GAIN:    // Line Gain setting

        // TODO: Revise, since it now changes the currently selected tx source setting
        if(ts.tx_audio_source != TX_AUDIO_MIC)      // Allow adjustment only if in line-in mode
        {
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.tx_gain[ts.tx_audio_source],
                                                  LINE_GAIN_MIN,
                                                  LINE_GAIN_MAX,
                                                  LINE_GAIN_DEFAULT,
                                                  1
                                                 );

        }

        if(var_change)          // update on-screen info and codec if there was a change
        {
            if(ts.dmod_mode != DEMOD_CW)
            {
                UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
                if(ts.txrx_mode == TRX_MODE_TX)     // in transmit mode?
                {
                    // TODO: Think about this, this is a hack
                    Codec_LineInGainAdj(ts.tx_gain[TX_AUDIO_LINEIN_L]);     // change codec gain
                }
            }
        }

        if(ts.tx_audio_source == TX_AUDIO_MIC)  // Orange if in MIC mode
        {
            clr = Orange;
            snprintf(options,32, "  %u", ts.tx_gain[TX_AUDIO_LINEIN_L]);
        }
        else
        {
            snprintf(options,32, "  %u", ts.tx_gain[ts.tx_audio_source]);
        }
        break;
    case MENU_ALC_RELEASE:      // ALC Release adjust

        if(ts.tx_comp_level == TX_AUDIO_COMPRESSION_MAX)
        {
            var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.alc_decay_var,
                                                   0,
                                                   ALC_DECAY_MAX,
                                                   ALC_DECAY_DEFAULT,
                                                   1
                                                  );
            if(var_change)          // value changed?  Recalculate
            {
                AudioManagement_CalcALCDecay();
            }
        }
        else            // indicate RED if "Compression Level" below was nonzero
        {
            clr = Red;
            ts.alc_decay_var = 10;
        }

        if(ts.tx_comp_level == TX_AUDIO_COMPRESSION_SV) // in "selectable value" mode?
        {
            ts.alc_decay = ts.alc_decay_var;    // yes, save new value
        }
        snprintf(options,32, "  %d", (int)ts.alc_decay_var);
        break;
    case MENU_ALC_POSTFILT_GAIN:        // ALC TX Post-filter gain (Compressor level)
        if(ts.tx_comp_level == TX_AUDIO_COMPRESSION_MAX)
        {
            var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.alc_tx_postfilt_gain_var,
                                                   ALC_POSTFILT_GAIN_MIN,
                                                   ALC_POSTFILT_GAIN_MAX,
                                                   ALC_POSTFILT_GAIN_DEFAULT,
                                                   1
                                                  );

            if(var_change)
            {
                if(ts.dmod_mode != DEMOD_CW)    // In voice mode?
                {
                    UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
                }
            }
        }
        else            // indicate RED if "Compression Level" below was nonzero
        {
            clr = Red;
        }

        if(ts.tx_comp_level == TX_AUDIO_COMPRESSION_SV) // in "selectable value" mode?
        {
            ts.alc_tx_postfilt_gain = ts.alc_tx_postfilt_gain_var;  // yes, save new value
        }

        snprintf(options,32, "  %d", (int)ts.alc_tx_postfilt_gain_var);
        break;
    case MENU_TX_COMPRESSION_LEVEL:     // ALC TX Post-filter gain (Compressor level)
        var_change = UiDriverMenuItemChangeInt16(var, mode, &ts.tx_comp_level,
                                              -1,
                                              TX_AUDIO_COMPRESSION_MAX,
                                              TX_AUDIO_COMPRESSION_DEFAULT,
                                              1
                                             );

        if(var_change)
        {
            AudioManagement_CalcTxCompLevel();          // calculate parameters for selected amount of compression

            if(ts.dmod_mode != DEMOD_CW)    // In voice mode?
            {
                UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
            }
        }

        if(ts.tx_comp_level < TX_AUDIO_COMPRESSION_SV && 0 <= ts.tx_comp_level)  //  display numbers for all but the highest value
        {
            snprintf(options,32,"    %d",ts.tx_comp_level);
        }
        else                    // show "CUSTOM" (Stored Value) for highest value
        {
            if (ts.tx_comp_level == TX_AUDIO_COMPRESSION_MIN)
            {
                txt_ptr = "OFF";
            }
            else
            {
                txt_ptr = "CUS";
            }
        }
        break;
    case MENU_KEYER_MODE:   // Keyer mode
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.keyer_mode,
                                              0,
                                              CW_MODE_ULTIMATE,
                                              CW_MODE_IAM_B,
                                              1
                                             );

        switch(ts.keyer_mode)
        {
        case CW_MODE_IAM_B:
            txt_ptr = "IAM_B";
            break;
        case CW_MODE_IAM_A:
            txt_ptr = "IAM_A";
            break;
        case CW_MODE_STRAIGHT:
            txt_ptr = "STR_K";
            break;
        case CW_MODE_ULTIMATE:
            txt_ptr = "ULTIM";
            break;
        }
        break;

    case MENU_KEYER_SPEED:  // keyer speed
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.keyer_speed,
                                              MIN_KEYER_SPEED,
                                              MAX_KEYER_SPEED,
                                              DEFAULT_KEYER_SPEED,
                                              1
                                             );

        if(var_change && ts.dmod_mode == DEMOD_CW)         // did it change?
        {
            CwGen_SetSpeed(); // make sure keyerspeed is being used
            UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
        }
        snprintf(options,32, "  %u", ts.keyer_speed);
        break;
    case MENU_SIDETONE_GAIN:    // sidetone gain
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.st_gain,
                                              0,
                                              SIDETONE_MAX_GAIN,
                                              DEFAULT_SIDETONE_GAIN,
                                              1
                                             );
        if(var_change && ts.dmod_mode == DEMOD_CW)          // did it change?
        {
            UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
        }
        snprintf(options,32, "  %u", ts.st_gain);
        break;
    case MENU_SIDETONE_FREQUENCY:   // sidetone frequency
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.sidetone_freq,
                                               CW_SIDETONE_FREQ_MIN,
                                               CW_SIDETONE_FREQ_MAX*10,
                                               CW_SIDETONE_FREQ_DEFAULT,
                                               10
                                              );

        if(var_change && ts.dmod_mode == DEMOD_CW)         // did it change?
        {
            float freq[2] = { ts.sidetone_freq, 0.0 };

            softdds_setfreq_dbl(freq,ts.samp_rate,0);
            UiDriver_FrequencyUpdateLOandDisplay(false);
        }
        snprintf(options,32, "  %uHz", (uint)ts.sidetone_freq);
        break;

    case MENU_PADDLE_REVERSE:   // CW Paddle reverse
        UiDriverMenuItemChangeEnableOnOff(var, mode, &ts.paddle_reverse,0,options,&clr);
        break;
    case MENU_CW_TX_RX_DELAY:   // CW TX->RX delay
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.cw_rx_delay,
                                              0,
                                              CW_RX_DELAY_MAX,
                                              CW_RX_DELAY_DEFAULT,
                                              1
                                             );

        snprintf(options,32, "%3ums", ts.cw_rx_delay*10);
        break;

    case MENU_CW_AUTO_MODE_SELECT:     // Enable/Disable auto LSB/USB select
    {
        const cw_mode_map_entry_t* curr_mode = RadioManagement_CWConfigValueToModeEntry(ts.cw_offset_mode);
        temp_var_u8 = curr_mode->sideband_mode;

                var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                        0,
                        2,
                        2,
                        1
                );

        if(var_change)      // update parameters if changed
                {
            cw_mode_map_entry_t new_mode;
            new_mode.dial_mode = curr_mode->dial_mode;
            new_mode.sideband_mode = temp_var_u8;
            ts.cw_offset_mode = RadioManagement_CWModeEntryToConfigValue(&new_mode);

            ts.cw_lsb = RadioManagement_CalculateCWSidebandMode();
            UiDriver_ShowMode();
            UiDriver_FrequencyUpdateLOandDisplay(true); // update frequency display and local oscillator
                }

        switch(temp_var_u8)
        {
        case CW_SB_LSB:
            txt_ptr = " LSB";
            break;
        case CW_SB_USB:
            txt_ptr = " USB";
            break;
        case CW_SB_AUTO:
            txt_ptr = "AUTO";
            break;
        }
        break;
    }
    case MENU_CW_OFFSET_MODE:   // CW offset mode (e.g. USB, LSB, etc.)
    {
        const cw_mode_map_entry_t* curr_mode = RadioManagement_CWConfigValueToModeEntry(ts.cw_offset_mode);
          temp_var_u8 = curr_mode->dial_mode;

                  var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                          0,
                          2,
                          2,
                          1
                  );

          if(var_change)      // update parameters if changed
                  {
              cw_mode_map_entry_t new_mode;
              new_mode.sideband_mode = curr_mode->sideband_mode;
              new_mode.dial_mode = temp_var_u8;
              ts.cw_offset_mode = RadioManagement_CWModeEntryToConfigValue(&new_mode);

              ts.cw_lsb = RadioManagement_CalculateCWSidebandMode();
              UiDriver_ShowMode();
              UiDriver_FrequencyUpdateLOandDisplay(true); // update frequency display and local oscillator
                  }

          switch(temp_var_u8)
          {
          case CW_OFFSET_RX:
              txt_ptr = "   RX";
              break;
          case CW_OFFSET_TX:
              txt_ptr = "   TX";
              break;
          case CW_OFFSET_SHIFT:
              txt_ptr = "SHIFT";
              break;
          }
          break;
    }
    case MENU_TCXO_MODE:    // TCXO On/Off
        temp_var_u8 = RadioManagement_TcxoGetMode();     // get current setting without upper nibble
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                                              0,
                                              TCXO_TEMP_STATE_MAX,
                                              TCXO_OFF,
                                              1
                                             );

        if(lo.sensor_present == false)            // no sensor present
        {
            temp_var_u8 = TCXO_OFF; // force TCXO disabled
        }

        RadioManagement_TcxoSetMode(temp_var_u8);   // overlay new temperature setting with old status of upper nibble
        if(var_change)
        {
            UiDriver_CreateTemperatureDisplay();
        }

        switch(temp_var_u8) {
        case TCXO_OFF:
            txt_ptr = " OFF";
            break;
        case TCXO_ON:
            txt_ptr = "  ON";
            break;
        case TCXO_STOP:
            txt_ptr = "STOP";
            break;
        }
        break;

        case CONFIG_IQ_AUTO_CORRECTION:    // On/Off
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.iq_auto_correction,
                                                  0,
                                                  1,
                                                  0,
                                                  1
                                                 );

            switch(ts.iq_auto_correction) {
            case 0:
                txt_ptr = " OFF";
                ts.display_rx_iq = true;
                break;
            case 1:
                txt_ptr = "  ON";
                ts.display_rx_iq = false;
                break;
            }
//            if(var_change) temporarily disabled because function does not provide dynamical hiding recently
//            {
//          	  UiMenu_RenderMenu(MENU_RENDER_ONLY);
//			}

            break;

    case MENU_TCXO_C_F: // TCXO display C/F mode
        temp_var_u8 = (RadioManagement_TcxoIsFahrenheit() == true) ? 1 : 0;  // Yes - Is Fahrenheit mode enabled?

        if(RadioManagement_TcxoIsEnabled())       // is temperature display enabled at all?
        {
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                    0,
                    1,
                    0,
                    1
            );
            RadioManagement_TcxoSetUnit(temp_var_u8 != 0? TCXO_UNIT_F: TCXO_UNIT_C);
        }
        else
        {
            clr = Orange;
        }
        if(var_change)      // update screen if a change was made
        {
            UiDriver_CreateTemperatureDisplay();
        }

        txt_ptr =temp_var_u8?"F":"C";
        break;
    case MENU_SCOPE_SPEED:  // spectrum scope speed
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.scope_speed,
                                              0,
                                              SPECTRUM_SCOPE_SPEED_MAX,
                                              SPECTRUM_SCOPE_SPEED_DEFAULT,
                                              1
                                             );
        if(ts.scope_speed)
        {
            snprintf(options,32, "  %u", ts.scope_speed);
        }
        else
        {
            txt_ptr = "OFF";
        }
        break;
    case MENU_SPECTRUM_FILTER_STRENGTH: // spectrum filter strength
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.spectrum_filter,
                                              SPECTRUM_FILTER_MIN,
                                              SPECTRUM_FILTER_MAX,
                                              SPECTRUM_FILTER_DEFAULT,
                                              1
                                             );
        snprintf(options,32, "  %u", ts.spectrum_filter);
        break;
    case MENU_SCOPE_TRACE_COLOUR:   // spectrum scope trace colour
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.scope_trace_colour,
                                              0,
                                              SPEC_MAX_COLOUR,
                                              SPEC_COLOUR_TRACE_DEFAULT,
                                              1
                                             );
        UiMenu_MapColors(ts.scope_trace_colour,options,&clr);
        break;
    case MENU_SCOPE_GRID_COLOUR:    // spectrum scope grid colour
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.scope_grid_colour,
                                              0,
                                              SPEC_MAX_COLOUR,
                                              SPEC_COLOUR_GRID_DEFAULT,
                                              1
                                             );
        UiMenu_MapColors(ts.scope_grid_colour,options,&clr);
        break;
    case MENU_SPECTRUM_FREQSCALE_COLOUR:    // spectrum scope/waterfall  scale colour
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.spectrum_freqscale_colour,
                                              0,
                                              SPEC_MAX_COLOUR,
                                              SPEC_COLOUR_SCALE_DEFAULT,
                                              1
                                             );
        UiMenu_MapColors(ts.spectrum_freqscale_colour,options,&clr);
        break;
    case MENU_SPECTRUM_MAGNIFY: // WF/Spectrum magnifying
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &sd.magnify,
                                              MAGNIFY_MIN,
                                              MAGNIFY_MAX,
                                              MAGNIFY_DEFAULT,
                                              1
                                             );
        switch(sd.magnify)
        {
        case 1:
            txt_ptr = " x2";
            break;
        case 2:
            txt_ptr = " x4";
            break;
        case 3:
            txt_ptr = " x8";
            break;
        case 4:
            txt_ptr = "x16";
            break;
        case 5:
            txt_ptr = "x32";
            break;
        case 0:
        default:
            txt_ptr = " x1";
            break;
        }
        AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
        break;
    case MENU_SCOPE_AGC_ADJUST: // Spectrum scope AGC adjust
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.scope_agc_rate,
                                              SPECTRUM_SCOPE_AGC_MIN,
                                              SPECTRUM_SCOPE_AGC_MAX,
                                              SPECTRUM_SCOPE_AGC_DEFAULT,
                                              1
                                             );

        if(var_change)          // update system variable if rate changed
        {
            sd.agc_rate = (float)ts.scope_agc_rate; // calculate agc rate
            sd.agc_rate = sd.agc_rate/SPECTRUM_AGC_SCALING;
        }
        snprintf(options,32, "  %u", ts.scope_agc_rate);
        break;
    case MENU_SCOPE_DB_DIVISION:    // Adjustment of dB/division of spectrum scope
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.spectrum_db_scale,
                                              DB_DIV_ADJUST_MIN,
                                              DB_DIV_ADJUST_MAX,
                                              DB_DIV_ADJUST_DEFAULT,
                                              1
                                             );
        switch(ts.spectrum_db_scale)        // convert variable to setting
        {
        case DB_DIV_5:
            txt_ptr = "    5dB";
            break;
        case DB_DIV_7:
            txt_ptr = "  7.5dB";
            break;
        case DB_DIV_15:
            txt_ptr = "   15dB";
            break;
        case DB_DIV_20:
            txt_ptr = "   20dB";
            break;
        case S_1_DIV:
            txt_ptr = "1S-Unit";
            break;
        case S_2_DIV:
            txt_ptr = "2S-Unit";
            break;
        case S_3_DIV:
            txt_ptr = "3S-Unit";
            break;
        case DB_DIV_10:
        default:
            txt_ptr = "   10dB";
            break;
        }
        break;

    case MENU_SPECTRUM_CENTER_LINE_COLOUR:  // spectrum scope grid center line colour
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.spectrum_centre_line_colour,
                                              0,
                                              SPEC_GREY2,
                                              SPEC_COLOUR_GRID_DEFAULT,
                                              1
                                             );
        UiMenu_MapColors(ts.spectrum_centre_line_colour,options,&clr);
        break;
    case MENU_SCOPE_LIGHT_ENABLE:   // Spectrum light: no grid, larger, only points, no bars
        temp_var_u8 = (ts.flags1 & FLAGS1_SCOPE_LIGHT_ENABLE)? 1 : 0;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if (temp_var_u8)
            {
                ts.flags1 |= FLAGS1_SCOPE_LIGHT_ENABLE;
            }
            else
            {
                ts.flags1 &= ~FLAGS1_SCOPE_LIGHT_ENABLE;
            }
        }
        break;
    case MENU_SPECTRUM_MODE:
        temp_var_u8 = (ts.flags1 & FLAGS1_WFALL_SCOPE_TOGGLE)?1:0;

        UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);

        if (temp_var_u8)
        {
            ts.flags1 |= FLAGS1_WFALL_SCOPE_TOGGLE;
        }
        else
        {
            ts.flags1 &= ~FLAGS1_WFALL_SCOPE_TOGGLE ;
        }


        txt_ptr = (ts.flags1 & FLAGS1_WFALL_SCOPE_TOGGLE)?"WFALL":"SCOPE";
        // is waterfall mode active?
        // yes - indicate waterfall mode

        break;
    case MENU_WFALL_COLOR_SCHEME:   // Adjustment of dB/division of spectrum scope
        UiDriverMenuItemChangeUInt8(var, mode, &ts.waterfall_color_scheme,
                                    WATERFALL_COLOR_MIN,
                                    WATERFALL_COLOR_MAX,
                                    WATERFALL_COLOR_DEFAULT,
                                    1
                                   );
        switch(ts.waterfall_color_scheme)       // convert variable to setting
        {
        case WFALL_HOT_COLD:
            txt_ptr = "HotCold";
            break;
        case WFALL_RAINBOW:
            txt_ptr = "Rainbow";
            break;
        case WFALL_BLUE:
            txt_ptr = "   Blue";
            break;
        case WFALL_GRAY_INVERSE:
            txt_ptr = "INVGrey";
            break;
        case WFALL_GRAY:
        default:
            txt_ptr = "   Grey" ;
            break;
        }
        break;

    case    MENU_DBM_DISPLAY:
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.display_dbm,
                                              0,
                                              2,
                                              0,
                                              1
                                             );

       switch(ts.display_dbm)
        {
        case 1:     //
            txt_ptr = "     dBm";       // dbm display
            break;
        case 2: //
            txt_ptr = "  dBm/Hz";       // dbm/Hz display
            break;
        default:
        txt_ptr =  "     OFF";      // dbm display off
            break;
        }
        break;

       case    MENU_S_METER:
           var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.s_meter,
                                                 0,
                                                 2,
                                                 0,
                                                 1
                                                );

          switch(ts.s_meter)
           {
           case DISPLAY_S_METER_DBM:        //
               txt_ptr = "    based on dBm";        // dbm S-Meter
               break;
           case DISPLAY_S_METER_DBMHZ:  //
               txt_ptr = " based on dBm/Hz";        // dbm/Hz display and old school S-Meter
               break;
           default:
           txt_ptr =  "old school style";       // oldschool S-Meter
            break;
           }
           break;
    case MENU_METER_COLOUR_UP:              // upper meter colour
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.meter_colour_up,
                                              0,
                                              SPEC_MAX_COLOUR,
                                              SPEC_BLUE,
                                              1
                                             );
        UiMenu_MapColors(ts.meter_colour_up,options,&clr);
        break;
    case MENU_METER_COLOUR_DOWN:            // lower meter colour
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.meter_colour_down,
                                              0,
                                              SPEC_MAX_COLOUR,
                                              SPEC_CYAN,
                                              1
                                             );
        UiMenu_MapColors(ts.meter_colour_down,options,&clr);
        break;
    case MENU_REVERSE_TOUCHSCREEN:  // Touchscreen x mirrored?
        temp_var_u8 = (ts.flags1 & FLAGS1_REVERSE_TOUCHSCREEN)? 1 : 0;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if (temp_var_u8)
            {
                ts.flags1 |= FLAGS1_REVERSE_TOUCHSCREEN;
                ts.tp->reversed = 1;
            }
            else
            {
                ts.flags1 &= ~FLAGS1_REVERSE_TOUCHSCREEN;
                ts.tp->reversed = 0;
            }
        }
        break;
    case MENU_WFALL_STEP_SIZE:  // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt8(var, mode, &ts.waterfall_vert_step_size,
                                    WATERFALL_STEP_SIZE_MIN,
                                    WATERFALL_STEP_SIZE_MAX,
                                    WATERFALL_STEP_SIZE_DEFAULT,
                                    1
                                   );
        snprintf(options,32, "  %u", ts.waterfall_vert_step_size);
        break;
    case MENU_WFALL_OFFSET: // set step size of of waterfall display?
        UiDriverMenuItemChangeInt32(var, mode, &ts.waterfall_offset,
                                     WATERFALL_OFFSET_MIN,
                                     WATERFALL_OFFSET_MAX,
                                     WATERFALL_OFFSET_DEFAULT,
                                     1
                                    );
        snprintf(options,32, "  %u", (unsigned int)ts.waterfall_offset);
        break;
    case MENU_WFALL_CONTRAST:   // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt32(var, mode, &ts.waterfall_contrast,
                                     WATERFALL_CONTRAST_MIN,
                                     WATERFALL_CONTRAST_MAX,
                                     WATERFALL_CONTRAST_DEFAULT,
                                     2
                                    );
        snprintf(options,32, "  %u", (unsigned int)ts.waterfall_contrast);
        break;
    case MENU_WFALL_SPEED:  // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt8(var, mode, &ts.waterfall_speed,
                                    WATERFALL_SPEED_MIN,
                                    WATERFALL_SPEED_MAX,
                                    ts.display->display_type!=DISPLAY_HY28B_PARALLEL?WATERFALL_SPEED_DEFAULT_SPI:WATERFALL_SPEED_DEFAULT_PARALLEL,
                                    1
                                   );
        //
        if(ts.display->display_type != DISPLAY_HY28B_PARALLEL)
        {
            if(ts.waterfall_speed <= WATERFALL_SPEED_WARN_SPI)
                clr = Red;
            else if(ts.waterfall_speed <= WATERFALL_SPEED_WARN1_SPI)
                clr = Yellow;
        }
        else
        {
            if(ts.waterfall_speed <= WATERFALL_SPEED_WARN_PARALLEL)
                clr = Red;
            else if(ts.waterfall_speed <= WATERFALL_SPEED_WARN1_PARALLEL)
                clr = Yellow;
        }

        snprintf(options,32, "  %u", ts.waterfall_speed);
        break;
    case MENU_SCOPE_NOSIG_ADJUST:   // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt8(var, mode, &ts.spectrum_scope_nosig_adjust,
                                    SPECTRUM_SCOPE_NOSIG_ADJUST_MIN,
                                    SPECTRUM_SCOPE_NOSIG_ADJUST_MAX,
                                    SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT,
                                    1
                                   );
        snprintf(options,32, "  %u", ts.spectrum_scope_nosig_adjust);
        break;
    case MENU_WFALL_NOSIG_ADJUST:   // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt8(var, mode, &ts.waterfall_nosig_adjust,
                                    WATERFALL_NOSIG_ADJUST_MIN,
                                    WATERFALL_NOSIG_ADJUST_MAX,
                                    WATERFALL_NOSIG_ADJUST_DEFAULT,
                                    1
                                   );
        snprintf(options,32, "  %u", ts.waterfall_nosig_adjust);
        break;
    case MENU_SPECTRUM_SIZE:    // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt8(var, mode, &ts.spectrum_size,
                                    0,
                                    SPECTRUM_BIG,
                                    SPECTRUM_SIZE_DEFAULT,
                                    1
                                   );
        //
        switch(ts.spectrum_size)
        {
        case SPECTRUM_BIG:
            txt_ptr = "   Big";
            break;
        case SPECTRUM_NORMAL:
        default:
            txt_ptr = "Normal";
            break;
        }
        break;
    case MENU_BACKUP_CONFIG:
        txt_ptr = "n/a";
        if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_I2C)
        {
            txt_ptr = " Do it!";
            clr = White;
            if(var>=1)
            {
                UiMenu_DisplayValue("Working",Red,pos);
                ConfigStorage_CopySerial2Flash();
                txt_ptr = " Done...";
                clr = Green;
            }
        }
        break;
    case MENU_RESTORE_CONFIG:
        txt_ptr = "n/a";
        if(ts.ser_eeprom_in_use == SER_EEPROM_IN_USE_I2C)
        {
            txt_ptr = "Do it!";
            clr = White;
            if(var>=1)
            {

                UiMenu_DisplayValue("Working",Red,pos);
                ConfigStorage_CopyFlash2Serial();
                mchf_reboot();
            }
        }
        break;
    case MENU_RESTART_CODEC:
        txt_ptr = " Do it!";
        clr = White;
        if(var>=1)
        {

            UiMenu_DisplayValue("Restart",Red,pos);
            Codec_RestartI2S();
            var = 0;
        }
        break;
    case CONFIG_FREQ_STEP_MARKER_LINE:  // Frequency step marker line on/off
        temp_var_u8 = ts.freq_step_config & 0x0f;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)          // something changed?
        {
            if(temp_var_u8)     // yes, is line to be enabled?
                ts.freq_step_config |= 0x0f;    // yes, set lower nybble
            else            // line disabled?
                ts.freq_step_config &= 0xf0;    // no, clear lower nybble
            //
            UiDriver_ShowStep(df.tuning_step);  // update screen
        }
        break;
    case CONFIG_STEP_SIZE_BUTTON_SWAP:  // Step size button swap on/off
        temp_var_u8 = ts.freq_step_config & 0xf0;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if(temp_var_u8) // is button to be swapped?
                ts.freq_step_config |= 0xf0;    // set upper nybble
            else            // line disabled?
                ts.freq_step_config &= 0x0f;    // clear upper nybble
        }
        break;
    case CONFIG_BAND_BUTTON_SWAP:   // Swap position of Band+ and Band- buttons
        temp_var_u8 = ts.flags1 & FLAGS1_SWAP_BAND_BTN;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if(temp_var_u8) // band up/down swap is to be enabled
                ts.flags1 |= FLAGS1_SWAP_BAND_BTN;      // set LSB
            else            // band up/down swap is to be disabled
                ts.flags1 &= ~FLAGS1_SWAP_BAND_BTN;     // clear LSB
        }
        break;
    case CONFIG_TX_DISABLE: // Step size button swap on/off
        temp_var_u8 = ts.tx_disable & TX_DISABLE_ALWAYS;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            // FIXME: Call "abstract" function to update status of tune,
            // do not redraw menu button here directly
            UiDriver_FButtonLabel(5,"TUNE",temp_var_u8?Grey1:White);
            ts.tx_disable = temp_var_u8;
        }
        break;
    case CONFIG_TX_OUT_ENABLE:      // Enable transmitting outside HAM bands
        temp_var_u8 = (ts.flags1 & FLAGS1_TX_OUTSIDE_BANDS)? 1 : 0;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if (temp_var_u8)
                ts.flags1 |= FLAGS1_TX_OUTSIDE_BANDS;
            else
                ts.flags1 &= ~FLAGS1_TX_OUTSIDE_BANDS;
        }
        break;
    case CONFIG_AUDIO_MAIN_SCREEN_MENU_SWITCH:  // AFG/(STG/CMP) and RIT/(WPM/MIC/LIN) are to change automatically with TX/RX
        temp_var_u8 = ts.flags1 & FLAGS1_TX_AUTOSWITCH_UI_DISABLE;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if(temp_var_u8) // change-on-tx is to be disabled
                ts.flags1 |= FLAGS1_TX_AUTOSWITCH_UI_DISABLE;       // set LSB
            else            // change-on-tx is to be enabled
                ts.flags1 &= ~FLAGS1_TX_AUTOSWITCH_UI_DISABLE;      // clear LSB
        }
        break;
    case CONFIG_MUTE_LINE_OUT_TX:   // Enable/disable MUTE of TX audio on LINE OUT
        temp_var_u8 = ts.flags1 & FLAGS1_MUTE_LINEOUT_TX;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if((var_change) && (!ts.iq_freq_mode))          // did the status change and is translate mode NOT active?
        {
            if(temp_var_u8) // Yes - MUTE of TX audio on LINE OUT is enabled
                ts.flags1 |= FLAGS1_MUTE_LINEOUT_TX;        // set LSB
            else            // MUTE of TX audio on LINE OUT is disabled
                ts.flags1 &= ~FLAGS1_MUTE_LINEOUT_TX;       // clear LSB
        }
        if(ts.iq_freq_mode) // Mark RED if translate mode is active
            clr = Red;
        break;
    case CONFIG_TXRX_SWITCH_AUDIO_MUTE: // maximum RX gain setting
        UiDriverMenuItemChangeUInt8(var, mode, &ts.txrx_switch_audio_muting_timing,
                                    0,
                                    TXRX_SWITCH_AUDIO_MUTE_DELAY_MAX,
                                    0,
                                    1
                                   );
        snprintf(options,32, "%3ums", ts.txrx_switch_audio_muting_timing*10);
        break;
    case CONFIG_LCD_AUTO_OFF_MODE:  // LCD auto-off mode control
        temp_var_u8 = ts.lcd_backlight_blanking;        // get control variable
        temp_var_u8 &= LCD_BLANKING_TIMEMASK;                           // mask off upper nybble
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                                              0,
                                              LCD_BLANKING_TIMEMASK,
                                              BACKLIGHT_BLANK_TIMING_DEFAULT,
                                              1
                                             );
        if(var_change)                              // timing has been changed manually
        {
            if(temp_var_u8)                 // is the time non-zero?
            {
                ts.lcd_backlight_blanking = temp_var_u8;    // yes, copy current value into variable
                ts.lcd_backlight_blanking |= LCD_BLANKING_ENABLE;       // set MSB to enable auto-blanking
            }
            else
            {
                ts.lcd_backlight_blanking = 0;          // zero out variable
            }
            UiDriver_LcdBlankingStartTimer();       // update the LCD timing parameters
        }
        //
        if(ts.lcd_backlight_blanking & LCD_BLANKING_ENABLE)         // timed auto-blanking enabled?
            snprintf(options,32,"%02d sec",ts.lcd_backlight_blanking & LCD_BLANKING_TIMEMASK);  // yes - Update screen indicator with number of seconds
        else
            snprintf(options,32,"   OFF");                      // Or if turned off
        break;
    case CONFIG_VOLTMETER_CALIBRATION:      // Voltmeter calibration
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.voltmeter_calibrate,
                                               POWER_VOLTMETER_CALIBRATE_MIN,
                                               POWER_VOLTMETER_CALIBRATE_MAX,
                                               POWER_VOLTMETER_CALIBRATE_DEFAULT,
                                               1
                                              );
        snprintf(options,32, "  %u", (unsigned int)ts.voltmeter_calibrate);
        break;
    case CONFIG_DISP_FILTER_BANDWIDTH: // Display filter bandwidth
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.filter_disp_colour,
                                              0,
                                              SPEC_BLACK,
                                              SPEC_COLOUR_GRID_DEFAULT,
                                              1
                                             );
        UiMenu_MapColors(ts.filter_disp_colour,options,&clr);
        break;
    //
    case CONFIG_MAX_VOLUME: // maximum audio volume
        UiDriverMenuItemChangeUInt8(var, mode, &ts.rx_gain[RX_AUDIO_SPKR].max,
                                    MAX_VOLUME_MIN,
                                    MAX_VOLUME_MAX,
                                    MAX_VOLUME_DEFAULT,
                                    1
                                   );
        if(ts.rx_gain[RX_AUDIO_SPKR].value > ts.rx_gain[RX_AUDIO_SPKR].max)             // is the volume currently higher than the new setting?
        {
            ts.rx_gain[RX_AUDIO_SPKR].value = ts.rx_gain[RX_AUDIO_SPKR].max;        // yes - force the volume to the new value
            UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
        }
        snprintf(options,32, "    %u", ts.rx_gain[RX_AUDIO_SPKR].max);
        //
        if(ts.rx_gain[RX_AUDIO_SPKR].max <= MAX_VOL_RED_THRESH)         // Indicate that gain has been reduced by changing color
            clr = Red;
        else if(ts.rx_gain[RX_AUDIO_SPKR].max <= MAX_VOLT_YELLOW_THRESH)
            clr = Orange;
        break;
    case CONFIG_MAX_RX_GAIN:    // maximum RX gain setting
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.max_rf_gain,
                                              0,
                                              MAX_RF_GAIN_MAX,
                                              MAX_RF_GAIN_DEFAULT,
                                              1
                                             );
        if(var_change)
        {
            AudioManagement_CalcAGCVals();  // calculate new internal AGC values from user settings
        }
        snprintf(options,32, "    %u", ts.max_rf_gain);
        break;
    case CONFIG_LINEOUT_GAIN:
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.lineout_gain,
                                              LINEOUT_GAIN_MIN,
                                              LINEOUT_GAIN_MAX,
                                              LINEOUT_GAIN_DEFAULT,
                                              1
                                             );
        if(var_change)
        {
            Codec_VolumeLineOut(ts.txrx_mode);
        }
        snprintf(options,32, "%3u", ts.lineout_gain);
        break;
    case CONFIG_BEEP_ENABLE:    //
        temp_var_u8 = ts.flags2 & FLAGS2_KEY_BEEP_ENABLE;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if(temp_var_u8) // beep is to be enabled
                ts.flags2 |= FLAGS2_KEY_BEEP_ENABLE;        // set LSB+2
            else            // beep is to be disabled
                ts.flags2 &= ~FLAGS2_KEY_BEEP_ENABLE;       // clear LSB+2
            UiMenu_RenderMenu(MENU_RENDER_ONLY);
        }
        break;
    case CONFIG_BEEP_FREQ:      // Beep frequency
        if(ts.flags2 & FLAGS2_KEY_BEEP_ENABLE)      // is beep enabled?
        {
            var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.beep_frequency,
                                                   MIN_BEEP_FREQUENCY,
                                                   MAX_BEEP_FREQUENCY,
                                                   DEFAULT_BEEP_FREQUENCY,
                                                   25);
            if(var_change)
            {
                AudioManagement_LoadBeepFreq();
                AudioManagement_KeyBeep();      // make beep to demonstrate frequency
            }
        }
        else    // beep not enabled - display frequency in red
            clr = Orange;
        snprintf(options,32, "   %uHz", (uint)ts.beep_frequency);   // casted to int because display errors if uint32_t
        break;
    //
    case CONFIG_BEEP_VOLUME:    // beep loudness
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.beep_loudness,
                                              0,
                                              MAX_BEEP_LOUDNESS,
                                              DEFAULT_BEEP_LOUDNESS,
                                              1);
        if(var_change)
        {
            AudioManagement_LoadBeepFreq(); // calculate new beep loudness values
            AudioManagement_KeyBeep();      // make beep to demonstrate loudness
        }
        snprintf(options,32, "    %u", ts.beep_loudness);
        break;
    //
    //
    // *****************  WARNING *********************
    // If you change CAT mode, THINGS MAY GET "BROKEN" - for example, you may not be able to reliably save to EEPROM!
    // This needs to be investigated!
    //
    case CONFIG_CAT_ENABLE:
        temp_var_u8 = (ts.flags1 & FLAGS1_CAT_MODE_ACTIVE)? 1 : 0;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if (temp_var_u8)
            ts.flags1 |= FLAGS1_CAT_MODE_ACTIVE;
        else
            ts.flags1 &= ~FLAGS1_CAT_MODE_ACTIVE;
        if (var_change)
        {
            if(ts.flags1 & FLAGS1_CAT_MODE_ACTIVE)
            {
                CatDriver_InitInterface();
            }
            else
            {
                CatDriver_StopInterface();
            }
        }
        break;
    case CONFIG_FREQUENCY_CALIBRATE:        // Frequency Calibration
        if(var >= 1)        // setting increase?
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            ts.freq_cal += 1; // df.tuning_step;
            var_change = 1;
        }
        else if(var <= -1)      // setting decrease?
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            ts.freq_cal -= 1 ; // df.tuning_step;
            var_change = 1;
        }
        if(ts.freq_cal < MIN_FREQ_CAL)
        {
            ts.freq_cal = MIN_FREQ_CAL;
        }
        else if(ts.freq_cal > MAX_FREQ_CAL)
        {
            ts.freq_cal = MAX_FREQ_CAL;
        }
        //
        if(mode == MENU_PROCESS_VALUE_SETDEFAULT)
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            ts.freq_cal = 0;
            var_change = 1;
        }
        if(var_change)
        {
            Si570_SetPPM(((float32_t)ts.freq_cal)/10.0);
            // Update LO PPM (will automatically adjust frequency)
        }
        {
            char numstr[16];
            float2fixedstr(numstr, 16, ((float32_t)ts.freq_cal)/10.0, 4, 1);
            snprintf(options,32, "%sppm", numstr );
        }
        break;
    //
    case CONFIG_FREQ_LIMIT_RELAX:   // Enable/disable Frequency tuning limits
        temp_var_u8 = ts.flags1 & FLAGS1_FREQ_LIMIT_RELAX;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)          // did the status change and is translate mode NOT active?
        {
            if(temp_var_u8) // tuning limit is disabled
                ts.flags1 |= FLAGS1_FREQ_LIMIT_RELAX;       // set bit
            else            // tuning limit is enabled
                ts.flags1 &= ~FLAGS1_FREQ_LIMIT_RELAX;      // clear bit
        }
        if(ts.flags1 & FLAGS1_FREQ_LIMIT_RELAX)             // tuning limit is disabled
        {
            clr = Orange;                   // warn user!
        }
        break;
    case CONFIG_FREQ_MEM_LIMIT_RELAX:   // Enable/disable Frequency memory limits
        temp_var_u8 = ts.flags2 & FLAGS2_FREQ_MEM_LIMIT_RELAX;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)          // did the status change?
        {
            if(temp_var_u8) // freq/mem limit is disabled
                ts.flags2 |= FLAGS2_FREQ_MEM_LIMIT_RELAX;       // set bit
            else            // freq/mem limit is enabled
                ts.flags2 &= ~FLAGS2_FREQ_MEM_LIMIT_RELAX;      // clear bit
        }
        if(ts.flags2 & FLAGS2_FREQ_MEM_LIMIT_RELAX)             // frequency/memory limit is disabled
        {
            clr = Orange;                   // warn user!
        }
        break;
    case CONFIG_80M_RX_IQ_GAIN_BAL:     // LSB RX IQ Gain balance
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &ts.rx_iq_gain_balance[IQ_80M], TRX_MODE_RX, IQ_TRANS_ON);
        break;
    case CONFIG_80M_RX_IQ_PHASE_BAL:        // LSB RX IQ Phase balance
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &ts.rx_iq_phase_balance[IQ_80M], TRX_MODE_RX, IQ_TRANS_ON);
        break;
    case CONFIG_10M_RX_IQ_GAIN_BAL:     // USB/CW RX IQ Gain balance
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &ts.rx_iq_gain_balance[IQ_10M], TRX_MODE_RX, IQ_TRANS_ON);
        break;
    case CONFIG_10M_RX_IQ_PHASE_BAL:        // USB RX IQ Phase balance
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &ts.rx_iq_phase_balance[IQ_10M], TRX_MODE_RX, IQ_TRANS_ON);
        break;
    case CONFIG_80M_TX_IQ_GAIN_BAL:     // LSB TX IQ Gain balance
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &ts.tx_iq_gain_balance[IQ_80M], TRX_MODE_TX, IQ_TRANS_ON);
        break;
    case CONFIG_80M_TX_IQ_PHASE_BAL:        // LSB TX IQ Phase balance
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &ts.tx_iq_phase_balance[IQ_80M], TRX_MODE_TX, IQ_TRANS_ON);
        break;
    case CONFIG_10M_TX_IQ_GAIN_BAL:     // USB/CW TX IQ Gain balance
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &ts.tx_iq_gain_balance[IQ_10M], TRX_MODE_TX, IQ_TRANS_ON);
        break;
    case CONFIG_10M_TX_IQ_PHASE_BAL:        // USB TX IQ Phase balance
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &ts.tx_iq_phase_balance[IQ_10M], TRX_MODE_TX, IQ_TRANS_ON);
        break;
    case    CONFIG_80M_TX_IQ_GAIN_BAL_TRANS_OFF:     // AM TX IQ Phase balance
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &ts.tx_iq_gain_balance[IQ_80M], TRX_MODE_TX, IQ_TRANS_OFF);
        break;
    case    CONFIG_80M_TX_IQ_PHASE_BAL_TRANS_OFF:        // FM TX IQ Phase balance
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &ts.tx_iq_phase_balance[IQ_80M], TRX_MODE_TX, IQ_TRANS_OFF);
        break;
    case    CONFIG_10M_TX_IQ_GAIN_BAL_TRANS_OFF:     // AM TX IQ Phase balance
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &ts.tx_iq_gain_balance[IQ_10M], TRX_MODE_TX, IQ_TRANS_OFF);
        break;
    case    CONFIG_10M_TX_IQ_PHASE_BAL_TRANS_OFF:        // FM TX IQ Phase balance
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &ts.tx_iq_phase_balance[IQ_10M], TRX_MODE_TX, IQ_TRANS_OFF);
        break;

    case CONFIG_CW_PA_BIAS:     // CW PA Bias adjust
        if((ts.tune) || (ts.txrx_mode == TRX_MODE_TX))      // enable only in TUNE mode
        {
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.pa_cw_bias,
                                                  0,
                                                  MAX_PA_BIAS,
                                                  0,
                                                  1);

            if(var_change)
            {
                RadioManagement_SetPaBias();
            }
            if(ts.pa_cw_bias < MIN_BIAS_SETTING)
            {
                clr = Yellow;
            }
        }
        else        // Orange if not in TUNE or TX mode
        {
            clr = Orange;
        }
        snprintf(options,32, "  %u", ts.pa_cw_bias);
        break;
    case CONFIG_PA_BIAS:        // PA Bias adjust (Including CW if CW bias == 0)
        if((ts.tune) || (ts.txrx_mode == TRX_MODE_TX))      // enable only in TUNE mode
        {
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.pa_bias,
                                                  0,
                                                  MAX_PA_BIAS,
                                                  0,
                                                  1);

            if(var_change)
            {
                RadioManagement_SetPaBias();
            }
            if(ts.pa_bias < MIN_BIAS_SETTING)
            {
                clr = Yellow;
            }
        }
        else        // Orange if not in TUNE or TX mode
        {
            clr = Orange;
        }
        snprintf(options,32, "  %u", ts.pa_bias);
        break;
    case CONFIG_FWD_REV_PWR_DISP:   // Enable/disable display of FWD/REV A/D inputs on power sensor
        temp_var_u8 = swrm.pwr_meter_disp;
        UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        swrm.pwr_meter_disp = temp_var_u8;
        break;
    case CONFIG_RF_FWD_PWR_NULL:        // RF power FWD power meter calibrate
        if(swrm.pwr_meter_disp)
        {
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &swrm.sensor_null,
                                                  SWR_CAL_MIN,
                                                  SWR_CAL_MAX,
                                                  SWR_CAL_DEFAULT,
                                                  1);
            if(ts.txrx_mode != TRX_MODE_TX) // Orange if not in TX mode
            {
                clr = Orange;
            }
        }
        else    // numerical display NOT active
        {
            clr = Orange;       // make it red to indicate that adjustment is NOT available
        }
        snprintf(options,32, "  %u", swrm.sensor_null);
        break;
    case CONFIG_FWD_REV_COUPLING_2200M_ADJ:     // RF power sensor coupling adjust (2200m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_2200M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_630M_ADJ:      // RF power sensor coupling adjust (630m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_630M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_160M_ADJ:      // RF power sensor coupling adjust (160m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_160M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_80M_ADJ:       // RF power sensor coupling adjust (80m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_80M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_40M_ADJ:       // RF power sensor coupling adjust (40m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_40M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_20M_ADJ:       // RF power sensor coupling adjust (20m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_20M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_15M_ADJ:       // RF power sensor coupling adjust (15m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_15M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_6M_ADJ:        // RF power sensor coupling adjust (6m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_6M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_2M_ADJ:        // RF power sensor coupling adjust (2m)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_2M, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_70CM_ADJ:      // RF power sensor coupling adjust (70cm)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_70CM, options, &clr);
        break;
    case CONFIG_FWD_REV_COUPLING_23CM_ADJ:      // RF power sensor coupling adjust (23cm)
        UiDriverMenuBandRevCouplingAdjust(var, mode, COUPLING_23CM, options, &clr);
        break;
    case CONFIG_FWD_REV_SENSE_SWAP: // Enable/disable swap of FWD/REV A/D inputs on power sensor
        temp_var_u8 = ts.flags1 & FLAGS1_SWAP_FWDREV_SENSE;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)          // did the status change and is translate mode NOT active?
        {
            if(temp_var_u8) // swapping of FWD/REV is enabled
            {
                ts.flags1 |= FLAGS1_SWAP_FWDREV_SENSE;      // set bit
            }
            else            // swapping of FWD/REV bit is disabled
            {
                ts.flags1 &= ~FLAGS1_SWAP_FWDREV_SENSE;     // clear bit
            }
        }
        if(ts.flags1 & FLAGS1_SWAP_FWDREV_SENSE)                // Display status FWD/REV swapping
        {
            clr = Orange;                   // warn user swapping is on!
        }
        break;
    case CONFIG_XVTR_OFFSET_MULT:   // Transverter Frequency Display Offset/Multiplier Mode On/Off
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.xverter_mode,
                                              0,
                                              XVERTER_MULT_MAX,
                                              0,
                                              1);
        if(var_change)          // change?
        {
            UiDriver_FrequencyUpdateLOandDisplay(true);
        }
        //
        if(ts.xverter_mode)
        {
            snprintf(options,32, " ON x%u", ts.xverter_mode);   // Display on/multiplication factor
            clr = Red;
        }
        else
        {
            txt_ptr = "    OFF";
        }
        break;
    case CONFIG_XVTR_FREQUENCY_OFFSET:      // Adjust transverter Frequency offset
        if(var >= 1)        // setting increase?
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            ts.xverter_offset += df.tuning_step;
            var_change = 1;
        }
        else if(var <= -1)      // setting decrease?
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            if(ts.xverter_offset >= df.tuning_step) // subtract only if we have room to do so
            {
                ts.xverter_offset -= df.tuning_step;
            }
            else
            {
                ts.xverter_offset = 0;              // else set to zero
            }
            //
            var_change = 1;
        }
        if(ts.xverter_offset > XVERTER_OFFSET_MAX)
        {
            ts.xverter_offset  = XVERTER_OFFSET_MAX;
        }
        if(mode == MENU_PROCESS_VALUE_SETDEFAULT)
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            ts.xverter_offset = 0;      // default for this option is to zero it out
            var_change = 1;
        }
        if(var_change)          // change?
        {
            UiDriver_FrequencyUpdateLOandDisplay(true);
        }

        if(ts.xverter_mode) // transvert mode active?
        {
            clr = Red;      // make number red to alert user of this!
        }

        snprintf(options,32, " %9uHz", (uint)ts.xverter_offset);    // print with nine digits
        break;


    case CONFIG_2200M_5W_ADJUST:        // 2200m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_2200, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_630M_5W_ADJUST:     // 630m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_630, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_160M_5W_ADJUST:     // 160m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_160, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_80M_5W_ADJUST:      // 80m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_80, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_60M_5W_ADJUST:      // 60m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_60, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_40M_5W_ADJUST:      // 40m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_40, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_30M_5W_ADJUST:      // 30m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_30, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_20M_5W_ADJUST:      // 20m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_20, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_17M_5W_ADJUST:      // 17m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_17, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_15M_5W_ADJUST:      // 15m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_15, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_12M_5W_ADJUST:      // 12m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_12, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_10M_5W_ADJUST:      // 10m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_10, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_6M_5W_ADJUST:       // 6m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_6, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_4M_5W_ADJUST:       // 4m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_4, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_2M_5W_ADJUST:       // 2m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_2, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_70CM_5W_ADJUST:     // 70cm 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_70, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_23CM_5W_ADJUST:     // 23cm 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_23, PA_LEVEL_5W, options, &clr);
        break;
    case CONFIG_2200M_FULL_POWER_ADJUST:        // 2200m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_2200, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_630M_FULL_POWER_ADJUST:     // 630m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_630, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_160M_FULL_POWER_ADJUST:     // 160m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_160, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_80M_FULL_POWER_ADJUST:      // 80m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_80, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_60M_FULL_POWER_ADJUST:      // 60m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_60, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_40M_FULL_POWER_ADJUST:      // 40m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_40, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_30M_FULL_POWER_ADJUST:      // 30m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_30, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_20M_FULL_POWER_ADJUST:      // 20m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_20, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_17M_FULL_POWER_ADJUST:      // 17m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_17, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_15M_FULL_POWER_ADJUST:      // 15m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_15, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_12M_FULL_POWER_ADJUST:      // 12m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_12, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_10M_FULL_POWER_ADJUST:      // 10m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_10, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_6M_FULL_POWER_ADJUST:       // 6m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_6, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_4M_FULL_POWER_ADJUST:       // 4m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_4, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_2M_FULL_POWER_ADJUST:       // 2m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_2, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_70CM_FULL_POWER_ADJUST:     // 70cm 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_70, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_23CM_FULL_POWER_ADJUST:     // 23cm 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_23, PA_LEVEL_FULL, options, &clr);
        break;
    case CONFIG_REDUCE_POWER_ON_LOW_BANDS:
        temp_var_u8 = ts.flags2 & FLAGS2_LOW_BAND_BIAS_REDUCE;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if(temp_var_u8)
                ts.flags2 |= FLAGS2_LOW_BAND_BIAS_REDUCE;
            else
                ts.flags2 &= ~FLAGS2_LOW_BAND_BIAS_REDUCE;
        }
        break;
    case CONFIG_REDUCE_POWER_ON_HIGH_BANDS:
        temp_var_u8 = ts.flags2 & FLAGS2_HIGH_BAND_BIAS_REDUCE;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if(temp_var_u8)
                ts.flags2 |= FLAGS2_HIGH_BAND_BIAS_REDUCE;
            else
                ts.flags2 &= ~FLAGS2_HIGH_BAND_BIAS_REDUCE;
        }
        break;
    case CONFIG_DSP_NR_DECORRELATOR_BUFFER_LENGTH:      // Adjustment of DSP noise reduction de-correlation delay buffer length
        ts.dsp_nr_delaybuf_len &= 0xfff0;   // mask bottom nybble to enforce 16-count boundary
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.dsp_nr_delaybuf_len,
                                               DSP_NR_BUFLEN_MIN,
                                               DSP_NR_BUFLEN_MAX,
                                               DSP_NR_BUFLEN_DEFAULT,
                                               16);

        if(ts.dsp_nr_delaybuf_len <= ts.dsp_nr_numtaps) // is buffer smaller/equal to number of taps?
            ts.dsp_nr_delaybuf_len = ts.dsp_nr_numtaps + 16;    // yes - it must always be larger than number of taps!

        if(var_change)      // did something change?
        {
            if(ts.dsp_active & DSP_NR_ENABLE)   // only update if DSP NR active
                AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
        }
        if(!(ts.dsp_active & DSP_NR_ENABLE))    // mark orange if DSP NR not active
            clr = Orange;
        if(ts.dsp_nr_numtaps >= ts.dsp_nr_delaybuf_len) // Warn if number of taps greater than/equal buffer length!
            clr = Red;
        snprintf(options,32, "  %u", (uint)ts.dsp_nr_delaybuf_len);
        break;
    case CONFIG_DSP_NR_FFT_NUMTAPS:     // Adjustment of DSP noise reduction de-correlation delay buffer length
        ts.dsp_nr_numtaps &= 0xf0;  // mask bottom nybble to enforce 16-count boundary
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp_nr_numtaps,
                                              DSP_NR_NUMTAPS_MIN,
                                              DSP_NR_NUMTAPS_MAX,
                                              DSP_NR_NUMTAPS_DEFAULT,
                                              16);
        if(ts.dsp_nr_numtaps >= ts.dsp_nr_delaybuf_len) // is number of taps equal or greater than buffer length?
            ts.dsp_nr_delaybuf_len = ts.dsp_nr_numtaps + 16;    // yes - make buffer larger

        if(var_change)      // did something change?
        {
            if(ts.dsp_active & DSP_NR_ENABLE)   // only update if DSP NR active
                AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
        }

        if(!(ts.dsp_active & DSP_NR_ENABLE))    // mark orange if DSP NR not active
            clr = Orange;
        if(ts.dsp_nr_numtaps >= ts.dsp_nr_delaybuf_len) // Warn if number of taps greater than/equal buffer length!
            clr = Red;
        snprintf(options,32, "  %u", ts.dsp_nr_numtaps);
        break;
    case CONFIG_DSP_NR_POST_AGC_SELECT:     // selection of location of DSP noise reduction - pre audio filter/AGC or post AGC/filter
        temp_var_u8 = ts.dsp_active & DSP_NR_POSTAGC_ENABLE;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(!(ts.dsp_active & DSP_NR_ENABLE))    // mark orange if DSP NR not active
        {
            clr = Orange;
        }
        if (temp_var_u8)
        {
            ts.dsp_active |= DSP_NR_POSTAGC_ENABLE;
        }
        else
        {
            ts.dsp_active &= ~DSP_NR_POSTAGC_ENABLE;
        }
        if(var_change)      // did something change?
        {
            if(ts.dsp_active & DSP_NR_ENABLE)   // only update if DSP NR active
            {
                AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
            }
        }
        break;
    case CONFIG_DSP_NOTCH_CONVERGE_RATE:        // Adjustment of DSP noise reduction de-correlation delay buffer length
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp_notch_mu,
                                              0,
                                              DSP_NOTCH_MU_MAX,
                                              DSP_NOTCH_MU_DEFAULT,
                                              1);

        if(var_change)      // did something change?
        {
            if(ts.dsp_active & DSP_NOTCH_ENABLE)    // only update if Notch DSP is active
            {
                AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp_active & DSP_NOTCH_ENABLE)) // mark orange if Notch DSP not active
        {
            clr = Orange;
        }
        snprintf(options,32, "  %u", ts.dsp_notch_mu);
        break;
    case CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH:       // Adjustment of DSP noise reduction de-correlation delay buffer length
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp_notch_delaybuf_len,
                                              DSP_NOTCH_BUFLEN_MIN,
                                              DSP_NOTCH_BUFLEN_MAX,
                                              DSP_NOTCH_DELAYBUF_DEFAULT,
                                              8);


        if(ts.dsp_notch_delaybuf_len <= ts.dsp_notch_numtaps)       // did we try to decrease it smaller than FFT size?
        {
            ts.dsp_notch_delaybuf_len = ts.dsp_notch_numtaps + 8;                       // yes - limit it to previous size
        }
        if(var_change)      // did something change?
        {
            if(ts.dsp_active & DSP_NOTCH_ENABLE)    // only update if DSP Notch active
            {
                AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp_active & DSP_NOTCH_ENABLE)) // mark orange if DSP Notch not active
        {
            clr = Orange;
        }
        if(ts.dsp_notch_numtaps >= ts.dsp_notch_delaybuf_len)
        {
            clr = Red;
        }
        snprintf(options,32, "  %u", (uint)ts.dsp_notch_delaybuf_len);
        break;
    case CONFIG_DSP_NOTCH_FFT_NUMTAPS:      // Adjustment of DSP noise reduction de-correlation delay buffer length
        ts.dsp_notch_numtaps &= 0xf0;   // mask bottom nybble to enforce 16-count boundary
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp_notch_numtaps,
                                              0,
                                              DSP_NOTCH_NUMTAPS_MAX,
                                              DSP_NOTCH_NUMTAPS_DEFAULT,
                                              16);
        if(ts.dsp_notch_numtaps >= ts.dsp_notch_delaybuf_len)   // force buffer size to always be larger than number of taps
            ts.dsp_notch_delaybuf_len = ts.dsp_notch_numtaps + 8;
        if(var_change)      // did something change?
        {
            if(ts.dsp_active & DSP_NOTCH_ENABLE)    // only update if DSP NR active
            {
                AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp_active & DSP_NOTCH_ENABLE)) // mark orange if DSP NR not active
        {
            clr = Orange;
        }
        if(ts.dsp_notch_numtaps >= ts.dsp_notch_delaybuf_len)   // Warn if number of taps greater than/equal buffer length!
        {
            clr = Red;
        }
        snprintf(options,32, "  %u", ts.dsp_notch_numtaps);
        break;
    case CONFIG_AGC_TIME_CONSTANT:      // Adjustment of Noise Blanker AGC Time Constant
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.nb_agc_time_const,
                                              0,
                                              NB_MAX_AGC_SETTING,
                                              NB_AGC_DEFAULT,
                                              1);
        if(var_change)                  // parameter changed?
        {
            AudioManagement_CalcNB_AGC();   // yes - recalculate new values for Noise Blanker AGC
        }

        snprintf(options,32, "  %u", ts.nb_agc_time_const);
        break;
    case CONFIG_AM_TX_FILTER_DISABLE:   // Enable/disable AM TX audio filter
        temp_var_u8 = ts.flags1 & FLAGS1_AM_TX_FILTER_DISABLE;
        var_change = UiDriverMenuItemChangeDisableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)          // did the status change and is translate mode NOT active?
        {
            if(temp_var_u8) // AM TX audio filter is disabled
            {
                ts.flags1 |= FLAGS1_AM_TX_FILTER_DISABLE;       // set LSB
            }
            else
            {   // AM TX audio filter is enabled
                ts.flags1 &= ~FLAGS1_AM_TX_FILTER_DISABLE;      // clear LSB
            }
        }
        if(ts.flags1 & FLAGS1_AM_TX_FILTER_DISABLE)             // Display status of TX audio filter
        {
            clr = Orange;                   // warn user that filter is off!
        }
        break;
/*    case CONFIG_SSB_TX_FILTER_DISABLE:    // Enable/disable SSB TX audio filter
        temp_var = ts.flags1 & FLAGS1_SSB_TX_FILTER_DISABLE;
        tchange = UiDriverMenuItemChangeDisableOnOff(var, mode, &temp_var,0,options,&clr);
        if(tchange)         // did the status change and is translate mode NOT active?
        {
            if(temp_var)    // SSB TX audio filter is disabled
            {
                ts.flags1 |= FLAGS1_SSB_TX_FILTER_DISABLE;      // set bit
            }
            else
            {   // SSB TX audio filter is enabled
                ts.flags1 &= ~FLAGS1_SSB_TX_FILTER_DISABLE;     // clear bit
            }
        }
        if(ts.flags1 & FLAGS1_SSB_TX_FILTER_DISABLE)                // Display status of TX audio filter
        {
            clr = Red;                  // warn user that filter is off!
        }
        break;
*/
    case CONFIG_SSB_TX_FILTER:  // Type of SSB TX audio filter
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.tx_filter,
//                0,
                1,
                                              TX_FILTER_BASS,
                                              TX_FILTER_SOPRANO,
                                              1
                                             );
        switch(ts.tx_filter) {
        case TX_FILTER_SOPRANO:
            txt_ptr = " SOPRANO";
            break;
        case TX_FILTER_BASS:
            txt_ptr = "    BASS";
            break;
        case TX_FILTER_TENOR:
            txt_ptr = "   TENOR";
            break;
        }
        if(var_change)
        {
            // switch FIR Hilberts
            AudioFilter_InitTxHilbertFIR();
            // switch IIR Filters
            AudioDriver_TxFilterInit(ts.dmod_mode);
        }
        break;

        case CONFIG_TUNE_TONE_MODE: // set power for antenne tuning
        temp_var_u8 = ts.menu_var_changed;
        // this is not save, so no need to mark as dirty,
        // we just remember the state and restore it
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.tune_tone_mode,
                                              TUNE_TONE_SINGLE,
                                              TUNE_TONE_TWO,
                                              TUNE_TONE_SINGLE,
                                              1);
        switch(ts.tune_tone_mode)
        {
        case TUNE_TONE_SINGLE:
            txt_ptr = "  Single";
            break;
        case TUNE_TONE_TWO:
            txt_ptr = "Two Tone";
            break;
        default:
            break;
        }
        ts.menu_var_changed = temp_var_u8;
        break;
        case CONFIG_TUNE_POWER_LEVEL: // set power for antenne tuning
            var_change = UiDriverMenuItemChangeUInt8(var*(-1), mode, &ts.tune_power_level,
                                                  0,
                                                  PA_LEVEL_TUNE_KEEP_CURRENT,
                                                  PA_LEVEL_TUNE_KEEP_CURRENT,
                                                  1);
            switch(ts.tune_power_level)
            {
            case PA_LEVEL_FULL:
                txt_ptr = "FULL POWER";
                break;
            case PA_LEVEL_5W:
                txt_ptr = "        5W";
                break;
            case PA_LEVEL_2W:
                txt_ptr = "        2W";
                break;
            case PA_LEVEL_1W:
                txt_ptr = "        1W";
                break;
            case PA_LEVEL_0_5W:
                txt_ptr = "      0.5W";
                break;
            case PA_LEVEL_TUNE_KEEP_CURRENT:
                txt_ptr = " as TX PWR";
                break;
            }
            break;

    case CONFIG_SPECTRUM_FFT_WINDOW_TYPE:   // set step size of of waterfall display?
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.fft_window_type,
                                              0,
                                              FFT_WINDOW_MAX-1,
                                              FFT_WINDOW_DEFAULT,
                                              1);

        switch(ts.fft_window_type)
        {
        case FFT_WINDOW_RECTANGULAR:
            txt_ptr = "Rectangular";
            break;
        case FFT_WINDOW_COSINE:
            txt_ptr = "     Cosine";
            break;
        case FFT_WINDOW_BARTLETT:
            txt_ptr = "   Bartlett";
            break;
        case FFT_WINDOW_WELCH:
            txt_ptr = "      Welch";
            break;
        case FFT_WINDOW_HANN:
            txt_ptr = "       Hann";
            break;
        case FFT_WINDOW_HAMMING:
            txt_ptr = "    Hamming";
            break;
        case FFT_WINDOW_BLACKMAN:
            txt_ptr = "   Blackman";
            break;
        case FFT_WINDOW_NUTTALL:
            txt_ptr = "    Nuttall";
            break;
        }
        break;
    case CONFIG_RESET_SER_EEPROM:
        if(SerialEEPROM_Exists() == false)
        {
            txt_ptr = "   n/a";
            clr = Red;
        }
        else
        {
            txt_ptr = "Do it!";
            clr = White;
            if(var>=1)
            {
                // clear EEPROM
                UiMenu_DisplayValue("Working",Red,pos);
                SerialEEPROM_Clear();
                Si570_ResetConfiguration();     // restore SI570 to factory default
                *(__IO uint32_t*)(SRAM2_BASE) = 0x55;
                NVIC_SystemReset();         // restart mcHF
            }
        }
        break;
    case    MENU_FP_CW_01:
    case    MENU_FP_CW_02:
    case    MENU_FP_CW_03:
    case    MENU_FP_CW_04:
        UiMenu_ChangeFilterPathMemory(var, mode, options, &clr, FILTER_MODE_CW,(select - MENU_FP_CW_01)+1);
        break;
    case    MENU_FP_AM_01:
    case    MENU_FP_AM_02:
    case    MENU_FP_AM_03:
    case    MENU_FP_AM_04:
        UiMenu_ChangeFilterPathMemory(var, mode, options, &clr, FILTER_MODE_AM,(select - MENU_FP_AM_01)+1);
        break;
    case    MENU_FP_SSB_01:
    case    MENU_FP_SSB_02:
    case    MENU_FP_SSB_03:
    case    MENU_FP_SSB_04:
        UiMenu_ChangeFilterPathMemory(var, mode, options, &clr, FILTER_MODE_SSB,(select - MENU_FP_SSB_01)+1);
        break;

//    case    MENU_FP_SAM_01:
//    case    MENU_FP_SAM_02:
//    case    MENU_FP_SAM_03:
//    case    MENU_FP_SAM_04:
//        UiMenu_ChangeFilterPathMemory(var, mode, options, &clr, FILTER_MODE_SAM,(select - MENU_FP_SAM_01)+1);
//        break;
    case CONFIG_CAT_IN_SANDBOX:
        temp_var_u8 = (ts.flags1 & FLAGS1_CAT_IN_SANDBOX)? 1 : 0;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            if (temp_var_u8)
            {
                ts.flags1 |= FLAGS1_CAT_IN_SANDBOX;
            }
            else
            {
                ts.flags1 &= ~FLAGS1_CAT_IN_SANDBOX;
            }
        }
        if(!(ts.flags1 & FLAGS1_CAT_IN_SANDBOX))
        {
            ts.cat_band_index = 255;
        }
        break;
    case CONFIG_CAT_XLAT:   // CAT xlat reporting
        temp_var_u8 = ts.xlat;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)
        {
            ts.xlat = temp_var_u8;
        }
        break;
    case MENU_DEBUG_TX_AUDIO:  // Step size button swap on/off
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.stream_tx_audio,
                0,
                STREAM_TX_AUDIO_NUM-1,
                STREAM_TX_AUDIO_OFF,
                1);
        switch(ts.stream_tx_audio)
        {
        case STREAM_TX_AUDIO_OFF:
            txt_ptr = "     Off";
            break;
        case STREAM_TX_AUDIO_SRC:
            txt_ptr = "  Source";
            break;
        case STREAM_TX_AUDIO_FILT:
            txt_ptr = "Filtered";
            break;
        case STREAM_TX_AUDIO_DIGIQ:
            txt_ptr = "Final IQ";
            break;
        }
        break;

    case MENU_DEBUG_I2C1_SPEED:      //
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.i2c_speed[I2C_BUS_1],
                1,
                20,
                I2C1_SPEED_DEFAULT,
                1
        );
        if(var_change)
        {
            mchf_hw_i2c1_init();
        }
        snprintf(options, 32, " %3dkHz",(unsigned int)(ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 );
        break;
    case MENU_DEBUG_I2C2_SPEED:      //
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.i2c_speed[I2C_BUS_2],
                1,
                20,
                I2C2_SPEED_DEFAULT,
                1
        );
        if(var_change)
        {
            mchf_hw_i2c2_init();
        }
        snprintf(options, 32, " %3ukHz",(unsigned int)(ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 );
        break;

    case CONFIG_RTC_HOUR:
    {
        RTC_TimeTypeDef rtc;
        MchfRtc_GetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
        rtc.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        rtc.StoreOperation = RTC_STOREOPERATION_SET;

        var_change = UiDriverMenuItemChangeUInt8(var, mode, &rtc.Hours,
                                              0,
                                              23,
                                              0,
                                              1);
        if(var_change)      // did something change?
        {
            HAL_RTC_SetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
        }
        snprintf(options,32, "  %2u", rtc.Hours);
        break;
    }
    case CONFIG_RTC_MIN:
    {
        RTC_TimeTypeDef rtc;
        MchfRtc_GetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
        rtc.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        rtc.StoreOperation = RTC_STOREOPERATION_SET;

        var_change = UiDriverMenuItemChangeUInt8(var, mode, &rtc.Minutes,
                                              0,
                                              59,
                                              0,
                                              1);
        if(var_change)      // did something change?
        {
            HAL_RTC_SetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
        }
        snprintf(options,32, "  %2u", rtc.Minutes);
        break;
    }
    case CONFIG_RTC_SEC:
    {
        RTC_TimeTypeDef rtc;
        MchfRtc_GetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
        rtc.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        rtc.StoreOperation = RTC_STOREOPERATION_SET;

        var_change = UiDriverMenuItemChangeUInt8(var, mode, &rtc.Seconds,
                                              0,
                                              59,
                                              0,
                                              1);
        if(var_change)      // did something change?
        {
            HAL_RTC_SetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
        }
        snprintf(options,32, "  %2u", rtc.Seconds);
        break;
    }

    case CONFIG_RTC_CALIB:
     {
         var_change = UiDriverMenuItemChangeInt16(var, mode, &ts.rtc_calib,
                                               RTC_CALIB_PPM_MIN,
                                               RTC_CALIB_PPM_MAX,
                                               RTC_CALIB_PPM_DEFAULT,
                                               1);
         if(var_change)      // did something change?
         {
             MchfRtc_SetPpm(ts.rtc_calib);
         }
         snprintf(options,32, "%4dppm", ts.rtc_calib);
         break;
     }

    case CONFIG_RTC_START:
        txt_ptr = "Do it!";
        clr = White;
        if(var>=1)
        {
            MchfRtc_Start();
            mchf_reboot();
            // TODO: we will not reach this but in future we may switch the keyboard dynamically...
            txt_ptr = " Done!";
            clr = Green;
        }
        break;

    case CONFIG_RTC_RESET:
        txt_ptr = "Do it!";
        clr = White;
        if(var>=1)
        {
            MchfRtc_FullReset();

            txt_ptr = " Done!";
            clr = Green;
        }
        break;
#ifdef USE_USB
     case MENU_DEBUG_CLONEOUT:
        txt_ptr = " Do it!";
        clr = White;
        if(var>=1)
        {
            CatDriver_CloneOutStart();
            txt_ptr = "Working";
            var = 1;
            clr = Green;
        }
        break;
     case MENU_DEBUG_CLONEIN:
        txt_ptr = " Do it!";
        clr = White;
        if(var>=1)
        {
            CatDriver_CloneInStart();
            txt_ptr = "Waiting";
            var = 1;
            clr = Green;
        }
        break;
#endif
    default:                        // Move to this location if we get to the bottom of the table!
        txt_ptr = "ERROR!";
        break;
    }

    *txt_ptr_ptr = txt_ptr;
    *clr_ptr = clr;
}
