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

// Common
#include "mchf_board.h"

#include <stdio.h>

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "cat_driver.h"
#include "ui_driver.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END ;

extern USB_OTG_CORE_HANDLE           USB_OTG_dev;
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);

extern __IO CatDriver kd;

//void OTG_FS_WKUP_IRQHandler(void)
//{
//if(USB_OTG_dev.cfg.low_power)
//{
//	*(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
//	SystemInit();
//	USB_OTG_UngateClock(&USB_OTG_dev);
//}
//EXTI_ClearITPendingBit(EXTI_Line18);
//}


void cat_driver_init(void)
{
	// Start driver
	USBD_Init(	&USB_OTG_dev,
			USB_OTG_FS_CORE_ID,
			&USR_desc,
			&USBD_CDC_cb,
			&USR_cb);

	printf("cat driver started\n\r");
}

void cat_driver_stop(void)
{
	// Stop driver
	USBD_DeInit(&USB_OTG_dev);

	printf("cat driver stopped\n\r");
}

void cat_driver_thread(void)
{
	if(!kd.enabled)
		return;
}

#define CAT_BUFFER_SIZE 256
__IO uint8_t cat_buffer[CAT_BUFFER_SIZE];
__IO int32_t cat_head = 0;
__IO int32_t cat_tail = 0;

CatInterfaceState cat_driver_state() { return USBD_User_GetStatus(); }

int cat_buffer_remove(uint8_t* c_ptr) {
	if (cat_head != cat_tail) {
		int c = cat_buffer[cat_tail];
		cat_tail = (cat_tail + 1) % CAT_BUFFER_SIZE;
		*c_ptr = (uint8_t)c;
		return 1;
	} else {
		return 0;
	}
}

int cat_buffer_add(uint8_t c) {
	int32_t next_head = (cat_head + 1) % CAT_BUFFER_SIZE;
	if (next_head != cat_tail) {
		/* there is room */
		cat_buffer[cat_head] = c;
		cat_head = next_head;
		return 1;
	} else {
		/* no room left in the buffer */
		return 0;
	}
}


uint8_t cat_driver_get_data(uint8_t* Buf,uint32_t Len) {
	uint8_t res = 0;
	if (cat_driver_has_data() >= Len) {
		int i;
		for  (i = 0; i < Len; i++) {
			cat_buffer_remove(&Buf[i]);
		}
		res = 1;
	}
	return res;
}
uint8_t cat_driver_put_data(uint8_t* Buf,uint32_t Len) {
	uint8_t res = 0;
	if (USBD_User_GetStatus()) {
		VCP_DataTx(Buf,Len);
		res = 1;
	}
	return res;
}

uint8_t cat_driver_has_data() {
	int32_t len = cat_head - cat_tail;
	return len < 0?len+CAT_BUFFER_SIZE:len;
}

// #define DEBUG_FT817

struct FT817 {
	uint8_t req[5];
#ifdef DEBUG_FT817
#define FT817_MAX_CMD 100
	uint8_t reqs[FT817_MAX_CMD*5];
	uint32_t cmd_cntr;
#endif
};

#include "ui_rotary.h"
#include "mchf_board.h"

extern __IO DialFrequency 				df;
extern __IO TransceiverState ts;
// FT817 Emulation
#if 0
// list of commands supported by hamlib
static const yaesu_cmd_set_t ncmd[] = {
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* lock on */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x80 } }, /* lock off */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } }, /* ptt on */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x88 } }, /* ptt off */
		{ 0, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* set freq */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main LSB */
		{ 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main USB */
		{ 1, { 0x02, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main CW */
		{ 1, { 0x03, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main CWR */
		{ 1, { 0x04, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main AM */
		{ 1, { 0x08, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main FM */
		{ 1, { 0x88, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main FM-N */
		{ 1, { 0x0a, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main DIG */
		{ 1, { 0x0c, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main PKT */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x05 } }, /* clar on */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x85 } }, /* clar off */
		{ 0, { 0x00, 0x00, 0x00, 0x00, 0xf5 } }, /* set clar freq */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x81 } }, /* toggle vfo a/b */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x02 } }, /* split on */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x82 } }, /* split off */
		{ 1, { 0x09, 0x00, 0x00, 0x00, 0x09 } }, /* set RPT shift MINUS */
		{ 1, { 0x49, 0x00, 0x00, 0x00, 0x09 } }, /* set RPT shift PLUS */
		{ 1, { 0x89, 0x00, 0x00, 0x00, 0x09 } }, /* set RPT shift SIMPLEX */
		{ 0, { 0x00, 0x00, 0x00, 0x00, 0xf9 } }, /* set RPT offset freq */
		{ 1, { 0x0a, 0x00, 0x00, 0x00, 0x0a } }, /* set DCS on */
		{ 1, { 0x2a, 0x00, 0x00, 0x00, 0x0a } }, /* set CTCSS on */
		{ 1, { 0x4a, 0x00, 0x00, 0x00, 0x0a } }, /* set CTCSS encoder on */
		{ 1, { 0x8a, 0x00, 0x00, 0x00, 0x0a } }, /* set CTCSS/DCS off */
		{ 0, { 0x00, 0x00, 0x00, 0x00, 0x0b } }, /* set CTCSS tone */
		{ 0, { 0x00, 0x00, 0x00, 0x00, 0x0c } }, /* set DCS code */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0xe7 } }, /* get RX status  */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0xf7 } }, /* get TX status  */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* pwr wakeup sequence */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* pwr on */
		{ 1, { 0x00, 0x00, 0x00, 0x00, 0x8f } }, /* pwr off */
		{ 0, { 0x00, 0x00, 0x00, 0x00, 0xbb } }, /* eeprom read */
}
#endif


struct FT817 ft817;

void CatDriverFT817CheckAndExecute() {
	uint8_t bc = 0;
	uint8_t resp[32];
	while (cat_driver_get_data(ft817.req,5))
	{
#ifdef DEBUG_FT817
		int debug_idx;
		for (debug_idx = 0; debug_idx < 5 && ft817.cmd_cntr < FT817_MAX_CMD; debug_idx++ ) {
			ft817.reqs[ft817.cmd_cntr*5+debug_idx] = ft817.req[debug_idx];
		}
		ft817.cmd_cntr++;
#endif

		switch(ft817.req[4]) {
		case 1: /* SET FREQ */
		{
			ulong f = 0;
			int fidx;
			for (fidx = 0; fidx < 4; fidx++) {
				f *= 100;
				f +=  (ft817.req[fidx] >> 4) * 10 + (ft817.req[fidx] & 0x0f);
			}
			f *= 40;
			df.tune_new = f;
			UiDriverUpdateFrequency(true,0);
			resp[0] = 0;
			bc = 1;
		}
		break;

		case 3: /* READ FREQ */
		{
			ulong f = (df.tune_new + 20)/ 40 ;
			ulong fbcd = 0;
			int fidx;
			for (fidx = 0; fidx < 8; fidx++)
			{
				fbcd >>= 4;
				fbcd |= (f % 10) << 28;
				f = f / 10;
			}

			resp[0] = (uint8_t)(fbcd >> 24);
			resp[1] = (uint8_t)(fbcd >> 16);
			resp[2] = (uint8_t)(fbcd >> 8);
			resp[3] = (uint8_t)fbcd;
		}
		switch(ts.dmod_mode) {
		case DEMOD_LSB: resp[4] = 0; break;
		case DEMOD_USB: resp[4] = 1; break;
		case DEMOD_CW: 	resp[4] = 2; break;
		case DEMOD_AM:  resp[4] = 4; break;
		case DEMOD_FM:  resp[4] = 8; break;
		default: resp[4] = 1;
		}
		bc = 5;
		break;
		case 7: /* set mode */
		{
			uint32_t new_mode = ts.dmod_mode;
			switch (ft817.req[0]) {
			case 0: // LSB
				new_mode = DEMOD_LSB;
				break;
			case 1: // USB
				new_mode = DEMOD_USB;
				break;
			case 2: // CW
			case 3: // CW-R
				new_mode = DEMOD_CW;
				break;
			case 4: // AM
				new_mode = DEMOD_AM;
				break;
			case 8: // FM
			case 0x88: // FM-N
				new_mode = DEMOD_FM;
				break;
			case 0x0a: // DIG - SSB, side band controlled by some menu configuration in ft817, we use USB here
				new_mode = DEMOD_USB;
			case 0x0c: // PKT - FM, 9k6
				new_mode = DEMOD_FM;
				break;
			}
			if  (new_mode != ts.dmod_mode) {
				UiDriverSetDemodMode(new_mode);
			}
		}
		break;
		case 8: /* PTT ON */
			if(!ts.tx_disable)    {
				ts.ptt_req = 1;
				kd.enabled = 1;
			}
			resp[0] = 0; /* 0xF0 if PTT was already on */
			bc = 1;
			break;
		case 15:
			resp[0] = 0;
			bc = 1;
			break;
		case 129: // SWITCH VFO
			resp[0] = 0;
			bc = 1;
			break;
		case 136: /* 0x88 PTT OFF */
			resp[0] = 0; /* 0xF0 if PTT was already off */
			ts.ptt_req = 0;
			kd.enabled = 0;
			bc = 1;
			break;
		case 167: /* A7 */
			resp[0]=0xA7;
			resp[1]=0x02; resp[2]=0x00;
			resp[3]=0x04; resp[4]=0x67;
			resp[5]=0xD8; resp[6]=0xBF;
			resp[7]=0xD8; resp[8]=0xBF;
			bc = 9;
			break;
		case 187: /* BB */
			resp[0]=0x00;
			resp[1]=0x00;
			resp[2]=0x00;
			resp[3]=0x00;
			bc = 4;
			break;
		case 188: /* BC Write EEPROM */
			resp[0] = 0;
			bc = 1;
		case 189: /* BD Read TX Status */
			resp[0] = 0;
			bc = 1;
		case 231: /* E7 */
			resp[0] = 0x09; // S9 signal;
			bc = 1;
			break;
		case 247: /* F7 */
			resp[0]=0x00;
			bc = 1;
			break;
		case 255: /* FF sent out by HRD */
			break;
		// default:
			// while (1);
		}
	}
	cat_driver_put_data(resp,bc);
	/* Return data back */
}

