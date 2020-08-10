/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _BOOTLOADER_MAIN_H
#define _BOOTLOADER_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


void Bootloader_UsbHostApplication(void);
int  Bootloader_Main(void);
void Bootloader_CheckAndGoForBootTarget(void);
void Bootloader_PrintLine(const char* txt);

enum
{
    BOOT_CLEARED = 0,
    BOOT_REBOOT = 0x55, // this command can be issue also by the firmware indicate immediate start
    BOOT_DFU = 0x99,
    BOOT_FIRMWARE = 0x66993300,
};

#ifdef __cplusplus
}
#endif

#endif  /* _BOOTLOADER_MAIN */



