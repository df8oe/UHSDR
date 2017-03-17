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

#include "mchf_types.h"
#include "arm_math.h"
//
extern arm_fir_instance_f32    FIR_Q_TX;
extern arm_fir_instance_f32    FIR_I_TX;
extern arm_fir_instance_f32    FIR_Q;
extern arm_fir_instance_f32    FIR_I;
extern arm_fir_instance_f32    FIR_Q_FREEDV;
extern arm_fir_instance_f32    FIR_I_FREEDV;
extern arm_fir_decimate_instance_f32 DECIMATE_SAM_I;
extern arm_fir_decimate_instance_f32 DECIMATE_SAM_Q;



// Audio filter select enumeration
//
void 	AudioFilter_InitRxHilbertFIR(uint8_t dmod_mode);
//void    AudioFilter_CalcRxPhaseAdj(void);
void 	AudioFilter_InitTxHilbertFIR(void);
//void    AudioFilter_CalcTxPhaseAdj(void);



typedef struct FilterCoeffs
{
    float   rx_filt_q[128];
    uint16_t    rx_q_num_taps;
    uint32_t    rx_q_block_size;
    float   rx_filt_i[128];
    uint16_t    rx_i_num_taps;
    uint32_t    rx_i_block_size;
    //
    //    float   tx_filt_q[128];
    float   tx_filt_q[201];
    uint16_t    tx_q_num_taps;
    uint32_t    tx_q_block_size;
    //    float   tx_filt_i[128];
    float   tx_filt_i[201];
    uint16_t    tx_i_num_taps;
    uint32_t    tx_i_block_size;

    uint16_t    freedv_q_num_taps;
    uint32_t    freedv_q_block_size;

    uint16_t    freedv_i_num_taps;
    uint32_t    freedv_i_block_size;


} FilterCoeffs;



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

//
//
#define AUDIO_DEFAULT_FILTER        AUDIO_2P3KHZ
//
// use below to define the lowest-used filter number
//
#define AUDIO_MIN_FILTER        0
//
// use below to define the highest-used filter number-1
//
#define AUDIO_MAX_FILTER        (AUDIO_FILTER_NUM-1)
//


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
    const uint8_t FIR_numTaps;
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
//
// Define visual widths of audio filters for on-screen indicator in Hz
//
//
#define HILBERT_3600HZ_WIDTH        3800    // Approximate bandwidth of 3.6 kHz wide Hilbert - This used to depict FM detection bandwidth
//
//
#define HILBERT3600         1900    // "width" of "3.6 kHz" Hilbert filter - This used to depict FM detection bandwidth
//

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
uint8_t  AudioFilter_NextApplicableFilter();
void     AudioFilter_SetDefaultMemories();

#endif /* DRIVERS_AUDIO_AUDIO_FILTER_H_ */
