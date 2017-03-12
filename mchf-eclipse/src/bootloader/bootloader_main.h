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

#include "usb_host.h"

#ifdef __cplusplus
extern "C" {
#endif


int BL_MSC_Application();
void BL_Idle_Application();
int bootloader_main();

extern ApplicationTypeDef Appli_state;

#ifdef __cplusplus
}
#endif

#endif  /* _BOOTLOADER_MAIN */



