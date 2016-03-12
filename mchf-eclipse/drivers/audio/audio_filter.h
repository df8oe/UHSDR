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
**  Licence:        CC BY-NC-SA 3.0                                                **
************************************************************************************/

#ifndef DRIVERS_AUDIO_AUDIO_FILTER_H_
#define DRIVERS_AUDIO_AUDIO_FILTER_H_

#include "mchf_types.h"
//
// Audio filter select enumeration
//
typedef struct FilterCoeffs
{
    float   rx_filt_q[128];
    uint16_t    rx_q_num_taps;
    uint32_t    rx_q_block_size;
    float   rx_filt_i[128];
    uint16_t    rx_i_num_taps;
    uint32_t    rx_i_block_size;
    //
    float   tx_filt_q[128];
    uint16_t    tx_q_num_taps;
    uint32_t    tx_q_block_size;
    float   tx_filt_i[128];
    uint16_t    tx_i_num_taps;
    uint32_t    tx_i_block_size;
} FilterCoeffs;



enum    {
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
    AUDIO_FILTER_NUM
};

enum {
  FILTER_CW = 1,
  FILTER_SSB = 2,
  FILTER_AM = 4,
  FILTER_FM = 8
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

typedef struct FilterConfig_s {
  const char*    label;
  const uint16_t offset;
} FilterConfig;

typedef struct FilterDescriptor_s {
  const uint8_t id;
  const char* name;
  const uint16_t width;
  const uint16_t allowed_modes;
  const uint16_t always_on_modes;
  const uint8_t configs_num;
  const uint8_t config_default;
  const FilterConfig* config;
} FilterDescriptor;

extern FilterDescriptor FilterInfo[AUDIO_FILTER_NUM];

typedef struct FilterPathDescriptor_s {
  const uint8_t id;
  const uint16_t mode;
  const uint8_t filter_select_id;
  const uint8_t FIR_numTaps;
  const float *FIR_I_coeff_file;
  const float *FIR_Q_coeff_file;
  const uint8_t FIR_dec_numTaps;
  const float *FIR_dec_coeff_file;
  const uint8_t sample_rate_dec;
  const bool IIR_PreFilter_yes;
  const uint16_t IIR_PreFilter_numTaps;
  const float *IIR_PreFilter_pk_file;
  const float *IIR_PreFilter_pv_file;
  const uint8_t FIR_int_numTaps;
  const float *FIR_int_coeff_file;
  const bool IIR_int_yes;
  const uint16_t *IIR_int_numTaps;
  const float *IIR_int_pk_file;
  const float *IIR_int_pv_file;
} FilterPathDescriptor;

extern FilterPathDescriptor FilterPathInfo[80]; // also change this figure in audio_filter.c
//
// Define visual widths of audio filters for on-screen indicator in Hz
//
//
#define HILBERT_3600HZ_WIDTH        3800    // Approximate bandwidth of 3.6 kHz wide Hilbert - This used to depict FM detection bandwidth
//
//
#define HILBERT3600         1900    // "width" of "3.6 kHz" Hilbert filter - This used to depict FM detection bandwidth
//

uint8_t AudioFilter_NextApplicableFilter();


#endif /* DRIVERS_AUDIO_AUDIO_FILTER_H_ */
