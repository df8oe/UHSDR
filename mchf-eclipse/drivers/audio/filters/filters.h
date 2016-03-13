#ifndef _filters_h_
#define _filters_h_

#define	RX_DECIMATE_NUM_TAPS	4
#define	RX_DECIMATE_MIN_LPF_NUM_TAPS	4
#define	RX_INTERPOLATE_NUM_TAPS	16
#define	RX_INTERPOLATE_4_NUM_TAPS	4
#define	RX_INTERPOLATE_10KHZ_NUM_TAPS	16
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define I_TX_BLOCK_SIZE		1
#define I_TX_NUM_TAPS			89
#define I_TX_BLOCK_SIZE		1
#define I_TX_NUM_TAPS			89
#define IIR_1k4_numStages 10
#define IIR_1k6_numStages 10
#define IIR_1k8_numStages 10
//#define NCoef 8
#define IIR_2k1_numStages 10
#define IIR_2k3_numStages 10
#define IIR_2k5_numStages 10
#define IIR_2k7_numStages 10
#define IIR_2k9_numStages 10
#define IIR_3k2_numStages 10
#define IIR_3k4_numStages 10
#define IIR_3k6_numStages 10
#define IIR_3k8_numStages 10
#define IIR_300hz_numStages 10
#define IIR_4k2_numStages 10
#define IIR_4k4_numStages 10
#define IIR_4k6_numStages 10
#define IIR_4k8_numStages 10
#define IIR_4k_numStages 10
#define IIR_5k5_numStages 10
#define IIR_500hz_numStages 10
#define IIR_6k5_numStages 10
#define IIR_6k_numStages 10
#define IIR_7k5_numStages 8
#define IIR_7k_numStages 8
#define IIR_8k_numStages 8
#define IIR_aa_5k_numStages 6
#define IIR_aa_10k_numStages 6
#define IIR_aa_8k_numStages 6
#define IIR_aa_8k5_numStages 6
#define IIR_aa_9k_numStages 6
#define IIR_aa_9k5_numStages 6
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define Q_TX_BLOCK_SIZE		1
#define Q_TX_NUM_TAPS			89

extern const float FirRxDecimate[];
extern const float FirRxDecimateMinLPF[];
extern const float FirRxInterpolate[];
extern const float FirRxInterpolate_4_5k[];
extern const float FirRxInterpolate_4_10k[];
extern const float FirRxInterpolate10KHZ[];
extern const float i_rx_coeffs[I_NUM_TAPS];
extern const float i_rx_10k_coeffs[I_NUM_TAPS];
extern const float i_rx_3k6_coeffs[I_NUM_TAPS];
extern const float i_rx_5k_coeffs[I_NUM_TAPS];
extern const float i_rx_6k_coeffs[I_NUM_TAPS];
extern const float i_rx_7k5_coeffs[I_NUM_TAPS];
extern const float i_rx_coeffs[I_NUM_TAPS];
extern const float i_tx_coeffs[I_NUM_TAPS];
extern const float i_tx_coeffs[I_NUM_TAPS];
extern const float i_tx_coeffs[I_NUM_TAPS];
extern const float IIR_1k4_LPF_pkCoeffs[];
extern const float IIR_1k4_LPF_pvCoeffs[];
extern const float IIR_1k4_BPF_pkCoeffs[];
extern const float IIR_1k4_BPF_pvCoeffs[];
extern const float IIR_1k4_SSTV_pkCoeffs[];
extern const float IIR_1k4_SSTV_pvCoeffs[];
extern const float IIR_1k6_LPF_pkCoeffs[];
extern const float IIR_1k6_LPF_pvCoeffs[];
extern const float IIR_1k6_BPF_pkCoeffs[];
extern const float IIR_1k6_BPF_pvCoeffs[];
extern const float IIR_1k8_1k425_pkCoeffs[];
extern const float IIR_1k8_1k425_pvCoeffs[];
extern const float IIR_1k8_1k275_pkCoeffs[];
extern const float IIR_1k8_1k275_pvCoeffs[];
extern const float IIR_1k8_1k125_pkCoeffs[];
extern const float IIR_1k8_1k125_pvCoeffs[];
extern const float IIR_1k8_1k575_pkCoeffs[];
extern const float IIR_1k8_1k575_pvCoeffs[];
extern const float IIR_1k8_1k725_pkCoeffs[];
extern const float IIR_1k8_1k725_pvCoeffs[];
extern const float IIR_1k8_LPF_pkCoeffs[];
extern const float IIR_1k8_LPF_pvCoeffs[];
#define IIR_10k_numStages 8
extern const float IIR_10k_pkCoeffs[];
extern const float IIR_10k_pvCoeffs[];
extern const float IIR_10k_LPF_pkCoeffs[];
extern const float IIR_10k_LPF_pvCoeffs[];
#define IIR_15k_hpf_numStages 6
extern const float IIR_15k_hpf_pkCoeffs[];
extern const float IIR_15k_hpf_pvCoeffs[];
extern const float IIR_2k1_LPF_pkCoeffs[];
extern const float IIR_2k1_LPF_pvCoeffs[];
extern const float IIR_2k1_BPF_pkCoeffs[];
extern const float IIR_2k1_BPF_pvCoeffs[];
extern const float IIR_2k3_1k412_pkCoeffs[];
extern const float IIR_2k3_1k412_pvCoeffs[];
extern const float IIR_2k3_1k275_pkCoeffs[];
extern const float IIR_2k3_1k275_pvCoeffs[];
extern const float IIR_2k3_1k562_pkCoeffs[];
extern const float IIR_2k3_1k562_pvCoeffs[];
extern const float IIR_2k3_1k712_pkCoeffs[];
extern const float IIR_2k3_1k712_pvCoeffs[];
extern const float IIR_2k3_LPF_pkCoeffs[];
extern const float IIR_2k3_LPF_pvCoeffs[];
extern const float IIR_2k5_LPF_pkCoeffs[];
extern const float IIR_2k5_LPF_pvCoeffs[];
extern const float IIR_2k5_BPF_pkCoeffs[];
extern const float IIR_2k5_BPF_pvCoeffs[];
extern const float IIR_2k7_LPF_pkCoeffs[];
extern const float IIR_2k7_LPF_pvCoeffs[];
extern const float IIR_2k7_BPF_pkCoeffs[];
extern const float IIR_2k7_BPF_pvCoeffs[];
extern const float IIR_2k9_LPF_pkCoeffs[];
extern const float IIR_2k9_LPF_pvCoeffs[];
extern const float IIR_2k9_BPF_pkCoeffs[];
extern const float IIR_2k9_BPF_pvCoeffs[];
#define IIR_TX_2k7_FM_numStages 10
extern const float IIR_TX_2k7_pkCoeffs[];
extern const float IIR_TX_2k7_pvCoeffs[];
#define IIR_TX_2k7_numStages 10
extern const float IIR_TX_2k7_FM_pkCoeffs[];
extern const float IIR_TX_2k7_FM_pvCoeffs[];
extern const float IIR_3k2_LPF_pkCoeffs[];
extern const float IIR_3k2_LPF_pvCoeffs[];
extern const float IIR_3k2_BPF_pkCoeffs[];
extern const float IIR_3k2_BPF_pvCoeffs[];
extern const float IIR_3k4_LPF_pkCoeffs[];
extern const float IIR_3k4_LPF_pvCoeffs[];
extern const float IIR_3k4_BPF_pkCoeffs[];
extern const float IIR_3k4_BPF_pvCoeffs[];
extern const float IIR_3k6_LPF_pkCoeffs[];
extern const float IIR_3k6_LPF_pvCoeffs[];
extern const float IIR_3k6_BPF_pkCoeffs[];
extern const float IIR_3k6_BPF_pvCoeffs[];
extern const float IIR_3k8_LPF_pkCoeffs[];
extern const float IIR_3k8_LPF_pvCoeffs[];
extern const float IIR_3k8_BPF_pkCoeffs[];
extern const float IIR_3k8_BPF_pvCoeffs[];
extern const float IIR_300hz_750_pkCoeffs[];
extern const float IIR_300hz_750_pvCoeffs[];
extern const float IIR_300hz_800_pkCoeffs[];
extern const float IIR_300hz_800_pvCoeffs[];
extern const float IIR_300hz_850_pkCoeffs[];
extern const float IIR_300hz_850_pvCoeffs[];
extern const float IIR_300hz_900_pkCoeffs[];
extern const float IIR_300hz_900_pvCoeffs[];
extern const float IIR_300hz_950_pkCoeffs[];
extern const float IIR_300hz_950_pvCoeffs[];
extern const float IIR_300hz_700_pkCoeffs[];
extern const float IIR_300hz_700_pvCoeffs[];
extern const float IIR_300hz_650_pkCoeffs[];
extern const float IIR_300hz_650_pvCoeffs[];
extern const float IIR_300hz_600_pkCoeffs[];
extern const float IIR_300hz_600_pvCoeffs[];
extern const float IIR_300hz_550_pkCoeffs[];
extern const float IIR_300hz_550_pvCoeffs[];
extern const float IIR_300hz_500_pkCoeffs[];
extern const float IIR_300hz_500_pvCoeffs[];
#define IIR_3k_numStages 10
extern const float IIR_3k_pkCoeffs[];
extern const float IIR_3k_pvCoeffs[];
extern const float IIR_4k2_LPF_pkCoeffs[];
extern const float IIR_4k2_LPF_pvCoeffs[];
extern const float IIR_4k4_LPF_pkCoeffs[];
extern const float IIR_4k4_LPF_pvCoeffs[];
extern const float IIR_4k6_LPF_pkCoeffs[];
extern const float IIR_4k6_LPF_pvCoeffs[];
extern const float IIR_4k8_LPF_pkCoeffs[];
extern const float IIR_4k8_LPF_pvCoeffs[];
extern const float IIR_4k_LPF_pkCoeffs[];
extern const float IIR_4k_LPF_pvCoeffs[];
extern const float IIR_5k5_LPF_pkCoeffs[];
extern const float IIR_5k5_LPF_pvCoeffs[];
extern const float IIR_500hz_750_pkCoeffs[];
extern const float IIR_500hz_750_pvCoeffs[];
extern const float IIR_500hz_850_pkCoeffs[];
extern const float IIR_500hz_850_pvCoeffs[];
extern const float IIR_500hz_950_pkCoeffs[];
extern const float IIR_500hz_950_pvCoeffs[];
extern const float IIR_500hz_650_pkCoeffs[];
extern const float IIR_500hz_650_pvCoeffs[];
extern const float IIR_500hz_550_pkCoeffs[];
extern const float IIR_500hz_550_pvCoeffs[];
#define IIR_5k_numStages 10
extern const float IIR_5k_LPF_pkCoeffs[];
extern const float IIR_5k_LPF_pvCoeffs[];
extern const float IIR_6k5_LPF_pkCoeffs[];
extern const float IIR_6k5_LPF_pvCoeffs[];
extern const float IIR_6k_LPF_pkCoeffs[];
extern const float IIR_6k_LPF_pvCoeffs[];
extern const float IIR_7k5_LPF_pkCoeffs[];
extern const float IIR_7k5_LPF_pvCoeffs[];
extern const float IIR_7k_LPF_pkCoeffs[];
extern const float IIR_7k_LPF_pvCoeffs[];
#define IIR_8k5_numStages 8
extern const float IIR_8k5_LPF_pkCoeffs[];
extern const float IIR_8k5_LPF_pvCoeffs[];
extern const float IIR_8k_LPF_pkCoeffs[];
extern const float IIR_8k_LPF_pvCoeffs[];
#define IIR_8k5_hpf_numStages 6
extern const float IIR_8k5_hpf_pkCoeffs[];
extern const float IIR_8k5_hpf_pvCoeffs[];
#define IIR_9k5_numStages 8
extern const float IIR_9k5_LPF_pkCoeffs[];
extern const float IIR_9k5_LPF_pvCoeffs[];
#define IIR_9k_numStages 8
extern const float IIR_9k_LPF_pkCoeffs[];
extern const float IIR_9k_LPF_pvCoeffs[];
extern const float IIR_aa_5k_pkCoeffs[];
extern const float IIR_aa_5k_pvCoeffs[];
extern const float IIR_aa_10k_pkCoeffs[];
extern const float IIR_aa_10k_pvCoeffs[];
extern const float IIR_aa_8k_pkCoeffs[];
extern const float IIR_aa_8k_pvCoeffs[];
extern const float IIR_aa_8k5_pkCoeffs[];
extern const float IIR_aa_8k5_pvCoeffs[];
extern const float IIR_aa_9k_pkCoeffs[];
extern const float IIR_aa_9k_pvCoeffs[];
extern const float IIR_aa_9k5_pkCoeffs[];
extern const float IIR_aa_9k5_pvCoeffs[];
extern const float iq_rx_am_10k_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_2k3_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_3k6_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_5k_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_6k_coeffs[Q_NUM_TAPS];
extern const float iq_rx_am_7k5_coeffs[Q_NUM_TAPS];
extern const float q_rx_coeffs[Q_NUM_TAPS];
extern const float q_rx_coeffs_minus[Q_NUM_TAPS];
extern const float q_rx_coeffs_plus[Q_NUM_TAPS];
extern const float q_rx_10k_coeffs[Q_NUM_TAPS];
extern const float q_rx_10k_coeffs_minus[Q_NUM_TAPS];
extern const float q_rx_10k_coeffs_plus[Q_NUM_TAPS];
extern const float q_rx_3k6_coeffs[Q_NUM_TAPS];
extern const float q_rx_3k6_coeffs_minus[Q_NUM_TAPS];
extern const float q_rx_3k6_coeffs_plus[Q_NUM_TAPS];
extern const float q_rx_5k_coeffs[Q_NUM_TAPS];
extern const float q_rx_5k_coeffs_minus[Q_NUM_TAPS];
extern const float q_rx_5k_coeffs_plus[Q_NUM_TAPS];
extern const float q_rx_6k_coeffs[Q_NUM_TAPS];
extern const float q_rx_6k_coeffs_minus[Q_NUM_TAPS];
extern const float q_rx_6k_coeffs_plus[Q_NUM_TAPS];
extern const float q_rx_7k5_coeffs[Q_NUM_TAPS];
extern const float q_rx_7k5_coeffs_minus[Q_NUM_TAPS];
extern const float q_rx_7k5_coeffs_plus[Q_NUM_TAPS];
extern const float q_tx_coeffs[Q_NUM_TAPS];
extern const float q_tx_coeffs_minus[Q_NUM_TAPS];
extern const float q_tx_coeffs_plus[Q_NUM_TAPS];
extern const float q_tx_coeffs[Q_NUM_TAPS];
extern const float q_tx_coeffs_minus[Q_NUM_TAPS];
extern const float q_tx_coeffs_plus[Q_NUM_TAPS];
#define IIR_1k6_numStages 10
#define IIR_1k8_numStages 10
#define IIR_10k_numStages 8
#define IIR_15k_hpf_numStages 6
#define IIR_15k_hpf_numStages 6
#define IIR_2k1_numStages 10
#define IIR_2k3_numStages 10 
#define IIR_2k5_numStages 10
#define IIR_2k9_numStages 10
#define IIR_TX_2k7_numStages 10
#define IIR_TX_2k7_numStages 10 
#define IIR_3k2_numStages 10
#define IIR_3k4_numStages 10
#define IIR_3k6_numStages 10
#define IIR_3k8_numStages 10
#define IIR_300hz_numStages 10 
#define IIR_4k2_numStages 10
#define IIR_4k4_numStages 10
#define IIR_4k6_numStages 10
#define IIR_4k8_numStages 10
#define IIR_5k5_numStages 10
#define IIR_500hz_numStages 10 
#define IIR_6k5_numStages 10
#define IIR_6k_numStages 10
#define IIR_7k5_numStages 8
#define IIR_7k_numStages 8
#define IIR_aa_5k_numStages 6
#define IIR_aa_10k_numStages 6
#define IIR_aa_8k_numStages 6
#define IIR_aa_8k5_numStages 6
#define IIR_aa_9k_numStages 6
#define IIR_aa_9k5_numStages 6
#endif
