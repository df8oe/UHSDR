/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
#ifndef _filters_h_
#define _filters_h_

#include "mchf_board.h"
#include "arm_math.h"

#define I_BLOCK_SIZE		IQ_BUFSZ
#define Q_BLOCK_SIZE        IQ_BUFSZ

#define I_NUM_TAPS			89
#define Q_NUM_TAPS			89

typedef struct {
    const float32_t* i;
    const float32_t* q;
    const int num_taps;
} IQ_FilterDescriptor;

extern const arm_fir_decimate_instance_f32 FirRxDecimate;
extern const arm_fir_decimate_instance_f32 FirZoomFFTDecimate[6];
extern const arm_fir_decimate_instance_f32 FirRxDecimateMinLPF;
extern const arm_fir_interpolate_instance_f32 FirRxInterpolate;
extern const arm_fir_interpolate_instance_f32 FirRxInterpolate_4_5k;
extern const arm_fir_interpolate_instance_f32 FirRxInterpolate_4_10k;
extern const arm_fir_interpolate_instance_f32 FirRxInterpolate10KHZ;
extern const arm_fir_instance_f32 FirFreeDVInterpolate;
extern const float FreeDV_FIR_interpolate[24];

extern const float iq_rx_am_10k_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_2k3_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_3k6_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_5k_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_4k5_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_6k_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_7k5_coeffs[Q_NUM_TAPS];

extern const float i_rx_10k_coeffs[I_NUM_TAPS];
extern const float i_rx_3k6_coeffs[I_NUM_TAPS];
extern const float i_rx_4k5_coeffs[I_NUM_TAPS];
extern const float i_rx_5k_coeffs[I_NUM_TAPS];
extern const float i_rx_6k_coeffs[I_NUM_TAPS];
extern const float i_rx_7k5_coeffs[I_NUM_TAPS];
extern const float i_rx_coeffs[I_NUM_TAPS];

extern const float q_rx_coeffs[Q_NUM_TAPS];
extern const float q_rx_10k_coeffs[Q_NUM_TAPS];
extern const float q_rx_3k6_coeffs[Q_NUM_TAPS];
extern const float q_rx_4k5_coeffs[Q_NUM_TAPS];
extern const float q_rx_5k_coeffs[Q_NUM_TAPS];
extern const float q_rx_6k_coeffs[Q_NUM_TAPS];
extern const float q_rx_7k5_coeffs[Q_NUM_TAPS];


extern const arm_iir_lattice_instance_f32 IIR_1k4_LPF;
extern const arm_iir_lattice_instance_f32 IIR_1k4_BPF;
//extern const arm_iir_lattice_instance_f32 IIR_1k4_SSTV;
extern const arm_iir_lattice_instance_f32 IIR_1k6_LPF;
extern const arm_iir_lattice_instance_f32 IIR_1k6_BPF;
extern const arm_iir_lattice_instance_f32 IIR_1k8_1k425;
extern const arm_iir_lattice_instance_f32 IIR_1k8_1k275;
extern const arm_iir_lattice_instance_f32 IIR_1k8_1k125;
extern const arm_iir_lattice_instance_f32 IIR_1k8_1k575;
extern const arm_iir_lattice_instance_f32 IIR_1k8_1k725;
extern const arm_iir_lattice_instance_f32 IIR_1k8_LPF;
extern const arm_iir_lattice_instance_f32 IIR_10k;
extern const arm_iir_lattice_instance_f32 IIR_10k_LPF;
extern const arm_iir_lattice_instance_f32 IIR_15k_hpf;
extern const arm_iir_lattice_instance_f32 IIR_15k_hpf;
extern const arm_iir_lattice_instance_f32 IIR_2k1_LPF;
extern const arm_iir_lattice_instance_f32 IIR_2k1_BPF;
extern const arm_iir_lattice_instance_f32 IIR_2k3_1k412;
extern const arm_iir_lattice_instance_f32 IIR_2k3_1k275;
extern const arm_iir_lattice_instance_f32 IIR_2k3_1k562;
extern const arm_iir_lattice_instance_f32 IIR_2k3_1k712;
extern const arm_iir_lattice_instance_f32 IIR_2k3_LPF;
extern const arm_iir_lattice_instance_f32 IIR_2k5_LPF;
extern const arm_iir_lattice_instance_f32 IIR_2k5_BPF;
extern const arm_iir_lattice_instance_f32 IIR_2k7_LPF;
extern const arm_iir_lattice_instance_f32 IIR_2k7_BPF;
extern const arm_iir_lattice_instance_f32 IIR_2k9_LPF;
extern const arm_iir_lattice_instance_f32 IIR_2k9_BPF;
extern const arm_iir_lattice_instance_f32 IIR_TX_2k7;
extern const arm_iir_lattice_instance_f32 IIR_TX_WIDE_BASS;
extern const arm_iir_lattice_instance_f32 IIR_TX_WIDE_TREBLE;
extern const arm_iir_lattice_instance_f32 IIR_TX_SOPRANO;
extern const arm_iir_lattice_instance_f32 IIR_TX_2k7_FM;
extern const arm_iir_lattice_instance_f32 IIR_3k2_LPF;
extern const arm_iir_lattice_instance_f32 IIR_3k2_BPF;
extern const arm_iir_lattice_instance_f32 IIR_3k4_LPF;
extern const arm_iir_lattice_instance_f32 IIR_3k4_BPF;
extern const arm_iir_lattice_instance_f32 IIR_3k6_LPF;
extern const arm_iir_lattice_instance_f32 IIR_3k6_BPF;
extern const arm_iir_lattice_instance_f32 IIR_3k8_LPF;
extern const arm_iir_lattice_instance_f32 IIR_3k8_BPF;
extern const arm_iir_lattice_instance_f32 IIR_300hz_750;
extern const arm_iir_lattice_instance_f32 IIR_300hz_800;
extern const arm_iir_lattice_instance_f32 IIR_300hz_850;
extern const arm_iir_lattice_instance_f32 IIR_300hz_900;
extern const arm_iir_lattice_instance_f32 IIR_300hz_950;
extern const arm_iir_lattice_instance_f32 IIR_300hz_700;
extern const arm_iir_lattice_instance_f32 IIR_300hz_650;
extern const arm_iir_lattice_instance_f32 IIR_300hz_600;
extern const arm_iir_lattice_instance_f32 IIR_300hz_550;
extern const arm_iir_lattice_instance_f32 IIR_300hz_500;
extern const arm_iir_lattice_instance_f32 IIR_3k;
extern const arm_iir_lattice_instance_f32 IIR_4k2_LPF;
extern const arm_iir_lattice_instance_f32 IIR_4k4_LPF;
extern const arm_iir_lattice_instance_f32 IIR_4k6_LPF;
extern const arm_iir_lattice_instance_f32 IIR_4k8_LPF;
extern const arm_iir_lattice_instance_f32 IIR_4k_LPF;
extern const arm_iir_lattice_instance_f32 IIR_5k5_LPF;
extern const arm_iir_lattice_instance_f32 IIR_500hz_750;
extern const arm_iir_lattice_instance_f32 IIR_500hz_850;
extern const arm_iir_lattice_instance_f32 IIR_500hz_950;
extern const arm_iir_lattice_instance_f32 IIR_500hz_650;
extern const arm_iir_lattice_instance_f32 IIR_500hz_550;
extern const arm_iir_lattice_instance_f32 IIR_5k_LPF;
extern const arm_iir_lattice_instance_f32 IIR_6k5_LPF;
extern const arm_iir_lattice_instance_f32 IIR_6k_LPF;
extern const arm_iir_lattice_instance_f32 IIR_7k5_LPF;
extern const arm_iir_lattice_instance_f32 IIR_7k_LPF;
extern const arm_iir_lattice_instance_f32 IIR_8k5_LPF;
extern const arm_iir_lattice_instance_f32 IIR_8k_LPF;
extern const arm_iir_lattice_instance_f32 IIR_8k5_hpf;
extern const arm_iir_lattice_instance_f32 IIR_9k5_LPF;
extern const arm_iir_lattice_instance_f32 IIR_9k_LPF;
extern const arm_iir_lattice_instance_f32 IIR_aa_5k;
extern const arm_iir_lattice_instance_f32 IIR_aa_10k;
extern const arm_iir_lattice_instance_f32 IIR_aa_8k;
extern const arm_iir_lattice_instance_f32 IIR_aa_8k5;
extern const arm_iir_lattice_instance_f32 IIR_aa_9k;
extern const arm_iir_lattice_instance_f32 IIR_aa_9k5;
#endif
