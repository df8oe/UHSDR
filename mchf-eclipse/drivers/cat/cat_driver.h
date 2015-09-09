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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

#ifndef __CAT_DRIVER_H
#define __CAT_DRIVER_H

// CAT driver public structure
typedef struct CatDriver
{
	uchar	enabled;

} CatDriver;

// Exports
void cat_driver_init(void);
void cat_driver_stop(void);
void cat_driver_thread(void);

#endif
