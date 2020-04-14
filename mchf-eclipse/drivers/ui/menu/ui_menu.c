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
#include <src/uhsdr_version.h>
#include "uhsdr_board.h"
#include "ui_menu.h"
#include "ui_menu_internal.h"
#include "ui_configuration.h"
#include "config_storage.h"
#include "serial_eeprom.h"
#include "ui_spectrum.h"
#include "rtc.h"
#include "uhsdr_hmc1023.h"

#include <stdio.h>
#include <stdlib.h>

#include "arm_math.h"
#include "math.h"
#include "codec.h"
#include "radio_management.h"
#include "soft_tcxo.h"
#include "cw_decoder.h"

#include "osc_si5351a.h"
#include "osc_si570.h"

#include "audio_nr.h"
#include "audio_agc.h"

#include "fm_subaudible_tone_table.h"
#include "tx_processor.h"

#define CLR_OR_SET_BITMASK(cond,value,mask) ((value) = (((cond))? ((value) | (mask)): ((value) & ~(mask))))

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

#include "uhsdr_hw_i2c.h"
#include "uhsdr_rtc.h"

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
bool __attribute__ ((noinline)) UiDriverMenuItemChangeUInt8(int var, MenuProcessingMode_t mode, volatile uint8_t* val_ptr,uint8_t val_min,uint8_t val_max, uint8_t val_default, uint8_t increment)
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

bool __attribute__ ((noinline)) UiDriverMenuItemChangeUInt32(int var, MenuProcessingMode_t mode, volatile uint32_t* val_ptr,uint32_t val_min,uint32_t val_max, uint32_t val_default, uint32_t increment)
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

bool __attribute__ ((noinline)) UiDriverMenuItemChangeInt(int var, MenuProcessingMode_t mode, volatile int* val_ptr,int val_min,int val_max, int val_default, uint32_t increment)
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

inline bool UiDriverMenuItemChangeInt32(int var, MenuProcessingMode_t mode, volatile int32_t* val_ptr,int val_min,int val_max, int val_default, uint32_t increment)
{
    return UiDriverMenuItemChangeInt(var, mode, (int*)val_ptr,val_min,val_max, val_default, increment);
}


bool __attribute__ ((noinline)) UiDriverMenuItemChangeInt16(int var, MenuProcessingMode_t mode, volatile int16_t* val_ptr,int16_t val_min,int16_t val_max, int16_t val_default, uint16_t increment)
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

bool __attribute__ ((noinline)) UiDriverMenuItemChangeOnOff(int var, MenuProcessingMode_t mode, volatile uint8_t* val_ptr, uint8_t val_default)
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
bool __attribute__ ((noinline)) UiDriverMenuItemChangeDisableOnOff(int var, MenuProcessingMode_t mode, volatile uint8_t* val_ptr, uint8_t val_default, char* options, uint32_t* clr_ptr)
{
    bool res = UiDriverMenuItemChangeOnOff(var, mode, val_ptr, val_default);
    strcpy(options, *val_ptr?"OFF":" ON");
    if (*val_ptr)
    {
        *clr_ptr = Orange;
    }

    return res;
}

bool __attribute__ ((noinline)) UiDriverMenuItemChangeEnableOnOff(int var, MenuProcessingMode_t mode, volatile uint8_t* val_ptr, uint8_t val_default, char* options, uint32_t* clr_ptr)
{
    bool res = UiDriverMenuItemChangeOnOff(var, mode, val_ptr, val_default);
    strcpy(options, *val_ptr?" ON":"OFF");
    if (!*val_ptr)
    {
        *clr_ptr = Orange;
    }

    return res;
}
bool UiDriverMenuItemChangeEnableOnOffBool(int var, MenuProcessingMode_t mode, volatile bool* val_ptr, uint8_t val_default, char* options, uint32_t* clr_ptr)
{
    uint8_t temp = *val_ptr;

    bool res = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp, val_default, options, clr_ptr);
    *val_ptr = temp;
    return res;
}
bool UiDriverMenuItemChangeEnableOnOffFlag(int var, MenuProcessingMode_t mode, volatile uint16_t* val_ptr, uint8_t val_default, char* options, uint32_t* clr_ptr, uint16_t mask)
{
    uint8_t temp = (*val_ptr & mask)?1:0;

    bool res = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp, val_default, options, clr_ptr);

    CLR_OR_SET_BITMASK(temp,*val_ptr,mask);

    return res;
}


bool __attribute__ ((noinline)) UiMenu_ChangeFilterPathMemory(int var, MenuProcessingMode_t mode, char* options, uint32_t* clr_ptr, uint16_t filter_mode,uint8_t memory_idx)
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

void UiMenu_HandleDemodModeDisable(int var, MenuProcessingMode_t mode, char* options, uint32_t* clr_ptr, uint16_t demod_mode_disable)
{
	uint8_t var_change;
    uint8_t mode_disable = 1;
    if (ts.demod_mode_disable & demod_mode_disable)
    {
  	  mode_disable = 0;
    }
    var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &mode_disable,0,options,clr_ptr);
	if (var_change)
	{
  	  if(mode_disable == false)
  	  {
        ts.demod_mode_disable |= demod_mode_disable;
  	  }
  	  else
  	  {
        ts.demod_mode_disable &= ~demod_mode_disable;
  	  }
  	}
}

void UiMenu_HandleIQAdjust(int var, MenuProcessingMode_t mode, char* options, uint32_t* clr_ptr, volatile int32_t* val_ptr, const uint16_t txrx_mode, int32_t min, int32_t max, iq_trans_idx_t valid_for)
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
        tchange = UiDriverMenuItemChangeInt32(var, mode, val_ptr,
                min,
                max,
                min,
                1);
        if(tchange)
        {
            AudioManagement_CalcIqPhaseGainAdjust(ts.tune_freq);
        }
    }
    else        // Orange if not in correct mode
    {
        *clr_ptr = Orange;
    }
    if (*val_ptr == IQ_BALANCE_OFF)
    {
        snprintf(options,32, " OFF");
    }
    else
    {
        snprintf(options,32, "%4d", (int)*val_ptr);
    }
}

void UiMenu_HandleIQAdjustGain(int var, MenuProcessingMode_t mode, char* options, uint32_t* clr_ptr, volatile int32_t* val_ptr, const uint16_t txrx_mode, iq_trans_idx_t valid_for)
{
    UiMenu_HandleIQAdjust(var, mode, options, clr_ptr, val_ptr, txrx_mode, MIN_IQ_GAIN_BALANCE, MAX_IQ_GAIN_BALANCE, valid_for);
}

void UiMenu_HandleIQAdjustPhase(int var, MenuProcessingMode_t mode, char* options, uint32_t* clr_ptr, volatile int32_t* val_ptr, const uint16_t txrx_mode, iq_trans_idx_t valid_for)
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
        { Red3,     "Red3"},
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
        outs = UiLcdHy28_DisplayInfoGet(ts.display->display_type)->name;
        break;
    }
    case INFO_DISPLAY_CTRL:
    {
        // const char* disp_com = ts.display_type==3?"parallel":"SPI";
        // snprintf(out,32,"ILI%04x %s",ts.DeviceCode,disp_com);
    	if(ts.display->DeviceCode==0x8875)
    	{
    		snprintf(out,32,"RA%04x",ts.display->DeviceCode);
    	}
    	else
    	{
    		snprintf(out,32,"ILI%04x",ts.display->DeviceCode);
    	}
        break;
    }
#ifdef USE_OSC_SI570
    case INFO_SI570:
    {
        if (osc->type == OSC_SI570) {
            float suf = Si570_GetStartupFrequency();
            int vorkomma = (int)(suf);
            int nachkomma = (int) roundf((suf-vorkomma)*10000);
            snprintf(out,32,"%xh / %u.%04u MHz",(Si570_GetI2CAddress() >> 1),vorkomma,nachkomma);
        }
        else
        {
            outs = "Not applicable";
            if (osc == NULL)
            {
            	*m_clr_ptr = Red;
            }
        }
    }
    break;
#endif
    case INFO_OSC_NAME:
    {
        if (osc->isPresent) {
        	outs = osc->name;
        }
        else
        {
            outs = "Not present";
            *m_clr_ptr = Red;
        }
    }
    break;
    case INFO_TP:
        outs = (ts.tp->present == 0)?"n/a":"XPT2046";
        break;
    case INFO_RFBOARD:
        switch(ts.rf_board)
        {
        case FOUND_RF_BOARD_OVI40:
            outs = "OVI40 RF Board";
            break;
        default:
            outs = "mcHF RF Board";
            break;
        }
        break;
    case INFO_FLASH:
            snprintf(out,32,"%d",(STM32_GetFlashSize()));
            break;
    case INFO_CPU:
            snprintf(out,32,"%3lx:%04lxh",HAL_GetDEVID(),HAL_GetREVID());
            break;
    case INFO_RAM:
            snprintf(out,32,"%d",(ts.ramsize));
            break;
    case INFO_EEPROM:
    {
        const char* label = "";
        switch(ts.configstore_in_use)
         {
         case CONFIGSTORE_IN_USE_I2C:
             label = " [used]";
             *m_clr_ptr = Green;
             break; // in use & ok
         case CONFIGSTORE_IN_USE_ERROR: // not ok
             label = " [error]";
             *m_clr_ptr = Red;
             break;
		 default:
            label = " [not used]";
            if (ts.ser_eeprom_type >= SERIAL_EEPROM_DESC_REAL &&
                    SerialEEPROM_eepromTypeDescs[ts.ser_eeprom_type].size < SERIAL_EEPROM_MIN_USEABLE_SIZE)
            {
                label = " [too small]";
            }

             *m_clr_ptr = Red;
         }

        const char* i2c_size_unit = "K";
        uint i2c_size = 0;

        if(ts.ser_eeprom_type < SERIAL_EEPROM_DESC_NUM)
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
        outs = Board_BootloaderVersion();
    }
    break;
    case INFO_FW_VERSION:
    {
  		#ifdef OFFICIAL_BUILD
  		  #ifdef IS_SMALL_BUILD
      		snprintf(out,32, "S%s", UHSDR_VERSION+4);
		  #else
      		snprintf(out,32, "D%s", UHSDR_VERSION+4);
		  #endif
		#else
			snprintf(out,32, "%s", UHSDR_VERSION+4);
    	#endif
    }
    break;
    case INFO_BUILD:
    {
        snprintf(out,32, "%s", UHSDR_BUILD_DAT+4);
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
    case INFO_CODEC:
    {
        snprintf(out,32, "%s", ts.codec_present?"Yes":"N/A");
    }
    break;
    case INFO_CODEC_TWINPEAKS:
    {
        if (ts.iq_auto_correction == 1)
        {
            switch (ts.twinpeaks_tested)
            {
            case TWINPEAKS_UNCORRECTABLE:
                outs = "Failed";
                break;
            case TWINPEAKS_DONE:
                outs = "Done";
                break;
            default:
                outs = "Not done";
            }
        }
        else
        {
            outs = "Deactivated"; // IQ correction is off, Twinpeaks auto correction cannot be done.
        }
    }
    break;
    case INFO_LICENCE:
    {
        snprintf(out,32, "%s", UHSDR_LICENCE);
    }
    break;

#ifdef TRX_HW_LIC
    case INFO_HWLICENCE:
    {
        snprintf(out,32, "%s", TRX_HW_LIC);
    }
    break;
#endif
    default:
        outs = "NO INFO";
    }
    if (outs == NULL) {
        outs = out;
    }
    return outs;
}



bool __attribute__ ((noinline)) UiDriverMenuBandPowerAdjust(int var, MenuProcessingMode_t mode, uint8_t band_mode, uint8_t pa_level, char* options, uint32_t* clr_ptr)
{
     volatile uint8_t* adj_ptr = &ts.pwr_adj[pa_level == PA_LEVEL_FULL?ADJ_FULL_POWER:ADJ_REF_PWR][band_mode];

    bool tchange = false;
    const BandInfo* band = RadioManagement_GetBand(df.tune_old);
    if((band_mode == band->band_mode) && (ts.power_level == pa_level))
    {
        tchange = UiDriverMenuItemChangeUInt8(var, mode, adj_ptr,
                                              TX_POWER_FACTOR_MIN,
                                              RadioManagement_IsPowerFactorReduce(df.tune_old)?TX_POWER_FACTOR_MAX:TX_POWER_FACTOR_MAX/4,
                                              TX_POWER_FACTOR_MIN,
                                              1
                                             );

        if(tchange)	 		// did something change?
        {
            RadioManagement_SetPowerLevel(band, pa_level);	// yes, update the power factor
        }
    }
    else
    {
        // not enabled
        *clr_ptr = Orange;
    }

    sprintf(options, "  %u", *adj_ptr);
    return tchange;
}

bool __attribute__ ((noinline))  UiDriverMenuBandRevCouplingAdjust(int var, MenuProcessingMode_t mode, uint8_t coupling_band, char* options, uint32_t* clr_ptr)
{
    bool tchange = false;
    volatile uint8_t *adj_ptr = &swrm.coupling_calc[coupling_band];
    if(ts.coupling_band == coupling_band)	 	// is this band selected?
    {
        tchange = UiDriverMenuItemChangeUInt8(var, mode, adj_ptr,
                                              SWR_COUPLING_MIN,
                                              SWR_COUPLING_MAX,
                                              SWR_COUPLING_DEFAULT,
                                              1
                                             );
    }
    if((ts.txrx_mode != TRX_MODE_TX) || (ts.coupling_band != coupling_band))	// Orange if not in TX mode or NOT on this band
        *clr_ptr = Orange;
    sprintf(options, "  %u", *adj_ptr);
    return tchange;
}


/**
 *
 * @param select
 * @param mode selects if the entry is just to be displayed or can be changed.
 * @param pos which line the menu item is being displayed on, used for positioning value display in some cases.
 * @param var the actual menu item to change / display
 * @param options the to be displayed value is returned as a string in this array. Shown unless txt_ptr_ptr does not point to a NULL ptr
 * @param txt_ptr_ptr this is a return value. if the address in the referenced pointer is not NULL, this is displayed, not options.
 * @param clr_ptr pointer to the value display color
 */
void UiMenu_UpdateItem(uint16_t select, MenuProcessingMode_t mode, int pos, int var, char* options, const char** txt_ptr_ptr, uint32_t* clr_ptr)
{

    const char* txt_ptr = NULL;
    uint32_t clr = *clr_ptr;
	uint8_t nr_step;

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
    	nr_step = DSP_NR_STRENGTH_STEP;
    	if(ts.dsp.nr_strength >= 190 || ts.dsp.nr_strength <= 10)
    	{
    		nr_step = 1;
    	}
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.nr_strength,
                                              DSP_NR_STRENGTH_MIN,
                                              DSP_NR_STRENGTH_MAX,
                                              DSP_NR_STRENGTH_DEFAULT,
											  nr_step
                                             );
        if(var_change)
        {
        	if(ts.dsp.nr_strength == 189)
        	{
        		ts.dsp.nr_strength = 185;
        	}
        	if(ts.dsp.nr_strength == 11)
        	{
        		ts.dsp.nr_strength = 15;
        	}
        	// did it change?
            if(ts.dsp.active & DSP_NR_ENABLE)   // only change if DSP active
            {
				// this causes considerable noise
				//AudioDriver_SetRxAudioProcessing(ts.dmod_mode, false);
				// we do this instead
			    nr_params.alpha = 0.799 + ((float32_t)ts.dsp.nr_strength / 1000.0);
            }
        }
#ifdef OBSOLETE_NR
        if(!(ts.dsp.active & DSP_NR_ENABLE))    // make red if DSP not active
        {
            clr = Orange;
        }
        else
        {
            if(ts.dsp.nr_strength >= DSP_STRENGTH_RED)
                clr = Red;
            else if(ts.dsp.nr_strength >= DSP_STRENGTH_ORANGE)
                clr = Orange;
            else if(ts.dsp.nr_strength >= DSP_STRENGTH_YELLOW)
                clr = Yellow;
        }
#endif
        //
        snprintf(options,32, "  %u", ts.dsp.nr_strength);
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
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_SAM_ENABLE);
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
            var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags2,0,options,&clr, FLAGS2_FM_MODE_ENABLE);
            break;
        case MENU_FM_GEN_SUBAUDIBLE_TONE:   // Selection of subaudible tone for FM transmission
            var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.fm_subaudible_tone_gen_select,
                                     0,
                                     NUM_SUBAUDIBLE_TONES,
                                     FM_SUBAUDIBLE_TONE_OFF,
                                     1
                                    );
            if (var_change)
            {
                AudioManagement_CalcSubaudibleGenFreq(fm_subaudible_tone_table[ts.fm_subaudible_tone_gen_select]);
            }

        if(ts.fm_subaudible_tone_gen_select != FM_SUBAUDIBLE_TONE_OFF)        // tone select not zero (tone activated
        {
            int a = (int)(ads.fm_conf.subaudible_tone_gen_freq * 10);        // convert to integer, Hz*10
            snprintf(options,32, "%d.%01dHz", a/10, a%10);
        }
        else                                // tone is off
        {
            snprintf(options,32, "     OFF");       // make it dislay "off"
        }

        if(ts.dmod_mode != DEMOD_FM)    // make orange if we are NOT in FM mode
        {
            clr = Orange;
        }
        else if(ads.fm_conf.subaudible_tone_det_freq > 200)      // yellow for tones above 200 Hz as they are more audible
        {
            clr = Yellow;
        }
        break;
    case MENU_FM_DET_SUBAUDIBLE_TONE:   // Selection of subaudible tone for FM reception
        UiDriverMenuItemChangeUInt32(var, mode, &ts.fm_subaudible_tone_det_select,
                                     0,
                                     NUM_SUBAUDIBLE_TONES,
                                     FM_SUBAUDIBLE_TONE_OFF,
                                     1
                                    );

        AudioManagement_CalcSubaudibleDetFreq(fm_subaudible_tone_table[ts.fm_subaudible_tone_det_select]);
        if(ts.fm_subaudible_tone_det_select)        // tone select not zero (tone activated
        {
                  // calculate frequency word
            int a = (int)(ads.fm_conf.subaudible_tone_det_freq * 10);        // convert to integer, Hz*10
            snprintf(options,32, "%d.%01dHz", a/10, a%10);
        }
        else                                // tone is off
        {
            snprintf(options,32, "     OFF");       // make it dislay "off"
        }

        if(ts.dmod_mode != DEMOD_FM)    // make orange if we are NOT in FM
        {
            clr = Orange;
        }
        else if(ads.fm_conf.subaudible_tone_det_freq > 200)      // yellow for tones above 200 Hz as they are more audible
        {
            clr = Yellow;
        }
        break;
    case MENU_FM_TONE_BURST_MODE:
        UiDriverMenuItemChangeUInt8(var, mode, &ts.fm_tone_burst_mode,
                                    0,
                                    FM_TONE_BURST_MAX,
                                    FM_TONE_BURST_OFF,
                                    1
                                   );

        ads.fm_conf.tone_burst_active = 0;           // make sure it is turned off
        AudioManagement_LoadToneBurstMode();    // activate setting

        if (ts.fm_tone_burst_mode != FM_TONE_BURST_OFF)
        {
            snprintf(options, 32, "  %lu Hz", fm_tone_burst_freq[ts.fm_tone_burst_mode]);
        }
        else
        {
            txt_ptr = "    OFF";
        }

        if(ts.dmod_mode != DEMOD_FM)    // make orange if we are NOT in FM
        {
            clr = Orange;
        }
        break;

      case MENU_FM_DEV_MODE:  // Select +/- 2.5 or 5 kHz deviation on RX and TX
        if(ts.iq_freq_mode)
        {
            temp_var_u8 = RadioManagement_FmDevIs5khz();
            var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
            if(var_change)
            {
                RadioManagement_FmDevSet5khz(temp_var_u8 != 0); // band up/down swap is to be enabled
            }

            if(RadioManagement_FmDevIs5khz())               // Check state of bit indication 2.5/5 kHz
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
#if 0
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
#endif
    case MENU_AGC_WDSP_MODE: // AGC mode
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &agc_wdsp_conf.mode,
                                              0, //
                                              5,
                                              2,
                                              1
                                             );
        switch(agc_wdsp_conf.mode) {
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
            agc_wdsp_conf.switch_mode = 1; // set flag to 1 for parameter change
            AudioDriver_AgcWdsp_Set();
            UiMenu_RenderMenu(MENU_RENDER_ONLY);
        }
        if(ts.txrx_mode == TRX_MODE_TX) // Orange if in TX mode
        {
            clr = Orange;
        }
        break;

    case MENU_AGC_WDSP_SLOPE:      //
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &agc_wdsp_conf.slope,
                                            0,
                                            200,
                                            40,
                                            10
                                           );
        if(var_change)
        {
            AudioDriver_AgcWdsp_Set();
        }
        snprintf(options, 32, "  %ddB", agc_wdsp_conf.slope / 10);
        break;

    case MENU_AGC_WDSP_THRESH:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &agc_wdsp_conf.thresh,
                                            -20,
                                            120,
                                            40,
                                            1
                                           );
        if(var_change)
        {
            AudioDriver_AgcWdsp_Set();
        }
        snprintf(options, 32, "  %ddB", agc_wdsp_conf.thresh);
        break;

    case MENU_AGC_WDSP_HANG_THRESH:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &agc_wdsp_conf.hang_thresh,
                                            -20,
                                            120,
                                            40,
                                            1
                                           );
        if(var_change)
        {
            AudioDriver_AgcWdsp_Set();
        }
        snprintf(options, 32, "  %ddB", agc_wdsp_conf.hang_thresh);
        break;

    case MENU_AGC_WDSP_TAU_DECAY:      //
       var_change = UiDriverMenuItemChangeInt(var, mode, &agc_wdsp_conf.tau_decay[agc_wdsp_conf.mode],
                                           100,
                                           5000,
                                           1000,
                                           100
                                          );
       if(var_change)
       {
           AudioDriver_AgcWdsp_Set();
       }
       snprintf(options, 32, "  %ums", agc_wdsp_conf.tau_decay[agc_wdsp_conf.mode]);
       break;

    case MENU_AGC_WDSP_TAU_HANG_DECAY:      //
       var_change = UiDriverMenuItemChangeInt(var, mode, &agc_wdsp_conf.tau_hang_decay,
                                           100,
                                           5000,
                                           1000,
                                           100
                                          );
       if(var_change)
       {
           AudioDriver_AgcWdsp_Set();
       }
       snprintf(options, 32, "  %ums", agc_wdsp_conf.tau_hang_decay);
       break;

     case MENU_DBM_CALIBRATE:      //
        var_change = UiDriverMenuItemChangeInt32(var, mode, &ts.dbm_constant,
                                            -100,
                                            100,
                                            0,
                                            1
                                           );
        snprintf(options, 32, "  %lddB", ts.dbm_constant);
        break;

     case MENU_UI_INVERSE_SCROLLING:      //
         var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags2,0,options,&clr,FLAGS2_UI_INVERSE_SCROLLING);
         if(var_change)
         {
             UiMenu_RenderMenu(MENU_RENDER_ONLY);
         }
         break;

    case MENU_AGC_WDSP_HANG_TIME:      //
        var_change = UiDriverMenuItemChangeInt(var, mode, &agc_wdsp_conf.hang_time,
                                            10,
                                            5000,
                                            250,
                                            10
                                           );
        if(var_change)
        {
            AudioDriver_AgcWdsp_Set();
        }
        snprintf(options, 32, "  %dms", agc_wdsp_conf.hang_time);
        break;
#if 0
        case MENU_AGC_WDSP_SWITCH:     // Enable/Disable wdsp AGC
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &agc_wdsp_conf,
                                                  0,
                                                  1,
                                                  0,
                                                  1
                                                 );
            switch(agc_wdsp_conf)
            {
            case 1:       //
                txt_ptr = "    WDSP AGC";        //
            break;
            default:
                txt_ptr = "Standard AGC";        //
            }
            break;
#endif
            case MENU_AGC_WDSP_HANG_ENABLE:     //
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &agc_wdsp_conf.hang_enable,
                                                  0,
                                                  1,
                                                  0,
                                                  1
                                                 );
            switch(agc_wdsp_conf.hang_enable)
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
#if 0
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
#endif
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
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.nb_setting,
                                              0,
                                              MAX_NB_SETTING,
                                              10,
                                              1
                                             );
        clr = UiDriver_GetNBColor();
        snprintf(options,32,"   %u", ts.dsp.nb_setting);

        break;
    case MENU_RX_FREQ_CONV:     // Enable/Disable receive frequency conversion
  		;
  		uchar firstmode = FREQ_IQ_CONV_MODE_OFF;
		if(ts.dmod_mode == DEMOD_AM || ts.dmod_mode == DEMOD_SAM || ts.dmod_mode == DEMOD_FM)
		{
		  firstmode = FREQ_IQ_CONV_P6KHZ;
		}
        var_change = UiDriverMenuItemChangeInt32(var, mode, &ts.iq_freq_mode,
                                              firstmode,
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
        break;
    case MENU_MIC_TYPE:    // selecting a mic type determines a mic boost level (for now: one of two boost gains)
        UiDriverMenuItemChangeUInt8(var, mode, &ts.tx_mic_boost,
                                    0,
                                    MIC_BOOST_DYNAMIC, // 14 dB (Dynamic)
                                    MIC_BOOST_DEFAULT, //  0 dB (Electret)
                                    1
                                   );
        //
        if(ts.tx_mic_boost > 0)
		{
            txt_ptr = " Dynamic";
		}
		else
		{
            txt_ptr = "Electret";
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
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.cw_keyer_mode,
                                              0,
                                              CW_KEYER_MODE_ULTIMATE,
                                              CW_KEYER_MODE_IAM_B,
                                              1
                                             );

        switch(ts.cw_keyer_mode)
        {
        case CW_KEYER_MODE_IAM_B:
            txt_ptr = "IAM_B";
            break;
        case CW_KEYER_MODE_IAM_A:
            txt_ptr = "IAM_A";
            break;
        case CW_KEYER_MODE_STRAIGHT:
            txt_ptr = "STR_K";
            break;
        case CW_KEYER_MODE_ULTIMATE:
            txt_ptr = "ULTIM";
            break;
        }
        break;

    case MENU_KEYER_SPEED:  // keyer speed
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.cw_keyer_speed,
                                              CW_KEYER_SPEED_MIN,
                                              CW_KEYER_SPEED_MAX,
                                              CW_KEYER_SPEED_DEFAULT,
                                              1
                                             );

        if(var_change && ts.dmod_mode == DEMOD_CW)         // did it change?
        {
            CwGen_SetSpeed(); // make sure keyerspeed is being used
            UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
        }
        snprintf(options,32, "  %u", ts.cw_keyer_speed);
        break;

    case MENU_KEYER_WEIGHT:  // keyer weight
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.cw_keyer_weight,
                                              CW_KEYER_WEIGHT_MIN,
                                              CW_KEYER_WEIGHT_MAX,
                                              CW_KEYER_WEIGHT_DEFAULT,
                                              1
                                             );

        if(var_change && ts.dmod_mode == DEMOD_CW)         // did it change?
        {
            CwGen_SetSpeed(); // make sure keyerspeed is being used
            UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
        }
        snprintf(options,32, "  %u.%02u", ts.cw_keyer_weight/100,ts.cw_keyer_weight%100);
        break;

    case MENU_SIDETONE_GAIN:    // sidetone gain
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.cw_sidetone_gain,
                                              0,
                                              SIDETONE_MAX_GAIN,
                                              DEFAULT_SIDETONE_GAIN,
                                              1
                                             );
        if(var_change && ts.dmod_mode == DEMOD_CW)          // did it change?
        {
            UiDriver_RefreshEncoderDisplay(); // maybe shown on encoder boxes
        }
        snprintf(options,32, "  %u", ts.cw_sidetone_gain);
        break;
    case MENU_SIDETONE_FREQUENCY:   // sidetone frequency
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.cw_sidetone_freq,
                                               CW_SIDETONE_FREQ_MIN,
                                               CW_SIDETONE_FREQ_MAX*10,
                                               CW_SIDETONE_FREQ_DEFAULT,
                                               10
                                              );

        if(var_change && ts.dmod_mode == DEMOD_CW)         // did it change?
        {
            float freq[2] = { ts.cw_sidetone_freq, 0.0 };

            softdds_configRunIQ(freq,ts.samp_rate,0);
            UiDriver_FrequencyUpdateLOandDisplay(false);
            CwDecode_Filter_Set();
        }
        snprintf(options,32, "  %uHz", (uint)ts.cw_sidetone_freq);
        break;

    case MENU_PADDLE_REVERSE:   // CW Paddle reverse
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &ts.cw_paddle_reverse,0,options,&clr);
        break;
    case MENU_CW_TX_RX_DELAY:   // CW TX->RX delay
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.cw_rx_delay,
                                              0,
                                              CW_RX_DELAY_MAX,
                                              CW_TX2RX_DELAY_DEFAULT,
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
            UiDriver_DisplayDemodMode();
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
            UiDriver_DisplayDemodMode();
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

    case MENU_CW_DECODER_THRESH:   //
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &cw_decoder_config.thresh,
                                               CW_DECODER_THRESH_MIN,
                                               CW_DECODER_THRESH_MAX,
                                               CW_DECODER_THRESH_DEFAULT,
                                               500
                                              );
        snprintf(options,32, "  %u", (unsigned int)cw_decoder_config.thresh);
        break;


//ts.cw_decode_average - wieviele Goertzel-Sch�tzwerte werden in einem
//moving window zusammengefasst [uint8_t 1 - 20]
#if 0
    case MENU_CW_DECODER_AVERAGE:  // averager for CW decode [1 - 20]
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &cw_decoder_config.average,
                                              2,
                                              20,
                                              2,
                                              1
                                             );
        if(cw_decoder_config.average !=1)
        {
            snprintf(options,32, "  %u", cw_decoder_config.average);
        }
        else
        {
            txt_ptr = "OFF";
        }
        break;
#endif
    case MENU_CW_DECODER_BLOCKSIZE:  // processing block size for cw decoder
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &cw_decoder_config.blocksize,
                                              CW_DECODER_BLOCKSIZE_MIN,
											  CW_DECODER_BLOCKSIZE_MAX,
											  CW_DECODER_BLOCKSIZE_DEFAULT,
                                              8
                                             );
            snprintf(options,32, "  %u", cw_decoder_config.blocksize);
            CwDecode_Filter_Set();
        break;

#if 0
        case MENU_CW_DECODER_AGC:    // On/Off
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &cw_decoder_config.AGC_enable,
                                                  0,
                                                  1,
                                                  0,
                                                  1
                                                 );

            switch(cw_decoder_config.AGC_enable) {
            case 0:
                txt_ptr = " OFF";
                break;
            case 1:
                txt_ptr = "  ON";
                break;
            }
            break;
#endif
            case MENU_CW_DECODER_NOISECANCEL:    // On/Off
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &cw_decoder_config.noisecancel_enable,
                                                  0,
                                                  1,
                                                  1,
                                                  1
                                                 );

            switch(cw_decoder_config.noisecancel_enable) {
            case 0:
                txt_ptr = " OFF";
                break;
            case 1:
                txt_ptr = "  ON";
                break;
            }
            break;
        case MENU_CW_DECODER_SPIKECANCEL:    // On/Off
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &cw_decoder_config.spikecancel,
                                                  0,
                                                  2,
                                                  0,
                                                  1
                                                 );

            switch(cw_decoder_config.spikecancel)
            {
            case 0:
                txt_ptr = "  OFF";
                break;
            case 1:
                txt_ptr = "SPIKE";
                break;
            case 2:
                txt_ptr = "SHORT";
                break;
            }
            break;
    case MENU_TCXO_MODE:    // TCXO On/Off
        temp_var_u8 = RadioManagement_TcxoGetMode();     // get current setting without upper nibble
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                0,
                TCXO_STATE_NUMBER-1,
                                              TCXO_ON,
                                              1
                                             );

        if(lo.sensor_present == false)            // no sensor present
        {
            temp_var_u8 = TCXO_OFF; // force TCXO disabled
            var_change = true;
        }

        RadioManagement_TcxoSetMode(temp_var_u8);   // overlay new temperature setting with old status of upper nibble
        if(var_change)
        {
            UiDriver_CreateTemperatureDisplay();
        }

        switch(temp_var_u8)
        {
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

            switch(ts.iq_auto_correction)
            {
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
                                              SPECTRUM_SCOPE_SPEED_MIN,
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
    case MENU_SCOPE_TRACE_HL_COLOUR: //spectrum highlighted bandwidth colour
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.scope_trace_BW_colour,
                                              0,
                                              SPEC_MAX_COLOUR,
											  SPEC_COLOUR_TRACEBW_DEFAULT,
                                              1
                                             );
        UiMenu_MapColors(ts.scope_trace_BW_colour,options,&clr);
        break;
    case MENU_SCOPE_BACKGROUND_HL_COLOUR:  // set step size of of waterfall display?
         UiDriverMenuItemChangeUInt8(var, mode, &ts.scope_backgr_BW_colour,
                                     0,
                                     100,
									 SPEC_COLOUR_BACKGRBW_DEFAULT,
                                     10
                                    );
         snprintf(options,32, "  %u%%", ts.scope_backgr_BW_colour);
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
        UiDriver_SpectrumChangeLayoutParameters();
        break;
    case MENU_SCOPE_AGC_ADJUST: // Spectrum scope AGC adjust
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.spectrum_agc_rate,
                                              SPECTRUM_SCOPE_AGC_MIN,
                                              SPECTRUM_SCOPE_AGC_MAX,
                                              SPECTRUM_SCOPE_AGC_DEFAULT,
                                              1
                                             );

        if(var_change)          // update system variable if rate changed
        {
            sd.agc_rate = (float)ts.spectrum_agc_rate; // calculate agc rate
            sd.agc_rate = sd.agc_rate/SPECTRUM_AGC_SCALING;
        }
        snprintf(options,32, "  %u", ts.spectrum_agc_rate);
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
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr,FLAGS1_SCOPE_LIGHT_ENABLE);
        break;
    case MENU_SPECTRUM_MODE:
        temp_var_u8 = UiDriver_GetSpectrumMode();

        var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                                    SPECTRUM_WATERFALL,
                                    SPECTRUM_DUAL,
                                    SPECTRUM_DUAL,
                                    1
                                   );
        switch(temp_var_u8)
        {
        case SPECTRUM_DUAL:
            txt_ptr = " DUAL";
            break;
        case SPECTRUM_SCOPE:
            txt_ptr = "SCOPE";
            break;
        case SPECTRUM_WATERFALL:
            txt_ptr = "WFALL";
            break;
        case SPECTRUM_BLANK:
            txt_ptr = "  OFF";
            break;
        }
        UiDriver_SetSpectrumMode(temp_var_u8);
        UiSpectrum_ResetSpectrum();
        break;
    case MENU_WFALL_COLOR_SCHEME:   // Adjustment of dB/division of spectrum scope
        UiDriverMenuItemChangeUInt8(var, mode, &ts.waterfall.color_scheme,
                                    WATERFALL_COLOR_MIN,
                                    WATERFALL_COLOR_MAX,
                                    WATERFALL_COLOR_DEFAULT,
                                    1
                                   );
        switch(ts.waterfall.color_scheme)       // convert variable to setting
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
#ifdef USE_8bit_FONT
    case MENU_FREQ_FONT:
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.FreqDisplayFont,
                                              0,
                                              1,
                                              0,
                                              1
                                             );
        switch(ts.FreqDisplayFont)
        {
        case 0:
            txt_ptr = "     old";       //old font
            break;
        case 1:
            txt_ptr = "  modern";       //old font
            break;
        }

        UiDriver_FrequencyUpdateLOandDisplay(true);
        break;
#endif
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
    case MENU_WFALL_STEP_SIZE:  // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt8(var, mode, &ts.waterfall.vert_step_size,
                                    WATERFALL_STEP_SIZE_MIN,
                                    WATERFALL_STEP_SIZE_MAX,
                                    WATERFALL_STEP_SIZE_DEFAULT,
                                    1
                                   );
        snprintf(options,32, "  %u", ts.waterfall.vert_step_size);
        break;
#if 0
    case MENU_WFALL_OFFSET: // set step size of of waterfall display?
        UiDriverMenuItemChangeInt32(var, mode, &ts.waterfall.offset,
                                     WATERFALL_OFFSET_MIN,
                                     WATERFALL_OFFSET_MAX,
                                     WATERFALL_OFFSET_DEFAULT,
                                     1
                                    );
        snprintf(options,32, "  %u", (unsigned int)ts.waterfall.offset);
        break;
#endif
    case MENU_WFALL_CONTRAST:   // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt32(var, mode, &ts.waterfall.contrast,
                                     WATERFALL_CONTRAST_MIN,
                                     WATERFALL_CONTRAST_MAX,
                                     WATERFALL_CONTRAST_DEFAULT,
                                     2
                                    );
        snprintf(options,32, "  %u", (unsigned int)ts.waterfall.contrast);
        break;
    case MENU_WFALL_SPEED:  // set step size of of waterfall display?
        UiDriverMenuItemChangeUInt8(var, mode, &ts.waterfall.speed,
                WATERFALL_SPEED_MIN,
                WATERFALL_SPEED_MAX,
                WATERFALL_SPEED_DEFAULT,
                1
        );

        if(ts.waterfall.speed <= WATERFALL_SPEED_WARN)
        {
            clr = Red;
        }
        else if(ts.waterfall.speed <= WATERFALL_SPEED_WARN1)
        {
            clr = Yellow;
        }

        if (ts.waterfall.speed > 0)
        {
            snprintf(options,32, "  %u", ts.waterfall.speed);
        }
        else
        {
            txt_ptr = "OFF";
        }
        break;
#if 0
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
        UiDriverMenuItemChangeUInt8(var, mode, &ts.waterfall.nosig_adjust,
                                    WATERFALL_NOSIG_ADJUST_MIN,
                                    WATERFALL_NOSIG_ADJUST_MAX,
                                    WATERFALL_NOSIG_ADJUST_DEFAULT,
                                    1
                                   );
        snprintf(options,32, "  %u", ts.waterfall.nosig_adjust);
        break;
#endif
    case MENU_SPECTRUM_SIZE:    // set view size of of spectrum display (big disables title bar)?
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
#ifdef USE_CONFIGSTORAGE_FLASH
    case MENU_BACKUP_CONFIG:
      	txt_ptr = "n/a";
        if(ts.configstore_in_use == CONFIGSTORE_IN_USE_I2C)
        {
		  uint8_t test = 0;
      	  clr = White;
      	  UiDriverMenuItemChangeUInt8(var, mode, &test,
                                              0,
                                              0,
                                              10,
                                              1
                                             );
      	  switch(test)
      	  {
      		case 0:
          	  txt_ptr = "press DEFLT";
          	  break;
      		case 10:
          	  UiMenu_DisplayValue("    Working",Red,pos);
              ConfigStorage_CopySerial2Flash();
    		  clr = Green;
          	  txt_ptr = "    Done...";
          	  break;
      	  }
		}
        break;
    case MENU_RESTORE_CONFIG:
        txt_ptr = "n/a";
        if(ts.configstore_in_use == CONFIGSTORE_IN_USE_I2C)
        {
		  uint8_t test = 0;
      	  clr = White;
      	  UiDriverMenuItemChangeUInt8(var, mode, &test,
                                              0,
                                              0,
                                              10,
                                              1
                                             );
      	  switch(test)
      	  {
      		case 0:
          	  txt_ptr = "press DEFLT";
          	  break;
      		case 10:
          	  UiMenu_DisplayValue("    Working",Red,pos);
              ConfigStorage_CopyFlash2Serial();
              Board_Reboot();
          	  break;
      	  }
		}
        break;
#endif
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
    case MENU_DEBUG_TWINPEAKS_CORR_RUN:
        if (ts.iq_auto_correction == 0)
        {
            txt_ptr = "Not possible";
            clr = Red;
        }
        else
        {
            txt_ptr = "  Do it!";
            clr = White;
        }
        if(var>=1)
        {

            UiMenu_DisplayValue(" Started",Green, pos);
            ts.twinpeaks_tested = TWINPEAKS_WAIT;
            while(ts.twinpeaks_tested != TWINPEAKS_DONE && ts.twinpeaks_tested != TWINPEAKS_UNCORRECTABLE ) { asm("nop");}
            UiMenu_DisplayValue("Finished",Green, pos);
            var = 0;
        }
        break;
    case CONFIG_FREQ_STEP_MARKER_LINE:  // Frequency step marker line on/off
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.freq_step_config,0,options,&clr, FREQ_STEP_SHOW_MARKER);
        if(var_change)          // something changed?
        {
            UiDriver_DisplayFreqStepSize();  // update screen
        }
        break;
    case CONFIG_STEP_SIZE_BUTTON_SWAP:  // Step size button swap on/off
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.freq_step_config,0,options,&clr, FREQ_STEP_SWAP_BTN);
        break;
    case MENU_DYNAMICTUNE:  // Dynamic Tune on/off
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_DYN_TUNE_ENABLE);
        if(var_change)
        {
            UiDriver_DisplayFreqStepSize();
        }
        break;
    case CONFIG_BAND_BUTTON_SWAP:   // Swap position of Band+ and Band- buttons
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_SWAP_BAND_BTN);
        break;
    case CONFIG_BANDEF_SELECT:
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &bandinfo_idx,0,BAND_INFO_SET_NUM-1,0,1);
        if (var_change)
        {
            bandInfo = bandInfos[bandinfo_idx].bands;
            UiDriver_UpdateDisplayAfterParamChange();
        }
        snprintf(options,32,"     %s",bandInfos[bandinfo_idx].name);
        break;
    case CONFIG_TX_DISABLE: // Step size button swap on/off
    {
        uint16_t flag = ts.tx_disable;
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &flag,0,options,&clr, TX_DISABLE_ALWAYS);
        ts.tx_disable = flag;
        if(var_change)
        {
            // FIXME: Call "abstract" function to update status of tune,
            // do not redraw menu button here directly
            UiDriver_DrawFButtonLabel(5,"TUNE",ts.tx_disable?Grey1:White);
        }
    }
        break;
    case CONFIG_TX_OUT_ENABLE:      // Enable transmitting outside HAM bands
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_TX_OUTSIDE_BANDS);
        break;
    case CONFIG_AUDIO_MAIN_SCREEN_MENU_SWITCH:  // AFG/(STG/CMP) and RIT/(WPM/MIC/LIN) are to change automatically with TX/RX
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_TX_AUTOSWITCH_UI_DISABLE);
        break;
    case CONFIG_MUTE_LINE_OUT_TX:   // Enable/disable MUTE of TX audio on LINE OUT
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_MUTE_LINEOUT_TX);
        if(ts.iq_freq_mode) // Mark RED if translate mode is active
        {
            clr = Red;
        }
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
    case MENU_LOW_POWER_SHUTDOWN:   // Auto shutdown when below low voltage threshold
        temp_var_u8 = (ts.low_power_config & LOW_POWER_ENABLE_MASK) == LOW_POWER_ENABLE? 1 : 0 ;        // get control variable
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8, 0,options,&clr);
        if (var_change)
        {
            CLR_OR_SET_BITMASK(temp_var_u8, ts.low_power_config, LOW_POWER_ENABLE);
        }
        break;

    case CONFIG_LOW_POWER_THRESHOLD:  // Configure low voltage threshold

        temp_var_u8 = ts.low_power_config & LOW_POWER_THRESHOLD_MASK;        // get control variable

        var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                                              LOW_POWER_THRESHOLD_MIN,
                                              LOW_POWER_THRESHOLD_MAX,
                                              LOW_POWER_THRESHOLD_DEFAULT,
                                              1
                                             );
        if(var_change)
        {
            ts.low_power_config = (ts.low_power_config & ~LOW_POWER_THRESHOLD_MASK) | (temp_var_u8 & LOW_POWER_THRESHOLD_MASK);

        }

        snprintf(options,32,"%2d.%dV",(temp_var_u8 + LOW_POWER_THRESHOLD_OFFSET) / 10, (temp_var_u8 + LOW_POWER_THRESHOLD_OFFSET) % 10);

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
        if(ts.rx_gain[RX_AUDIO_SPKR].max <= MAX_VOLUME_RED_THRESH)         // Indicate that gain has been reduced by changing color
            clr = Red;
        else if(ts.rx_gain[RX_AUDIO_SPKR].max <= MAX_VOLUME_YELLOW_THRESH)
            clr = Orange;
        break;
#if 0
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
#endif
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
                AudioManagement_KeyBeepPrepare();
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
            if(ts.beep_loudness)
            {
                ts.flags2 |= FLAGS2_KEY_BEEP_ENABLE;
                AudioManagement_KeyBeepPrepare();
                AudioManagement_KeyBeep();      // make beep to demonstrate loudness
            }
            else
            {
                ts.flags2 &= ~FLAGS2_KEY_BEEP_ENABLE;
            }
        }

        if(ts.beep_loudness)
        {
            snprintf(options,32, "    %u", ts.beep_loudness);
        }
        else
        {
            snprintf(options,32, "%s", "OFF");
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
            osc->setPPM(((float32_t)ts.freq_cal)/10.0);
            // Update LO PPM (will automatically adjust frequency)
            // this is a little trick
            // FIXME: Use a better approach to trigger retuning
            df.temp_factor_changed = true;
        }
        {
            char numstr[16];
            float2fixedstr(numstr, 16, ((float32_t)ts.freq_cal)/10.0, 4, 1);
            snprintf(options,32, "%sppm", numstr );
        }
        break;
    //
    case CONFIG_FREQ_LIMIT_RELAX:   // Enable/disable Frequency tuning limits
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_FREQ_LIMIT_RELAX);

        if(ts.flags1 & FLAGS1_FREQ_LIMIT_RELAX)             // tuning limit is disabled
        {
            clr = Orange;                   // warn user!
        }
        break;
    case CONFIG_FREQ_MEM_LIMIT_RELAX:   // Enable/disable Frequency memory limits
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags2,0,options,&clr, FLAGS2_FREQ_MEM_LIMIT_RELAX);

        if(ts.flags2 & FLAGS2_FREQ_MEM_LIMIT_RELAX)             // frequency/memory limit is disabled
        {
            clr = Orange;                   // warn user!
        }
        break;

#define UI_MENU_CONFIG_IQ_RX_ADJ(bandName) \
    case CONFIG_##bandName##_RX_IQ_GAIN_BAL: \
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &iq_adjust[IQ_##bandName].adj.rx.gain, TRX_MODE_RX, IQ_TRANS_ON); \
        break; \
    case CONFIG_##bandName##_RX_IQ_PHASE_BAL: \
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &iq_adjust[IQ_##bandName].adj.rx.phase, TRX_MODE_RX, IQ_TRANS_ON); \
        break;

    UI_MENU_CONFIG_IQ_RX_ADJ(80M)
    UI_MENU_CONFIG_IQ_RX_ADJ(10M)

#define UI_MENU_CONFIG_IQ_TX_ADJ(bandName) \
    case CONFIG_##bandName##_TX_IQ_GAIN_BAL: \
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &iq_adjust[IQ_##bandName].adj.tx[IQ_TRANS_ON].gain, TRX_MODE_TX, IQ_TRANS_ON); \
        break;  \
    case CONFIG_##bandName##_TX_IQ_PHASE_BAL: \
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &iq_adjust[IQ_##bandName].adj.tx[IQ_TRANS_ON].phase, TRX_MODE_TX, IQ_TRANS_ON); \
        break; \
    case CONFIG_##bandName##_TX_IQ_GAIN_BAL_TRANS_OFF: \
        UiMenu_HandleIQAdjustGain(var, mode, options, &clr, &iq_adjust[IQ_##bandName].adj.tx[IQ_TRANS_OFF].gain, TRX_MODE_TX, IQ_TRANS_OFF); \
         break; \
    case CONFIG_##bandName##_TX_IQ_PHASE_BAL_TRANS_OFF: \
        UiMenu_HandleIQAdjustPhase(var, mode, options, &clr, &iq_adjust[IQ_##bandName].adj.tx[IQ_TRANS_OFF].phase, TRX_MODE_TX, IQ_TRANS_OFF); \
        break;

    UI_MENU_CONFIG_IQ_TX_ADJ(80M)
    UI_MENU_CONFIG_IQ_TX_ADJ(20M)
    UI_MENU_CONFIG_IQ_TX_ADJ(15M)
    UI_MENU_CONFIG_IQ_TX_ADJ(10M)
    UI_MENU_CONFIG_IQ_TX_ADJ(10M_UP)

        case CONFIG_VSWR_PROTECTION_THRESHOLD:
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.vswr_protection_threshold,
                    1,
                   10,
                    1,
                    1);
            if (ts.vswr_protection_threshold < 2)
            {
                txt_ptr = "OFF";
            }
            else
            {
                snprintf(options,32,"     %d",ts.vswr_protection_threshold);
            }
            break;

    case CONFIG_CW_PA_BIAS:     // CW PA Bias adjust
        if((ts.tune) || (ts.txrx_mode == TRX_MODE_TX))      // enable only in TUNE mode
        {
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.pa_cw_bias,
                                                  0,
                                                  PA_BIAS_MAX,
                                                  0,
                                                  1);

            if(var_change)
            {
                RadioManagement_SetPaBias();
            }
            if(ts.pa_cw_bias < PA_BIAS_LOW_LIMIT)
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
                                                  PA_BIAS_MAX,
                                                  0,
                                                  1);

            if(var_change)
            {
                RadioManagement_SetPaBias();
            }
            if(ts.pa_bias < PA_BIAS_LOW_LIMIT)
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
        var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &swrm.pwr_meter_disp,0,options,&clr);
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
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_SWAP_FWDREV_SENSE);

        if(ts.flags1 & FLAGS1_SWAP_FWDREV_SENSE)                // Display status FWD/REV swapping
        {
            clr = Orange;                   // warn user swapping is on!
        }
        break;
    case CONFIG_XVTR_OFFSET_MULT:   // Transverter Frequency Display Offset/Multiplier Mode On/Off

        temp_var_u8 = ts.xverter_mode & 0xf;
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                                              0,
                                              XVERTER_MULT_MAX,
                                              0,
                                              1);
        ts.xverter_mode = (ts.xverter_mode & 0xfffffff0) | temp_var_u8;
        if(var_change)          // change?
        {
            UiDriver_FrequencyUpdateLOandDisplay(true);
        }

        if(temp_var_u8)
        {
            snprintf(options,32, " ON x%u", (uint)ts.xverter_mode);   // Display on/multiplication factor
        }
        else
        {
            txt_ptr = "     OFF";
            clr = Orange;
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

        if(RadioManagement_Transverter_IsEnabled() == false) // transvert mode inactive?
        {
            clr = Orange;      // make number red to alert user of this!
        }

        uint32_t offset_offset = ts.xverter_offset > XVERTER_OFFSET_MAX_HZ? ((XVERTER_OFFSET_MAX_HZ)-(XVERTER_OFFSET_MAX_HZ)/1000):0;
        snprintf(options,32, ts.xverter_offset > XVERTER_OFFSET_MAX_HZ? " %9ukHz" : "   %9uHz", (uint)(ts.xverter_offset-offset_offset));    // print with nine digits
        break;
    case CONFIG_XVTR_FREQUENCY_OFFSET_TX:      // Adjust transverter tx Frequency offset
        if(var >= 1)        // setting increase?
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            ts.xverter_offset_tx += df.tuning_step;
            var_change = 1;
        }
        else if(var <= -1)      // setting decrease?
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            if(ts.xverter_offset_tx >= df.tuning_step) // subtract only if we have room to do so
            {
                ts.xverter_offset_tx -= df.tuning_step;
            }
            else
            {
                ts.xverter_offset_tx = 0;              // else set to zero
            }

            var_change = 1;
        }
        if(ts.xverter_offset_tx > XVERTER_OFFSET_MAX)
        {
            ts.xverter_offset_tx  = XVERTER_OFFSET_MAX;
        }
        if(mode == MENU_PROCESS_VALUE_SETDEFAULT)
        {
            ts.menu_var_changed = 1;    // indicate that a change has occurred
            ts.xverter_offset_tx = 0;      // default for this option is to zero it out
            var_change = 1;
        }
        if(var_change)          // change?
        {
            UiDriver_FrequencyUpdateLOandDisplay(true);
        }

        if(RadioManagement_Transverter_IsEnabled() == false) // transvert mode inactive?
        {
            clr = Orange;      // make number red to alert user of this!
        }

        if (ts.xverter_offset_tx != 0)
        {
            uint32_t offset_offset = ts.xverter_offset_tx > XVERTER_OFFSET_MAX_HZ? ((XVERTER_OFFSET_MAX_HZ)-(XVERTER_OFFSET_MAX_HZ)/1000):0;
            snprintf(options,32, ts.xverter_offset_tx > XVERTER_OFFSET_MAX_HZ? " %9ukHz" : "   %9uHz", (uint)(ts.xverter_offset_tx-offset_offset));    // print with nine digits
        }
        else
        {
            txt_ptr = "    Same as RX";
        }
        break;


    case CONFIG_2200M_5W_ADJUST:        // 2200m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_2200, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_630M_5W_ADJUST:     // 630m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_630, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_160M_5W_ADJUST:     // 160m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_160, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_80M_5W_ADJUST:      // 80m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_80, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_60M_5W_ADJUST:      // 60m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_60, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_40M_5W_ADJUST:      // 40m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_40, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_30M_5W_ADJUST:      // 30m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_30, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_20M_5W_ADJUST:      // 20m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_20, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_17M_5W_ADJUST:      // 17m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_17, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_15M_5W_ADJUST:      // 15m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_15, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_12M_5W_ADJUST:      // 12m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_12, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_10M_5W_ADJUST:      // 10m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_10, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_6M_5W_ADJUST:       // 6m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_6, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_4M_5W_ADJUST:       // 4m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_4, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_2M_5W_ADJUST:       // 2m 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_2, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_70CM_5W_ADJUST:     // 70cm 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_70, PA_LEVEL_HIGH, options, &clr);
        break;
    case CONFIG_23CM_5W_ADJUST:     // 23cm 5 watt adjust
        UiDriverMenuBandPowerAdjust(var, mode, BAND_MODE_23, PA_LEVEL_HIGH, options, &clr);
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
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags2,0,options,&clr, FLAGS2_LOW_BAND_BIAS_REDUCE);
        break;
    case CONFIG_REDUCE_POWER_ON_HIGH_BANDS:
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags2,0,options,&clr, FLAGS2_HIGH_BAND_BIAS_REDUCE);
        break;
#ifdef OBSOLETE_NR
    case CONFIG_DSP_NR_DECORRELATOR_BUFFER_LENGTH:      // Adjustment of DSP noise reduction de-correlation delay buffer length
        ts.dsp.nr_delaybuf_len &= 0xfff0;   // mask bottom nybble to enforce 16-count boundary
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.dsp.nr_delaybuf_len,
                                               DSP_NR_BUFLEN_MIN,
                                               DSP_NR_BUFLEN_MAX,
                                               DSP_NR_BUFLEN_DEFAULT,
                                               16);

        if(ts.dsp.nr_delaybuf_len <= ts.dsp.nr_numtaps) // is buffer smaller/equal to number of taps?
            ts.dsp.nr_delaybuf_len = ts.dsp.nr_numtaps + 16;    // yes - it must always be larger than number of taps!

        if(var_change)      // did something change?
        {
            if(ts.dsp.active & DSP_NR_ENABLE)   // only update if DSP NR active
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
        }
        if(!(ts.dsp.active & DSP_NR_ENABLE))    // mark orange if DSP NR not active
            clr = Orange;
        if(ts.dsp.nr_numtaps >= ts.dsp.nr_delaybuf_len) // Warn if number of taps greater than/equal buffer length!
            clr = Red;
        snprintf(options,32, "  %u", (uint)ts.dsp.nr_delaybuf_len);
        break;
    case CONFIG_DSP_NR_FFT_NUMTAPS:     // Adjustment of DSP noise reduction de-correlation delay buffer length
        ts.dsp.nr_numtaps &= 0xf0;  // mask bottom nybble to enforce 16-count boundary
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.nr_numtaps,
                                              DSP_NR_NUMTAPS_MIN,
                                              DSP_NR_NUMTAPS_MAX,
                                              DSP_NR_NUMTAPS_DEFAULT,
                                              16);
        if(ts.dsp.nr_numtaps >= ts.dsp.nr_delaybuf_len) // is number of taps equal or greater than buffer length?
        {
            ts.dsp.nr_delaybuf_len = ts.dsp.nr_numtaps + 16;    // yes - make buffer larger
        }

        if(var_change)      // did something change?
        {
            if(ts.dsp.active & DSP_NR_ENABLE)   // only update if DSP NR active
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
        }

        if(!(ts.dsp.active & DSP_NR_ENABLE))    // mark orange if DSP NR not active
        {
            clr = Orange;
        }
        if(ts.dsp.nr_numtaps >= ts.dsp.nr_delaybuf_len) // Warn if number of taps greater than/equal buffer length!
        {
            clr = Red;
        }
        snprintf(options,32, "  %u", ts.dsp.nr_numtaps);
        break;
    case CONFIG_DSP_NR_POST_AGC_SELECT:     // selection of location of DSP noise reduction - pre audio filter/AGC or post AGC/filter
        temp_var_u8 = ts.dsp.active & DSP_NR_POSTAGC_ENABLE;
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)      // did something change?
        {
            CLR_OR_SET_BITMASK(temp_var_u8, ts.dsp.active, DSP_NR_POSTAGC_ENABLE);

            if(ts.dsp.active & DSP_NR_ENABLE)   // only update if DSP NR active
            {
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
            }
        }

        if(!(ts.dsp.active & DSP_NR_ENABLE))    // mark orange if DSP NR not active
        {
            clr = Orange;
        }

        break;

    case CONFIG_DSP_NOTCH_CONVERGE_RATE:        // Adjustment of DSP noise reduction de-correlation delay buffer length
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.notch_mu,
                                              0,
                                              DSP_NOTCH_MU_MAX,
                                              DSP_NOTCH_MU_DEFAULT,
                                              1);

        if(var_change)      // did something change?
        {
            if(ts.dsp.active & DSP_NOTCH_ENABLE)    // only update if Notch DSP is active
            {
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp.active & DSP_NOTCH_ENABLE)) // mark orange if Notch DSP not active
        {
            clr = Orange;
        }
        snprintf(options,32, "  %u", ts.dsp.notch_mu);
        break;
    case CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH:       // Adjustment of DSP noise reduction de-correlation delay buffer length
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.notch_delaybuf_len,
                                              DSP_NOTCH_BUFLEN_MIN,
                                              DSP_NOTCH_BUFLEN_MAX,
                                              DSP_NOTCH_DELAYBUF_DEFAULT,
                                              8);


        if(ts.dsp.notch_delaybuf_len <= ts.dsp.notch_numtaps)       // did we try to decrease it smaller than FFT size?
        {
            ts.dsp.notch_delaybuf_len = ts.dsp.notch_numtaps + 8;                       // yes - limit it to previous size
        }
        if(var_change)      // did something change?
        {
            if(ts.dsp.active & DSP_NOTCH_ENABLE)    // only update if DSP Notch active
            {
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp.active & DSP_NOTCH_ENABLE)) // mark orange if DSP Notch not active
        {
            clr = Orange;
        }
        if(ts.dsp.notch_numtaps >= ts.dsp.notch_delaybuf_len)
        {
            clr = Red;
        }
        snprintf(options,32, "  %u", (uint)ts.dsp.notch_delaybuf_len);
        break;
    case CONFIG_DSP_NOTCH_FFT_NUMTAPS:      // Adjustment of DSP noise reduction de-correlation delay buffer length
        ts.dsp.notch_numtaps &= 0xf0;   // mask bottom nybble to enforce 16-count boundary
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.notch_numtaps,
                                              0,
                                              DSP_NOTCH_NUMTAPS_MAX,
                                              DSP_NOTCH_NUMTAPS_DEFAULT,
                                              16);
        if(ts.dsp.notch_numtaps >= ts.dsp.notch_delaybuf_len)   // force buffer size to always be larger than number of taps
            ts.dsp.notch_delaybuf_len = ts.dsp.notch_numtaps + 8;
        if(var_change)      // did something change?
        {
            if(ts.dsp.active & DSP_NOTCH_ENABLE)    // only update if DSP NR active
            {
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp.active & DSP_NOTCH_ENABLE)) // mark orange if DSP NR not active
        {
            clr = Orange;
        }
        if(ts.dsp.notch_numtaps >= ts.dsp.notch_delaybuf_len)   // Warn if number of taps greater than/equal buffer length!
        {
            clr = Red;
        }
        snprintf(options,32, "  %u", ts.dsp.notch_numtaps);
        break;
/*
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
*/
#endif

#ifdef USE_LMS_AUTONOTCH
    case CONFIG_DSP_NOTCH_CONVERGE_RATE:        // Adjustment of DSP autonotch convergence rate
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.notch_mu,
                                              0,
                                              DSP_NOTCH_MU_MAX,
                                              DSP_NOTCH_MU_DEFAULT,
                                              1);

        if(var_change)      // did something change?
        {
            if(ts.dsp.active & DSP_NOTCH_ENABLE)    // only update if Notch DSP is active
            {
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp.active & DSP_NOTCH_ENABLE)) // mark orange if Notch DSP not active
        {
            clr = Orange;
        }
        snprintf(options,32, "  %u", ts.dsp.notch_mu);
        break;
    case CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH:       // Adjustment of DSP noise reduction de-correlation delay buffer length
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.notch_delaybuf_len,
                                              DSP_NOTCH_BUFLEN_MIN,
                                              DSP_NOTCH_BUFLEN_MAX,
                                              DSP_NOTCH_DELAYBUF_DEFAULT,
                                              8);


        if(ts.dsp.notch_delaybuf_len <= ts.dsp.notch_numtaps)       // did we try to decrease it smaller than FFT size?
        {
            ts.dsp.notch_delaybuf_len = ts.dsp.notch_numtaps + 8;                       // yes - limit it to previous size
        }
        if(var_change)      // did something change?
        {
            if(ts.dsp.active & DSP_NOTCH_ENABLE)    // only update if DSP Notch active
            {
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp.active & DSP_NOTCH_ENABLE)) // mark orange if DSP Notch not active
        {
            clr = Orange;
        }
        if(ts.dsp.notch_numtaps >= ts.dsp.notch_delaybuf_len)
        {
            clr = Red;
        }
        snprintf(options,32, "  %u", (uint)ts.dsp.notch_delaybuf_len);
        break;
    case CONFIG_DSP_NOTCH_FFT_NUMTAPS:      // Adjustment of DSP noise reduction de-correlation delay buffer length
        ts.dsp.notch_numtaps &= 0xf0;   // mask bottom nybble to enforce 16-count boundary
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.dsp.notch_numtaps,
                                              16,
                                              DSP_NOTCH_NUMTAPS_MAX,
                                              DSP_NOTCH_NUMTAPS_DEFAULT,
                                              16);
//        if(ts.dsp.notch_numtaps >= ts.dsp.notch_delaybuf_len)   // force buffer size to always be larger than number of taps
//            ts.dsp.notch_delaybuf_len = ts.dsp.notch_numtaps + 8;
        if(var_change)      // did something change?
        {
            if(ts.dsp.active & DSP_NOTCH_ENABLE)    // only update if DSP NR active
            {
                AudioDriver_SetProcessingChain(ts.dmod_mode, false);
            }
        }
        if(!(ts.dsp.active & DSP_NOTCH_ENABLE)) // mark orange if DSP NR not active
        {
            clr = Orange;
        }
        if(ts.dsp.notch_numtaps >= ts.dsp.notch_delaybuf_len)   // Warn if number of taps greater than/equal buffer length!
        {
            clr = Red;
        }
        snprintf(options,32, "  %u", ts.dsp.notch_numtaps);
        break;
#endif

    case CONFIG_AM_TX_FILTER_DISABLE:   // Enable/disable AM TX audio filter
        temp_var_u8 = !(ts.flags1 & FLAGS1_AM_TX_FILTER_DISABLE);
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &temp_var_u8,0,options,&clr);
        if(var_change)          // did the status change and is translate mode NOT active?
        {
            CLR_OR_SET_BITMASK(!temp_var_u8,  ts.flags1, FLAGS1_AM_TX_FILTER_DISABLE);
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
            TxProcessor_Set(ts.dmod_mode);
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

            if (ts.tune_power_level < mchf_power_levelsInfo.count)
            {
                char txt[5];
                UiDriver_Power2String(txt,sizeof(txt),mchf_power_levelsInfo.levels[ts.tune_power_level].mW);
                snprintf(options,32,"       %s",txt);
            }
            else
            {
                txt_ptr = " as TX PWR";
            }
            break;
#if 0
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
#endif
        case CONFIG_RESET_SER_EEPROM:
        if(SerialEEPROM_24xx_Exists() == false)
        {
            txt_ptr = "   n/a";
            clr = Red;
        }
        else
        {
            txt_ptr = "n/a";
            uint8_t test = 0;
            clr = White;
            UiDriverMenuItemChangeUInt8(var, mode, &test,
                                            0,
                                            0,
                                            10,
                                            1
                                           );
            switch(test)
            {
              case 0:
                txt_ptr = "press DEFLT";
                break;
              case 10:
                UiMenu_DisplayValue("    Working",Red,pos);
                // clear EEPROM
                SerialEEPROM_Clear_Signature();
                SerialEEPROM_Clear_AllVariables();
                Board_Reboot();
                break;
            }
        }
        break;
        case CONFIG_RESET_SER_EEPROM_SIGNATURE:
        if(SerialEEPROM_24xx_Exists() == false)
        {
            txt_ptr = "   n/a";
            clr = Red;
        }
        else
        {
      		txt_ptr = "n/a";
			uint8_t test = 0;
      		clr = White;
      		UiDriverMenuItemChangeUInt8(var, mode, &test,
        	                            	0,
                                        	0,
                                            10,
                                            1
                                           );
      		switch(test)
      		{
      		  case 0:
          		txt_ptr = "press DEFLT";
          		break;
      		  case 10:
          		UiMenu_DisplayValue("    Working",Red,pos);
          		// clear EEPROM
                SerialEEPROM_Clear_Signature();
          		Board_Reboot();
          		break;
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
        var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.flags1,0,options,&clr, FLAGS1_CAT_IN_SANDBOX);

        if(!(ts.flags1 & FLAGS1_CAT_IN_SANDBOX))
        {
            ts.cat_band_index = 255;
        }
        break;
    case CONFIG_CAT_XLAT:   // CAT xlat reporting
        var_change = UiDriverMenuItemChangeEnableOnOff(var, mode, &ts.xlat,0,options,&clr);
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
            txt_ptr = "         OFF";
            break;
        case STREAM_TX_AUDIO_SRC:
            txt_ptr = "      Source";
            break;
        case STREAM_TX_AUDIO_FILT:
            txt_ptr = "    Filtered";
            break;
        case STREAM_TX_AUDIO_DIGIQ:
            txt_ptr = "    Final IQ";
            break;
        case STREAM_TX_AUDIO_GENIQ:
            txt_ptr = "Generated IQ";
            break;
        }
        break;
#ifdef STM32F4
    case CONFIG_I2C1_SPEED:      //
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.i2c_speed[I2C_BUS_1],
                1,
                20,
                I2C1_SPEED_DEFAULT,
                1
        );
        if(var_change)
        {
            UhsdrHw_I2C_ChangeSpeed(&hi2c1);
        }
        snprintf(options, 32, " %3dkHz",(unsigned int)(ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 );
		if((ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 < 50 || (ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 > 250)
		{
		  clr = Red;
		}
		if(((ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 > 50 && (ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 < 90) || ((ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 > 210 && (ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 < 250))
		{
		  clr = Yellow;
		}
		if((ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 > 90 && (ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 < 210)
		{
		  clr = Green;
		}
        break;
    case CONFIG_I2C2_SPEED:      //
        var_change = UiDriverMenuItemChangeUInt32(var, mode, &ts.i2c_speed[I2C_BUS_2],
                1,
                20,
                I2C2_SPEED_DEFAULT,
                1
        );
        if(var_change)
        {
            UhsdrHw_I2C_ChangeSpeed(&hi2c2);
        }
        snprintf(options, 32, " %3ukHz",(unsigned int)(ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 );
		if((ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 < 50 || (ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 > 250)
		{
		  clr = Red;
		}
		if(((ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 > 50 && (ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 < 90) || ((ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 > 210 && (ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 < 250))
		{
		  clr = Yellow;
		}
		if((ts.i2c_speed[I2C_BUS_2]*I2C_BUS_SPEED_MULT) / 1000 > 90 && (ts.i2c_speed[I2C_BUS_1]*I2C_BUS_SPEED_MULT) / 1000 < 210)
		{
		  clr = Green;
		}
        break;
#endif
    case CONFIG_RTC_HOUR:
    {
        RTC_TimeTypeDef rtc;
        Rtc_GetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
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
        Rtc_GetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
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
        Rtc_GetTime(&hrtc, &rtc, RTC_FORMAT_BIN);
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
             Rtc_SetPpm(ts.rtc_calib);
         }
         snprintf(options,32, "%4dppm", ts.rtc_calib);
         break;
     }

    case CONFIG_RTC_START:
        txt_ptr = "Do it!";
        clr = White;
        if(var>=1)
        {
            Rtc_Start();
            Board_Reboot();
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
            Rtc_FullReset();

            txt_ptr = " Done!";
            clr = Green;
        }
        break;
    case MENU_DEBUG_ENABLE_INFO:  // Debug infos on LCD on/off

        var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &ts.show_debug_info,0,options,&clr);
        if(var_change)
        {
            UiDriver_DebugInfo_DisplayEnable(ts.show_debug_info);
        }
        break;
#ifdef USE_TWO_CHANNEL_AUDIO
    case MENU_DEBUG_ENABLE_STEREO:  // stereo demodulation on/off

        var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &ts.stereo_enable,0,options,&clr);
        switch(ts.stereo_enable)
        {
        case 0:
            txt_ptr = " OFF";
            break;
        case 1:
            txt_ptr = "  ON";
            break;
        }
        break;
#endif
#ifdef USE_LEAKY_LMS
        case MENU_DEBUG_LEAKY_LMS:
            var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &ts.enable_leaky_LMS,0,options,&clr);
            break;


        case MENU_DEBUG_ANR_GAIN:      //
            var_change = UiDriverMenuItemChangeUInt32(var, mode, &leakyLMS.two_mu_int,
                    1,
                    10000,
                    100,
                    10
            );
            if(var_change)
            {
            	leakyLMS.two_mu = leakyLMS.two_mu_int / 1000000;
            }
            snprintf(options, 32, " %4u",(unsigned int)leakyLMS.two_mu_int);

        break;
        case MENU_DEBUG_ANR_LEAK:      //
            var_change = UiDriverMenuItemChangeUInt32(var, mode, &leakyLMS.gamma_int,
                    1,
                    1000,
                    100,
                    10
            );
            if(var_change)
            {
            	leakyLMS.gamma = leakyLMS.gamma_int / 1000;
            }
            snprintf(options, 32, " %4u",(unsigned int)leakyLMS.gamma_int);

        break;

        case MENU_DEBUG_ANR_TAPS:      //
            var_change = UiDriverMenuItemChangeInt16(var, mode, &leakyLMS.n_taps,
                    1,
                    192,
                    64,
                    2
            );
            if(var_change)
            {
//            	leakyLMS.gamma = leakyLMS.gamma_int / 1000;
            }
            snprintf(options, 32, " %3u",(unsigned int)leakyLMS.n_taps);

        break;
        case MENU_DEBUG_ANR_DELAY:      //
            var_change = UiDriverMenuItemChangeInt16(var, mode, &leakyLMS.delay,
                    1,
                    64,
                    16,
                    2
            );
            if(var_change)
            {
//            	leakyLMS.gamma = leakyLMS.gamma_int / 1000;
            }
            snprintf(options, 32, " %3u",(unsigned int)leakyLMS.delay);

        break;
#endif

// this is now adjusted by ts.dsp.nr_strength
/*        case MENU_DEBUG_NR_ALPHA:      //
            var_change = UiDriverMenuItemChangeInt16(var, mode, &nr_params.alpha_int,
                    700,
                    999,
                    920,
                    2
            );
            if(var_change)
            {
            	nr_params.alpha = (float32_t)nr_params.alpha_int / 1000.0;
            }
            snprintf(options, 32, " %3u",(unsigned int)nr_params.alpha_int);

        break;
*/
        case MENU_DEBUG_NR_GAIN_SHOW:      //
            var_change = UiDriverMenuItemChangeInt16(var, mode, &ts.nr_gain_display,
                    0,
                    1,
                    0,
                    1
            );
            switch(ts.nr_gain_display)
            {
            case 0:
                txt_ptr = "        OFF";
                break;
            case 1:
                txt_ptr = "        Hk";
                break;
            /*case 2:
                txt_ptr = "     notch";
                break;
            case 3:
                txt_ptr = "HK & notch";
                break; */
            }

        break;

        /*case MENU_DEBUG_NR_GAIN_SMOOTH_ALPHA:      //
            var_change = UiDriverMenuItemChangeInt16(var, mode, &nr_params.gain_smooth_alpha_int,
                    100,
                    990,
                    250,
                    2
            );
            if(var_change)
            {
            	nr_params.gain_smooth_alpha = (float32_t)nr_params.gain_smooth_alpha_int / 1000.0;
            }
            snprintf(options, 32, " %3u",(unsigned int)nr_params.gain_smooth_alpha_int);

        break;

        case MENU_DEBUG_NR_LONG_TONE_ALPHA:      //
            var_change = UiDriverMenuItemChangeUInt32(var, mode, &nr_params.long_tone_alpha_int,
                    90000,
                    99999,
                    99900,
                    10
            );
            if(var_change)
            {
            	nr_params.long_tone_alpha = (float32_t)nr_params.long_tone_alpha_int / 100000.0;
            }
            snprintf(options, 32, " %5u",(unsigned int)nr_params.long_tone_alpha_int);

        break;
        case MENU_DEBUG_NR_LONG_TONE_THRESH:      //
            var_change = UiDriverMenuItemChangeInt16(var, mode, &nr_params.long_tone_thresh,
                    10,
                    16000,
                    600,
                    200
            );
            if(var_change)
            {
            //	nr_params.vad_thresh = (float32_t)nr_params.vad_thresh_int / 1000.0;
            }
            snprintf(options, 32, " %5u",(unsigned int)nr_params.long_tone_thresh);

        break;

        case MENU_DEBUG_NR_THRESH:      //
            var_change = UiDriverMenuItemChangeUInt32(var, mode, &nr_params.vad_thresh_int,
                    100,
                    20000,
                    1000,
                    50
            );
            if(var_change)
            {
            	nr_params.vad_thresh = (float32_t)nr_params.vad_thresh_int / 1000.0;
            }
            snprintf(options, 32, " %5u",(unsigned int)nr_params.vad_thresh_int);

        break;*/

        case MENU_DEBUG_NR_BETA:      //
        {

            int16_t beta_int = nr_params.beta * 1000;
            var_change = UiDriverMenuItemChangeInt16(var, mode, &beta_int,
                    700,
                    999,
                    960,
                    2
            );
            if(var_change)
            {
            	nr_params.beta = (float32_t)beta_int / 1000.0;
            }
            snprintf(options, 32, " %3u",(unsigned int)beta_int);
        }
        break;

        /*case MENU_DEBUG_NR_Mode:      //
            var_change = UiDriverMenuItemChangeInt16(var, mode, &nr_params.mode,
                    0,
                    2,
                    0,
                    1
            );
            switch(nr_params.mode)
            {
				case 0:
					txt_ptr = "    Release";

					break;
				case 1:
					txt_ptr = "   Devel1";

					break;
				case 2:
					txt_ptr = "   Devel2";
            }

            if(var_change)
			{
				switch(nr_params.mode)
				{
				case 0:
					nr_params.beta = 0.850;
					nr_params.beta_int=850;
					nr_params.first_time = 1; //Restart the noisereduction
					break;
				case 1:
					nr_params.beta = 0.960;
					nr_params.beta_int=960;
					nr_params.first_time = 1; //Restart the noisereduction
				case 2:
					nr_params.beta = 0.960;
					nr_params.beta_int=960;
					nr_params.first_time = 1; //Restart the noisereduction
					break;
				}
			}
        break; */

            case MENU_DEBUG_NR_ASNR:
             var_change = UiDriverMenuItemChangeInt16(var, mode, &NR2.asnr,
                                                   2,
                                                   30,
                                                   30,
                                                   1);
             if(var_change)      // did something change?
             {
             	//nr_params.first_time = 1;
             }
             snprintf(options,32, "  %3u", (unsigned int)NR2.asnr);
             break;


        case MENU_DEBUG_NR_GAIN_SMOOTH_WIDTH:
        var_change = UiDriverMenuItemChangeInt16(var, mode, &NR2.width,
                                              1,
                                              5,
                                              4,
                                              1);
        if(var_change)      // did something change?
        {
        	//nr_params.first_time = 1;
        }
        snprintf(options,32, "  %3u", (unsigned int)NR2.width);
        break;

        case MENU_DEBUG_NR_GAIN_SMOOTH_THRESHOLD:
                var_change = UiDriverMenuItemChangeInt16(var, mode, &NR2.power_threshold_int,
                                                      10,
                                                      100,
                                                      40,
                                                      5);
                if(var_change)      // did something change?
                {
                	//nr_params.first_time = 1;
                }
                snprintf(options,32, "  %3u", (unsigned int)NR2.power_threshold_int);
                break;

/*
        case MENU_DEBUG_NR_VAD_DELAY:      //
            var_change = UiDriverMenuItemChangeInt16(var, mode, &nr_params.vad_delay,
                    0,
                    20,
                    2,
                    1
            );
            if(var_change)
            {

            }
            snprintf(options, 32, " %2u",(unsigned int)nr_params.vad_delay);

        break;
*/
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
        //     case MENU_DEBUG_NEW_NB:
        //         var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &ts.new_nb,0,options,&clr);
        //         break;//
//#if defined(STM32F7) || defined(STM32H7)
/*     case MENU_DEBUG_NR_FFT_SIZE:
                 var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &nr_params.fft_256_enable,0,options,&clr);
                 ts.NR_FFT_L = 128;
                 ts.NR_FFT_LOOP_NO = 2;
                 if(nr_params.fft_256_enable)
                 {
                	 ts.NR_FFT_LOOP_NO = 1;
                	 ts.NR_FFT_L = 256;
                 }
        break;
     case MENU_DEBUG_NR_DEC_ENABLE:
                 var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &ts.NR_decimation_enable,0,options,&clr);
        break;
//#endif
//     case MENU_DEBUG_NR_ENABLE:
//             var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &nr_params.enable,0,options,&clr);
//         break;
     case MENU_DEBUG_NR_LONG_TONE_ENABLE:
             var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &nr_params.long_tone_enable,0,options,&clr);
             if(var_change)
             {
            	 nr_params.long_tone_reset = true;
             }
         break;*/

/*     case MENU_DEBUG_NR_VAD_TYPE:
         var_change = UiDriverMenuItemChangeUInt8(var, mode, &NR2.VAD_type,
                 0,
                 2,
                 0,
                 1);
         switch(NR2.VAD_type)
         {
         case 0:
             txt_ptr = "       Sohn";
             break;
         case 1:
             txt_ptr = "Esch & Vary";
             break;
		 case 2:
			 txt_ptr = "        ZCR";
			 break;
         }
         break;

     case MENU_DEBUG_NR_GAIN_SMOOTH_ENABLE:
             var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &nr_params.gain_smooth_enable,0,options,&clr);
         break;*/

//     case MENU_DEBUG_RTTY_ATC:
//         var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &ts.rtty_atc_enable,0,options,&clr);
//         break;

     case MENU_CW_DECODER:
         var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.cw_decoder_enable,
                 0,
                 1,
                 0,
                 1);
         if (var_change)
         {
             if (ts.dmod_mode == DEMOD_CW)
             {
                 UiDriver_UpdateDemodSpecificDisplayAfterParamChange();
             }
             Board_RedLed(LED_STATE_OFF);
         }
         switch(ts.cw_decoder_enable)
         {
         case 0:
             txt_ptr = "OFF";
             break;
         case 1:
             txt_ptr = " ON";
             break;
         }
         break;

     case MENU_DEBUG_CW_OFFSET_SHIFT_KEEP_SIGNAL:
         var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &ts.cw_offset_shift_keep_signal,0,options,&clr);
         break;

     case MENU_CW_DECODER_USE_3_GOERTZEL:
         var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &cw_decoder_config.use_3_goertzels,0,options,&clr);
    	 break;

     case MENU_CW_DECODER_SHOW_CW_LED:
         var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &cw_decoder_config.show_CW_LED,0,options,&clr);
         if (cw_decoder_config.show_CW_LED == false)
         {
             Board_RedLed(LED_STATE_OFF);
         }
    	 break;
     case MENU_CW_DECODER_SNAP_ENABLE:
         var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &cw_decoder_config.snap_enable,0,options,&clr);
         if (var_change)
         {
             if (ts.dmod_mode == DEMOD_CW)
             {
                 UiDriver_UpdateDemodSpecificDisplayAfterParamChange();
             }
         }
    	 break;

    case MENU_DIGITAL_MODE_SELECT:
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.digital_mode,0,DigitalMode_Num_Modes-1,0,1);
        if (var_change)
        {
            // TODO: Factor this out into a Ui function for (de-)activating Rtty mode
            if (RadioManagement_IsApplicableDemodMode(DEMOD_DIGI))
            {
                UiDriver_SetDemodMode(DEMOD_DIGI);
            }
        }
        snprintf(options,32,"     %s",digimodes[ts.digital_mode].label);
        clr = digimodes[ts.digital_mode].enabled?White:Red;
        break;

    case CONFIG_CAT_PTT_RTS:
        var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &ts.enable_ptt_rts,0,options,&clr);
        break;

    case MENU_DEBUG_OSC_SI5351_PLLRESET:
        var_change = UiDriverMenuItemChangeUInt8(var, mode, &ts.debug_si5351a_pllreset,
                0,
                3,
                0,
                1);
        switch(ts.debug_si5351a_pllreset)
        {
        case 0:
            txt_ptr = "    Always";
            break;
        case 1:
            txt_ptr = "   Divider";
            break;
        case 2:
            txt_ptr = "IQ Divider";
            break;
        case 3:
            txt_ptr = "      Never";
            break;
        }
        break;
#ifdef USE_HMC1023
        case MENU_DEBUG_HMC1023_COARSE:
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &hmc1023.coarse,0,8,0,1);
            if (var_change)
            {
                hmc1023_set_coarse(hmc1023.coarse);
            }
            snprintf(options,32," %2d",hmc1023.coarse);
            break;
        case MENU_DEBUG_HMC1023_FINE:
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &hmc1023.fine,0,11,0,1);
            if (var_change)
            {
                hmc1023_set_fine(hmc1023.fine);
            }
            snprintf(options,32," %2d",hmc1023.fine);
            break;
        case MENU_DEBUG_HMC1023_OPAMP:
             var_change = UiDriverMenuItemChangeUInt8(var, mode, &hmc1023.opamp,0,3,0,1);
             if (var_change)
             {
                 hmc1023_set_bias_opamp(hmc1023.opamp);
             }
             snprintf(options,32," %2d",hmc1023.opamp);
             break;
        case MENU_DEBUG_HMC1023_DRVR:
             var_change = UiDriverMenuItemChangeUInt8(var, mode, &hmc1023.drvr,0,3,0,1);
             if (var_change)
             {
                 hmc1023_set_bias_drvr(hmc1023.drvr);
             }
             snprintf(options,32," %2d",hmc1023.drvr);
             break;

        case MENU_DEBUG_HMC1023_GAIN:
            var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &hmc1023.gain,0,options,&clr);
            if (var_change)
            {
                hmc1023_set_gain(hmc1023.gain);
            }
            break;

        case MENU_DEBUG_HMC1023_BYPASS:
            var_change = UiDriverMenuItemChangeEnableOnOffBool(var, mode, &hmc1023.bypass,0,options,&clr);
            if (var_change)
            {
                hmc1023_set_bypass(hmc1023.bypass);
            }

            break;

#endif
        case CONFIG_SMETER_ATTACK:
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &sm.config.alphaSplit.AttackAlpha, SMETER_ALPHA_MIN, SMETER_ALPHA_MAX, SMETER_ALPHA_ATTACK_DEFAULT,1);
            snprintf(options,32,"     %d",sm.config.alphaSplit.AttackAlpha);
            break;
        case CONFIG_SMETER_DECAY:
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &sm.config.alphaSplit.DecayAlpha, SMETER_ALPHA_MIN, SMETER_ALPHA_MAX, SMETER_ALPHA_DECAY_DEFAULT,1);
            snprintf(options,32,"     %d",sm.config.alphaSplit.DecayAlpha);
            break;


#ifdef USE_FREEDV
        case MENU_DEBUG_FREEDV_MODE:
            temp_var_u8 = freedv_conf.mode;
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &temp_var_u8,
                    0,
                    freedv_modes_num-1,
                    0,
                    1);
            if (var_change)
            {
                if (FreeDV_SetMode(temp_var_u8, false) == false)
                {
                    clr = Red;
                }
            }

            txt_ptr = freedv_modes[freedv_conf.mode].name;
            break;

        case MENU_DEBUG_FREEDV_SQL_THRESHOLD:
            var_change = UiDriverMenuItemChangeUInt8(var, mode, &freedv_conf.squelch_snr_thresh,
                    FDV_SQUELCH_OFF,
                    FDV_SQUELCH_MAX,
                    FDV_SQUELCH_DEFAULT,
                    1);

            if (var_change)
            {
                FreeDV_Squelch_Update(&freedv_conf);
            }

            if (FreeDV_Is_Squelch_Enable(&freedv_conf))
            {
                snprintf(options,32,"     %ld",FreeDV_Get_Squelch_SNR(&freedv_conf));
            }
            else
            {
                txt_ptr = "OFF";
            }
            break;
#endif
        case MENU_DEBUG_SMOOTH_DYN_TUNE:
            var_change = UiDriverMenuItemChangeEnableOnOffFlag(var, mode, &ts.expflags1,0,options,&clr, EXPFLAGS1_SMOOTH_DYNAMIC_TUNE);
            clr = White;
            break;

    default:                        // Move to this location if we get to the bottom of the table!
        txt_ptr = "ERROR!";
        break;
    }

    *txt_ptr_ptr = txt_ptr;
    *clr_ptr = clr;
}
