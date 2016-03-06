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


FilterDescriptor FilterInfo[AUDIO_FILTER_NUM] =
{
    // ID, NAME, MAX_CONFIG, , DEFAULT_CONFIG, CONFIG_LABELS
    { AUDIO_300HZ, "300Hz", 300, 10, 6, filter_list_300Hz},
    { AUDIO_500HZ, "500Hz", 500, 6, 3, filter_list_500Hz},
    {  AUDIO_1P4KHZ, "1.4k",  1400, 3, 2, filter_stdLabelsLpfBpf},
    {  AUDIO_1P6KHZ, "1.6k",  1600, 3, 2, filter_stdLabelsLpfBpf},
    {  AUDIO_1P8KHZ, "1.8k",  1800, 7, 6, filter_list_1P8KHz},
    {  AUDIO_2P1KHZ, "2.1k",  2100, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P3KHZ, "2.3k",  2300, 6, 2, filter_list_2P3KHz },
    {  AUDIO_2P5KHZ, "2.5k",  2500, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P7KHZ, "2.7k",  2700, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_2P9KHZ, "2.9k",  2900, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P2KHZ, "3.2k",  3200, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P4KHZ, "3.4k",  3400, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P6KHZ, "3.6k",  3600, 3, 2, filter_stdLabelsLpfBpf },
    {  AUDIO_3P8KHZ, "3.8k",  3800, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P0KHZ, "4.0k",  4000, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P2KHZ, "4.2k",  4200, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P4KHZ, "4.4k",  4400, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P6KHZ, "4.6k",  4600, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_4P8KHZ, "4.8k",  4800, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_5P0KHZ, "5.0k",  5000, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_5P5KHZ, "5.5k",  5500, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_6P0KHZ, "6.0k",  6000, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_6P5KHZ, "6.5k",  6500, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_7P0KHZ, "7.0k",  7000, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_7P5KHZ, "7.5k",  7500, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_8P0KHZ, "8.0k",  8000, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_8P5KHZ, "8.5k",  8500, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_9P0KHZ, "9.0k",  9000, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_9P5KHZ, "9.5k",  9500, 2, 1, filter_stdLabelsOnOff },
    {  AUDIO_10P0KHZ, "10k", 10000 ,2, 1, filter_stdLabelsOnOff }
};
