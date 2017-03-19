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
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BOOTLOADER_MAIN_H
#define _BOOTLOADER_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


void BL_Application();
int bootloader_main();
void bootTypeSelector();
void mchfBl_CheckAndGoForDfuBoot();
void BL_InfoScreen();
void BL_PrintLine(const char* txt);

#ifdef __cplusplus
}
#endif

#endif  /* _BOOTLOADER_MAIN */



