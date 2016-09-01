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
#include "codec.h"

#include "cw_gen.h"

// States
#define CW_IDLE             0
#define CW_DIT_CHECK        1
#define CW_DAH_CHECK        3
#define CW_KEY_DOWN         4
#define CW_KEY_UP           5
#define CW_PAUSE            6
#define CW_WAIT             7

#define CW_DIT_L            0x01
#define CW_DAH_L            0x02
#define CW_DIT_PROC         0x04

#define CW_IAMBIC_A         0x00
#define CW_IAMBIC_B         0x10

#define CW_SMOOTH_LEN       16
//
//
typedef struct PaddleState
{
    // State machine and port states
    ulong   port_state;
    ulong   cw_state;

    // Smallest element duration
    ulong   dit_time;

    // Timers
    ulong   key_timer;
    ulong   break_timer;

    // Key clicks smoothing table current ptr
    ulong   sm_tbl_ptr;

} PaddleState;

// Public paddle state
__IO PaddleState                ps;

static ulong   cw_gen_process_strk(float32_t *i_buffer,float32_t *q_buffer,ulong size);
static ulong   cw_gen_process_iamb(float32_t *i_buffer,float32_t *q_buffer,ulong size);


#define CW_SMOOTH_TBL_SIZE  32

static const float sm_table[CW_SMOOTH_TBL_SIZE] =
{
    0,
    0.0323949034866865,
    0.0664377813382162,
    0.1021744106202792,
    0.1394369420920119,
    0.1780727855344472,
    0.2178988326848249,
    0.2587167162584878,
    0.3003585870145724,
    0.3425955596246281,
    0.3852292668039979,
    0.4280308232242313,
    0.4707866025787747,
    0.5132677195391775,
    0.5552605477988861,
    0.5965362020294499,
    0.6368657969024186,
    0.6760204470893416,
    0.7138170443274586,
    0.7500114442664225,
    0.784420538643473,
    0.8168459601739528,
    0.8471046005951019,
    0.8750286106660563,
    0.9004654001678492,
    0.9232623788815137,
    0.9432822156099794,
    0.9604180971999695,
    0.9745784695201038,
    0.9856717784390021,
    0.9936369878690776,
    0.9984283207446403
};


//*----------------------------------------------------------------------------
//* Function Name       : cw_gen_init
//* Object              : publics init
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

void cw_set_speed()
{
    ps.dit_time         = 1650/ts.keyer_speed;      //100;
}

void cw_gen_set_break_time()
{
    ps.break_timer = ts.cw_rx_delay*50;      // break timer value
}

void cw_gen_init(void)
{

    cw_set_speed();

    if (ts.txrx_mode != TRX_MODE_TX  ||  ts.dmod_mode != DEMOD_CW)
    {
        // do not change if currently in CW transmit
        ps.cw_state         = CW_IDLE;
        cw_gen_set_break_time();
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


/**
 * Is the logically DIT pressed (may reverse logic of HW contacts)
 */
static bool cw_dit_requested() {
    bool retval;
    if(ts.paddle_reverse)      // Paddles ARE reversed
    {
        retval =  mchf_ptt_dah_line_pressed();
    }
    else        // Paddles NOT reversed
    {
        retval =  mchf_dit_line_pressed();
    }
    return retval;
}

/**
 * Is the logically DAH pressed (may reverse logic of HW contacts)
 */
static bool cw_dah_requested() {
    bool retval;
    if(!ts.paddle_reverse)      // Paddles NOT reversed
    {
        retval =  mchf_ptt_dah_line_pressed();
    }
    else        // Paddles ARE reversed
    {
        retval =  mchf_dit_line_pressed();
    }
    return retval;
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
    if (cw_dah_requested()) {
            ps.port_state |= CW_DAH_L;
    }
    if (cw_dit_requested()) {
            ps.port_state |= CW_DIT_L;
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
ulong cw_gen_process(float32_t *i_buffer,float32_t *q_buffer,ulong blockSize)
{
    if(ts.keyer_mode == CW_MODE_STRAIGHT)
    {
        return cw_gen_process_strk(i_buffer,q_buffer,blockSize);
    }
    else
    {
        return cw_gen_process_iamb(i_buffer,q_buffer,blockSize);
    }
}

static ulong cw_gen_process_strk(float32_t *i_buffer,float32_t *q_buffer,ulong blockSize)
{
    uint32_t retval;

    // Exit to RX if key_timer is zero and break_timer is zero as well
    if(ps.key_timer == 0)
    {
        if(ps.break_timer == 0)
        {
            ts.audio_unmute = 1;		// Assure that TX->RX timer gets reset at the end of an element
            // RadioManagement_SwitchTxRx(TRX_MODE_RX,false);				// straight
            ts.tx_stop_req = true;
        }
        if(ps.break_timer)
        {
            ps.break_timer--;
        }
        retval = 0;
    }
    else
    {

        softdds_runf(i_buffer,q_buffer,blockSize);

        // ----------------------------------------------------------------
        // Raising slope
        //
        // Smooth start of element
        // key_timer set to 24 in Paddle DAH IRQ
        // on every audio driver sample request shape the form
        // then stop at key_timer = 12
        if(ps.key_timer > 12)
        {
            cw_gen_remove_click_on_rising_edge(i_buffer,q_buffer,blockSize);
            if(ps.key_timer)
            {
                ps.key_timer--;
            }
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
        if(ps.key_timer < 12)
        {
            cw_gen_remove_click_on_falling_edge(i_buffer,q_buffer,blockSize);
            if(ps.key_timer)
            {
                ps.key_timer--;
            }
        }

        // Key released ?, then shape falling edge, on next 12 audio sample requests
        // the audio driver
        if(mchf_ptt_dah_line_pressed() == false && (ps.key_timer == 12))
        {
            if(ps.key_timer)
            {
                ps.key_timer--;
            }
        }
        retval = 1;
    }
    return retval;
}

static ulong cw_gen_process_iamb(float32_t *i_buffer,float32_t *q_buffer,ulong blockSize)
{
    uint32_t retval = 0;
    switch(ps.cw_state)
    {
    case CW_IDLE:
    {
        // at least one paddle is still or has been recently pressed
        if( mchf_dit_line_pressed() || mchf_ptt_dah_line_pressed()	||
            (ps.port_state & (CW_DAH_L|CW_DIT_L)))
        {
            cw_gen_check_keyer_state();
            ps.cw_state = CW_WAIT;		// Note if Dit/Dah is discriminated in this function, it breaks the Iambic-ness!
        }
        else
        {
            if(ps.break_timer == 0)
             {
                 ts.audio_unmute = 1;        // Assure that TX->RX timer gets reset at the end of an element
                 // RadioManagement_SwitchTxRx(TRX_MODE_RX,false);               // straight
                 ts.tx_stop_req = true;
             }
             if(ps.break_timer)
             {
                 ps.break_timer--;
             }
             retval = 0;
        }
    }
    break;
    case CW_WAIT:		// This is an extra state called after detection of an element to allow the other state machines to settle.
    {
        // It is NECESSARY to eliminate a "glitch" at the beginning of the first Iambic Morse DIT element in a string!
        ps.cw_state = CW_DIT_CHECK;
    }
    break;
    case CW_DIT_CHECK:
    {
        if (ps.port_state & CW_DIT_L)
        {
            ps.port_state |= CW_DIT_PROC;
            ps.key_timer   = ps.dit_time;
            ps.cw_state    = CW_KEY_DOWN;
        }
        else
        {
            ps.cw_state = CW_DAH_CHECK;
        }
    }
    break;
    case CW_DAH_CHECK:
    {
        if (ps.port_state & CW_DAH_L)
        {
            ps.key_timer = (ps.dit_time) * 3;
            ps.cw_state  = CW_KEY_DOWN;
        }
        else
        {
            ps.cw_state  = CW_IDLE;
            cw_gen_set_break_time();
        }
    }
    break;
    case CW_KEY_DOWN:
    {
        softdds_runf(i_buffer,q_buffer,blockSize);
        ps.key_timer--;

        // Smooth start of element - initial
        ps.sm_tbl_ptr = 0;
        cw_gen_remove_click_on_rising_edge(i_buffer,q_buffer,blockSize);

        ps.port_state &= ~(CW_DIT_L + CW_DAH_L);
        ps.cw_state    = CW_KEY_UP;
        ts.audio_unmute = 1;		// Assure that TX->RX timer gets reset at the end of an element
        retval = 1;
    }
    break;
    case CW_KEY_UP:
    {
        if(ps.key_timer == 0)
        {
            ps.key_timer = ps.dit_time;
            ps.cw_state  = CW_PAUSE;
        }
        else
        {
            softdds_runf(i_buffer,q_buffer,blockSize);
            ps.key_timer--;

            // Smooth start of element - continue
            if(ps.key_timer > (ps.dit_time/2))
            {
                cw_gen_remove_click_on_rising_edge(i_buffer,q_buffer,blockSize);
            }
            // Smooth end of element
            if(ps.key_timer < 12)
            {
                cw_gen_remove_click_on_falling_edge(i_buffer,q_buffer,blockSize);
            }
            if(ts.keyer_mode == CW_MODE_IAM_B)
            {
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
        if(ps.key_timer == 0)
        {
            if (ps.port_state & CW_DIT_PROC)
            {
                ps.port_state &= ~(CW_DIT_L + CW_DIT_PROC);
                ps.cw_state    = CW_DAH_CHECK;
            }
            else
            {
                ps.port_state &= ~(CW_DAH_L);
                ps.cw_state    = CW_IDLE;
                cw_gen_set_break_time();
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
            cw_gen_set_break_time();
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


