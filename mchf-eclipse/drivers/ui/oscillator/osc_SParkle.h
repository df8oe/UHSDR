/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  File name:     osc_FPGA_DDC.h                                                 **
 **  Description:   SParkle interface, FPGA DDC management                         **
 **  Licence:       GNU GPLv3                                                      **
 **  Author:        Slawomir Balon/SP9BSL                                          **
 ************************************************************************************/

#ifndef UI_OSCILLATOR_OSC_SPARKLE_H_
#define UI_OSCILLATOR_OSC_SPARKLE_H_


typedef struct
{
    bool is_present;
#ifdef USE_OSC_SParkle
    uint8_t version_major;
    uint8_t version_minor;
    uint32_t current_frequency;
    uint32_t next_frequency;
    uint8_t next_BB_reg1;
    uint8_t next_BB_reg2;
    uint8_t current_BB_reg1;
    uint8_t current_BB_reg2;
    float32_t ppm;
    uint32_t DDC_RegConfig;
    uint8_t Nyquist_Zone;       //number of Nyquist zone for current frequency
    uint32_t AntiAliasFilterSeting;
    uint32_t prevAntiAliasFilterSeting;
    uint8_t TestStatus;
    uint8_t RX_amp_idx;
    uint8_t current_RX_amp_idx;
#endif
}SParkleState_t;

extern SParkleState_t SParkleState;

#define oscDDC_f_sample 122880000
#define SParkleStat_BaseBoardPresent 0x01

#define SParkleDacType_orig 0       //AD9744 original
#define SParkleDacType_clone 1      //AD9744 chinese clone (reversed MSB for U2 data interface)

enum SParkle_DDCboard_{SParkle_DDCboard_OK=0,SParkle_DDCboard_Fail};
bool SParkle_IsPresent(void);
void osc_SParkle_Init(void);
bool SParkle_SetTXpower(float32_t pf);
void SParkle_SetDacType(bool DacType);
bool SParkle_GetDacType(void);

#endif /* UI_OSCILLATOR_OSC_SPARKLE_H_ */
