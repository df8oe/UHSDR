/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               UHSDR FIRMWARE                                    **
**                                                                                 **
**---------------------------------------------------------------------------------**
**  Licence:		GNU GPLv3, see LICENSE.md                                                      **
************************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PSK_H
#define __PSK_H

#include "uhsdr_types.h"


typedef enum {
    PSK_SPEED_31,
    PSK_SPEED_63,
	PSK_SPEED_125,
    PSK_SPEED_NUM
} psk_speed_t;

typedef struct
{
    psk_speed_t id;
    float32_t value;
    char* label;
} psk_speed_item_t;

extern const psk_speed_item_t psk_speeds[PSK_SPEED_NUM];

typedef struct
{
    psk_speed_t speed_idx;
}  psk_ctrl_t;

extern psk_ctrl_t psk_ctrl_config;

void PskDecoder_Init(void);

#endif
