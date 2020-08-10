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
**  Licence:        GNU GPLv3                                                      **
************************************************************************************/

#ifndef DRIVERS_AUDIO_AUDIO_FILTER_H_
#define DRIVERS_AUDIO_AUDIO_FILTER_H_

#include "uhsdr_types.h"
#include "arm_math.h"

// TODO: Decide if we switch to use this struct
typedef struct
{
    arm_fir_instance_f32 I;
    arm_fir_instance_f32 Q;
} FIR_IQ_t;

extern arm_fir_instance_f32    Fir_Tx_Hilbert_Q;
extern arm_fir_instance_f32    Fir_Tx_Hilbert_I;
extern arm_fir_instance_f32    Fir_Rx_Hilbert_Q;
extern arm_fir_instance_f32    Fir_Rx_Hilbert_I;
extern arm_fir_instance_f32    Fir_TxFreeDV_Interpolate_Q;
extern arm_fir_instance_f32    Fir_TxFreeDV_Interpolate_I;
extern arm_fir_decimate_instance_f32 DECIMATE_RX_I;
extern arm_fir_decimate_instance_f32 DECIMATE_RX_Q;


void 	AudioFilter_SetRxHilbertAndDecimationFIR(uint8_t dmod_mode);
void 	AudioFilter_SetTxHilbertFIR(void);

enum
{
    AUDIO_300HZ = 0,
    AUDIO_500HZ,
    AUDIO_1P4KHZ,
    AUDIO_1P6KHZ,
    AUDIO_1P8KHZ,
    AUDIO_2P1KHZ,
    AUDIO_2P3KHZ,
    AUDIO_2P5KHZ,
    AUDIO_2P7KHZ,
    AUDIO_2P9KHZ,
    AUDIO_3P2KHZ,
    AUDIO_3P4KHZ,
    AUDIO_3P6KHZ,
    AUDIO_3P8KHZ,
    AUDIO_4P0KHZ,
    AUDIO_4P2KHZ,
    AUDIO_4P4KHZ,
    AUDIO_4P6KHZ,
    AUDIO_4P8KHZ,
    AUDIO_5P0KHZ,
    AUDIO_5P5KHZ,
    AUDIO_6P0KHZ,
    AUDIO_6P5KHZ,
    AUDIO_7P0KHZ,
    AUDIO_7P5KHZ,
    AUDIO_8P0KHZ,
    AUDIO_8P5KHZ,
    AUDIO_9P0KHZ,
    AUDIO_9P5KHZ,
    AUDIO_10P0KHZ,
    AUDIO_OFF,
    AUDIO_FILTER_NUM
};

enum
{
    FILTER_MODE_CW = 0,
    FILTER_MODE_SSB,
    FILTER_MODE_AM,
    FILTER_MODE_FM,
    FILTER_MODE_SAM,
    FILTER_MODE_MAX
};

#define AUDIO_DEFAULT_FILTER        AUDIO_2P3KHZ
//
// use below to define the lowest-used filter number
//
#define AUDIO_MIN_FILTER        0
//
// use below to define the highest-used filter number-1
//
#define AUDIO_MAX_FILTER        (AUDIO_FILTER_NUM-1)


typedef struct FilterDescriptor_s
{
    const uint8_t id;
    const char* const name;
    const uint16_t width;
} FilterDescriptor;

extern const FilterDescriptor FilterInfo[AUDIO_FILTER_NUM];
extern uint16_t filterpath_mode_map[FILTER_MODE_MAX];

typedef struct FilterPathDescriptor_s
{
    const uint8_t id;
    const char* name;
    const uint16_t mode;
    const uint8_t filter_select_id;


    // arm_fir_instance_f32*
    const uint16_t FIR_numTaps;
    const float *FIR_I_coeff_file;
    // arm_fir_instance_f32*
    const float *FIR_Q_coeff_file;

    // arm_fir_decimate_instance_f32*
    const arm_fir_decimate_instance_f32* dec;

    const uint8_t sample_rate_dec;

    const arm_iir_lattice_instance_f32* pre_instance;
    const arm_fir_interpolate_instance_f32* interpolate;
    const arm_iir_lattice_instance_f32* iir_instance;
//  const arm_biquad_casd_df1_inst_f32* notch_instance;

    const uint16_t offset; // how much offset in Hz has the center frequency of the filter from base frequency.
    // remark: this IS in fact IDENTICAL to the CENTRE FREQUENCY of the filter
    // For most non-CW filters 0 is okay,
    // in this case bandwidth/2 is being used here.
} FilterPathDescriptor;


#define AUDIO_FILTER_PATH_NUM 87

extern const FilterPathDescriptor FilterPathInfo[AUDIO_FILTER_PATH_NUM];

enum
{
    PATH_ALL_APPLICABLE = 0,
    PATH_NEXT_BANDWIDTH = 1,
    PATH_SAME_BANDWITH =2,
    PATH_UP = 4,
    PATH_DOWN = 8,
    PATH_USE_RULES = 16,
    PATH_LAST_USED_IN_MODE = 32,
    PATH_DONT_STORE = 64,
};

uint8_t  AudioFilter_NextApplicableFilterPath(const uint16_t query, const uint16_t filter_mode, const uint8_t current_path);
bool     AudioFilter_IsApplicableFilterPath(const uint16_t query, const uint16_t filter_mode, const uint8_t filter_path);
void     AudioFilter_GetNamesOfFilterPath(uint16_t filter_path,const char** filter_names);
uint16_t AudioFilter_GetFilterModeFromDemodMode(uint8_t dmod_mode);
uint8_t  AudioFilter_NextApplicableFilter(void);
void     AudioFilter_SetDefaultMemories(void);


typedef struct
{
	int a;
	float32_t b;
	float32_t sin;
	float32_t cos;
	float32_t r;
	float32_t buf[3];
} Goertzel;


void AudioFilter_CalcGoertzel(Goertzel* g, float32_t freq, const uint32_t size, const float goertzel_coeff, float32_t samplerate);
void AudioFilter_GoertzelInput(Goertzel* goertzel, float32_t in);
float32_t AudioFilter_GoertzelEnergy(Goertzel* goertzel);

#endif /* DRIVERS_AUDIO_AUDIO_FILTER_H_ */
