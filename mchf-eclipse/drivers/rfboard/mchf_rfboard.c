/*
 * mchf_rfboard.c
 *
 *      Author: db4ple
 */
#include "uhsdr_board.h"
#include "mchf_rfboard.h"


// DIRECT HARDWARE ACCESS INCLUDES START
// required for Bias Control
#include "dac.h"

// for
#include "radio_management.h"

// for nr_params
#include "audio_nr.h"

// DIRECT HARDWARE ACCESS INCLUDES END

typedef struct {
} mchf_rf_board_data_t;

// static mchf_rf_board_data_t mchf_rf_board_data;


// Band control GPIOs setup
//
// -------------------------------------------
//   BAND       BAND0       BAND1       BAND2
//
//   80m        1           1           x
//   40m        1           0           x
//   20/30m     0           0           x
//   15-10m     0           1           x
//
// -------------------------------------------
//
void MchfRfBoard_BandCntr_Init(void)
{
#ifdef UI_BRD_MCHF
    // FIXME: USE HAL Init here as well, this handles also the multiple Ports case
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Mode     = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull     = GPIO_NOPULL;
    GPIO_InitStructure.Speed    = GPIO_SPEED_LOW;

    GPIO_InitStructure.Pin = BAND0|BAND1|BAND2;
    HAL_GPIO_Init(BAND0_PIO, &GPIO_InitStructure);
#endif
    // Set initial state - low (20m band)
    GPIO_ResetBits(BAND0_PIO,BAND0);
    GPIO_ResetBits(BAND1_PIO,BAND1);

    // Pulse the latch relays line, active low, so set high to disable
    GPIO_SetBits(BAND2_PIO,BAND2);
}



static void Board_Mchf_BandFilterPulseRelays()
{
    // FIXME: Replace non_os_delay with HAL_Delay
    GPIO_ResetBits(BAND2_PIO, BAND2);
    // TODO: Check if we can go down to 10ms as per datasheet
    // HAL_Delay(20);
    non_os_delay();
    GPIO_SetBits(BAND2_PIO, BAND2);
}

/**
 * @brief switches one of the four LPF&BPF groups into the RX/TX signal path
 * @param group 0: 80m, 1: 40m, 2: 20m , 3:10m
 */
void MchfRfBoard_SelectLpfBpf(uint8_t group)
{
    // -------------------------------------------
    //   BAND       BAND0       BAND1       BAND2
    //
    //   80m        1           1           x
    //   40m        1           0           x
    //   20/30m     0           0           x
    //   15-10m     0           1           x
    //
    // ---------------------------------------------
    // Set LPFs:
    // Set relays in groups, internal first, then external group
    // state change via two pulses on BAND2 line, then idle
    //
    // then
    //
    // Set BPFs
    // Constant line states for the BPF filter,
    // always last - after LPF change
    switch(group)
    {
    case 0:
    {
        // Internal group - Set(High/Low)
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        Board_Mchf_BandFilterPulseRelays();

        // External group -Set(High/High)
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        Board_Mchf_BandFilterPulseRelays();

        // BPF
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        break;
    }

    case 1:
    {
        // Internal group - Set(High/Low)
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        Board_Mchf_BandFilterPulseRelays();

        // External group - Reset(Low/High)
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        Board_Mchf_BandFilterPulseRelays();

        // BPF
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        break;
    }

    case 2:
    {
        // Internal group - Reset(Low/Low)
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        Board_Mchf_BandFilterPulseRelays();

        // External group - Reset(Low/High)
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        Board_Mchf_BandFilterPulseRelays();

        // BPF
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        break;
    }

    case 3:
    {
        // Internal group - Reset(Low/Low)
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_ResetBits(BAND1_PIO, BAND1);

        Board_Mchf_BandFilterPulseRelays();

        // External group - Set(High/High)
        GPIO_SetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        Board_Mchf_BandFilterPulseRelays();

        // BPF
        GPIO_ResetBits(BAND0_PIO, BAND0);
        GPIO_SetBits(BAND1_PIO, BAND1);

        break;
    }

    default:
        break;
    }

}




/**
 * @brief switch off the PA Bias to mute HF output ( even if PTT is on )
 * Using this method the PA will be effectively muted no matter what setting
 * the main bias switch has (which directly connected to the PTT HW Signal)
 * Used to suppress signal path reconfiguration noise during rx/tx and tx/rx switching
 */
static void MchfRfBoard_DisablePaBias()
{
    Mchf_Rf_Board_SetPaBiasValue(0);
}



bool Mchf_PrepareTx(void)
{
    Board_EnableTXSignalPath(true); // switch antenna to output and codec output to QSE mixer
    return true;
}

bool Mchf_PrepareRx(void)
{
    MchfRfBoard_DisablePaBias(); // kill bias to mute the HF output quickly
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


bool Mchf_Rf_Board_Init(void)
{
    MchfRfBoard_BandCntr_Init();
    return true;
}


/**
 * @brief set PA bias at the LM2931CDG (U18) using DAC Channel 2
 */
bool Mchf_Rf_Board_SetPaBiasValue(uint32_t bias)
{
    // Set DAC Channel 1 DHR12L register
    // DAC_SetChannel2Data(DAC_Align_8b_R,bias);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_8B_R, bias);

    return true;
}

typedef struct BandFilterDescriptor
{
    uint32_t upper;
    uint16_t band_mode;
} BandFilterDescriptor;

// TODO: This code below approach assumes that all filter hardware uses a set of separate filter banks
// other approaches such as configurable filters need a different approach, should be factored out
// into some hardware abstraction at some point

// The descriptor array below has to be ordered from the lowest BPF frequency filter
// to the highest.
static const BandFilterDescriptor mchf_rf_bandFilters[] =
{
    {  4000000,  0 },
    {  8000000,  1 },
    { 16000000,  2 },
    { 32000000,  3 },
};

const int BAND_FILTER_NUM = sizeof(mchf_rf_bandFilters)/sizeof(BandFilterDescriptor);


/**
 * @brief Select and activate the correct BPF for the frequency given in @p freq
 *
 *
 * @param freq The frequency to activate the BPF for in Hz
 *
 * @warning  If the frequency given in @p freq is too high for any of the filters, no filter change is executed.
 */
bool Mchf_SetHWFiltersForFrequency(uint32_t freq)
{
    bool retval = false;
    for (int idx = 0; idx < BAND_FILTER_NUM; idx++)
    {
        if(freq < mchf_rf_bandFilters[idx].upper)       // are we low enough if frequency for this band filter?
        {
            if(ts.filter_band != mchf_rf_bandFilters[idx].band_mode)
            {
                MchfRfBoard_SelectLpfBpf(mchf_rf_bandFilters[idx].band_mode);
                ts.filter_band = mchf_rf_bandFilters[idx].band_mode;

                // TODO: get that out of here, it should be generalized to work whenever we make a band switch
                // not just with the mcHF board
                nr_params.first_time = 1; // in case of any Bandfilter change restart the NR routine
            }
            retval = true;
            break;
        }
    }

    return retval;
}
