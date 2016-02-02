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

typedef enum CatInterfaceState {
	DISCONNECTED = 0,
	CONNECTED
} CatInterfaceState;

typedef enum {
	UNKNOWN = 0,
	FT817 = 1
} CatInterfaceProtocol;


// CAT driver public structure
typedef struct CatDriver
{
	uchar	enabled;
	CatInterfaceState state;
	CatInterfaceProtocol protocol;

} CatDriver;

// Exports
void cat_driver_init(void);
void cat_driver_stop(void);
void cat_driver_thread(void);

CatInterfaceState cat_driver_state();
uint8_t cat_driver_get_data(uint8_t* Buf,uint32_t Len);
uint8_t cat_driver_put_data(uint8_t* Buf,uint32_t Len);
uint8_t cat_driver_has_data();

void CatDriverFT817CheckAndExecute();

#endif
