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
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

// ----------------------------------------------------------------------------
// re-implemented from:
// https://www.wrbishop.com/ham/arduino-iambic-keyer-and-side-tone-generator/
// by Steven T. Elliott
// ----------------------------------------------------------------------------

// Common
#include "mchf_board.h"

#include "ui_driver.h"
#include "softdds.h"
#include "cw_sm_tbl.h"
#include "codec.h"

#include "cw_gen.h"

// Transceiver state public structure
extern __IO TransceiverState 	ts;

// Public paddle state
__IO PaddleState				ps;

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_init
//* Object              : publics init
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

void cw_set_speed() {
  ps.dit_time         = 1650/ts.keyer_speed;      //100;
}
void cw_gen_init(void)
{

    cw_set_speed();

	if (ts.txrx_mode != TRX_MODE_TX  ||  ts.dmod_mode != DEMOD_CW) {
	  // do not change if currently in CW transmit
	  ps.cw_state         = CW_IDLE;
	  ps.key_timer		= 0;
	}

	switch(ts.keyer_mode)
	{
		case CW_MODE_IAM_B:
			ps.port_state = CW_IAMBIC_B;
			break;
		case CW_MODE_IAM_A:
			ps.port_state = CW_IAMBIC_A;
			break;
		default:
			break;
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_remove_click_on_rising_edge
//* Object              : remove clicks
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void cw_gen_remove_click_on_rising_edge(float *i_buffer,float *q_buffer,ulong size)
{
	ulong i,j;

	// Do not overload
	if(ps.sm_tbl_ptr > (CW_SMOOTH_TBL_SIZE - 1))
		return;

	for(i = 0,j = 0; i < size; i++)
	{
		i_buffer[i] = i_buffer[i] * sm_table[ps.sm_tbl_ptr];
		q_buffer[i] = q_buffer[i] * sm_table[ps.sm_tbl_ptr];

		j++;
		if(j == CW_SMOOTH_LEN)
		{
			j = 0;

			(ps.sm_tbl_ptr)++;
			if(ps.sm_tbl_ptr > (CW_SMOOTH_TBL_SIZE - 1))
				return;
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_remove_click_on_falling_edge
//* Object              : remove clicks
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void cw_gen_remove_click_on_falling_edge(float *i_buffer,float *q_buffer,ulong size)
{
	ulong i,j;

	// Do not overload
	if(ps.sm_tbl_ptr == 0)
		return;

	// Fix ptr, so we start from the last element
	if(ps.sm_tbl_ptr > (CW_SMOOTH_TBL_SIZE - 1))
		ps.sm_tbl_ptr = (CW_SMOOTH_TBL_SIZE - 1);

	for(i = 0,j = 0; i < size; i++)
	{
		i_buffer[i] = i_buffer[i] * sm_table[ps.sm_tbl_ptr];
		q_buffer[i] = q_buffer[i] * sm_table[ps.sm_tbl_ptr];

		j++;
		if(j == CW_SMOOTH_LEN)
		{
			j = 0;

			(ps.sm_tbl_ptr)--;
			if(ps.sm_tbl_ptr == 0)
				return;
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_check_keyer_state
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static void cw_gen_check_keyer_state(void)
{
	if(!ts.paddle_reverse)	{	// Paddles NOT reversed
		if(!GPIO_ReadInputDataBit(PADDLE_DAH_PIO,PADDLE_DAH))
			ps.port_state |= CW_DAH_L;

		if(!GPIO_ReadInputDataBit(PADDLE_DIT_PIO,PADDLE_DIT))
			ps.port_state |= CW_DIT_L;
	}
	else	{	// Paddles ARE reversed
		if(!GPIO_ReadInputDataBit(PADDLE_DAH_PIO,PADDLE_DAH))
			ps.port_state |= CW_DIT_L;

		if(!GPIO_ReadInputDataBit(PADDLE_DIT_PIO,PADDLE_DIT))
			ps.port_state |= CW_DAH_L;
	}

}

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_process
//* Object              :
//* Object              : called every 600uS from I2S IRQ
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
ulong cw_gen_process(float32_t *i_buffer,float32_t *q_buffer,ulong size)
{
	if(ts.keyer_mode == CW_MODE_STRAIGHT)
		return cw_gen_process_strk(i_buffer,q_buffer,size);
	else
		return cw_gen_process_iamb(i_buffer,q_buffer,size);
}

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_process_iamb
//* Object              : strait key implementation
//* Object              :
//* Input Parameters    : This is called via audio driver I2S IRQ!
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
ulong cw_gen_process_strk(float32_t *i_buffer,float32_t *q_buffer,ulong size)
{
  uint32_t retval;

  // Exit to RX if key_timer is zero and break_timer is zero as well
  if(ps.key_timer == 0) {
    if(ps.break_timer == 0) {
      ts.audio_unmute = 1;		// Assure that TX->RX timer gets reset at the end of an element
      RadioManagement_SwitchTXRX(TRX_MODE_RX);				// straight
    }
    if(ps.break_timer) { ps.break_timer--; }
    retval = 0;
  } else {

    softdds_runf(i_buffer,q_buffer,size/2);

    // ----------------------------------------------------------------
    // Raising slope
    //
    // Smooth start of element
    // key_timer set to 24 in Paddle DAH IRQ
    // on every audio driver sample request shape the form
    // then stop at key_timer = 12
    if(ps.key_timer > 12) {
      cw_gen_remove_click_on_rising_edge(i_buffer,q_buffer,size/2);
      if(ps.key_timer) { ps.key_timer--; }
    }

    // -----------------------------------------------------------------
    // Middle of a symbol - no shaping, just
    // pass soft DDS data (key_timer = 12)
    // ..................

    // -----------------------------------------------------------------
    // Failing edge
    //
    // Do smooth the falling edge
    // key was released, so key_timer goes from 12 to zero
    // then finally switch to RX is performed (here, but on next request)
    if(ps.key_timer < 12) {
      cw_gen_remove_click_on_falling_edge(i_buffer,q_buffer,size/2);
      if(ps.key_timer) { ps.key_timer--; }
    }

    // Key released ?, then shape falling edge, on next 12 audio sample requests
    // the audio driver
    if((GPIO_ReadInputDataBit(PADDLE_DAH_PIO,PADDLE_DAH)) && (ps.key_timer == 12)) {
      if(ps.key_timer) { ps.key_timer--; }
    }
    retval = 1;
  }
  return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_process_iamb
//* Object              : iambic keyer state machine
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
ulong cw_gen_process_iamb(float32_t *i_buffer,float32_t *q_buffer,ulong size)
{
    uint32_t retval = 0;
	switch(ps.cw_state)
	{
		case CW_IDLE:
		{
			if( (!GPIO_ReadInputDataBit(PADDLE_DAH_PIO,PADDLE_DAH)) ||
				(!GPIO_ReadInputDataBit(PADDLE_DIT_PIO,PADDLE_DIT))	||
				(ps.port_state & 3)) {

				cw_gen_check_keyer_state();
				ps.cw_state = CW_WAIT;		// Note if Dit/Dah is discriminated in this function, it breaks the Iambic-ness!
			} else {
				// Back to RX
				ts.audio_unmute = 1;		// Assure that TX->RX timer gets reset at the end of an element
				RadioManagement_SwitchTXRX(TRX_MODE_RX);				// iambic
			}
		}
		break;
		case CW_WAIT:		// This is an extra state called after detection of an element to allow the other state machines to settle.
		{					// It is NECESSARY to eliminate a "glitch" at the beginning of the first Iambic Morse DIT element in a string!
			ps.cw_state = CW_DIT_CHECK;
		}
        break;
		case CW_DIT_CHECK:
		{
			if (ps.port_state & CW_DIT_L) {
			     ps.port_state |= CW_DIT_PROC;
			     ps.key_timer   = ps.dit_time;
			     ps.cw_state    = CW_KEY_DOWN;
			} else {
			     ps.cw_state = CW_DAH_CHECK;
			}
		}
		break;
		case CW_DAH_CHECK:
		{
			if (ps.port_state & CW_DAH_L) {
			     ps.key_timer = (ps.dit_time) * 3;
			     ps.cw_state  = CW_KEY_DOWN;
			} else {
			     ps.cw_state  = CW_IDLE;
			}
		}
		break;
		case CW_KEY_DOWN:
		{
			softdds_runf(i_buffer,q_buffer,size/2);
			ps.key_timer--;

			// Smooth start of element - initial
			ps.sm_tbl_ptr = 0;
			cw_gen_remove_click_on_rising_edge(i_buffer,q_buffer,size/2);

			ps.port_state &= ~(CW_DIT_L + CW_DAH_L);
			ps.cw_state    = CW_KEY_UP;
			ts.audio_unmute = 1;		// Assure that TX->RX timer gets reset at the end of an element
			retval = 1;
		}
		break;
		case CW_KEY_UP:
		{
			if(ps.key_timer == 0) {
				ps.key_timer = ps.dit_time;
				ps.cw_state  = CW_PAUSE;
			} else {
				softdds_runf(i_buffer,q_buffer,size/2);
				ps.key_timer--;

				// Smooth start of element - continue
				if(ps.key_timer > (ps.dit_time/2)) {
					cw_gen_remove_click_on_rising_edge(i_buffer,q_buffer,size/2);
				}
				// Smooth end of element
				if(ps.key_timer < 12) {
					cw_gen_remove_click_on_falling_edge(i_buffer,q_buffer,size/2);
				}
				if(ps.port_state & CW_IAMBIC_B) {
					cw_gen_check_keyer_state();
				}
				retval = 1;
			}
		}
		break;
		case CW_PAUSE:
		{
			cw_gen_check_keyer_state();

			ps.key_timer--;
			if(ps.key_timer == 0) {
				if (ps.port_state & CW_DIT_PROC) {
					ps.port_state &= ~(CW_DIT_L + CW_DIT_PROC);
				    ps.cw_state    = CW_DAH_CHECK;
				} else {
				    ps.port_state &= ~(CW_DAH_L);
				    ps.cw_state    = CW_IDLE;
				}
			}
		}
		break;
		default:
			break;
	}
	return retval;
}

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_dah_IRQ
//* Object              : switch to tx
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void cw_gen_dah_IRQ(void)
{
    ts.ptt_req = true;
    // Just flag change - nothing to call

	if(ts.keyer_mode == CW_MODE_STRAIGHT)
	{
		// Reset publics, but only when previous is sent
		if(ps.key_timer == 0)
		{
			ps.sm_tbl_ptr  = 0;				// smooth table start
			ps.key_timer   = 24;			// smooth steps * 2
			ps.break_timer = CW_BREAK;		// break timer value
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_dit_IRQ
//* Object              : switch to tx
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void cw_gen_dit_IRQ(void)
{
	// CW mode handler - no dit interrupt in straight key mode
	if(ts.keyer_mode != CW_MODE_STRAIGHT)
	{
	    ts.ptt_req = true;
		// Just flag change - nothing to call
	}
}
