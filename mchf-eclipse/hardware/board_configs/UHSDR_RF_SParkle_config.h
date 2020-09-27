/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                **
 **                                        UHSDR                                   **
 **               a powerful firmware for STM32 based SDR transceivers             **
 **                                                                                **
 **--------------------------------------------------------------------------------**
 **                                                                                **
 **  File name:     osc_FPGA_DDC.h                                                 **
 **  Description:   SParkle RF board, additional configuration                     **
 **  Licence:       GNU GPLv3                                                      **
 **  Author:        Slawomir Balon/SP9BSL                                          **
 ************************************************************************************/
#ifndef BOARD_CONFIGS_UHSDR_RF_SPARKLE_CONFIG_H_
#define BOARD_CONFIGS_UHSDR_RF_SPARKLE_CONFIG_H_

#if defined(STM32F7) //temporary until the port for h7 will be ready...
#define USE_OSC_SParkle
#endif

#if defined(STM32F7)
#define TRX_NAME_SParkle "SParkle F7"
#endif
#if defined(STM32H7)
#define TRX_NAME_SParkle "SParkle H7"
#endif

#define TRX_ID_SParkle "SPsdr"

#define SParkle_DEVICE_STRING TRX_NAME_SParkle " Transceiver"
#define SParkleTRX_HW_LIC "CERN-OHL-S v2"
#define SParkleTRX_HW_CREATOR "Slawek SP9BSL, sp9bsl.pl"
#endif /* BOARD_CONFIGS_UHSDR_RF_SPARKLE_CONFIG_H_ */
