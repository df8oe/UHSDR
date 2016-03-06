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
**  Licence:        CC BY-NC-SA 3.0                                                **
************************************************************************************/
#include "mchf_board.h"

/*
typedef struct FilterDescriptor_s {
  uint8_t id;
  const char*  name;
  uint8_t configs_num;
  uint8_t config_default;
  const char** config_labels;
} FilterDescriptor;
*/
static const FilterConfig filter_stdLabelsLpfBpf[] =
{
    {"OFF", 0},
    {"LPF", 0 },
    {"BPF",0}
};

static const FilterConfig filter_stdLabelsOnOff[] =
{
    { "OFF", 0},
    { "ON",0}
};


static const FilterConfig filter_list_300Hz[] =      {
    { "  OFF", 0 },
    { "500Hz", 500 },
    { "550Hz", 550 },
    { "600Hz", 600 },
    { "650Hz", 650 },
    { "700Hz",700},
    { "750Hz",750},
    {"800Hz",800},
    {"850Hz",850},
    {"900Hz",900}
} ;

static const FilterConfig filter_list_500Hz[] =      {
    { "  OFF", 0},
    {"550Hz", 550},
    {"650Hz", 650},
    {"750Hz", 750},
    {"850Hz",850},
    {"950Hz",950},
} ;

static const FilterConfig filter_list_1P8KHz[] =      {
    {"   OFF",0},
    {"1125Hz",1125},
    {"1275Hz",1275},
    {"1427Hz",1427},
    {"1575Hz",1575},
    {"1725Hz",1725},
    {"   LPF",900},
} ;

static const FilterConfig filter_list_2P3KHz[] =      {
    {"   OFF",0},
    {"1262Hz",1262},
    {"1412Hz",1412},
    {"1562Hz",1562},
    {"1712Hz",1712},
    {"   LPF",1150},
} ;

#define FILTER_ALL (FILTER_CW|FILTER_SSB|FILTER_AM|FILTER_FM)
#define FILTER_NOFM (FILTER_CW|FILTER_SSB|FILTER_AM)
#define FILTER_SSBAM (FILTER_SSB|FILTER_AM)
#define FILTER_NONE (0)
#define FILTER_SSBAMFM (FILTER_SSB|FILTER_AM|FILTER_FM)

FilterDescriptor FilterInfo[AUDIO_FILTER_NUM] =
{
    // ID, NAME, MAX_CONFIG, VALID MODES, ALWAYS IN MODE , DEFAULT_CONFIG, CONFIG_LABELS
    {  AUDIO_300HZ, "300Hz",   300, FILTER_NOFM,    FILTER_CW,  10, 6, filter_list_300Hz},
    {  AUDIO_500HZ, "500Hz",   500, FILTER_NOFM,    FILTER_CW,   6, 3, filter_list_500Hz},
    {  AUDIO_1P4KHZ, "1.4k",  1400, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf},
    {  AUDIO_1P6KHZ, "1.6k",  1600, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf},
    {  AUDIO_1P8KHZ, "1.8k",  1800, FILTER_SSBAM,   FILTER_SSB,  7, 6, filter_list_1P8KHz},
    {  AUDIO_2P1KHZ, "2.1k",  2100, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P3KHZ, "2.3k",  2300, FILTER_SSBAM,   FILTER_SSB,  6, 2, filter_list_2P3KHz },
    {  AUDIO_2P5KHZ, "2.5k",  2500, FILTER_SSBAMFM, FILTER_FM,   3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P7KHZ, "2.7k",  2700, FILTER_NOFM,    FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P9KHZ, "2.9k",  2900, FILTER_SSBAM,   FILTER_AM,   3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P2KHZ, "3.2k",  3200, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P4KHZ, "3.4k",  3400, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P6KHZ, "3.6k",  3600, FILTER_SSBAM,   FILTER_NONE, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P8KHZ, "3.8k",  3800, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P0KHZ, "4.0k",  4000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P2KHZ, "4.2k",  4200, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P4KHZ, "4.4k",  4400, FILTER_NOFM,    FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P6KHZ, "4.6k",  4600, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P8KHZ, "4.8k",  4800, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_5P0KHZ, "5.0k",  5000, FILTER_SSBAMFM, FILTER_FM,   2, 1, filter_stdLabelsOnOff },
    {  AUDIO_5P5KHZ, "5.5k",  5500, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_6P0KHZ, "6.0k",  6000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_6P5KHZ, "6.5k",  6500, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_7P0KHZ, "7.0k",  7000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_7P5KHZ, "7.5k",  7500, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_8P0KHZ, "8.0k",  8000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_8P5KHZ, "8.5k",  8500, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_9P0KHZ, "9.0k",  9000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_9P5KHZ, "9.5k",  9500, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_10P0KHZ, "10k", 10000, FILTER_SSBAM,   FILTER_NONE, 2, 1, filter_stdLabelsOnOff }
};


/*
 * @brief Find Next Applicable Filter based on the information in the filter data structure
 *
 * Takes into account the current mode (SSB, CW, ..., some special rules, etc,). It will wrap around and always return a
 * valid filter id. In case not applicable filter was found, it returns the currently selected filter_id
 *
 * @returns next applicable filter id
 */
uint8_t AudioFilter_NextApplicableFilter()
{

  bool  voice_mode;
  uint8_t retval = ts.filter_id;
  // by default we do not change the filter selection

  uint16_t myMode;
  switch(ts.dmod_mode) {
  case DEMOD_AM:
    myMode = FILTER_AM;
    break;
  case DEMOD_FM:
    myMode = FILTER_FM;
    break;
  case DEMOD_CW:
    myMode = FILTER_CW;
    break;
  // case DEMOD_LSB:
  // case DEMOD_USB:
  // case DEMOD_DIGI:
  default:
    myMode = FILTER_SSB;
  }

  if(ts.dmod_mode != DEMOD_FM) {        // bail out if FM as filters are selected in configuration menu
    int idx;

    //
    // Scan through filters to determine if the selected filter is disabled - and skip if it is.
    // NOTE:  The 2.3 kHz filter CANNOT be disabled
    //
    // This also handles filters that are disabled according to mode (e.g. CW filters in SSB mode, SSB filters in CW mode)
    //

    if((ts.dmod_mode == DEMOD_USB) || (ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_AM))    // check to see if we are set to a "voice" mode
    {
      voice_mode = 1;
    } else {                    // not in voice mode
      voice_mode = 0;
    }

    // we run through all audio filters, starting with the next following, making sure to wrap around
    // we leave this loop once we found a filter that is applicable using "break"
    // or skip to next filter to check using "continue"
    for (idx = (ts.filter_id+1)%AUDIO_FILTER_NUM; idx != ts.filter_id; idx = (idx+1)%AUDIO_FILTER_NUM)
    {

      // these rules handle special cases
      if((idx == AUDIO_1P8KHZ) && ((ts.filter_cw_wide_disable) && (ts.dmod_mode == DEMOD_CW))) { idx = AUDIO_300HZ; break; }
      // in this case, next applicable mode is 300 Hz, so selected and leave loop

      // we have to check if this mode is IN the list of modes always offered in this mode, regardless of enablement
      if ((FilterInfo[idx].always_on_modes & myMode) != 0) {
              // okay, applicable, leave loop
              break;
      }

      // now the rules for excluding a mode follow, in this case we call continue to go to next filter

      // not enable; next please
      if (!ts.filter_select[idx]) { continue; }

      if((idx == AUDIO_300HZ || idx == AUDIO_500HZ) && ((ts.filter_ssb_narrow_disable) && (voice_mode))) { continue; }
      // jump over 300 Hz / 500 Hz if ssb_narrow_disable and voice mode

      // last we have to check if this mode is NOT in the list of allowed modes for the filter
      if ((FilterInfo[idx].allowed_modes & myMode) == 0) {
        // okay, not applicable, next please
        continue;
      }

      // if we have arrived here, all is good, we can  use the index, so lets bail out here
      break;
    }
    retval = idx;
  }
  return retval;
}
