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
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#ifndef __DDS_TABLE_H
#define __DDS_TABLE_H

// 256 points
//#define DDS_ACC_SHIFT     8

// 512 points
//#define DDS_ACC_SHIFT     7

// 1024 points
//#define DDS_ACC_SHIFT       6
#define DDS_ACC_SHIFT       22

#define DDS_TBL_SIZE		1024

extern const short DDS_TABLE[DDS_TBL_SIZE];

#endif
