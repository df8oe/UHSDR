#ifndef _filters_h_
#define _filters_h_

#define __FIR_RX_DECIMATE_4_H
#define	RX_DECIMATE_NUM_TAPS	4
#define __FIR_RX_DECIMATE_4_MIN_LPF_H
#define	RX_DECIMATE_MIN_LPF_NUM_TAPS	4
#define __FIR_RX_INTERPOLATE_16_H
#define	RX_INTERPOLATE_NUM_TAPS	16
#define	RX_INTERPOLATE_4_NUM_TAPS	4
#define __FIR_RX_INTERPOLATE_16_10KHZ_H
#define	RX_INTERPOLATE_10KHZ_NUM_TAPS	16
#define __I_RX_FILTER_H
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define __I_RX_FILTER_10K_H
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define __I_RX_FILTER_3K6_H
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define __I_RX_FILTER_5K_H
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define __I_RX_FILTER_6K_H
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define __I_RX_FILTER_7K5_H
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define __I_RX_FILTER_H
#define I_BLOCK_SIZE		1
#define I_NUM_TAPS			89
#define __I_TX_FILTER_H
#define I_TX_BLOCK_SIZE		1
#define I_TX_NUM_TAPS			89
#define __I_TX_FILTER_H
#define I_TX_BLOCK_SIZE		1
#define I_TX_NUM_TAPS			89
#define __IIR_1K4
#define IIR_1k4_numStages 10
#define __IIR_1K6
#define IIR_1k6_numStages 10
#define __IIR_1_8K
#define IIR_1k8_numStages 10
#define __IIR_10K
//#define NCoef 8
#define __IIR_10K
#define __IIR_HPF_15K
#define __IIR_2K1
#define IIR_2k1_numStages 10
#define __IIR_2_3K
#define IIR_2k3_numStages 10
#define __IIR_2K5
#define IIR_2k5_numStages 10
#define __IIR_2_7K
#define IIR_2k7_numStages 10
#define __IIR_2_9K
#define IIR_2k9_numStages 10
#define __IIR_TX_2_7K
#define NCoef 10
#define NCoef 10
#define __IIR_TX_2_7K_FM
#define NCoef 10
#define __IIR_3K2
#define IIR_3k2_numStages 10
#define __IIR_3K4
#define IIR_3k4_numStages 10
#define __IIR_3_6K
#define IIR_3k6_numStages 10
#define __IIR_3K8
#define IIR_3k8_numStages 10
#define __IIR_300HZ
#define IIR_300hz_numStages 10
#define __IIR_3K
#define __IIR_4K2
#define IIR_4k2_numStages 10
#define __IIR_4_4K
#define IIR_4k4_numStages 10
#define __IIR_4_6K
#define IIR_4k6_numStages 10
#define __IIR_4_8K
#define IIR_4k8_numStages 10
#define __IIR_4K
#define IIR_4k_numStages 10
#define __IIR_5K5
#define IIR_5k5_numStages 10
#define __IIR_500HZ
#define IIR_500hz_numStages 10
#define __IIR_5K
#define __IIR_6K5
#define IIR_6k5_numStages 10
#define __IIR_6K
#define IIR_6k_numStages 10
#define __IIR_7K5
#define IIR_7k5_numStages 8
#define __IIR_7K
#define IIR_7k_numStages 8
#define __IIR_8K5
#define __IIR_8K
#define IIR_8k_numStages 8
#define __IIR_HPF_8K5
//#define NCoef 8
#define __IIR_9K5
#define __IIR_9K
#define __IIR_ANTIALIAS
//#define NCoef 6
#define IIR_aa_5k_numStages 6
#define IIR_aa_10k_numStages 6
#define IIR_aa_8k_numStages 6
#define IIR_aa_8k5_numStages 6
#define IIR_aa_9k_numStages 6
#define IIR_aa_9k5_numStages 6
#define __IQ_RX_FILTER_AM_10KHZ_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __IQ_RX_FILTER_AM_2k3_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __IQ_RX_FILTER_AM_3k6_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __IQ_RX_FILTER_AM_5KHZ_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __IQ_RX_FILTER_AM_6KHZ_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __IQ_RX_FILTER_AM_5KHZ5_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __Q_RX_FILTER_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __Q_RX_FILTER_10K_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __Q_RX_FILTER_3K6_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __Q_RX_FILTER_5K_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __Q_RX_FILTER_6K_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __Q_RX_FILTER_7K5_H
#define Q_BLOCK_SIZE		1
#define Q_NUM_TAPS			89
#define __Q_TX_FILTER_H
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
extern const uint16_t IIR_10k_numStages; 
extern const float IIR_10k_pkCoeffs[];
extern const float IIR_10k_pvCoeffs[];
extern const uint16_t IIR_10k_numStages;
extern const float IIR_10k_LPF_pkCoeffs[];
extern const float IIR_10k_LPF_pvCoeffs[];
extern const uint16_t IIR_15k_hpf_numStages; 
extern const float IIR_15k_hpf_pkCoeffs[];
extern const float IIR_15k_hpf_pvCoeffs[];
extern const uint16_t IIR_15k_hpf_numStages;
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
extern const uint16_t IIR_TX_2k7_numStages;
extern const float IIR_TX_2k7_pkCoeffs[];
extern const float IIR_TX_2k7_pvCoeffs[];
extern const uint16_t IIR_TX_2k7_numStages;
extern const float IIR_TX_2k7_pkCoeffs[];
extern const float IIR_TX_2k7_pvCoeffs[];
extern const uint16_t IIR_TX_2k7_FM_numStages;
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
extern const uint16_t IIR_3k_numStages;
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
extern const uint16_t IIR_5k_numStages;
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
extern const uint16_t IIR_8k5_numStages;
extern const float IIR_8k5_LPF_pkCoeffs[];
extern const float IIR_8k5_LPF_pvCoeffs[];
extern const float IIR_8k_LPF_pkCoeffs[];
extern const float IIR_8k_LPF_pvCoeffs[];
extern const uint16_t IIR_8k5_hpf_numStages;
extern const float IIR_8k5_hpf_pkCoeffs[];
extern const float IIR_8k5_hpf_pvCoeffs[];
extern const uint16_t IIR_9k5_numStages;
extern const float IIR_9k5_LPF_pkCoeffs[];
extern const float IIR_9k5_LPF_pvCoeffs[];
extern const uint16_t IIR_9k_numStages;
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
#endif
