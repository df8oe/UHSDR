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
#include "ui_driver.h"
#include "freedv_uhsdr.h"
#include "ui_lcd_layouts.h"
#include "profiling.h"
#include "ui_lcd_hy28.h"
#include "radio_management.h"

#if defined(USE_FREEDV) || defined(USE_ALTERNATE_NR)

MultiModeBuffer_t __MCHF_SPECIALMEM mmb;
#endif

#ifdef USE_FREEDV

#include "freedv_api.h"
#include "codec2_fdmdv.h"


struct freedv *f_FREEDV;

FDV_Audio_Buffer fdv_audio_buff[FDV_BUFFER_AUDIO_NUM];

FDV_IQ_Buffer* fdv_iq_buffers[FDV_BUFFER_IQ_FIFO_SIZE];


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

int32_t fdv_iq_has_data()
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

int32_t fdv_audio_has_data()
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

static uint16_t freedv_display_x_offset;



static void FreeDv_DisplayBer()
{
    int ber = 0;
    char ber_string[12];



    ber = 1000*freedv_get_total_bit_errors(f_FREEDV)/freedv_get_total_bits(f_FREEDV);
    snprintf(ber_string,12,"0.%03d",ber);  //calculate and display the bit error rate
    UiLcdHy28_PrintText(ts.Layout->FREEDV_BER.x + freedv_display_x_offset,ts.Layout->FREEDV_BER.y, ber_string,Yellow,Black, ts.Layout->FREEDV_FONT);

}

static void FreeDv_DisplaySnr()
{
    static float SNR = 1;
    float SNR_est;
    char SNR_string[12];
    int sync;

    freedv_get_modem_stats(f_FREEDV,&sync,&SNR_est);

    SNR = 0.95*SNR + 0.05 * SNR_est; //some averaging to keep it more calm
    if (SNR<0) SNR=0.0;
    snprintf(SNR_string,12,"%02d",(int)(SNR+0.5));  //Display the current SNR and round it up to the next int
    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x + freedv_display_x_offset, ts.Layout->FREEDV_SNR.y ,SNR_string,Yellow,Black, ts.Layout->FREEDV_FONT);
}

void FreeDv_DisplayClear()
{
//    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x, ts.Layout->FREEDV_SNR.y,"            ",Yellow,Black,ts.Layout->FREEDV_FONT);
//   UiLcdHy28_PrintText(ts.Layout->FREEDV_BER.x, ts.Layout->FREEDV_BER.y,"            ",Yellow,Black,ts.Layout->FREEDV_FONT);
    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x, ts.Layout->FREEDV_SNR.y,"         ",Yellow,Black,ts.Layout->FREEDV_FONT);		//SNR=00
    UiLcdHy28_PrintText(ts.Layout->FREEDV_BER.x, ts.Layout->FREEDV_BER.y,"         ",Yellow,Black,ts.Layout->FREEDV_FONT);		//BER=0.000	(max 9 chars)
    UiDriver_TextMsgClear();
}

void FreeDv_DisplayPrepare()
{
	UiDriver_TextMsgClear();
	freedv_display_x_offset = UiLcdHy28_TextWidth("SNR=", ts.Layout->FREEDV_FONT);
    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x, ts.Layout->FREEDV_SNR.y,"SNR=",Yellow,Black, ts.Layout->FREEDV_FONT);
    UiLcdHy28_PrintText(ts.Layout->FREEDV_BER.x, ts.Layout->FREEDV_BER.y,"BER=",Yellow,Black, ts.Layout->FREEDV_FONT);
}

void FreeDv_DisplayUpdate()
{
    FreeDv_DisplayBer();
    FreeDv_DisplaySnr();
    UiDriver_TextMsgDisplay();
}


void FreeDv_HandleFreeDv()
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
    if (ts.digital_mode == DigitalMode_FreeDV) {  // if we are in freedv1-mode and ...
        if ((ts.txrx_mode == TRX_MODE_TX) && fdv_audio_has_data() && fdv_iq_has_room())
        {           // ...and if we are transmitting and samples from dv_tx_processor are ready

            tx_was_here = true;
            fdv_current_buffer_idx %= FDV_BUFFER_IQ_NUM;

            FDV_Audio_Buffer* input_buf = NULL;
            fdv_audio_buffer_remove(&input_buf);

            freedv_comptx(f_FREEDV,
                    mmb.fdv_iq_buff[fdv_current_buffer_idx].samples,
                    input_buf->samples); // start the encoding process

            fdv_iq_buffer_add(&mmb.fdv_iq_buff[fdv_current_buffer_idx]);

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
                FreeDv_DisplayClear();
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


void my_put_next_rx_char(void *callback_state, char ch) {
    UiDriver_TextMsgPutChar(ch);
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
