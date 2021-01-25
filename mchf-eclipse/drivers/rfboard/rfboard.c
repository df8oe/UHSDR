/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Description:                                                                   **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

// Common
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "radio_management.h"
#include "rfboard_interface.h"
#include "profiling.h"


#include "osc_interface.h"
#include "osc_ducddc_df8oe.h"



//specyfic hardware features structure
__MCHF_SPECIALMEM HardwareRFBoard RFboard;

// this structure MUST match the order of entries in power_level_t !
static const power_level_desc_t mchf_rf_power_levels[] =
{
        { .id = PA_LEVEL_FULL,   .mW = 0,    }, // we use 0 to indicate max power
        { .id = PA_LEVEL_HIGH,   .mW = 5000, },
        { .id = PA_LEVEL_MEDIUM, .mW = 2000, },
        { .id = PA_LEVEL_LOW,    .mW = 1000, },
        { .id = PA_LEVEL_MINIMAL,.mW =  500, },
};


const pa_power_levels_info_t mchf_power_levelsInfo =
{
        .levels = mchf_rf_power_levels,
        .count = sizeof(mchf_rf_power_levels)/sizeof(*mchf_rf_power_levels),
};

#ifdef RF_BRD_LAPWING
const pa_info_t mchf_pa =
{
        .name  = "Lapwing PA",
        .reference_power = 5000.0,
        .max_freq = 1300 * 1000000,
        .min_freq = 1240 * 1000000,
        .max_am_power = 2000,
        .max_power = 20000,
};
#else
const pa_info_t mchf_pa =
{
        .name  = "mcHF PA",
        .reference_power = 5000.0,
        .max_freq = 32000000,
        .min_freq =  1800000,
        .max_am_power = 2000,
        .max_power = 10000,
};
#endif // LAPWING

#ifdef USE_OSC_SParkle
// this structure MUST match the order of entries in power_level_t !
static const power_level_desc_t SParkle_rf_power_levels[] =
{
        { .id = PA_LEVEL_FULL,   .mW = 0,    }, // we use 0 to indicate max power
        { .id = PA_LEVEL_HIGH,   .mW = 10000, },
        { .id = PA_LEVEL_MEDIUM, .mW = 5000, },
        { .id = PA_LEVEL_LOW,    .mW = 2000, },
        { .id = PA_LEVEL_MINIMAL,.mW = 1000, },
};

const pa_power_levels_info_t SParkle_power_levelsInfo =
{
        .levels = SParkle_rf_power_levels,
        .count = sizeof(SParkle_rf_power_levels)/sizeof(*SParkle_rf_power_levels),
};

const pa_info_t SParkle_pa =
{
        .name  = "SParkle PA",
        .reference_power = 10000.0,
        .max_freq = 150000000,
        .min_freq =  1800000,
        .max_am_power = 25000,
        .max_power = 50000,
};
#endif

/**
 * @brief switch off the PA Bias to mute HF output ( even if PTT is on )
 * Using this method the PA will be effectively muted no matter what setting
 * the main bias switch has (which directly connected to the PTT HW Signal)
 * Used to suppress signal path reconfiguration noise during rx/tx and tx/rx switching
 */
static void RFBoard_Mchf_DisablePaBias()
{
    Board_SetPaBiasValue(0);
}



static bool Mchf_PrepareTx(void)
{
    Board_EnableTXSignalPath(true); // switch antenna to output and codec output to QSE mixer
    return true;
}

bool Mchf_PrepareRx(void)
{
    RFBoard_Mchf_DisablePaBias(); // kill bias to mute the HF output quickly
    return true;
}

bool Mchf_EnableRx(void)
{
    Board_EnableTXSignalPath(false); // switch antenna to input and codec output to QSD mixer
    return true;
}

bool Mchf_EnableTx(void)
{
    RadioManagement_SetPaBias();
    return true;
}

bool RFBoard_Dummy_PrepareTx(void)
{
    return true;
}

bool RFBoard_Dummy_PrepareRx(void)
{
    return true;
}

bool RFBoard_Dummy_EnableRx(void)
{
    return true;
}

bool RFBoard_Dummy_EnableTx(void)
{
    return true;
}


/**
 * This has to be called after rf board hardware detection and before using any other RFboard related functions
 */
void RFBoard_Init_Board(void)
{

    // Initialize LO, by which we (at least for now) can detect the RF board
    Osc_Init();

    // we determine and set the correct RF board here
    switch(osc->type)
    {
        case OSC_SI5351A: ts.rf_board = RF_BOARD_RS928; break;
        case OSC_DUCDDC_DF8OE  : ts.rf_board = RF_BOARD_DDCDUC_DF8OE; break;
        case OSC_SI570: ts.rf_board = RF_BOARD_MCHF; break;
        case OSC_SPARKLE: ts.rf_board = RF_BOARD_SPARKLE; break;
        default: ts.rf_board = RF_BOARD_UNKNOWN;
    }

    osc->setPPM((float)ts.freq_cal/10.0);

    switch(ts.rf_board)
    {
        case RF_BOARD_SPARKLE:
#ifdef USE_OSC_SParkle
            RFboard.pa_info=&SParkle_pa;       //default setting for mchf PA (overwitten later when other hardware was detected)
            RFboard.power_levelsInfo=&SParkle_power_levelsInfo;
            RFboard.EnableTx  = osc_SParkle_EnableTx;
            RFboard.EnableRx = osc_SParkle_EnableRx;
            RFboard.PrepareTx  = osc_SParkle_PrepareTx;
            RFboard.PrepareRx = osc_SParkle_PrepareRx;

            SParkle_ConfigurationInit();
#endif

            break;
        case RF_BOARD_DDCDUC_DF8OE:
            RFboard.pa_info=&mchf_pa;       //default setting for mchf PA (overwitten later when other hardware was detected)
            RFboard.power_levelsInfo=&mchf_power_levelsInfo;
            RFboard.EnableTx  = DucDdc_Df8oe_EnableTx;
            RFboard.EnableRx = DucDdc_Df8oe_EnableRx;
            RFboard.PrepareTx  = DucDdc_Df8oe_PrepareTx;
            RFboard.PrepareRx = DucDdc_Df8oe_PrepareRx;

            break;
        case RF_BOARD_MCHF:
        case RF_BOARD_RS928:
        default: // HACK: in case we don't detect a board, we still initialize to mcHF RF for now.
            RFboard.pa_info=&mchf_pa;
            RFboard.power_levelsInfo=&mchf_power_levelsInfo;
            RFboard.EnableTx  = Mchf_EnableTx;
            RFboard.EnableRx = Mchf_EnableRx;
            RFboard.PrepareTx  = Mchf_PrepareTx;
            RFboard.PrepareRx = Mchf_PrepareRx;
    }
}
