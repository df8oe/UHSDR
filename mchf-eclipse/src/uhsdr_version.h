/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name: uhsdr_version.h                                                      **
**  Description: this file carries version number                                  **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#define 	UHSDR_VER_MAJOR			"2"
#define 	UHSDR_VER_MINOR			"5"
#define 	UHSDR_VER_RELEASE		"43"

#define		UHSDR_BOOT_VERS			"3.4.1"


#ifdef SBLA									// Bootloader enables additional rf bands PCB
  #define	UHSDR_BOOT_VER			UHSDR_BOOT_VERS"a"
#elif SBLS									// Bootloader enables special debug capabilities
  #define	UHSDR_BOOT_VER			UHSDR_BOOT_VERS"s"
#else										// Standard Bootloader
  #define	UHSDR_BOOT_VER			UHSDR_BOOT_VERS
#endif

// trailing characters are needed for identifying version and building date+time in binary
#define	    UHSDR_VERSION           "fwv-"UHSDR_VER_MAJOR"."UHSDR_VER_MINOR"."UHSDR_VER_RELEASE
#define	    UHSDR_BUILD_DAT         "fwt-"__DATE__ " - " __TIME__
#define	    UHSDR_LICENCE           "GNU GPLv3"
#define     UHSDR_REPO              "github.com/df8oe/UHSDR"
