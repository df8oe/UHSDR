/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  Licence:       GNU GPLv3                                                       **
 ************************************************************************************/
#include <assert.h>

#include "uhsdr_board_config.h"
#include "uhsdr_keypad.h"
#include "gpio.h"

// Key map structure
// represents a physical key which can be pressed (via GPIO)
typedef struct
{
    GPIO_TypeDef*   keyPort;
    uint16_t        keyPin;
    uint16_t        button_id;
    const char*     label;

} Keypad_KeyPhys_t;

// represents a logical button
typedef struct
{
    uint16_t        button_id;
    const char*     label;
} UhsdrButtonLogical_t;

typedef struct
{
    const Keypad_KeyPhys_t* map;
    uint32_t num;
} UhsdrHwKey_t;

const Keypad_KeyPhys_t* bm_set_normal;
#ifdef UI_BRD_MCHF
const Keypad_KeyPhys_t* bm_set_rtc;
#endif

// -------------------------------------------------------
// Constant declaration of the buttons map across ports
// - update if moving buttons around !!!
// the order in this array is not relevant for functional aspects
const Keypad_KeyPhys_t bm_set_normal_arr[] =
{
        {BUTTON_M2_PIO,     BUTTON_M2,      BUTTON_M2_PRESSED,      "S3"},
        {BUTTON_G3_PIO,     BUTTON_G3,      BUTTON_G3_PRESSED,      "S2"},
        {BUTTON_G2_PIO,     BUTTON_G2,      BUTTON_G2_PRESSED,      "S1"},
        {BUTTON_BNDM_PIO,   BUTTON_BNDM,    BUTTON_BNDM_PRESSED,    "S4"},
        {BUTTON_G4_PIO,     BUTTON_G4,      BUTTON_G4_PRESSED,      "S5"},
        {BUTTON_M3_PIO,     BUTTON_M3,      BUTTON_M3_PRESSED,      "S6"},
        {BUTTON_STEPM_PIO,  BUTTON_STEPM,   BUTTON_STEPM_PRESSED,   "S7"},
        {BUTTON_STEPP_PIO,  BUTTON_STEPP,   BUTTON_STEPP_PRESSED,   "S8"},
        {BUTTON_M1_PIO,     BUTTON_M1,      BUTTON_M1_PRESSED,      "S9"},
        {BUTTON_F3_PIO,     BUTTON_F3,      BUTTON_F3_PRESSED,      "S10"},
        {BUTTON_F1_PIO,     BUTTON_F1,      BUTTON_F1_PRESSED,      "S11"},
        {BUTTON_F2_PIO,     BUTTON_F2,      BUTTON_F2_PRESSED,      "S12"},
        {BUTTON_F4_PIO,     BUTTON_F4,      BUTTON_F4_PRESSED,      "S13"},
        {BUTTON_BNDP_PIO,   BUTTON_BNDP,    BUTTON_BNDP_PRESSED,    "S14"},
        {BUTTON_F5_PIO,     BUTTON_F5,      BUTTON_F5_PRESSED,      "S15"},
        {BUTTON_G1_PIO,     BUTTON_G1,      BUTTON_G1_PRESSED,      "S16"},
        {BUTTON_PWR_PIO,    BUTTON_PWR,     BUTTON_PWR_PRESSED,     "S17"},
        {TP_IRQ_PIO,        TP_IRQ,         TOUCHSCREEN_ACTIVE,     "TPIRQ"},
#ifdef UI_BRD_OVI40
        {BUTTON_E1_PIO,     BUTTON_E1,      BUTTON_E1_PRESSED,      "E1"},
        {BUTTON_E2_PIO,     BUTTON_E2,      BUTTON_E2_PRESSED,      "E2"},
        {BUTTON_E3_PIO,     BUTTON_E3,      BUTTON_E3_PRESSED,      "E3"},
        {BUTTON_E4_PIO,     BUTTON_E4,      BUTTON_E4_PRESSED,      "E4"},
        {BUTTON_S18_PIO,    BUTTON_S18,     BUTTON_L1_PRESSED,      "S18"},
        {BUTTON_S19_PIO,    BUTTON_S19,     BUTTON_F6_PRESSED,      "S19"},
#endif
        // this must be the last entry
        {NULL,              0,              0,                      NULL}
};

const Keypad_KeyPhys_t* bm_set_normal = &bm_set_normal_arr[0];

#ifdef UI_BRD_MCHF
const Keypad_KeyPhys_t bm_set_rtc_arr[] =
{
        // alternative mapping for RTC Modification
        {BUTTON_M2_PIO,         BUTTON_M2,      BUTTON_M2_PRESSED,      "S3"},
        {BUTTON_G3_PIO,         BUTTON_G3,      BUTTON_G3_PRESSED,      "S2"},
        {BUTTON_G2_PIO,         BUTTON_G2,      BUTTON_G2_PRESSED,      "S1"},
        {BUTTON_BNDM_PIO,       BUTTON_BNDM,    BUTTON_BNDM_PRESSED,    "S4"},
        {BUTTON_G4_PIO,         BUTTON_G4,      BUTTON_G4_PRESSED,      "S5"},
        {BUTTON_M3_PIO,         BUTTON_M3,      BUTTON_M3_PRESSED,      "S6"},
        {BUTTON_STEPM_PIO,      BUTTON_STEPM,   BUTTON_STEPM_PRESSED,   "S7"},
        {BUTTON_STEPP_PIO,      BUTTON_STEPP,   BUTTON_STEPP_PRESSED,   "S8"},
        {BUTTON_M1_PIO_RTC,     BUTTON_M1_RTC,  BUTTON_M1_PRESSED,      "S9"},
        {BUTTON_F3_PIO_RTC,     BUTTON_F3_RTC,  BUTTON_F3_PRESSED,      "S10"},
        {BUTTON_F1_PIO,         BUTTON_F1,      BUTTON_F1_PRESSED,      "S11"},
        {BUTTON_F2_PIO,         BUTTON_F2,      BUTTON_F2_PRESSED,      "S12"},
        {BUTTON_F4_PIO,         BUTTON_F4,      BUTTON_F4_PRESSED,      "S13"},
        {BUTTON_BNDP_PIO,       BUTTON_BNDP,    BUTTON_BNDP_PRESSED,    "S14"},
        {BUTTON_F5_PIO,         BUTTON_F5,      BUTTON_F5_PRESSED,      "S15"},
        {BUTTON_G1_PIO,         BUTTON_G1,      BUTTON_G1_PRESSED,      "S16"},
        {BUTTON_PWR_PIO,        BUTTON_PWR,     BUTTON_PWR_PRESSED,     "S17"},
        {TP_IRQ_PIO,            TP_IRQ,         TOUCHSCREEN_ACTIVE,     "TPIRQ"},

        // this must be the last entry
        {NULL,                  0,              0,                      NULL},
};

const Keypad_KeyPhys_t* bm_set_rtc = &bm_set_rtc_arr[0];
#endif

// all supported logical buttons
// the order in this list must be identical to the order
// in the enum
const UhsdrButtonLogical_t buttons[BUTTON_NUM] =
{
        {BUTTON_M2_PRESSED,     "M2"},     // 0 / S3
        {BUTTON_G3_PRESSED,     "G3"},     // 1 / S2
        {BUTTON_G2_PRESSED,     "G2"},     // 2 / S1
        {BUTTON_BNDM_PRESSED,   "Band-"},   // 3 / S4
        {BUTTON_G4_PRESSED,     "G4"},     // 4 / S5
        {BUTTON_M3_PRESSED,     "M3"},     // 5 / S6
        {BUTTON_STEPM_PRESSED,  "Step-"},  // 6 / S7
        {BUTTON_STEPP_PRESSED,  "Step+"},  // 7 / S8
        {BUTTON_M1_PRESSED,     "M1"},     // 8 / S9
        {BUTTON_F3_PRESSED,     "F3"},     // 9 / S10
        {BUTTON_F1_PRESSED,     "F1"},     // 10 / S11
        {BUTTON_F2_PRESSED,     "F2"},     // 11 / S12
        {BUTTON_F4_PRESSED,     "F4"},     // 12 / S13
        {BUTTON_BNDP_PRESSED,   "Band+"},   // 13 / S14
        {BUTTON_F5_PRESSED,     "F5"},     // 14 / S15
        {BUTTON_G1_PRESSED,     "G1"},     // 15 / S16
        {BUTTON_PWR_PRESSED,    "Power"},       // 16 / S17 Power Button
        {TOUCHSCREEN_ACTIVE,    "Touch"},                 // 17 TP "Button"
#ifdef UI_BRD_OVI40
        {BUTTON_E1_PRESSED,     "E1" },
        {BUTTON_E2_PRESSED,     "E2" },
        {BUTTON_E3_PRESSED,     "E3" },
        {BUTTON_E4_PRESSED,     "E4" },
        {BUTTON_F6_PRESSED,     "F6" },
        {BUTTON_L1_PRESSED,     "L1" },
#endif
};

// the initial button map is the default one
UhsdrHwKey_t hwKeys = { .map = &bm_set_normal_arr[0], .num = 0 };

// FIXME? Do we change state of these in IRQ? Maybe more save mark them as volatile?
uint32_t buttonStates; // logical buttons
uint32_t keyStates; // hw scan keys

#ifdef UI_BRD_MCHF
// this function invoked only on the UI_MCHF_BRD with RTC as they have different buttons layout
inline void Keypad_SetLayoutRTC_MCHF()
{
    hwKeys.map = &bm_set_rtc[0];
}
#endif

bool Keypad_IsButtonPressed(uint32_t button_num)
{
    return ((1 << button_num) & buttonStates) != 0;
}

bool Keypad_IsAnyButtonPressed()
{
    return buttonStates != 0;
}

bool Keypad_IsKeyPressed(uint32_t key_num)
{
    return ((1 << key_num) & keyStates) != 0;
}

bool Keypad_IsAnyKeyPressed()
{
    return keyStates != 0;
}
uint32_t Keypad_KeyStates()
{
    return keyStates;
}
uint32_t Keypad_ButtonStates()
{
    return buttonStates;
}

/*
 * @brief keypad hardware initialization based on the given keyMap
 *
 */
void Keypad_KeypadInit()
{
    UhsdrHwKey_t* keyMap = &hwKeys;
    GPIO_InitTypeDef GPIO_InitStructure;

    // Common init
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStructure.Pull = GPIO_PULLUP;

    // Init all hw gpio from public struct declaration
    for(keyMap->num = 0; keyMap->map[keyMap->num].keyPort != NULL; keyMap->num++)
    {
        GPIO_InitStructure.Pin = keyMap->map[keyMap->num].keyPin;
        HAL_GPIO_Init(keyMap->map[keyMap->num].keyPort, &GPIO_InitStructure);
    }
}

/*
 *  @brief Keypad direct HW access reading returning if a key is pressed (or not)
 */
static bool Keypad_GetKeyGPIOState(const Keypad_KeyPhys_t* key)
{
    return HAL_GPIO_ReadPin(key->keyPort, key->keyPin) == 0;
}

/*
 * We scan all known physical buttons and map them to the logical button states
 * has to be called before processing key presses.
 */
void Keypad_Scan()
{
    for (uint32_t key_num = 0; key_num < hwKeys.num; key_num++)
    {
        if (Keypad_GetKeyGPIOState(&hwKeys.map[key_num]))
        {
            // in normal mode - return key value
            SET_BIT( buttonStates, ( 1 << hwKeys.map[key_num].button_id ));
            SET_BIT( keyStates, ( 1 << key_num ));
        }
        else
        {
            CLEAR_BIT( buttonStates, ( 1 << hwKeys.map[key_num].button_id ));
            CLEAR_BIT( keyStates, ( 1 << key_num ));
        }
    }
}

inline const char* Keypad_GetLabelOfButton( uint32_t id_button )
{
    assert( id_button < sizeof( buttons ) / sizeof( UhsdrButtonLogical_t ));
    return buttons[ id_button ].label;
}
