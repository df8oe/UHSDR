/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                               UHSDR FIRMWARE                                    **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **  Licence:        GNU GPLv3, see LICENSE.md                                                      **
 ************************************************************************************/

// Common
#include "psk.h"
#include "ui_driver.h"

const psk_speed_item_t psk_speeds[PSK_SPEED_NUM] =
{
		{ .id =PSK_SPEED_31, .value = 31.25, .label = " 31" },
		{ .id =PSK_SPEED_63, .value = 62.5, .label = " 63"  },
		{ .id =PSK_SPEED_125, .value = 125, .label = "125" }
};

psk_ctrl_t psk_ctrl_config =
{
		.speed_idx = PSK_SPEED_31
};

void PskDecoder_Init(void)
{
	//FIXME To be implemented;
	char s[] = "- PSK mode not implemented yet";
	char *p;

	p = s;
	while (*(p++) != '\0')
	{
		UiDriver_TextMsgPutChar(*p);
	}
}

