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
**  Licence:		GNU GPLv3                                                          **
************************************************************************************/

// ----------------------------------------------------------------------------
// re-implemented from:
// https://www.wrbishop.com/ham/arduino-iambic-keyer-and-side-tone-generator/
// by Steven T. Elliott
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// Major changes to improve CW edge smoothing
//
// 28.01.2017	hb9ewy
// - Blackman-Harris smoothing for minimal sidelobes
//   See: https://en.wikipedia.org/wiki/Window_function
// - CW_SMOOTH_TBL_SIZE=128 and cw_smooth_len=2 to remove spurious at +/- 3 kHz
// - Avoid clipping of smoothed edges and cw spike after signal
//   by improved timing
//
// Open issues
// - in internal keyer pause between dits and dots is too long
//   Not changes as issue had not been reported yet
// - Try interpolation with smoothing table to use less RAM
// ----------------------------------------------------------------------------

// Common
#include "mchf_board.h"
#include "softdds.h"
#include "cw_gen.h"
#include "cat_driver.h"


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

#define CW_SMOOTH_LEN       2	// with sm_table size of 128 ~5.3ms for edges, ~ 9 steps of 0.6 ms
#define CW_SMOOTH_STEPS		  9	// 1 step = 0.6ms; 13 for 8ms, 9 for 5.4 ms, for internal keyer
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

    ulong	ultim;

} PaddleState;

// Public paddle state
__IO PaddleState                ps;

static bool   CwGen_ProcessStraightKey(float32_t *i_buffer,float32_t *q_buffer,ulong size);
static bool   CwGen_ProcessIambic(float32_t *i_buffer,float32_t *q_buffer,ulong size);
static void    CwGen_TestFirstPaddle();

// Blackman-Harris function to keep CW signal bandwidth narrow
#define CW_SMOOTH_TBL_SIZE  128

static const float sm_table[CW_SMOOTH_TBL_SIZE] =
{
    0.0,
		0.0000686004957883,
		0.0000945542356652,
		0.0001383179721175,
		0.0002006529396865,
		0.0002826248158738,
		0.0003856036471126,
		0.0005112637206964,
		0.0006615833583608,
		0.0008388446022225,
		0.0010456327590236,
		0.0012848357641460,
		0.0015596433227128,
		0.0018735457812732,
		0.0022303326801620,
		0.0026340909336119,
		0.0030892025821568,
		0.0036003420597745,
		0.0041724729166402,
		0.0048108439372987,
		0.0055209845935232,
		0.0063086997711494,
		0.0071800637107376,
		0.0081414131030473,
		0.0091993392820062,
		0.0103606794601087,
		0.0116325069539978,
		0.0130221203513468,
		0.0145370315740592,
		0.0161849527972270,
		0.0179737821882093,
		0.0199115884355984,
		0.0220065940436903,
		0.0242671573743564,
		0.0267017534248783,
		0.0293189533373318,
		0.0321274026424438,
    0.0351357982484650,
		0.0383528641934459,
		0.0417873261873527,
		0.0454478849786327,
		0.0493431885881256,
		0.0534818034615354,
		0.0578721846000008,
		0.0625226447365667,
		0.0674413226345205,
		0.0726361505915551,
		0.0781148212415210,
		0.0838847537530600,
		0.0899530595316440,
		0.0963265075384118,
		0.1030114893456630,
		0.1100139840548870,
		0.1173395232087210,
		0.1249931558332360,
		0.1329794137513460,
		0.1413022773119520,
		0.1499651416825930,
		0.1589707838558700,
		0.1683213305216760,
		0.1780182269583640,
		0.1880622070962730,
		0.1984532649066170,
		0.2091906272675180,
		0.2202727284569900,
		0.2316971864198910,
		0.2434607809523230,
		0.2555594339426260,
		0.2679881918029900,
		0.2807412102198870,
		0.2938117413448940,
		0.3071921235401840,
		0.3208737737849510,
		0.3348471828403530,
		0.3491019132612700,
		0.3636266003332790,
		0.3784089560027740,
		0.3934357758572420,
		0.4086929492012480,
		0.4241654722618500,
		0.4398374645449960,
		0.4556921883519000,
		0.4717120714516800,
		0.4878787328935640,
		0.5041730119288920,
		0.5205749999999980,
		0.5370640757398780,
		0.5536189429134430,
		0.5702176712181650,
		0.5868377398491150,
		0.6034560837207990,
		0.6200491422259460,
		0.6365929103995070,
		0.6530629923446170,
		0.6694346567663080,
		0.6856828944482830,
		0.7017824774982180,
		0.7177080201778180,
		0.7334340411253520,
		0.7489350267705930,
		0.7641854957350820,
		0.7791600640044710,
		0.7938335106543350,
		0.8081808439064380,
		0.8221773672888840,
    0.8357987456709900,
		0.8490210709420900,
		0.8618209271027980,
		0.8741754545375520,
		0.8860624132385800,
		0.8974602447536510,
		0.9083481326332570,
		0.9187060611570670,
		0.9285148721246550,
		0.9377563195016140,
		0.9464131217191720,
		0.9544690114333140,
		0.9619087825581750,
		0.9687183343980040,
		0.9748847127123550,
		0.9803961475602210,
		0.9852420877805500,
		0.9894132319790110,
		0.9929015559037870,
		0.9957003361066840,
		0.9978041697997700,
		0.9992089908321180,
		0.9999120817258750
};

void CwGen_SetSpeed()
{
    ps.dit_time         = 1650/ts.keyer_speed;      //100;
}

static void CwGen_SetBreakTime()
{
    ps.break_timer = ts.cw_rx_delay*50;      // break timer value
}

/**
 * @brief Initializes CW generator, called once during init phase
 */
void CwGen_Init(void)
{

    CwGen_SetSpeed();

    if (ts.txrx_mode != TRX_MODE_TX  ||  ts.dmod_mode != DEMOD_CW)
    {
        // do not change if currently in CW transmit
        ps.cw_state         = CW_IDLE;
        CwGen_SetBreakTime();
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

/**
 * @brief remove clicks at start of tone
 */
static void CwGen_RemoveClickOnRisingEdge(float *i_buffer,float *q_buffer,ulong size)
{
    // Do not overload
    if(ps.sm_tbl_ptr < (CW_SMOOTH_TBL_SIZE ))
    {
        for(int i = 0, j = 0; i < size; i++)
        {
            i_buffer[i] = i_buffer[i] * sm_table[ps.sm_tbl_ptr];
            q_buffer[i] = q_buffer[i] * sm_table[ps.sm_tbl_ptr];

            j++;
            if(j == CW_SMOOTH_LEN)
            {
                j = 0;

                ps.sm_tbl_ptr++;
                if(ps.sm_tbl_ptr > (CW_SMOOTH_TBL_SIZE - 1))
                {
                    break;
                    // leave loop and return
                }
            }
        }
    }
}

/**
 * @brief remove clicks at end of tone
 */
static void CwGen_RemoveClickOnFallingEdge(float *i_buffer,float *q_buffer,ulong size)
{
  // Do not overload
	if(ps.sm_tbl_ptr >= 0)
    {
        // Fix ptr, so we start from the last element
        if(ps.sm_tbl_ptr > (CW_SMOOTH_TBL_SIZE - 1))
        {
            ps.sm_tbl_ptr = (CW_SMOOTH_TBL_SIZE - 1);
        }

        for(int i = 0,j = 0; i < size; i++) // iterate over the I&Q audio buffer elements
        {
            i_buffer[i] = i_buffer[i] * sm_table[ps.sm_tbl_ptr];
            q_buffer[i] = q_buffer[i] * sm_table[ps.sm_tbl_ptr];

            j++;
            if(j == CW_SMOOTH_LEN)
            {
                j = 0;

                if(ps.sm_tbl_ptr > 0) // keep value @ ptr=0 to prevent trailing CW spike
                {
                    ps.sm_tbl_ptr--;
                }
            }
        }
    }
}


/**
 * @brief Is the logically DIT pressed (may reverse logic of HW contacts)
 */
static bool CwGen_DitRequested() {
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
 * @brief Is the logically DAH pressed (may reverse logic of HW contacts)
 */
static bool CwGen_DahRequested() {
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

static void CwGen_CheckKeyerState(void)
{
    if (CwGen_DahRequested()) {
            ps.port_state |= CW_DAH_L;
    }
    if (CwGen_DitRequested()) {
            ps.port_state |= CW_DIT_L;
    }
}

/**
 * @brief called every 600uS from I2S IRQ, does cw tone generation
 * @returns true if a tone is currently being active, false if silence/no tone is requested
 */
bool CwGen_Process(float32_t *i_buffer,float32_t *q_buffer,ulong blockSize)
{
    bool retval;


    if(ts.keyer_mode == CW_MODE_STRAIGHT || CatDriver_CatPttActive())
    {
        // we make sure the remaining code will see the "right" keyer mode
        // since we are running in an interrupt, none will change that outside
        // and we can safely restore after we're done
        uint8_t keyer_mode = ts.keyer_mode;
        ts.keyer_mode = CW_MODE_STRAIGHT;
        retval = CwGen_ProcessStraightKey(i_buffer,q_buffer,blockSize);
        ts.keyer_mode = keyer_mode;
    }
    else
    {
        retval = CwGen_ProcessIambic(i_buffer,q_buffer,blockSize);
    }
    return retval;
}


static bool CwGen_ProcessStraightKey(float32_t *i_buffer,float32_t *q_buffer,ulong blockSize)
{
    uint32_t retval;

    bool cat_ptt_active = CatDriver_CatPttActive();
    bool cat_cw_key_pressed = CatDriver_CWKeyPressed();

    // simulate paddle interrupt if we do virtual keying via CAT / RS232
    if (cat_ptt_active && cat_cw_key_pressed)
    {
        CwGen_DahIRQ();
    }

    // Exit to RX if key_timer is zero and break_timer is zero as well
    if(ps.key_timer == 0)
    {
        if(ps.break_timer == 0)
        {
            ts.tx_stop_req = true;
        }
        else
        {
            ps.break_timer--;
        }
        retval = false;
    }
    else
    {

        softdds_runf(i_buffer,q_buffer,blockSize);

        // ----------------------------------------------------------------
        // Raising slope
        //
        // Smooth start of element
        // key_timer is used to track signal phases for Straight Key mode
        // 	3: rising edge, 2: constant signal, 1: falling edge, 0: signal off
        // key_timer set to 3 (rising edge) in Paddle DAH IRQ
        // on every audio driver sample request shape the form
        // then stop at key_timer = 2 (constant signal)
        if(ps.key_timer > 2)
        {
            CwGen_RemoveClickOnRisingEdge(i_buffer,q_buffer,blockSize);
            if(ps.sm_tbl_ptr >= CW_SMOOTH_TBL_SIZE) // end of rising edge when pointer at end of table
            {
                ps.key_timer = 2;	// at end of rising edge change to constant signal phase
            }
        }

        // -----------------------------------------------------------------
        // Middle of a symbol - no shaping, just
        // pass soft DDS data (key_timer = 2)
        // ..................

        // -----------------------------------------------------------------
        // Failing edge
        //
        // Do smooth the falling edge
        // key was released, so key_timer goes from 2 to zero
        // then finally switch to RX is performed (here, but on next request)
        if(ps.key_timer < 2)
        {
            CwGen_RemoveClickOnFallingEdge(i_buffer,q_buffer,blockSize);
            if(ps.sm_tbl_ptr <= 0) // end of falling edge when pointer at end of table
            {
                ps.key_timer = 0;
            }
        }

        // Key released ?, then shape falling edge until the end of the smooth table is reached
        //

        if
        (
                (ps.key_timer == 2)
                &&
                (
                        (mchf_ptt_dah_line_pressed() == false)
                        &&
                        (cat_ptt_active == false || CatDriver_CWKeyPressed() == false)
                )
        )
        {
            ps.key_timer = 1;	// begin of falling edge
        }
        retval = true;
    }
    return retval;
}

static bool CwGen_ProcessIambic(float32_t *i_buffer,float32_t *q_buffer,ulong blockSize)
{
    uint32_t retval = false;
    switch(ps.cw_state)
    {
    case CW_IDLE:
    {
        // at least one paddle is still or has been recently pressed
        if( mchf_dit_line_pressed() || mchf_ptt_dah_line_pressed()	||
            (ps.port_state & (CW_DAH_L|CW_DIT_L)))
        {
            CwGen_CheckKeyerState();
            ps.cw_state = CW_WAIT;		// Note if Dit/Dah is discriminated in this function, it breaks the Iambic-ness!
        }
        else
        {
            if(ps.break_timer == 0)
            {
                ts.tx_stop_req = true;
            }
            else
            {
                ps.break_timer--;
            }
            retval = false;
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
            CwGen_SetBreakTime();
        }
    }
    break;
    case CW_KEY_DOWN:
    {
        softdds_runf(i_buffer,q_buffer,blockSize);
        ps.key_timer--;

        // Smooth start of element - initial
        ps.sm_tbl_ptr = 0;
        CwGen_RemoveClickOnRisingEdge(i_buffer,q_buffer,blockSize);

        ps.port_state &= ~(CW_DIT_L + CW_DAH_L);
        ps.cw_state    = CW_KEY_UP;
        retval = true;
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
                CwGen_RemoveClickOnRisingEdge(i_buffer,q_buffer,blockSize);
            }
            // Smooth end of element
            if(ps.key_timer < CW_SMOOTH_STEPS)
            {
            	CwGen_RemoveClickOnFallingEdge(i_buffer,q_buffer,blockSize);
            }

            if(ts.keyer_mode == CW_MODE_IAM_B)
            {
                CwGen_CheckKeyerState();
            }
            retval = true;
        }
    }
    break;
    case CW_PAUSE:
    {
        CwGen_CheckKeyerState();

        ps.key_timer--;
        if(ps.key_timer == 0)
        {
		  if (ts.keyer_mode == CW_MODE_IAM_A || ts.keyer_mode == CW_MODE_IAM_B)
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
                CwGen_SetBreakTime();
            }
          }
          else
          {
        	CwGen_TestFirstPaddle();
//			if(cw_dah_requested() && ps.ultim == 0)
			if((ps.port_state & CW_DAH_L) && ps.ultim == 0)
			{
          	  ps.port_state &= ~(CW_DIT_L + CW_DIT_PROC);
              ps.cw_state    = CW_DAH_CHECK;
			}
			else
			{
          	  ps.port_state &= ~(CW_DAH_L);
          	  ps.cw_state    = CW_IDLE;
          	  CwGen_SetBreakTime();
            }
          }
        }
    }
    break;
    default:
        break;
    }
    return retval;
}


static void CwGen_TestFirstPaddle()
{
  if(ts.keyer_mode == CW_MODE_ULTIMATE)
  {
    if(!CwGen_DitRequested() && CwGen_DahRequested())
    {
      ps.ultim = 1;
    }
    if(CwGen_DitRequested() && !CwGen_DahRequested())
    {
      ps.ultim = 0;
    }
  }
}


/**
 * @brief request switch to TX and sets timers
 */
void CwGen_DahIRQ(void)
{
    ts.ptt_req = true;

    if(ts.keyer_mode == CW_MODE_STRAIGHT)
    {
        // Reset publics, but only when previous is sent
        if(ps.key_timer == 0)
        {
            ps.sm_tbl_ptr  = 0;			// smooth table start
            ps.key_timer   = 3;			// rising edge
            CwGen_SetBreakTime();
        }
    }
    else
    {
  	  CwGen_TestFirstPaddle();
    }
}

/**
 * @brief request switch to TX and sets timers, only does something if not in straight key mode
 */
void CwGen_DitIRQ(void)
{
    // CW mode handler - no dit interrupt in straight key mode
    if(ts.keyer_mode != CW_MODE_STRAIGHT)
    {
        ts.ptt_req = true;
  		CwGen_TestFirstPaddle();
    }
}

void CwGen_PrepareTx()
{
    CwGen_SetSpeed();
    ps.key_timer        = 0;
}
