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
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

// Common
#include "uhsdr_board.h"
#include "cat_driver.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include <stdio.h>
#include "audio_driver.h"
#include "radio_management.h"
#include "config_storage.h"

uint8_t limit_4bits(uint32_t in)
{
    return in > 15 ? 15 : in;
}
// CAT driver internal structure
typedef struct CatDriver
{
    bool    cat_ptt_active;
    CatInterfaceState state;
    CatInterfaceProtocol protocol;
    uint32_t lastbufferadd_time;

} CatDriver;

// CAT driver state
CatDriver                  cat_driver;


static void CatDriver_CatEnableTX(bool enable)
{
    if (enable)
    {
        if(RadioManagement_IsTxDisabled() == false)
        {
            RadioManagement_Request_TxOn();
            cat_driver.cat_ptt_active = true;
        }
    }
    else
    {
        cat_driver.cat_ptt_active = false;
        RadioManagement_Request_TxOff();
    }
}


/**
 * @brief returns true if the current TX state has been initiated by a CAT PTT command
 */
bool CatDriver_CatPttActive()
{
    return cat_driver.cat_ptt_active;
}

bool CatDriver_CWKeyPressed()
{
    return cdcvcp_ctrllines.dtr != 0;
}

bool CatDriver_PTTKeyPressed()
{
    return ts.enable_ptt_rts? (cdcvcp_ctrllines.rts != 0) : false;
}

bool CatDriver_PTTKeyChangedState()
{
    static bool ptt_key_last_state;

    bool curval = CatDriver_PTTKeyPressed();
    bool retval = ptt_key_last_state != curval;
    ptt_key_last_state = curval;

    return retval;
}


CatInterfaceState CatDriver_GetInterfaceState()
{
    return hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED?CAT_CONNECTED:CAT_DISCONNECTED;
}


#define CAT_BUFFER_SIZE 256
__IO uint8_t cat_buffer[CAT_BUFFER_SIZE];
__IO int32_t cat_head = 0;
__IO int32_t cat_tail = 0;

static uint8_t CatDriver_InterfaceBufferHasData()
{
    int32_t len = cat_head - cat_tail;
    return len < 0?len+CAT_BUFFER_SIZE:len;
}



static int cat_buffer_remove(uint8_t* c_ptr)
{
	int ret = 0;

    if (cat_head != cat_tail)
    {
        int c = cat_buffer[cat_tail];
        cat_tail = (cat_tail + 1) % CAT_BUFFER_SIZE;
        *c_ptr = (uint8_t)c;
        ret++;
    }
    return ret;
}

/* no room left in the buffer returns 0 */
int CatDriver_InterfaceBufferAddData(uint8_t c)
{
	int ret = 0;
    int32_t next_head = (cat_head + 1) % CAT_BUFFER_SIZE;

    if (next_head != cat_tail)
    {
        /* there is room */
        cat_buffer[cat_head] = c;
        cat_head = next_head;
        cat_driver.lastbufferadd_time = ts.sysclock;
        ret ++;
    }
    return ret;
}

static void cat_buffer_reset()
{
    cat_tail = cat_head;
}

#define CAT_DRIVER_TIMEOUT 30
// defined in increments of 10ms, needs to be longer than the longest running operation
// the mcHF can do. It seems band switching is taking longest time.
// According to the FT817 manual, timeout is 200ms, so we use that plus a little extra and it seems to work

/**
 * Synchronize CAT data and CAT decoder mechanism.
 * As there is a chance that buffers have received erroneous data
 * at least at startup or if the external program stops without completely sending the
 * request data, we expire old data in the request data buffer.
 */
static void cat_driver_sync_data( void)
{
    uint32_t bufsz = CatDriver_InterfaceBufferHasData();
    // we now know how much data is available
    // this MUST BE DONE BEFORE find out how old the data is
    // otherwise a race condition  can occur and we throw away new data.

    if (bufsz)
    {
        if ( ts.sysclock - CAT_DRIVER_TIMEOUT > cat_driver.lastbufferadd_time)
        {
            // if we are here the first bufsz bytes are older than 200ms
            // if in the meantime new bytes arrive, no problem, we keep them
            // since we remove the first bufsz "old data" bytes only from
            // the front of the buffer
            while(bufsz)
            {
                uint8_t c;
                cat_buffer_remove(&c);
                bufsz--;
            }
        }
    }
}

static uint8_t CatDriver_InterfaceBufferGetData(uint8_t* Buf,uint32_t Len)
{
    uint8_t res = 0;
    if (CatDriver_InterfaceBufferHasData() >= Len)
    {
        for  (int i = 0; i < Len; i++)
        {
            cat_buffer_remove(&Buf[i]);
        }
        res = 1;
    }
    return res;
}


static uint8_t CatDriver_InterfaceBufferPutData(uint8_t* Buf,uint32_t Len)
{
    uint8_t res = 0;
    if (CatDriver_GetInterfaceState() == CAT_CONNECTED && Len > 0)
    {
        ;
        res = CDC_Transmit_FS(Buf,Len) == USBD_OK;
    }
    return res;
}


/*

    DUPLEX = ["", "-", "+", "split"]
    # narrow modes has to be at end
    MODES = ["LSB", "USB", "CW", "CWR", "AM", "FM", "DIG", "PKT", "NCW",
             "NCWR", "NFM"]
    TMODES = ["", "Tone", "TSQL", "DTCS"]
    STEPSFM = [5.0, 6.25, 10.0, 12.5, 15.0, 20.0, 25.0, 50.0]
    STEPSAM = [2.5, 5.0, 9.0, 10.0, 12.5, 25.0]
    STEPSSSB = [1.0, 2.5, 5.0]

    # warning ranges has to be in this exact order
    VALID_BANDS = [
                    (100000, 33000000),  0  // USED
                    (33000000, 56000000), 1
                    (76000000, 108000000), 2  // NOT USED
                    (108000000, 137000000), 3 // NOT USED
                    (137000000, 154000000), 4
                    (420000000, 470000000) 5
                    ]
 */
// based on CHIRP ft817.py, gcc needs reversal of allocations inside 8bit
typedef struct {
    uint8_t  mode:3, // LSB, USB, ...
    unknown1:3,
    tag_default:1, // do we show the automatically generate name CH-XXX
    tag_on_off:1; // do we show the stored name?

    uint8_t freq_range:3,
    is_fm_narrow:1,
    is_cwdig_narrow:1,
    is_duplex:1, // is a duplex memory
    duplex:2; // how to interpret the offset with relation to the freq entry
    // 0 -> no offset / second freq
    // 1 -> - RX is freq - offset
    // 2 -> + RX is freq + offset
    // 3 -> split frequency, offset is used to store second frequency
    uint8_t unknown3:4,
    att:1, // attenuator on, NOT USED
    ipo:1, // IPO attenuator on, NOT USED
    unknown2:1,
    skip:1;

    uint8_t  fm_step:3,
        am_step:3,
        ssb_step:2;

    uint8_t tmode:2,
    unknown4:6;

    uint8_t tx_freq_range:3,  // VFO B?
    tx_mode:3, // VFO B?
    unknown5:2;

    uint8_t tone:6,
    unknown_toneflag:1,
    unknown6:1;

    uint8_t  dcs:7,
    unknown7:1;

    uint16_t rit; // ul16
    uint32_t freq; // ul32 -> USED VFO A
    uint32_t offset; // ul32 -> USED, VFO B
    uint8_t  name[8];  // USED
} __attribute__((packed)) ft817_memory_t ;

typedef struct {
    uint8_t  fst:1,
        lock:1,
        nb:1,
        pbt:1,
        unknownb:1,
        dsp:1,
        agc:2;
    uint8_t  vox:1,
        vlt:1,
        bk:1,
        kyr:1,
        unknown5:1,
        cw_paddle:1,
        pwr_meter_mode:2;
    uint8_t  vfob_band_select:4,
        vfoa_band_select:4;
    uint8_t  unknowna;
    uint8_t  backlight:2,
        color:2,
        contrast:4;
    uint8_t  beep_freq:1,
        beep_volume:7;
    uint8_t  arts_beep:2,
        main_step:1,
        cw_id:1,
        scope:1,
        pkt_rate:1,
        resume_scan:2;
    uint8_t  op_filter:2,
        lock_mode:2,
        cw_pitch:4;
    uint8_t  sql_rf_gain:1,
        ars_144:1,
        ars_430:1,
        cw_weight:5;
    uint8_t  cw_delay;
    uint8_t  unknown8:1,
        sidetone:7;
    uint8_t  batt_chg:2,
        cw_speed:6;
    uint8_t  disable_amfm_dial:1,
        vox_gain:7;
    uint8_t  cat_rate:2,
        emergency:1,
        vox_delay:5;
    uint8_t  dig_mode:3,
        mem_group:1,
        unknown9:1,
        apo_time:3;
    uint8_t  dcs_inv:2,
        unknown10:1,
        tot_time:5;
    uint8_t  mic_scan:1,
        ssb_mic:7;
    uint8_t  mic_key:1,
        am_mic:7;
    uint8_t  unknown11:1,
        fm_mic:7;
    uint8_t  unknown12:1,
        dig_mic:7;
    uint8_t  extended_menu:1,
        pkt_mic:7;
    uint8_t  unknown14:1,
        pkt9600_mic:7;
    int16_t dig_shift; // il16
    int16_t dig_disp;  // il16
    int8_t  r_lsb_car;
    int8_t  r_usb_car;
    int8_t  t_lsb_car;
    int8_t  t_usb_car;
    uint8_t  unknown15:2,
        menu_item:6;
    uint8_t  unknown16:4,
        menu_sel:4;
    uint16_t unknown17;
    uint8_t  art:1,
        scn_mode:2,
        dw:1,
        pri:1,
        unknown18:1,
        tx_power:2;
    uint8_t  spl:1,
        unknown:1,
        uhf_antenna:1,
        vhf_antenna:1,
        air_antenna:1,
        bc_antenna:1,
        sixm_antenna:1,
        hf_antenna:1;
} __attribute__((packed)) ft871_settings_t ;

typedef struct {
    uint8_t len;
    uint8_t count;
} ft817_block_t;

// FT817 (not ND!)
const ft817_block_t cloneblock_len[] =
{
        {2,1},      //0  -> 2
        {40,1},     // -> 42
        {208,1},    // -> 250
        {182,1},    // -> 432
        {208,1},    // -> 640
        {182,1},    // -> 822
        {198,1},    // -> 1020
        {53,1},     // -> 1073
        {130,40},   // -> 6273 (+5200)
        {118,1},    // -> 6391
        {118,1}     // -> 6509
};

/*
@0x4:
    ft817_settings_t settings;
@0x2A: -> @42 -> block [2]
        struct mem_struct vfoa[15]; // block[2+3]
        struct mem_struct vfob[15]; // block[4+5]
        struct mem_struct home[4]; //  block[6]...
        struct mem_struct qmb;
        struct mem_struct mtqmb;
        struct mem_struct mtune;   //  ...block[6]

@0x3FD: @1021               // block[7]+1
        uint8_t visible[25];
        uint8_t pmsvisible;

@0x417: @1047
        uint8_t filled[25];      // block[7]+27
        uint8_t pmsfilled;

@0x431: @1073
        struct mem_struct memory[200]; block[9]-block[48]
        struct mem_struct pms[2]; block[49]

@0x18cf: @6351
        uint8_t callsign[7];

@0x1979: @6521
        struct mem_struct sixtymeterchannels[5]; // not FT817, only ND US

*/

typedef enum {
    CLONEOUT_INIT = 0,
    CLONEOUT_BLOCK_SEND,
    CLONEOUT_BLOCK_ACK_WAIT,
    CLONEOUT_BLOCK_ACK_NACK,
    CLONEOUT_DONE

} ft817_clone_out_st;
// #define DEBUG_FT817
typedef enum {
    CLONEIN_INIT = 0,
    CLONEIN_BLOCK_RECV,
    CLONEIN_BLOCK_RECV_START,
    CLONEIN_FINAL_PROCESSING,
    CLONEIN_DONE

} ft817_clone_in_st;


typedef enum {
    CAT_INIT = 0,
    CAT_CAT,
    CAT_CLONEOUT,
    CAT_CLONEIN
} ft817_cat_st;


struct FT817
{
    uint8_t req[5];
    ft817_cat_st state;
    ft817_clone_out_st cloneout_state;
    ft817_clone_in_st clonein_state;
// #define DEBUG_FT817
#ifdef DEBUG_FT817
#define FT817_MAX_CMD 100
    uint8_t reqs[FT817_MAX_CMD*5];
    uint32_t cmd_cntr;
#endif
};

#include "ui_driver.h"
#include "uhsdr_board.h"

// FT817 Emulation
#if 0
// list of commands supported by hamlib
static const yaesu_cmd_set_t ncmd[] =
{
    { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* lock on */
    { 1, { 0x00, 0x00, 0x00, 0x00, 0x80 } }, /* lock off */
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0x08 } }, /* ptt on */
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0x88 } }, /* ptt off */
    +{ 0, { 0x00, 0x00, 0x00, 0x00, 0x01 } }, /* set freq */
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main LSB */
    +{ 1, { 0x01, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main USB */
    +{ 1, { 0x02, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main CW */
    +{ 1, { 0x03, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main CWR */
    +{ 1, { 0x04, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main AM */
    +{ 1, { 0x08, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main FM */
    +{ 1, { 0x88, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main FM-N */
    +{ 1, { 0x0a, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main DIG */
    +{ 1, { 0x0c, 0x00, 0x00, 0x00, 0x07 } }, /* mode set main PKT */
    { 1, { 0x00, 0x00, 0x00, 0x00, 0x05 } }, /* clar on */
    { 1, { 0x00, 0x00, 0x00, 0x00, 0x85 } }, /* clar off */
    { 0, { 0x00, 0x00, 0x00, 0x00, 0xf5 } }, /* set clar freq */
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0x81 } }, /* toggle vfo a/b */
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0x02 } }, /* split on */
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0x82 } }, /* split off */
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
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0xe7 } }, /* get RX status  */
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0xf7 } }, /* get TX status  */
    { 1, { 0x00, 0x00, 0x00, 0x00, 0x03 } }, /* get FREQ and MODE status */
    { 1, { 0x00, 0x00, 0x00, 0x00, 0x00 } }, /* pwr wakeup sequence */
    +{ 1, { 0x00, 0x00, 0x00, 0x00, 0x0f } }, /* pwr on */
    { 1, { 0x00, 0x00, 0x00, 0x00, 0x8f } }, /* pwr off */
    ?{ 0, { 0x00, 0x00, 0x00, 0x00, 0xbb } }, /* eeprom read */
}




#endif

typedef enum {
    FT817EE_INVALID = 0,
    FT817EE_DATA,
    FT817EE_FUNC,
    FT817EE_STOP
} ft817_ee_entry_t;

typedef struct {
    ft817_ee_entry_t type;
    uint16_t start;
    uint16_t end;
    union {
    bool (*funcPtr)(bool read, uint16_t addr, uint8_t* data_p);
    uint8_t value;
    } content;
} ft817_eeprom_emul_t;


// here we defined the eeprom emulation
// for some tools we may want to support either reading a fixed value
// or even reading/writing actual information from/to mcHF
// all other addresses will indicate failure

bool CatDriver_Ft817_EEPROM_RW_Func(bool readEEPROM, uint16_t addr, uint8_t* data_p)
{
    bool retval = false;
    if (readEEPROM == false)
    {
        switch(addr)
        {

        case 0x55:
        {
            uint8_t vfo_selection = data_p[0] & 0x81;
            if (
                    (vfo_selection == 0x81 && is_vfo_b() == false)
                    ||
                    (vfo_selection == 0x80 && is_vfo_b() == true)
            )
            {
                // we have to switch active vfo
                UiAction_ToggleVfoAB();
                retval = true;
            }
            break;
        }
        }
    }
    else
    {
        switch(addr)
        {
        /*
        HRD:
        case 0x56:

        case 0x57:
        case 0x58:

        case 0x5F:
        case 0x60:

        case 0x80

        case 0x7B:
         */
        case 0x55:
            // Used by: HRD
            // Bit 0: 0 -> VFO A, 1 -> VFO B
            // Bit 5: 0 -> Memory, 1 -> MTUNE
            // Bit 6: Unknown
            // Bit 7: 0 -> Memory Mode, 1 -> VFO
            *data_p = is_vfo_b()? 0x81:0x80;

            // TODO: If memmode gets implmented, we need more complex handling here
            retval = true;
            break;
        case 0x65:
            // lower bits not relevant bits 5-7
            // 000 -> RTTY
            // 001 -> PSK31-L
            // 010 -> PSK31-U
            // 011 -> USER-L
            // 100 -> USER-U
            // we always report USER-U/USER-L independent from currently selected digital mode
            // on the UHSDR TRX
            *data_p = RadioManagement_LSBActive(DEMOD_DIGI) ? 0b01100000 : 0b10000000;
            retval = true;
            break;
        case 0x79:
        {
            // Used by: HRD
            // Bit 0 - 1: TX Power 00 - Hi -> 11: L1 (lowest)
            // Bit 2: Unknown
            // Bit 3: PRI On?
            // Bit 4: DW On?
            // Bit 5-6: Scan Mode: 00 No, 10 Scan Up, 11 Scan Down
            // Bit 7: ART On?
            uint8_t powerlevel = 0;
            switch (ts.power_level)
            {
            case PA_LEVEL_MINIMAL:
                powerlevel = 3;
                break;
            case PA_LEVEL_LOW:
                powerlevel = 2;
                break;
            case PA_LEVEL_MEDIUM:
                powerlevel = 1;
                break;
            default:
                // we report 5W and FULL as High
                // TODO: or shall we merge two other power settings?
                powerlevel = 0;
                break;
            }
            *data_p = powerlevel;
            retval = true;
            break;
        }
        case 0x7A:
        {
            // Used by: HRD, N1MM
            // Bit 0 - 5: Antenna Select   HF/6m/FM Broadcast/Airband/2m/70cm
            // Bit 6: Unknown
            // Bit 7 indicates split mode
            *data_p = is_splitmode() ? 0x80:00;
            retval = true;
            break;
        }
        }
    }
    return retval;
}


// WSJT-X: Wants to read from 0x64, no write
static const ft817_eeprom_emul_t ft817_eeprom[] =
{
        { .type = FT817EE_DATA,     .start = 0,     .end = 3,       .content.value = 0      }, // checksum
        { .type = FT817EE_DATA,     .start = 4,     .end = 4,       .content.value = 0xD8   }, // radio config
        { .type = FT817EE_DATA,     .start = 5,     .end = 5,       .content.value = 0xBF   }, // radio config
        { .type = FT817EE_FUNC,     .start = 0x55,  .end = 0x55,    .content.funcPtr = CatDriver_Ft817_EEPROM_RW_Func }, // VOX Delay and other stuff, fixed value, required by WSJT-X
        { .type = FT817EE_DATA,     .start = 0x64,  .end = 0x64,    .content.value = 0x00 }, // VOX Delay and other stuff, fixed value, required by WSJT-X
        { .type = FT817EE_FUNC,     .start = 0x65,  .end = 0x65,    .content.funcPtr = CatDriver_Ft817_EEPROM_RW_Func }, // APO and Dig Mode setting, fixed value, required by WSJT-X
        { .type = FT817EE_FUNC,     .start = 0x79,  .end = 0x79,    .content.funcPtr = CatDriver_Ft817_EEPROM_RW_Func }, // VOX Delay and other stuff, fixed value, required by WSJT-X
        { .type = FT817EE_FUNC,     .start = 0x7A,  .end = 0x7A,    .content.funcPtr = CatDriver_Ft817_EEPROM_RW_Func }, // VOX Delay and other stuff, fixed value, required by WSJT-X
        { .type = FT817EE_DATA,     .start = 0x7B,  .end = 0x7B,    .content.value = 0x00 }, // Chg related, not supported

        { .type = FT817EE_STOP }
};

static bool CatDriver_Ft817_EEPROM_Write(uint16_t addr,uint8_t* data_p)
{
    bool retval = false;

    for(int idx = 0; ft817_eeprom[idx].type != FT817EE_STOP && addr >= ft817_eeprom[idx].start;idx++)
    {
        if (ft817_eeprom[idx].start <= addr && addr <= ft817_eeprom[idx].end)
        {
            switch(ft817_eeprom[idx].type)
            {
            case FT817EE_FUNC:
                retval = (*ft817_eeprom[idx].content.funcPtr)(false, addr,data_p);
                break;
            default:
                break;
            }
            break;
        }
    }

    return retval;
}

static bool CatDriver_Ft817_EEPROM_Read(uint16_t addr,uint8_t* data_p)
{
    bool retval = false;

    for(int idx = 0; ft817_eeprom[idx].type != FT817EE_STOP && addr >= ft817_eeprom[idx].start;idx++)
    {
        if (ft817_eeprom[idx].start <= addr && addr <= ft817_eeprom[idx].end)
        {
            switch(ft817_eeprom[idx].type)
            {
            case FT817EE_DATA:
                *data_p = ft817_eeprom[idx].content.value;
                retval = true;
                break;
            case FT817EE_FUNC:
                retval = (*ft817_eeprom[idx].content.funcPtr)(true, addr,data_p);
                break;
            default:
                break;
            }

            break;
        }
    }

    return retval;
}


typedef enum
{
    FT817_SET_FREQ      = 0x01,
    FT817_GET_FREQ      = 0x03,
    FT817_SPLIT_ON      = 0x02,
    FT817_SPLIT_OFF     = 0x82,
    FT817_PTT_ON        = 0x08,
    FT817_PTT_OFF       = 0x88,
    FT817_MODE_SET      = 0x07,
    FT817_PWR_ON        = 0x0f,
    FT817_TOGGLE_VFO    = 0x81,
    FT817_A7            = 0xa7,
    FT817_EEPROM_READ   = 0xbb,
    FT817_EEPROM_WRITE  = 0xbc,
    FT817_READ_TX_STATE = 0xbd,
    FT817_READ_RX_STATE = 0xe7,
    FT817_PTT_STATE     = 0xf7,
    FT817_NOOP          = 0xff,

    UHSDR_ID            = 0x42, // this command is not known to the FT817 so we can use this to identify a UHSDR
} Ft817_CatCmd_t;

struct FT817 ft817;


uint8_t CatDriver_Clone_Checksum(uint8_t* buf, size_t len)
{
    uint8_t retval = 0;
    for (int idx = 0; idx < len; idx++)
    {
        retval += buf[idx];
    }
    return retval;
}

void CatDriver_BlockPrepare(uint8_t num, uint8_t idx, uint8_t rpt, uint8_t* buf, size_t maxlen)
{
    buf[0] = num;
    if (num == 7)
    {
        buf[2] = 0x1; // Memory[0] visible;
        buf[2+26] = 0x1; // Memory[0] filled;
    }
    if (num == 8)
    {
        ft817_memory_t* mem = (ft817_memory_t*)&buf[1];
        mem->freq=__builtin_bswap32(700100); // need to convert big / little endian!!
        mem->tag_on_off = 1;
        mem->freq_range = 0;
        mem->am_step = 1;
        mem->ssb_step = 1;
        mem->tone = 0x08;
        memcpy(&mem->name[0],"A1234567",8);
    }
    buf[cloneblock_len[idx ].len+1] = CatDriver_Clone_Checksum(&buf[1],cloneblock_len[idx].len);
}

bool CatDriver_BlockRecv(uint8_t num, uint8_t idx, uint8_t rpt, uint8_t* buf, size_t len)
{
    bool retval = false;

    if (buf[0] == num  && (buf[len - 1]  == CatDriver_Clone_Checksum(&buf[1],len-2)))
    {
        if (num == 8)
        {
            // we simply set the dial frequency here just for the show!
            ft817_memory_t* mem = (ft817_memory_t*)&buf[1];
            df.tune_new = __builtin_bswap32(mem[0].freq) * 10;
        }
        retval = true;
    }

    return retval;
}

#define CLONE_CMD_ACK (0x06)

void CatDriver_CloneSendAck()
{
    uint8_t cmd_ack = CLONE_CMD_ACK;
    CatDriver_InterfaceBufferPutData(&cmd_ack,1);
}

void CatDriver_BlockSend(uint8_t* buf, size_t len)
{
    CatDriver_InterfaceBufferPutData(buf,len);
}


bool CatDriver_BlockAck()
{
    bool retval = false;

    while(CatDriver_InterfaceBufferHasData())
    {
        uint8_t c;
        CatDriver_InterfaceBufferGetData(&c,1);
        if (c == CLONE_CMD_ACK)
        {
            retval = true;
            break;
        }
    }
    return retval;
}


static void CatDriver_HandleCloneOut()
{
    static uint16_t blockIdx = 0;
    static uint16_t blockRpt = 0;
    static uint16_t blockNum = 0;
    static uint8_t buf[256];
    static uint32_t last_sysclk;

    switch (ft817.cloneout_state)
    {
    case CLONEOUT_INIT:
    {
        blockIdx = 0;
        blockRpt = 0;
        blockNum = 0;
        ft817.cloneout_state = CLONEOUT_BLOCK_SEND;
        break;
    }
    case CLONEOUT_BLOCK_SEND:
    {
        CatDriver_BlockPrepare(blockNum,blockIdx,blockRpt,buf,256);
        CatDriver_BlockSend(buf,cloneblock_len[blockIdx].len+2);
        // +2 since we added blocknum and checksum, each 8bit.
        ft817.cloneout_state = CLONEOUT_BLOCK_ACK_WAIT;
        last_sysclk = ts.sysclock + 100;
        break;
    }
    case CLONEOUT_BLOCK_ACK_WAIT:
    {
        if (CatDriver_BlockAck())
        {
            blockNum++;
            blockRpt++;
            if (blockRpt == cloneblock_len[blockIdx].count)
            {
               blockIdx++;
               blockRpt = 0;
            }

            if (blockIdx == 11)
            {
                ft817.cloneout_state = CLONEOUT_DONE;
            }
            else
            {
                ft817.cloneout_state = CLONEOUT_BLOCK_SEND;
            }
        }
        else
        {
            if (last_sysclk < ts.sysclock)
            {
                // after a while we will give up
                ft817.cloneout_state = CLONEOUT_BLOCK_ACK_NACK;
            }
        }
        break;
    }
    case CLONEOUT_BLOCK_ACK_NACK:
    {
        ft817.cloneout_state = CLONEOUT_DONE;
        break;
    }
    case CLONEOUT_DONE:
    {
        // go back to normal CAT MODE and prepare for next round
        ft817.cloneout_state = CLONEOUT_INIT;
        ft817.state = CAT_INIT;
        break;
    }
    }

}

static void CatDriver_HandleCloneIn()
{
    static uint16_t blockIdx = 0;
    static uint16_t blockRpt = 0;
    static uint16_t blockNum = 0;
    static uint8_t buf[256];
    static uint8_t blockWant = 0;
    static uint32_t last_sysclk;

    switch (ft817.clonein_state)
    {
    case CLONEIN_INIT:
    {
        blockIdx = 0;
        blockRpt = 0;
        blockNum = 0;
        ft817.clonein_state = CLONEIN_BLOCK_RECV_START;
        break;
    }
    case CLONEIN_BLOCK_RECV_START:
    {
        last_sysclk = ts.sysclock + 600; // that is 6s, enough to start the CAT clone transmit on the PC
        blockWant = cloneblock_len[blockIdx].len + 2; // two more for blocknum and checksum
        ft817.clonein_state = CLONEIN_BLOCK_RECV;
        break;
    }
    case CLONEIN_BLOCK_RECV:
    {
        // we can ask for the full amount since our buffer will be able to hold all of the packets contents
        if (CatDriver_InterfaceBufferGetData(buf,blockWant))
        {
            // analyse block
            if (CatDriver_BlockRecv(blockNum,blockIdx,blockRpt,buf,blockWant))
            {
                // now continue
                CatDriver_CloneSendAck();
                blockNum++;
                blockRpt++;
                if (blockRpt == cloneblock_len[blockIdx].count)
                {
                    blockIdx++;
                    blockRpt = 0;
                }
                if (blockIdx == 11)
                {
                    // we are done receiving, so now lets do the final data processing
                    ft817.clonein_state = CLONEIN_FINAL_PROCESSING;
                }
                else
                {
                    ft817.clonein_state = CLONEIN_BLOCK_RECV_START;
                }

            }
            else
            {
                ft817.clonein_state = CLONEIN_DONE;
            }
        }
        else if (last_sysclk <= ts.sysclock)
        {
            // timeout
            ft817.clonein_state = CLONEIN_DONE;
        }
        break;
    }
    case CLONEIN_FINAL_PROCESSING:
    {
        // TODO: Now that all infos have been received, do the processing of it
        // TBW

        // once done, get back to normal CAT operation
        ft817.clonein_state = CLONEIN_DONE;
        break;
    }
    case CLONEIN_DONE:
    {
        // go back to normal CAT MODE and prepare for next round
        ft817.clonein_state = CLONEIN_INIT;
        ft817.state = CAT_INIT;
        break;
    }
    }

}


bool CatDriver_CloneOutStart()
{
    bool retval = false;
    if (ft817.state == CAT_CAT)
    {
        retval = true;
        ft817.state = CAT_CLONEOUT;
        ft817.cloneout_state = CLONEOUT_INIT;
    }
    return retval;
}

bool CatDriver_CloneInStart()
{
    bool retval = false;
    if (ft817.state == CAT_CAT)
    {
        retval = true;
        ft817.state = CAT_CLONEIN;
        ft817.clonein_state = CLONEIN_INIT;
    }
    return retval;
}


static void CatDriver_HandleCommands()
{
    uint8_t bc = 0;
    uint8_t resp[32];

    cat_driver_sync_data();

    while (CatDriver_InterfaceBufferGetData(ft817.req,5))
    {
#ifdef DEBUG_FT817
        int debug_idx;
        for (debug_idx = 0; debug_idx < 5 && ft817.cmd_cntr < FT817_MAX_CMD; debug_idx++ )
        {
            ft817.reqs[ft817.cmd_cntr*5+debug_idx] = ft817.req[debug_idx];
        }
        ft817.cmd_cntr++;
#endif

        switch((Ft817_CatCmd_t)ft817.req[4])
        {
        case FT817_SET_FREQ:
        {
            ulong f = 0;
            ulong fdelta;

            if(ts.xlat == 0)
            {
                fdelta = (ts.tx_audio_source == TX_AUDIO_DIGIQ)?AudioDriver_GetTranslateFreq():0;
                // If we are in DIGITAL IQ Output mode, use real tune frequency frequency instead
                // translated RX frequency
            }
            else
            {
                fdelta = 0;
            }

            int fidx;
            for (fidx = 0; fidx < 4; fidx++)
            {
                f *= 100;
                f +=  (ft817.req[fidx] >> 4) * 10 + (ft817.req[fidx] & 0x0f);
            }
            f *= 10;
            df.tune_new = f - fdelta;

            resp[0] = 0;
            bc = 1;
            if(ts.flags1 & FLAGS1_CAT_IN_SANDBOX)           // if running in sandbox store active band
            {
                ts.cat_band_index = ts.band->band_mode;
            }
        }
        break;

        case FT817_GET_FREQ:
        {
            ulong fdelta;

            if(ts.xlat == 0)
            {
                fdelta = (ts.tx_audio_source == TX_AUDIO_DIGIQ)?AudioDriver_GetTranslateFreq():0;
                // If we are in DIGITAL IQ Output mode, send real tune frequency frequency instead
                // translated RX frequency
            }
            else
            {
                fdelta = 0;
            }

            ulong f = (df.tune_new + fdelta  + 5)/ 10;
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
        switch(ts.dmod_mode)
        {
        case DEMOD_LSB:
            resp[4] = 0;
            break;
        case DEMOD_USB:
            resp[4] = 1;
            break;
        case DEMOD_CW:
            resp[4] = 2 + (ts.cw_lsb==true?1:0);
            break;
            // return 3 if CW in LSB aka CW-R
        case DEMOD_SAM:
        case DEMOD_AM:
            resp[4] = 4;
            break;
        case DEMOD_FM:
            resp[4] = 8;
            break;
        default:
            resp[4] = 1;
        }
        bc = 5;
        break;
        case FT817_MODE_SET:
        {
            resp[0] = 0; // ACK
            bc = 1;

            uint8_t new_mode = ts.dmod_mode;
            bool new_cwlsb = ts.cw_lsb;
            uint32_t new_fmdev5khz = RadioManagement_FmDevIs5khz();
            switch (ft817.req[0])
            {
            case 0: // LSB
                new_mode = DEMOD_LSB;
                break;
            case 1: // USB
                new_mode = DEMOD_USB;
                break;
            case 2: // CW
                new_cwlsb = false;
                new_mode = DEMOD_CW;
                break;
            case 3: // CW-R
                new_cwlsb = true;
                new_mode = DEMOD_CW;
                break;
            case 4: // AM
                new_mode = DEMOD_AM;
                break;
            case 8: // FM
                new_fmdev5khz = true;
                new_mode = DEMOD_FM;
                break;
            case 0x88: // FM-N
                new_fmdev5khz = false;
                new_mode = DEMOD_FM;
                break;
            case 0x0a: // DIG - SSB, side band controlled by some menu configuration in ft817, we use USB here
                new_mode = DEMOD_USB;
                break;
            case 0x0c: // PKT - FM, 9k6
                new_mode = DEMOD_FM;
                break;
            default:
                resp[0] = 0xFF; // NACK
            }
            if  (new_mode != ts.dmod_mode || new_cwlsb != ts.cw_lsb || new_fmdev5khz != RadioManagement_FmDevIs5khz() )
            {
                if(ts.flags1 & FLAGS1_CAT_IN_SANDBOX)           // if running in sandbox store active band
                {
                    ts.cat_band_index = ts.band->band_mode;
                }
                RadioManagement_FmDevSet5khz(new_fmdev5khz);
                ts.cw_lsb = new_cwlsb;
                RadioManagement_SetDemodMode(new_mode);
                UiDriver_UpdateDisplayAfterParamChange();
            }
        }
        break;
        case FT817_PTT_ON:
            resp[0] = cat_driver.cat_ptt_active?0xF0:0x00;
            /* 0xF0 if PTT was already on */

            CatDriver_CatEnableTX(true);

            bc = 1;
            break;
        case FT817_PWR_ON:
            resp[0] = 0;
            bc = 1;
            break;
        case FT817_TOGGLE_VFO:
            UiAction_ToggleVfoAB();
            resp[0] = 0;
            bc = 1;
            break;
        case FT817_SPLIT_ON:
            UiDriver_SetSplitMode(true);
            resp[0] = 0;
            bc = 1;
            break;
        case FT817_SPLIT_OFF:
            UiDriver_SetSplitMode(false);
            resp[0] = 0;
            bc = 1;
            break;
        case FT817_PTT_OFF:
            resp[0] = cat_driver.cat_ptt_active?0x00:0xF0; /* 0xF0 if PTT was already off */
            bc = 1;
            CatDriver_CatEnableTX(false);
            break;
        case FT817_A7: /* A7 */
            resp[0]=0xA7;
            resp[1]=0x02;
            resp[2]=0x00;
            resp[3]=0x04;
            resp[4]=0x67;
            resp[5]=0xD8;
            resp[6]=0xBF;
            resp[7]=0xD8;
            resp[8]=0xBF;
            bc = 9;
            break;
        case FT817_EEPROM_READ:
            // with a full simulation, we would return from 0 to 0x1925 two bytes: data@addr + data@(addr+1)
            // and above 0x1925 only 1 byte. So we call our function and see what it returns.
        {
            resp[0] = 0;
            resp[1] = 0;
            uint16_t ee_addr = (ft817.req[0] << 8) | ft817.req[1];
            if (CatDriver_Ft817_EEPROM_Read(ee_addr,&resp[0]) && ee_addr < 0x1925)
            {
                // please note: in case of second addr being invalid
                // we still return success and two bytes, second data byte is 0 in this case
                CatDriver_Ft817_EEPROM_Read(ee_addr+1,&resp[1]);
                bc = 2;
            }
            // we now check if we are supposed to return config values
            else if (ee_addr > 0x7FFF)
            {
                ConfigStorage_ReadVariable(ee_addr & 0x7FFF, (uint16_t*)&resp[0]);
                bc = 2;
            }
            else
            {
                bc = 1;
            }
            break;
        }
        case FT817_EEPROM_WRITE:
        {
            // no op in most cases
            resp[0] = 0;
            bc = 1;
            uint16_t ee_addr = (ft817.req[0] << 8) | ft817.req[1];
            if (ee_addr < 0x1925)
            {
                CatDriver_Ft817_EEPROM_Write(ee_addr,&ft817.req[2]);
                CatDriver_Ft817_EEPROM_Write(ee_addr+1,&ft817.req[3]);
            }
            else if (ee_addr > 0x7FFF)
            {
                uint16_t ee_value16 = (ft817.req[3] << 8) | ft817.req[2];
                ConfigStorage_WriteVariable(ee_addr & 0x7FFF, ee_value16);
            }

            break;
        }
        case FT817_READ_TX_STATE:
            if(RadioManagement_IsTxDisabled()||(ts.txrx_mode != TRX_MODE_TX))
            {
                resp[0] = 0;
                bc = 1;
            }
            else
            {
                // PWR / SWR
                resp[0] =(limit_4bits(roundf(swrm.fwd_pwr))<<4)+limit_4bits(roundf(swrm.vswr_dampened));
                // ALC / MOD
                resp[1] = (limit_4bits(0)<<4)+limit_4bits(0);
                bc = 2;
            }

            break;
        case FT817_READ_RX_STATE: /* E7 */
            resp[0] = (uint8_t)roundf(sm.s_count*0.5);   //S-Meter signal
            bc = 1;
            break;
        case FT817_PTT_STATE:
        {
            uint8_t tx_state = limit_4bits(roundf(swrm.fwd_pwr));
            tx_state |= is_splitmode() ? 0x20 : 0x00;
            tx_state |= ts.txrx_mode == TRX_MODE_TX ? 0x00 : 0x80;
            tx_state |= swrm.vswr_dampened > 3.0 ? 0x40 : 0x00;

            resp[0]= ts.txrx_mode == TRX_MODE_TX ? tx_state : 0x80;
            if(RadioManagement_IsTxDisabled())
            {
                resp[0] = 0xFF;
            }
            bc = 1;
            break;
        }
        case FT817_NOOP: /* FF sent out by HRD */
            break;
            // default:
            // while (1);

        case UHSDR_ID: /* used to identify the UHSDR EXTEND CAT SUPPORT */
            resp[0] = 'U';
            resp[1] = 'H';
            resp[2] = 'S';
            resp[3] = 'D';
            resp[4] = 'R';
            bc = 5;
            break;
            // default:
            // while (1);

        }
        CatDriver_InterfaceBufferPutData(resp,bc);
        /* Return data back */
    }
}

void CatDriver_HandleProtocol()
{

    if (CatDriver_GetInterfaceState() == CAT_DISCONNECTED)
    {
        if (ft817.state != CAT_INIT)
        {
            cat_buffer_reset();
            ft817.state = CAT_INIT;
        }
    }
    else
    {
        switch(ft817.state)
        {
        case CAT_CLONEOUT:
            CatDriver_HandleCloneOut();
            break;
        case CAT_CLONEIN:
            CatDriver_HandleCloneIn();
            break;
        case CAT_INIT:
            ft817.cloneout_state = CLONEOUT_INIT;
            ft817.clonein_state = CLONEIN_INIT;
            ft817.state = CAT_CAT;
            /* fall through */  // this is for the compiler, the following comment is for Eclipse
            /* no break */
        case CAT_CAT:
            CatDriver_HandleCommands();
            break;
        }
    }

#if 1
    // TODO: factor out
    // handle emulated serial PTT Pin Handling
    if (CatDriver_PTTKeyChangedState())
    {
        if (CatDriver_PTTKeyPressed())
        {
            if (CatDriver_CatPttActive() == false)
            {
                CatDriver_CatEnableTX(true);
            }
        }
        else
        {
            if (CatDriver_CatPttActive() == true)
            {
                CatDriver_CatEnableTX(false);
            }
        }
    }
#endif
}
