/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name: mchf_version.h                                                      **
**  Description: this file carries version number                                  **
**  Last Modified:                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#define 	TRX4M_VER_MAJOR			"2"
#define 	TRX4M_VER_MINOR			"2"
#define 	TRX4M_VER_RELEASE		"0"

// trailing characters are needed for identifying version and building date+time in binary
#define		TRX4M_VERSION			"fwv-"TRX4M_VER_MAJOR"."TRX4M_VER_MINOR"."TRX4M_VER_RELEASE
#define		TRX4M_BUILD_DAT			"fwt-"__DATE__ " - " __TIME__
#define		TRX4M_LICENCE			"GNU GPLv3"
