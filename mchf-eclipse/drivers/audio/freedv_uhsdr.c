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
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/
#include "freedv_uhsdr.h"




#include "profiling.h"
#include "ui_lcd_hy28.h"

#ifdef debug_alternate_NR

const float32_t NR_test_samp[128] = { 853.472351,629.066223,864.270813,1012.3078,738.378113,
        446.268219,635.763123,1062.71118,955.245667,22.6679211,-1130.45386,-1737.12817,
        -1728.79114,-1594.82227,-1545.75671,-1208.91003,-252.898315,993.880493,1820.26538,
        1915.65186,1597.90259,1248.58838,809.456909,28.6509247,-961.62677,-1604.66443,-1499.18225,
        -824.882935,-85.1342163,432.899261,782.52063,1029.38452,1040.57166,692.128662,138.820541,
        -286.785767,-420.356415,-384.165161,-348.958527,-308.304718,-171.111633,4.52698851,  //last value:4.52698851,
        -5.53196001,-368.999939,-1031.19165,-1766.01074,-2290.01587,-2293.98853,-1514.0238,
        23.0157223,1797.16394,3018.3894,3231.77148,2702.38745,2085.92676,1685.99255,1145.43176,
        -31.9259377,-1722.42847,-3112.2937,-3453.61426,-2790.31763,-1812.12769,-1028.70874,
        -1812 ,897.985779,2375.50903,3409.33472,3332.44238,2293.16602,1067.26196,183.806381,
        -548.479553,-1549.47034,-2692.18213,-3288.44702,-2873.70239,-1761.34033,-636.71936,
        250.664383,1198.7804,2336.43726,3121.80615,2848.64355,1556.67969,110.084801,-724.328186,
        -1013.82141,-1265.38879,-1506.06091,-1177.04529,-35.6577721,1209.823,1520.28088,679.406555,
        -514.541626,-1245.55945,-1508.29407,-1707.93408,-1736.12427,-965.137085,752.618347,2518.7168,
        3185.57031,2563.83838,1472.3927,613.243835,-172.269989,-1311.97058,-2534.06421,-2982.73169,
        -2282.05859,-1025.64673,12.714426,809.696228,1828.12854,2977.01709,3388.77612,2460.82178,
        751.800781,-567.183105,-1026.46143,-1190.80762,-1635.05701,-2060.84619,-1785.74683,-841.740173,
        -62.468441

};



const float32_t NR_test_sinus_samp[128] = {
        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302,

        0, 765.3668647302, 1414.2135623731, 1847.7590650226, 2000, 1847.7590650226, 1414.2135623731, 765.3668647302,
        0, -765.3668647302, -1414.2135623731, -1847.7590650226, -2000, -1847.7590650226, -1414.2135623731, -765.3668647302
};


#endif






void fdv_clear_display()
{ char clr_string[freedv_rx_buffer_max];

  snprintf(clr_string,freedv_rx_buffer_max,"                                                                           ");
  UiLcdHy28_PrintText(5,116,"            ",Yellow,Black,4);
  UiLcdHy28_PrintText(5,104,"            ",Yellow,Black,4);
  UiLcdHy28_PrintText(5,92,clr_string,Yellow,Black,4);

}

#ifdef USE_FREEDV

#include "freedv_api.h"
#include "codec2_fdmdv.h"


struct freedv *f_FREEDV;

FDV_Audio_Buffer fdv_audio_buff[FDV_BUFFER_AUDIO_NUM];
FDV_IQ_Buffer __MCHF_SPECIALMEM fdv_iq_buff[FDV_BUFFER_IQ_NUM];

#define FDV_BUFFER_IQ_FIFO_SIZE (FDV_BUFFER_IQ_NUM+1)
// we allow for one more pointer to a buffer as we have buffers
// why? because our implementation will only fill up the fifo only to N-1 elements

FDV_IQ_Buffer* fdv_iq_buffers[FDV_BUFFER_IQ_FIFO_SIZE];

char freedv_rx_buffer[freedv_rx_buffer_max];

__IO int32_t fdv_iq_head = 0;
__IO int32_t fdv_iq_tail = 0;

int fdv_iq_buffer_peek(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (fdv_iq_head != fdv_iq_tail)
    {
        FDV_IQ_Buffer* c = fdv_iq_buffers[fdv_iq_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int fdv_iq_buffer_remove(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (fdv_iq_head != fdv_iq_tail)
    {
        FDV_IQ_Buffer* c = fdv_iq_buffers[fdv_iq_tail];
        fdv_iq_tail = (fdv_iq_tail + 1) % FDV_BUFFER_IQ_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int fdv_iq_buffer_add(FDV_IQ_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (fdv_iq_head + 1) % FDV_BUFFER_IQ_FIFO_SIZE;

    if (next_head != fdv_iq_tail)
    {
        /* there is room */
        fdv_iq_buffers[fdv_iq_head] = c;
        fdv_iq_head = next_head;
        ret ++;
    }
    return ret;
}

void fdv_iq_buffer_reset()
{
    fdv_iq_tail = fdv_iq_head;
}

int8_t fdv_iq_has_data()
{
    int32_t len = fdv_iq_head - fdv_iq_tail;
    return len < 0?len+FDV_BUFFER_IQ_FIFO_SIZE:len;
}

int32_t fdv_iq_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return FDV_BUFFER_IQ_FIFO_SIZE - 1 - fdv_iq_has_data();
}



#define FDV_BUFFER_AUDIO_FIFO_SIZE (FDV_BUFFER_AUDIO_NUM+1)
// we allow for one more pointer to a buffer as we have buffers
// why? because our implementation will only fill up the fifo only to N-1 elements

FDV_Audio_Buffer* fdv_audio_buffers[FDV_BUFFER_AUDIO_FIFO_SIZE];
__IO int32_t fdv_audio_head = 0;
__IO int32_t fdv_audio_tail = 0;

int fdv_audio_buffer_peek(FDV_Audio_Buffer** c_ptr)
{
    int ret = 0;

    if (fdv_audio_head != fdv_audio_tail)
    {
        FDV_Audio_Buffer* c = fdv_audio_buffers[fdv_audio_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int fdv_audio_buffer_remove(FDV_Audio_Buffer** c_ptr)
{
    int ret = 0;

    if (fdv_audio_head != fdv_audio_tail)
    {
        FDV_Audio_Buffer* c = fdv_audio_buffers[fdv_audio_tail];
        fdv_audio_tail = (fdv_audio_tail + 1) % FDV_BUFFER_AUDIO_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int fdv_audio_buffer_add(FDV_Audio_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (fdv_audio_head + 1) % FDV_BUFFER_AUDIO_FIFO_SIZE;

    if (next_head != fdv_audio_tail)
    {
        /* there is room */
        fdv_audio_buffers[fdv_audio_head] = c;
        fdv_audio_head = next_head;
        ret ++;
    }
    return ret;
}

void fdv_audio_buffer_reset()
{
    fdv_audio_tail = fdv_audio_head;
}

int8_t fdv_audio_has_data()
{
    int32_t len = fdv_audio_head - fdv_audio_tail;
    return len < 0?len+FDV_BUFFER_AUDIO_FIFO_SIZE:len;
}

int32_t fdv_audio_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return FDV_BUFFER_AUDIO_FIFO_SIZE - 1 - fdv_audio_has_data();
}



typedef struct {
    int32_t start;
    int32_t offset;
    int32_t count;
} flex_buffer;


void fdv_print_txt_msg()
{
  char txt_out[freedv_rx_buffer_max];

  snprintf(txt_out,freedv_rx_buffer_max,freedv_rx_buffer);

  UiLcdHy28_PrintText(5,92,txt_out,Yellow,Black,4);

}

void fdv_print_ber()
{
  int ber = 0;
  char ber_string[12];

  ber = 1000*freedv_get_total_bit_errors(f_FREEDV)/freedv_get_total_bits(f_FREEDV);
  snprintf(ber_string,12,"BER=0.%03d",ber);  //calculate and display the bit error rate
  UiLcdHy28_PrintText(5,104,ber_string,Yellow,Black,4);

}

void fdv_print_SNR()
{
  static float SNR = 1;
  float SNR_est;
  char SNR_string[12];
  int sync;

  freedv_get_modem_stats(f_FREEDV,&sync,&SNR_est);

  SNR = 0.95*SNR + 0.05 * SNR_est; //some averaging to keep it more calm
  if (SNR<0) SNR=0.0;
  snprintf(SNR_string,12,"SNR=%02d",(int)(SNR+0.5));  //Display the current SNR and round it up to the next int
  UiLcdHy28_PrintText(5,116,SNR_string,Yellow,Black,4);

}



void FreeDV_mcHF_HandleFreeDV()
{

    // Freedv Test DL2FW
    static uint16_t fdv_current_buffer_idx = 0;
    // static FDV_Out_Buffer FDV_TX_out_im_buff;
    static bool tx_was_here = false;
    static bool rx_was_here = false;


    if ((tx_was_here == true && ts.txrx_mode == TRX_MODE_RX) || (rx_was_here == true && ts.txrx_mode == TRX_MODE_TX))
    {
        tx_was_here = false; //set to false to detect the first entry after switching to TX
        rx_was_here = false;
        fdv_current_buffer_idx = 0;
        fdv_audio_buffer_reset();
        fdv_iq_buffer_reset();
    }
    //will later be inside RX
    if (ts.digital_mode == 1) {  // if we are in freedv1-mode and ...
        if ((ts.txrx_mode == TRX_MODE_TX) && fdv_audio_has_data() && fdv_iq_has_room())
        {           // ...and if we are transmitting and samples from dv_tx_processor are ready

            tx_was_here = true;
            fdv_current_buffer_idx %= FDV_BUFFER_IQ_NUM;

            FDV_Audio_Buffer* input_buf = NULL;
            fdv_audio_buffer_remove(&input_buf);

            freedv_comptx(f_FREEDV,
                    fdv_iq_buff[fdv_current_buffer_idx].samples,
                    input_buf->samples); // start the encoding process

            fdv_iq_buffer_add(&fdv_iq_buff[fdv_current_buffer_idx]);

            // to bypass the encoding
            // for (s=0;s<320;s++)
            // {
            //    FDV_TX_out_buff[FDV_TX_pt].samples[s] = FDV_TX_in_buff[ts.FDV_TX_in_start_pt].samples[s];
            // }
            // was_here ensures, that at least 2 encoded blocks are in the buffer before we start
            fdv_current_buffer_idx++;

        }
        else if ((ts.txrx_mode == TRX_MODE_RX))
        {

            bool leave_now = false;

            static flex_buffer outBufCtrl = { 0, 0, 0 }; // Audio Buffer
            static flex_buffer inBufCtrl = { 0, 0, 0 };  // IQ Buffer

            static FDV_IQ_Buffer* inBuf = NULL; // used to point to the current IQ input buffer
            // since we are not always use the same amount of samples, we need to remember the last (partially)
            // used buffer here. The index to the first unused data element into the buffer is stored in inBufCtrl.offset

            static int16_t rx_buffer[FDV_RX_AUDIO_SIZE_MAX];
            static COMP    iq_buffer[FDV_RX_AUDIO_SIZE_MAX];
            // these buffers are large enough to hold the requested/provided amount of data for freedv_comprx
            // these are larger than the FDV_BUFFER_SIZE since some more bytes may be asked for.

            if (!rx_was_here) {
        	freedv_set_total_bit_errors(f_FREEDV,0);  //reset ber calculation after coming from TX
        	freedv_set_total_bits(f_FREEDV,0);
        	fdv_clear_display();
            }


            rx_was_here = true; // this is used to clear buffers when going into TX

            // while makes this highest prio
            // if may give more responsiveness but can cause interrupted reception
            while (fdv_iq_has_data() && fdv_audio_has_room())
            // while (fdv_audio_has_room())
            // if (fdv_iq_has_data() && fdv_audio_has_room())
            {
                // MchfBoard_GreenLed(LED_STATE_OFF);

                leave_now = false;
                fdv_current_buffer_idx %= FDV_BUFFER_AUDIO_NUM; // this makes sure we stay in our index range, i.e. the number of avail buffers

                int iq_nin = freedv_nin(f_FREEDV); // how many bytes are requested as input for freedv_comprx ?

                // now fill the rx_buffer with the samples from the IQ input buffer
                while (inBufCtrl.offset != iq_nin)
                {
                    // okay no buffer with fresh data, let's get one
                    // we will get one without delay since we checked for that
                    // See below
                    if  (inBuf == NULL)
                    {
                        fdv_iq_buffer_peek(&inBuf);
#ifdef DEBUG_FREEDV
                        // here we simulate input using pre-generated data
                        static  int iq_testidx  = 0;
                        iq_testidx %= FREEDV_TEST_BUFFER_FRAME_COUNT;
                        /*
                        memcpy(inBuf->samples,
                                &test_buffer[iq_testidx++*FREEDV_TEST_BUFFER_FRAME_SIZE],
                                FREEDV_TEST_BUFFER_FRAME_SIZE*sizeof(COMP));
                                */
                        inBuf = (FDV_IQ_Buffer*)&test_buffer[iq_testidx++*FREEDV_TEST_BUFFER_FRAME_SIZE];

#endif
                        inBufCtrl.start = 0;
                    }

                    // do we empty a complete buffer?
                    if ( (iq_nin - inBufCtrl.offset) >= (FDV_BUFFER_SIZE  - inBufCtrl.start)  )
                    {
                        memcpy(&iq_buffer[inBufCtrl.offset],&inBuf->samples[inBufCtrl.start],(FDV_BUFFER_SIZE-inBufCtrl.start)*sizeof(COMP));
                        inBufCtrl.offset += FDV_BUFFER_SIZE-inBufCtrl.start;
                        fdv_iq_buffer_remove(&inBuf);
                        inBuf = NULL;

                        // if there is no buffer available, leave the whole
                        // function, next time we'll have more data ready here
                        leave_now = (fdv_iq_has_data() == 0);
                        if (leave_now)
                        {
                            break;
                        }
                    }
                    else
                    {
                        // the input buffer data will not be used completely to fill the buffer for the encoder.
                        // fill encoder buffer, remember pointer in iq in buffer coming from the audio interrupt
                        memcpy(&iq_buffer[inBufCtrl.offset],&inBuf->samples[inBufCtrl.start],(iq_nin - inBufCtrl.offset)*sizeof(COMP));
                        inBufCtrl.start += (iq_nin - inBufCtrl.offset);
                        inBufCtrl.offset = iq_nin;
                    }
                }
                if (leave_now == false)
                {

                    if (outBufCtrl.count == 0)
                    {
                        // if we arrive here the rx_buffer for comprx is full and will be consumed now.
                        inBufCtrl.offset = 0;
                        // profileTimedEventStart(7);
                        outBufCtrl.count = freedv_comprx(f_FREEDV, rx_buffer, iq_buffer); // run the decoding process
                        // profileTimedEventStop(7);
                        // outBufCtrl.count = iq_nin;
                    }

                    // result tells us the number of returned audio samples
                    // place  these in the audio output buffer for sending them to the I2S Codec
                    do
                    {
                        // the output data will fill the current audio output buffer
                        // some data may be left for copying into the next audio output buffer.
                        if ((outBufCtrl.count - outBufCtrl.offset) + outBufCtrl.start >= FDV_BUFFER_SIZE)
                        {
                            memcpy(&fdv_audio_buff[fdv_current_buffer_idx].samples[outBufCtrl.start],&rx_buffer[outBufCtrl.offset],(FDV_BUFFER_SIZE-outBufCtrl.start)*sizeof(int16_t));

                            outBufCtrl.offset += FDV_BUFFER_SIZE-outBufCtrl.start;

                            fdv_audio_buffer_add(&fdv_audio_buff[fdv_current_buffer_idx]);
                            fdv_current_buffer_idx ++;
                            fdv_current_buffer_idx %= FDV_BUFFER_AUDIO_NUM;
                            outBufCtrl.start = 0;

                            if (outBufCtrl.count > outBufCtrl.offset) {
                                // do we have more data? no -> leave the whole function
                                leave_now = (fdv_audio_has_room() == 0);
                                if (leave_now)
                                {
                                    break;
                                }
                                // we have to wait until we can use the next buffer
                            }
                            else
                            {
                                // ready to decode next  buffer
                                outBufCtrl.offset = 0;
                                outBufCtrl.count = 0;
                            }
                        }
                        else
                        {
                            // copy all output data we have into the audio output buffer, but we will not fill it completely
                            memcpy(&fdv_audio_buff[fdv_current_buffer_idx].samples[outBufCtrl.start],&rx_buffer[outBufCtrl.offset],(outBufCtrl.count - outBufCtrl.offset)*sizeof(int16_t));
                            outBufCtrl.start += (outBufCtrl.count-outBufCtrl.offset);
                            outBufCtrl.offset = outBufCtrl.count = 0;
                        }
                    } while (outBufCtrl.count > outBufCtrl.offset);
                }
            }
        }
        // MchfBoard_GreenLed(LED_STATE_ON);
        fdv_print_ber();
        fdv_print_SNR();
        fdv_print_txt_msg();
    }
    // END Freedv Test DL2FW
}

struct my_callback_state  my_cb_state;

// FreeDV txt test - will be out of here
struct my_callback_state {
    char  tx_str[80];
    char *ptx_str;
};

char my_get_next_tx_char(void *callback_state) {
    struct my_callback_state* pstate = (struct my_callback_state*)callback_state;
    char  c = *pstate->ptx_str++;

    if (*pstate->ptx_str == 0) {
        pstate->ptx_str = pstate->tx_str;
    }

    return c;
}

void my_put_next_rx_char(void *callback_state, char c) {
    short ch = (short)c;
    static int idx_rx=0;

    if (ch==13) idx_rx=0;

    else if (idx_rx < (freedv_rx_buffer_max))
      {
	freedv_rx_buffer[idx_rx]=c; //fill from left to right
	idx_rx++;
      }
	 else
	   {
	   for (int shift_count = 0;shift_count < (freedv_rx_buffer_max-1);shift_count++)
	     {
	       freedv_rx_buffer[shift_count]=freedv_rx_buffer[shift_count+1];
	     }
	    freedv_rx_buffer[freedv_rx_buffer_max-1]=c;
	   }



}



// FreeDV txt test - will be out of here

void  FreeDV_mcHF_init()
{
    // Freedv Test DL2FW

    f_FREEDV = freedv_open(FREEDV_MODE_1600);
	if( *(__IO uint32_t*)(SRAM2_BASE+5) == 0x29)
	{
  	  sprintf(my_cb_state.tx_str, FREEDV_TX_DF8OE_MESSAGE);
  	}
  	else
	{
  	  sprintf(my_cb_state.tx_str, FREEDV_TX_MESSAGE);
  	}
    my_cb_state.ptx_str = my_cb_state.tx_str;
    freedv_set_callback_txt(f_FREEDV, &my_put_next_rx_char, &my_get_next_tx_char, &my_cb_state);
    // freedv_set_squelch_en(f_FREEDV,0);
    // freedv_set_snr_squelch_thresh(f_FREEDV,-100.0);

    // ts.dvmode = true;
    // ts.digital_mode = 1;
}
// end Freedv Test DL2FW

#endif



#ifdef alternate_NR

__IO int32_t NR_in_head = 0;
__IO int32_t NR_in_tail = 0;
__IO int32_t NR_out_head = 0;
__IO int32_t NR_out_tail = 0;

FDV_IQ_Buffer* NR_in_buffers[FDV_BUFFER_IQ_FIFO_SIZE];

FDV_IQ_Buffer* NR_out_buffers[FDV_BUFFER_IQ_FIFO_SIZE];


int NR_in_buffer_peek(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_in_head != NR_in_tail)
    {
        FDV_IQ_Buffer* c = NR_in_buffers[NR_in_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int NR_in_buffer_remove(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_in_head != NR_in_tail)
    {
        FDV_IQ_Buffer* c = NR_in_buffers[NR_in_tail];
        NR_in_tail = (NR_in_tail + 1) % FDV_BUFFER_IQ_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int NR_in_buffer_add(FDV_IQ_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (NR_in_head + 1) % FDV_BUFFER_IQ_FIFO_SIZE;

    if (next_head != NR_in_tail)
    {
        /* there is room */
        NR_in_buffers[NR_in_head] = c;
        NR_in_head = next_head;
        ret ++;
    }
    return ret;
}

void NR_in_buffer_reset()
{
    NR_in_tail = NR_in_head;
}

int8_t NR_in_has_data()
{
    int32_t len = NR_in_head - NR_in_tail;
    return len < 0?len+FDV_BUFFER_IQ_FIFO_SIZE:len;
}

int32_t NR_in_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return FDV_BUFFER_IQ_FIFO_SIZE - 1 - NR_in_has_data();
}


//*********Out Buffer handling

int NR_out_buffer_peek(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_out_head != NR_out_tail)
    {
        FDV_IQ_Buffer* c = NR_out_buffers[NR_out_tail];
        *c_ptr = c;
        ret++;
    }
    return ret;
}


int NR_out_buffer_remove(FDV_IQ_Buffer** c_ptr)
{
    int ret = 0;

    if (NR_out_head != NR_out_tail)
    {
        FDV_IQ_Buffer* c = NR_out_buffers[NR_out_tail];
        NR_out_tail = (NR_out_tail + 1) % FDV_BUFFER_IQ_FIFO_SIZE;
        *c_ptr = c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int NR_out_buffer_add(FDV_IQ_Buffer* c)
{
    int ret = 0;
    int32_t next_head = (NR_out_head + 1) % FDV_BUFFER_IQ_FIFO_SIZE;

    if (next_head != NR_out_tail)
    {
        /* there is room */
        NR_out_buffers[NR_out_head] = c;
        NR_out_head = next_head;
        ret ++;
    }
    return ret;
}

void NR_out_buffer_reset()
{
    NR_out_tail = NR_out_head;
}

int8_t NR_out_has_data()
{
    int32_t len = NR_out_head - NR_out_tail;
    return len < 0?len+FDV_BUFFER_IQ_FIFO_SIZE:len;
}

int32_t NR_out_has_room()
{
    // FIXME: Since we cannot completely fill the buffer
    // we need to say full 1 element earlier
    return FDV_BUFFER_IQ_FIFO_SIZE - 1 - NR_out_has_data();
}



void alternateNR_handle()
{
        static uint16_t NR_current_buffer_idx = 0;
        static bool NR_was_here = false;

        if (NR_was_here == false)
        {
            NR_was_here = true;
            NR_current_buffer_idx = 0;
            NR_in_buffer_reset();
            NR_out_buffer_reset();
        }

        if ( NR_in_has_data() && NR_out_has_room())
        {   // audio data is ready to be processed

                NR_current_buffer_idx %= FDV_BUFFER_IQ_NUM;

                FDV_IQ_Buffer* input_buf = NULL;
                NR_in_buffer_remove(&input_buf); //&input_buffer points to the current valid audio data

      // inside here do all the necessary noise reduction stuff!!!!!
      // here are the current input samples:  input_buf->samples
      // NR_output samples have to be placed here: fdv_iq_buff[NR_current_buffer_idx].samples
      // but starting at an offset of NR_FFT_SIZE as we are using the same buffer for in and out
      // here is the only place where we are referring to fdv_iq... as this is the name of the used freedv buffer

                profileTimedEventStart(ProfileTP8);

                do_alternate_NR(&input_buf->samples[0].real,&fdv_iq_buff[NR_current_buffer_idx].samples[NR_FFT_SIZE].real);

                profileTimedEventStop(ProfileTP8);

                NR_out_buffer_add(&fdv_iq_buff[NR_current_buffer_idx]);
                NR_current_buffer_idx++;

        }

}




void do_alternate_NR(float32_t* inputsamples, float32_t* outputsamples )
{

    float32_t* Energy=0;



    alt_noise_blanking(inputsamples,NR_FFT_SIZE,Energy);

    for (int k=0; k < NR_FFT_SIZE;  k++)
                                    {
                                        outputsamples[k] = inputsamples[k];
                                    }

}


//alt noise blanking is trying to localize some impulse noise within the samples and after that
//trying to replace corrupted samples by linear predicted samples.
//therefore, first we calculate the lpc coefficients which represent the actual status of the
//speech or sound generating "instrument" (in case of speech this is an estimation of the current
//filter-function of the voice generating tract behind our lips :-) )
//after finding this function we inverse filter the actual samples by this function
//so we are eliminating the speech, but not the noise. Then we do a matched filtering an thereby detecting impulses
//After that we threshold the remaining samples by some
//level and so detecting impulse noise's positions within the current frame - if one (or more) impulses are there.
//finally some area around the impulse position will be replaced by predicted samples from both sides (forward and
//backward prediction)
//hopefully we have enough processor power left....

void alt_noise_blanking(float* insamp,int Nsam, int order, float* E )
{
#define boundary_blank 14 // for first trials very large!!!!
#define impulse_length 7 // has to be odd!!!! 7 / 3 should be enough
#define PL             3 // has to be (impulse_length-1)/2 !!!!
#define order         10 // lpc's order
    arm_fir_instance_f32 LPC;
    float32_t lpcs[order+1]; // we reserve one more than "order" because of a leading "1"
    float32_t reverse_lpcs[order+1]; //this takes the reversed order lpc coefficients
    float32_t firStateF32[NR_FFT_SIZE + order];
    float32_t tempsamp[NR_FFT_SIZE];
    float32_t sigma2; //taking the variance of the inpo
    float32_t lpc_power;
    float32_t impulse_threshold;
    int impulse_positions[5];  //we allow a maximum of 5 impulses per frame
    int search_pos=0;
    int impulse_count=0;
    static float32_t last_frame_end[order+PL]; //this takes the last samples from the previous frame to do the prediction within the boundaries
#ifdef debug_alternate_NR
    static int frame_count=0;  //only used for the distortion insertion - can alter be deleted
    int dist_level=0;//only used for the distortion insertion - can alter be deleted
    int nr_setting = 0;
#endif

    float32_t R[11];  // takes the autocorrelation results
    float32_t k,alfa;

    float32_t any[order+1];  //some internal buffers for the levinson durben algorithm

    float32_t Rfw[impulse_length+order]; // takes the forward predicted audio restauration
    float32_t Rbw[impulse_length+order]; // takes the backward predicted audio restauration
    float32_t Wfw[impulse_length],Wbw[impulse_length]; // taking linear windows for the combination of fwd and bwd

    float32_t s;

#ifdef debug_alternate_NR  // generate test frames to test the noise blanker function
                           // using the NR-setting (0..55) to select the test frame
                           // 00 = noise blanker active on orig. audio; threshold factor=3
                           // 01 = frame of vocal "a" undistorted
                           // 02 .. 05 = frame of vocal "a" with different impulse distortion levels
                           // 06 .. 09 = frame of vocal "a" with different impulse distortion levels
                           //            noise blanker operating!!
                           //************
                           // 01..09 are now using the original received audio and applying a rythmic "click" distortion
                           // 06..09 is detecting and removing the click by restoring the predicted audio!!!
                           //************
                           // 5 / 9 is the biggest "click" and it is slightly noticeable in the restored audio (9)
                           // 10 = noise blanker active on orig. audio threshold factor=3
                           // 11  = sinusoidal signal undistorted
                           // 12 ..15 = sinusoidal signal with different impulse distortion levels
                           // 16 ..19 = sinusoidal signal with different impulse distortion levels
                           //            noise blanker operating!!
                           // 20 ..50   noise blanker active on orig. audio; threshold factor varying between 3 and 0.26

    nr_setting = (int)ts.dsp_nr_strength;
//*********************************from here just debug impulse / signal generation
    if ((nr_setting > 0) && (nr_setting < 10)) // we use the vocal "a" frame
        {
            //for (int i=0; i<128;i++)          // not using vocal "a" but the original signal
            //    insamp[i]=NR_test_samp[i];

            if ((frame_count > 19) && (nr_setting > 1))    // insert a distorting pulse
                {
                   dist_level=nr_setting;
                   if (dist_level > 5) dist_level=dist_level-4; // distortion level is 1...5
                   insamp[4]=insamp[4] + dist_level*3000; // overlaying a short  distortion pulse +/-
                   insamp[5]=insamp[5] - dist_level*1000;
                }

        }

    if ((nr_setting > 10) && (nr_setting < 20)) // we use the sinus frame
        {
            for (int i=0; i<128;i++)
                insamp[i]=NR_test_sinus_samp[i];

            if ((frame_count > 19) && (nr_setting > 11))    // insert a distorting pulse
                {
                   dist_level=nr_setting-10;
                   if (dist_level > 5) dist_level=dist_level-4;
                   insamp[24]=insamp[24] + dist_level*1000; // overlaying a short  distortion pulse +/-
                   insamp[25]=insamp[25] + dist_level*500;
                   insamp[26]=insamp[26] - dist_level*200; // overlaying a short  distortion pulse +/-
                   insamp[27]=insamp[27] - dist_level*100;


                }



        }


frame_count++;
if (frame_count > 20) frame_count=0;

    #endif

//*****************************end of debug impulse generation

//  start of test timing zone

    for (int i=0; i<impulse_length; i++)  // generating 2 Windows for the combination of the 2 predictors
    {                                     // will be a constant window later!
        Wbw[i]=1.0*i/(impulse_length-1);
        Wfw[impulse_length-i-1]=Wbw[i];
    }

    // calculate the autocorrelation of insamp (moving by max. of #order# samples)
      for(int i=0; i < (order+1); i++)
      {
          arm_dot_prod_f32(&insamp[0],&insamp[i],Nsam-i,&R[i]); // R is carrying the crosscorrelations
      }
    // end of autocorrelation



//alternative levinson durben algorithm to calculate the lpc coefficients from the crosscorrelation

  R[0] = R[0] * (1.0 + 1.0e-9);

  lpcs[0] = 1;   //set lpc 0 to 1

  for (int i=1; i < order+1; i++)
      lpcs[i]=0;                      // fill rest of array with zeros - could be done by memfill

  alfa = R[0];

  for (int m = 1; m <= order; m++)
   {
     s = 0.0;
     for (int u = 1; u < m; u++)
        s = s + lpcs[u] * R[m-u];

     k = -(R[m] + s) / alfa;

     for (int v = 1;v < m; v++)
         any[v] = lpcs[v] + k * lpcs[m-v];

     for (int w = 1; w < m; w++)
         lpcs[w] = any[w];

    lpcs[m] = k;
    alfa = alfa * (1 - k * k);
  }

    // end of levinson durben algorithm

    for (int o = 0; o < order+1; o++ )             //store the reverse order coefficients separately
        reverse_lpcs[order-o]=lpcs[o];        // for the matched impulse filter

    arm_fir_init_f32(&LPC,order+1,&reverse_lpcs[0],&firStateF32[0],NR_FFT_SIZE);                                         // we are using the same function as used in freedv

    arm_fir_f32(&LPC,insamp,tempsamp,Nsam); //do the inverse filtering to eliminate voice and enhance the impulses

    arm_fir_init_f32(&LPC,order+1,&lpcs[0],&firStateF32[0],NR_FFT_SIZE);                                         // we are using the same function as used in freedv

    arm_fir_f32(&LPC,tempsamp,tempsamp,Nsam); // do a matched filtering to detect an impulse in our now voiceless signal


    arm_var_f32(tempsamp,NR_FFT_SIZE,&sigma2); //calculate sigma2 of the original signal ? or tempsignal

    arm_power_f32(lpcs,order,&lpc_power);  // calculate the sum of the squares (the "power") of the lpc's

    impulse_threshold = 2.5 * sqrtf(sigma2 * lpc_power);  //set a detection level (3 is not really a final setting)

    //if ((nr_setting > 20) && (nr_setting <51))
    //    impulse_threshold = impulse_threshold / (0.9 + (nr_setting-20.0)/10);  //scaling the threshold by 1 ... 0.26

    search_pos = order+PL;  // lower boundary problem has been solved! - so here we start from 1 or 0?
    impulse_count=0;

    do {        //going through the filtered samples to find an impulse larger than the threshold

        if ((tempsamp[search_pos] > impulse_threshold)||(tempsamp[search_pos] < (-impulse_threshold)))
            {
            impulse_positions[impulse_count]=search_pos - order;  // save the impulse positions and correct it by the filter delay
            impulse_count++;
            search_pos+=PL;   //  set search_pos a bit away, cause we are already repairing this area later
                              //  and the next impulse should not be that close
            }

        search_pos++;

        } while ((search_pos < NR_FFT_SIZE-boundary_blank) && (impulse_count < 5));// avoid upper boundary

                        //boundary handling has to be fixed later
                        //as a result we now will not find any impulse in these areas

// from here: reconstruction of the impulse-distorted audio part:

// first we form the forward and backward prediction transfer functions from the lpcs
// that is easy, as they are just the negated coefficients  without the leading "1"
// we can do this in place of the lpcs, as they are not used here anymore and being recalculated in the next frame!

    arm_negate_f32(&lpcs[1],&lpcs[1],order);
    arm_negate_f32(&reverse_lpcs[0],&reverse_lpcs[0],order);


    for (int j=0; j<impulse_count; j++)
        {
        for (int k = 0; k<order; k++)   // we have to copy some samples from the original signal as
            {                           // basis for the reconstructions - could be done by memcopy

               if ((impulse_positions[j]-PL-order+k) < 0)// this solves the prediction problem at the left boundary
               {
                   Rfw[k]=last_frame_end[impulse_positions[j]+k];//take the sample from the last frame
               }
               else
               {
                   Rfw[k]=insamp[impulse_positions[j]-PL-order+k];//take the sample from this frame as we are away from the boundary
               }

               Rbw[impulse_length+k]=insamp[impulse_positions[j]+PL+k+1];



            }     //bis hier alles ok

            for (int i = 0; i < impulse_length; i++) //now we calculate the forward and backward predictions
                {
                    arm_dot_prod_f32(&reverse_lpcs[0],&Rfw[i],order,&Rfw[i+order]);
                    arm_dot_prod_f32(&lpcs[1],&Rbw[impulse_length-i],order,&Rbw[impulse_length-i-1]);

                }

                arm_mult_f32(&Wfw[0],&Rfw[order],&Rfw[order],impulse_length); // do the windowing, or better: weighing
                arm_mult_f32(&Wbw[0],&Rbw[0],&Rbw[0],impulse_length);



#ifdef debug_alternate_NR
                    // in debug mode do the restoration only in some cases
if (((ts.dsp_nr_strength > 0) && (ts.dsp_nr_strength < 6))||((ts.dsp_nr_strength > 10) && (ts.dsp_nr_strength < 16)))
{
    // just let the distortion pass at setting 1...5 and 11...15
    //    arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);
}
else
{
    //finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
    arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);
}
#else
    //finally add the two weighted predictions and insert them into the original signal - thereby eliminating the distortion
                arm_add_f32(&Rfw[order],&Rbw[0],&insamp[impulse_positions[j]-PL],impulse_length);

#endif
        }

 for (int p=0; p<(order+PL); p++)
 {
     last_frame_end[p]=insamp[NR_FFT_SIZE-1-order-PL+p];// store 13 samples from the current frame to use at the next frame
 }
    //end of test timing zone
}

#endif
