#include "ui_menu.h"
#include "ui_menu_internal.h"
#include "uhsdr_hmc1023.h"
#include "radio_management.h"
#include "soft_tcxo.h"
/*
 * How to create a new menu entry in an existing menu:
 * - Copy an existing entry of MENU_KIND and paste at the desired position
 * - Just assign a unique number to the "number" attribute
 * - Change the label, and implement handling
 *
 * How to create a menu group:
 * - Add menu group id entry to enum below
 * - Create a MenuDescriptor Array with the desired entries, make sure to have the MENU_STOP element at last position
 *   and that all elements have the enum value as first attribute (menuId)
 * - Added the menu group entry in the parent menu descriptor array.
 * - Create a MenuState element
 * - Added the menu group descriptor to the groups list at the position corresponding to the enum value
 *   using the descriptor array, the address of the MenuState and the address of the parent menu descriptor array.
 *
 *
 */

// ATTENTION: The numbering here has to be match in the groups
// data structure found more or less at the end of this
// menu definition block ! Otherwise menu display will not work
// as expected and may crash mcHF
// If you move menus around, make sure to change the groups structure of the move
// menu to reflect the new parent menu!

enum MENU_GROUP_ITEM
{
    MENU_TOP  = 0,
    MENU_BASE,
    MENU_CONF,
    MENU_DISPLAY,
    MENU_CW,
    MENU_FILTER,
    MENU_POW,
    MENU_MEN2TOUCH,
    MENU_SYSINFO,
    MENU_DEBUG,
    MENU_HALL_OF_FAME,
};


const MenuDescriptor topGroup[] =
{
    { MENU_TOP, MENU_GROUP, MENU_BASE, NULL, "Standard Menu", UiMenuDesc("Operational parameters") },
    { MENU_TOP, MENU_GROUP, MENU_CONF, NULL, "Configuration Menu", UiMenuDesc("Configuration settings typically not so often to be changed") },
    { MENU_TOP, MENU_GROUP, MENU_DISPLAY, NULL, "Display Menu", UiMenuDesc("Everything related to how the display presents data such as colors or speed of display") },
    { MENU_TOP, MENU_GROUP, MENU_CW, NULL, "CW Mode Settings", UiMenuDesc("Everything related to CW Mode operation (except CW PA Bias)") },
    { MENU_TOP, MENU_GROUP, MENU_FILTER, NULL, "Filter Selection", UiMenuDesc("Select the filters for filter button by mode. Each mode can have up to 4 filters for quick selection via short press Filter button G4.") },
    { MENU_TOP, MENU_GROUP, MENU_POW, NULL, "PA Configuration", UiMenuDesc("Menu for power amplifier parameter and tune function adjustments") },
    { MENU_TOP, MENU_GROUP, MENU_MEN2TOUCH, NULL, "Touchscreen via Menu", UiMenuDesc("All functions which are available by touchscreen here are available as menu selectables") },
    { MENU_TOP, MENU_GROUP, MENU_SYSINFO, NULL, "System Info", UiMenuDesc("Lists various system info values") },
    { MENU_TOP, MENU_GROUP, MENU_DEBUG, NULL, "Debug/Exper. Settings", UiMenuDesc("As the name says, contains debug or expert settings usually not relevant for operating the mcHF") },
    { MENU_TOP, MENU_GROUP, MENU_HALL_OF_FAME, NULL, "Hall of Fame", UiMenuDesc("Thanks to all who contributed to the project") },

    { MENU_TOP, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor baseGroup[] =
{
//    { MENU_BASE, MENU_ITEM, MENU_SSB_NARROW_FILT,"029","CW Filt in SSB Mode", UiMenuDesc(":soon:") },
    { MENU_BASE, MENU_ITEM, MENU_SSB_AUTO_MODE_SELECT, NULL, "LSB/USB Auto Select", UiMenuDesc("If enabled, the appropriate sideband mode for SSB and FreeDV is chosen as default for each band by its frequency.")},
    { MENU_BASE, MENU_ITEM, MENU_DIGI_DISABLE,  NULL,"Digital Modes", UiMenuDesc("Disable appearance of digital modes when pressing Mode button")},
    { MENU_BASE, MENU_ITEM, MENU_CW_DISABLE, NULL, "CW Mode", UiMenuDesc("Disable appearance of CW mode when pressing Mode button")},
    { MENU_BASE, MENU_ITEM, MENU_AM_DISABLE, NULL, "AM Mode", UiMenuDesc("Disable appearance of AM mode when pressing Mode button")},
    { MENU_BASE, MENU_ITEM, MENU_DEMOD_SAM,  NULL,"SyncAM Mode",UiMenuDesc("Disable appearance of SyncAM modes when pressing Mode button")  },
    { MENU_BASE, MENU_ITEM, MENU_SAM_PLL_LOCKING_RANGE, NULL, "SAM PLL locking range", UiMenuDesc("SAM PLL Locking Range in Hz: this determines how far up and down from the carrier frequency of an AM station we can offtune the receiver, so that the PLL will still lock to the carrier.") },
    { MENU_BASE, MENU_ITEM, MENU_SAM_PLL_STEP_RESPONSE, NULL, "SAM PLL step response", UiMenuDesc("Step response = Zeta = damping factor of the SAM PLL. Sets the stability and transient response of the PLL. Larger values give faster lock even if you are offtune, but PLL is also more sensitive.") },
    { MENU_BASE, MENU_ITEM, MENU_SAM_PLL_BANDWIDTH, NULL, "SAM PLL bandwidth in Hz", UiMenuDesc("Bandwidth of the PLL loop = OmegaN in Hz: smaller bandwidth = more stable lock. FAST LOCK SAM PLL - set Step response and PLL bandwidth to large values [eg. 80 / 350]; DX (SLOW & STABLE) SAM PLL - set Step response and PLL bandwidth to small values [eg. 30 / 100].") },
    { MENU_BASE, MENU_ITEM, MENU_SAM_FADE_LEVELER, NULL, "SAM Fade Leveler", UiMenuDesc("Fade leveler (in AM/SAM mode) ON/OFF. Fade leveler is helpful in situations with very fast QSB of the carrier ´flutter´. It is designed to remove the rapidly changing carrier and replace it with a more stable carrier. If there is no QSB on the carrier, there is no change.") },
    { MENU_BASE, MENU_ITEM, MENU_FM_MODE_ENABLE, NULL, "FM Mode", UiMenuDesc("Disable appearance of FM mode when pressing Mode button")},
    { MENU_BASE, MENU_ITEM, MENU_FM_GEN_SUBAUDIBLE_TONE, NULL, "FM Sub Tone Gen", UiMenuDesc("Enable generation of CTCSS tones during FM transmissions.") },
    { MENU_BASE, MENU_ITEM, MENU_FM_DET_SUBAUDIBLE_TONE, NULL, "FM Sub Tone Det", UiMenuDesc("Enable detection of CTCSS tones during FM receive. RX is muted unless tone is detected.") },
    { MENU_BASE, MENU_ITEM, MENU_FM_TONE_BURST_MODE, NULL, "FM Tone Burst", UiMenuDesc("Enabled sending of short tone at beginning of each FM transmission. Used to open repeaters. Available frequencies are 1750 Hz and 2135 Hz.") },
    { MENU_BASE, MENU_ITEM, MENU_FM_DEV_MODE, NULL, "FM Deviation", UiMenuDesc("Select between normal and narrow deviation (5 and 2.5kHz) for FM RX/TX") },
//    { MENU_BASE, MENU_ITEM, MENU_RF_GAIN_ADJ, NULL, "RF Gain", UiMenuDesc("RF Receive Gain. This setting is also accessible via Encoder 2, RFG.") }, // also via knob
//    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_SWITCH, NULL, "AGC Mode Switch", UiMenuDesc("You can choose between two different AGC systems here: ´Standard AGC´ and ´WDSP AGC´.") },
//    { MENU_BASE, MENU_ITEM, MENU_AGC_MODE, NULL, "AGC STD Mode", UiMenuDesc("Standard AGC: Automatic Gain Control Mode setting. You may select preconfigured settings (SLOW,MED,FAST), define settings yourself (CUSTOM) or use MANUAL (no AGC, use RFG to control gain") },
//    { MENU_BASE, MENU_ITEM, MENU_CUSTOM_AGC, NULL, "AGC STD Custom Speed (+=Slower)", UiMenuDesc("Standard AGC:  If AGC STD Mode is set to CUSTOM, this controls the speed setting of AGC") },
    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_MODE, NULL, "AGC WDSP Mode", UiMenuDesc("Choose a bundle of preset AGC parameters for the WDSP AGC: FAST / MED / SLOW / LONG / very LONG or switch OFF the AGC.") },
    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_SLOPE, NULL, "AGC WDSP Slope", UiMenuDesc("Slope of the AGC is the difference between the loudest signal and the quietest signal after the AGC action has taken place. Given in dB.") },
    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_TAU_DECAY, NULL, "AGC WDSP Decay", UiMenuDesc("Time constant for the AGC decay (speed of recovery of the AGC gain) in milliseconds.") },
    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_THRESH, NULL, "AGC WDSP Threshold", UiMenuDesc("´Threshold´ = ´Knee´ of the AGC: input signal level from which on the AGC action takes place. AGC threshold should be placed/adjusted just above the band noise for every particular RX situation to allow for optimal AGC action. The blue AGC box indicates when AGC action takes place and helps in adjusting this threshold.") },
    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_HANG_ENABLE, NULL, "AGC WDSP Hang enable", UiMenuDesc("Enable/Disable Hang AGC function: If enabled: after the signal has decreased, the gain of the AGC is held constant for a certain time period (the hang time) in order to allow for speech pauses without disturbing noise because of fast acting AGC.") },
    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_HANG_TIME, NULL, "AGC WDSP Hang time", UiMenuDesc("Hang AGC: hang time is the time period over which the AGC gain is held constant when in AGC Hang mode. After this period the gain is increased fast.") },
    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_HANG_THRESH, NULL, "AGC WDSP Hang threshold", UiMenuDesc("´Threshold´ for the Hang AGC: Hang AGC is useful for medium to strong signals. The Hang threshold determines the signal strength a signal has to exceed for Hang AGC to take place.") },
    { MENU_BASE, MENU_ITEM, MENU_AGC_WDSP_TAU_HANG_DECAY, NULL, "AGC WDSP Hang Decay", UiMenuDesc("Time constant for the Hang AGC decay (speed of recovery of the AGC gain after hang time has expired) in milliseconds.") },
    { MENU_BASE, MENU_ITEM, MENU_CODEC_GAIN_MODE, NULL, "RX Codec Gain", UiMenuDesc("Sets the Codec IQ signal gain. Higher values represent higher gain. If set to AUTO the mcHF controls the gain so that the best dynamic range is used.") },
    { MENU_BASE, MENU_ITEM, MENU_RX_FREQ_CONV, NULL, "RX/TX Freq Xlate", UiMenuDesc("Controls offset of the receiver IQ signal base frequency from the dial frequency. Use of +/-12Khz is recommended. Switching it to OFF is not recommended as it disables certain features.") },
    { MENU_BASE, MENU_ITEM, MENU_MIC_TYPE, NULL, "Mic Type", UiMenuDesc("Microphone type. Electret or Dynamic. ELECTRET is recommended. Selecting DYNAMIC when an Electret mic is present will likely cause terrible audio distortion") },
    { MENU_BASE, MENU_ITEM, MENU_MIC_GAIN, NULL, "Mic Input Gain", UiMenuDesc("Microphone gain. Also changeable via Encoder 3 if Microphone is selected as Input") },
    { MENU_BASE, MENU_ITEM, MENU_LINE_GAIN, NULL, "Line Input Gain", UiMenuDesc("LineIn gain. Also changeable via Encoder 3 if LineIn Left (L>L) or LineIn Right (L>R) is selected as Input") },
    { MENU_BASE, MENU_ITEM, MENU_TX_COMPRESSION_LEVEL, NULL, "TX Audio Compress", UiMenuDesc("Control the TX audio compressor. Higher values give more compression. Set to CUSTOM for user defined compression parameters. See below. Also changeable via Encoder 1 (CMP).") },
    { MENU_BASE, MENU_ITEM, MENU_ALC_RELEASE, NULL, "TX ALC Release Time", UiMenuDesc("If Audio Compressor Config is set to CUSTOM, sets the value of the Audio Compressor Release time. Otherwise shows predefined value of selected compression level.") },
    { MENU_BASE, MENU_ITEM, MENU_ALC_POSTFILT_GAIN, NULL, "TX ALC Input Gain", UiMenuDesc("If Audio Compressor Config is set to CUSTOM, sets the value of the ALC Input Gain. Otherwise shows predefined value of selected compression level.") },
    { MENU_BASE, MENU_ITEM, MENU_NOISE_BLANKER_SETTING, NULL, "RX NB Setting", UiMenuDesc("Set the Noise Blanker strength. Higher values mean more agressive blanking. Also changeable using Encoder 2 if Noise Blanker is active.") },
    { MENU_BASE, MENU_ITEM, MENU_DSP_NR_STRENGTH, NULL, "DSP NR Strength", UiMenuDesc("Set the Noise Reduction Strength. Higher values mean more agressive noise reduction but also higher CPU load. Use with extreme care. Also changeable using Encoder 2 if DSP is active.") }, // via knob
    { MENU_BASE, MENU_ITEM, MENU_TCXO_MODE, NULL, "TCXO Off/On/Stop", UiMenuDesc("The software TCXO can be turned ON (set frequency is adjusted so that generated frequency matches the wanted frequency); OFF (no correction or measurement done); or STOP (no correction but measurement).") },
    { MENU_BASE, MENU_ITEM, MENU_TCXO_C_F, &lo.sensor_present, "TCXO Temp. (C/F)", UiMenuDesc("Show the measure TCXO temperature in Celsius or Fahrenheit.") },
#ifdef USE_CONFIGSTORAGE_FLASH
    { MENU_BASE, MENU_ITEM, MENU_BACKUP_CONFIG, NULL, "Backup Config", UiMenuDesc("Backup your I2C Configuration to flash. If you don't have suitable I2C EEPROM installed this function is not available.") },
#endif
    { MENU_BASE, MENU_ITEM, MENU_LOW_POWER_SHUTDOWN, NULL, "Low Voltage Shutdown", UiMenuDesc("Shutdown automatically when supply voltage is below threshold for 60 seconds (only in RX).") },
#ifdef USE_CONFIGSTORAGE_FLASH
    { MENU_BASE, MENU_ITEM, MENU_RESTORE_CONFIG, NULL, "Restore Config", UiMenuDesc("Restore your I2C Configuration from flash. If you don't have suitable I2C EEPROM installed this function is not available.") },
#endif
    { MENU_BASE, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor confGroup[] =
{

    // Unused in firmware: { MENU_CONF, MENU_ITEM, CONFIG_FREQ_LIMIT_RELAX,"231","Freq. Limit Disable", UiMenuDesc(":soon:") },
    { MENU_CONF, MENU_ITEM, CONFIG_BANDEF_SELECT, NULL, "Band Definition", UiMenuDesc("Select which band definition to use for ham bands (Original UHSDR or IARU Region 1 - 3") },
    { MENU_CONF, MENU_ITEM, CONFIG_FREQ_MEM_LIMIT_RELAX, NULL, "Save Out-Of-Band Freq.", UiMenuDesc("Select ON to save and restore frequencies which do not fit into the band during configuration saving (Power-Off or long press on Menu button)") },
    { MENU_CONF, MENU_ITEM, CONFIG_TX_OUT_ENABLE, NULL, "TX on Out-Of-Band Freq.", UiMenuDesc("Permit low power transmission even if the frequency is out of the official ham bands. DO NOT USE WITH CONNECTED ANTENNA! Use a dummy load!") },
    { MENU_CONF, MENU_ITEM, CONFIG_TX_DISABLE, NULL, "Transmit Disable", UiMenuDesc("Disable all transmissions unconditionally. In CW you will be able to hear a sidetone but no transmission is made.") },
    { MENU_CONF, MENU_ITEM, CONFIG_AUDIO_MAIN_SCREEN_MENU_SWITCH, NULL, "Menu SW on TX disable", UiMenuDesc("Control if the screen automatically adapts Encoder value focus when switching between RX and TX.") },

    { MENU_CONF, MENU_ITEM, CONFIG_MUTE_LINE_OUT_TX, NULL, "TX Mute LineOut", UiMenuDesc("During transmission with frequency translation off, line out will carry one of the two signal channels. Good for CW but not very useful otherwise. You may switch this signal off here.") },
    { MENU_CONF, MENU_ITEM, CONFIG_TXRX_SWITCH_AUDIO_MUTE, NULL, "TX Initial Muting Time", UiMenuDesc("When switching from RX to TX the audio and HF output will be muted for roughly VALUE ms. There are now several minimum times for muting defined in the firmware:<br/><br/> Input from Mic: 40ms<br/> Input from Line In: 40ms<br/> Digital Inputs (CW, USB): less than 1ms.<br/><br/> If the user defined 'TX Initial Muting Time' is set to more than zero, the maximum of both fixed input time and user defined time is used. Your microphone PTT switch is a potential source of noise if Mic is input! You need to increase the delay or change switches!") },
    { MENU_CONF, MENU_ITEM, CONFIG_MAX_VOLUME, NULL, "Max Volume", UiMenuDesc("Set maximum speaker&headphone volume.") },
//    { MENU_CONF, MENU_ITEM, CONFIG_MAX_RX_GAIN, NULL, "Max RX Gain (0=Max)", UiMenuDesc("Here you can set maximum gain for RX. A good choice is 3...5. If set to 0 RX is too sensitive in most working conditions.") },
    { MENU_CONF, MENU_ITEM, CONFIG_LINEOUT_GAIN, NULL, "Lineout Gain", UiMenuDesc("Set the constant gain level for the analog lineout jack") },

    // UI Behavior / Key Beep
    { MENU_CONF, MENU_ITEM, CONFIG_BEEP_FREQ, NULL, "Key Beep Frequency", UiMenuDesc("Set key beep frequency in Hz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_BEEP_VOLUME, NULL, "Key Beep Volume", UiMenuDesc("Set key beep volume.") },

    // USB CAT Related
    { MENU_CONF, MENU_ITEM, CONFIG_CAT_IN_SANDBOX, NULL, "CAT Running In Sandbox", UiMenuDesc("If On, frequency Changes made via CAT will not automatically switch bands and affect the manually selected frequencies.") },
    { MENU_CONF, MENU_ITEM, CONFIG_CAT_XLAT, NULL, "CAT-DIQ-FREQ-XLAT", UiMenuDesc("Select which frequency is reported via CAT Interface to the connected PC in Digital IQ Mode. If ON, it reports the displayed frequency. If OFF, it reports the center frequency, which is more useful with SDR programs.") },
    { MENU_CONF, MENU_ITEM, CONFIG_CAT_PTT_RTS, NULL, "PTT via virtual RTS", UiMenuDesc("The virtual serial port signal RTS can be used to switch to TX. Use with care, many CAT-able programs also set RTS to aktiv and make the TRX go to TX mode.") },

    // Transverter Configuration
    { MENU_CONF, MENU_ITEM, CONFIG_XVTR_OFFSET_MULT, NULL, "XVTR Offs/Mult", UiMenuDesc("When connecting to a transverter, set this to 1 and set the XVERTER Offset to the LO Frequency of it. The mcHF frequency is multiplied by this factor before the offset is added, so anything but 1 will result in each Hz in the mcHF being displayed as 2 to 10 Hz change on display.") },
    { MENU_CONF, MENU_ITEM, CONFIG_XVTR_FREQUENCY_OFFSET, NULL, "XVTR Offs. RX", UiMenuDesc("When transverter mode is enabled, this value is added to the mcHF frequency after being multiplied with the XVTR Offs/Mult. Use Step+ to set a good step width, much less turns with the dial knob if it is set to 1Mhz") },
    // { MENU_CONF, MENU_ITEM, CONFIG_XVTR_OFFSET_MULT, NULL, "XVTR Offs/Mult", UiMenuDesc("When connecting to a transverter, set this to 1 and set the XVERTER Offset to the LO Frequency of it. The mcHF frequency is multiplied by this factor before the offset is added, so anything but 1 will result in each Hz in the mcHF being displayed as 2 to 10 Hz change on display.") },
    { MENU_CONF, MENU_ITEM, CONFIG_XVTR_FREQUENCY_OFFSET_TX, NULL, "XVTR Offs. TX", UiMenuDesc("When transverter mode is enabled, this value is added to the displayed mcHF TX frequency after being multiplied with the XVTR Offs/Mult. Setting it to 0 uses RX offset for TX. Use Step+ to set a good step width, much less turns with the dial knob if it is set to 1Mhz") },

    // Button Handling Setup
    { MENU_CONF, MENU_ITEM, CONFIG_STEP_SIZE_BUTTON_SWAP, NULL, "Step Button Swap", UiMenuDesc("If ON, Step- behaves like Step+ and vice versa.") },
    { MENU_CONF, MENU_ITEM, CONFIG_BAND_BUTTON_SWAP, NULL, "Band+/- Button Swap", UiMenuDesc("If ON, Band- behaves like Band+ and vice versa.") },

    // RTC Setup and Settings
    { MENU_CONF, MENU_ITEM, CONFIG_RTC_START, &ts.vbat_present,"RTC Start", UiMenuDesc("Start using the RTC and use the modified button layout. Will reboot your mcHF. Please use only if you completed the RTC mod otherwise you will need to disconnect battery and power and reboot to get a working mcHF. This menu is only visible if Backup RAM (VBat) was detected.") },
    { MENU_CONF, MENU_ITEM, CONFIG_RTC_HOUR, &ts.rtc_present,"RTC Hour", UiMenuDesc("Sets the Real Time Clock Hour. Needs HW Modifications.") },
    { MENU_CONF, MENU_ITEM, CONFIG_RTC_MIN, &ts.rtc_present,"RTC Min", UiMenuDesc("Sets the Real Time Clock Minutes. Needs HW Modifications.") },
    { MENU_CONF, MENU_ITEM, CONFIG_RTC_SEC, &ts.rtc_present,"RTC Seconds", UiMenuDesc("Sets the Real Time Clock Seconds. Needs HW Modifications.") },
    { MENU_CONF, MENU_ITEM, CONFIG_RTC_RESET, &ts.vbat_present,"RTC Reset", UiMenuDesc("Full Reset of STM32 RTC. Can be used to simulate first start with RTC mod completed") },
    { MENU_CONF, MENU_ITEM, CONFIG_RTC_CALIB, &ts.rtc_present,"RTC Calibration", UiMenuDesc("Sets the Real Time Clock Frequency calibration value in ppm. 1s/day deviation equals 11.57 ppm deviation") },

    // mcHF Setup Calibration (Initial Setup, never to be changed unless HW changes)

    { MENU_CONF, MENU_ITEM, CONFIG_VOLTMETER_CALIBRATION, NULL, "Voltmeter Cal.", UiMenuDesc("Adjusts the displayed value of the voltmeter.") },
    { MENU_CONF, MENU_ITEM, CONFIG_LOW_POWER_THRESHOLD, NULL, "Low Voltage Threshold", UiMenuDesc("Voltage threshold for voltage warning colors and auto shutdown.") },
    { MENU_CONF, MENU_ITEM, CONFIG_FREQUENCY_CALIBRATE, NULL, "Freq. Calibrate", UiMenuDesc("Adjust the frequency correction of the local oscillator. Measure TX frequency and adjust until both match. Or use receive a know reference signal and zero-beat it and then adjust. More information in the Wiki.") },
    { MENU_CONF, MENU_ITEM, CONFIG_FWD_REV_PWR_DISP, NULL, "Pwr. Display mW", UiMenuDesc("Shows the forward and reverse power values in mW, can be used to calibrate the SWR meter.") },
    { MENU_CONF, MENU_ITEM, CONFIG_RF_FWD_PWR_NULL, NULL, "Pwr. Det. Null", UiMenuDesc(" Set the forward and reverse power sensors ADC zero power offset. This setting is enabled ONLY when Disp. Pwr (mW), is enabled. Needs SWR meter hardware modification to work. See Wiki Adjustment and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_FWD_REV_SENSE_SWAP, NULL, "SWR/PWR Meter FWD/REV Swap", UiMenuDesc("Exchange the assignment of the Power/SWR FWD and REV measurement ADC. Use if your power meter does not show anything during TX.") },

#ifdef STM32F4
    // Not supported on STM32F7 HAL
    { MENU_CONF, MENU_ITEM, CONFIG_I2C1_SPEED, NULL,"I2C1 Bus Speed", UiMenuDesc("Sets speed of the I2C1 bus (Si570 oscillator and MCP9801 temperature sensor). Higher speeds provide quicker RX/TX switching but may also cause tuning issues (red digits). Be careful with speeds above 200 kHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_I2C2_SPEED, NULL,"I2C2 Bus Speed", UiMenuDesc("Sets speed of the I2C2 bus (Audio Codec and I2C EEPROM). Higher speeds provide quicker RX/TX switching, configuration save and power off. Speeds above 200 kHz are not recommended for unmodified mcHF. Many modified mcHF seem to run with 300kHz without problems.") },
#endif

    { MENU_CONF, MENU_ITEM, CONFIG_IQ_AUTO_CORRECTION, NULL, "RX IQ Auto Correction", UiMenuDesc("Receive IQ phase and amplitude imbalance can be automatically adjusted by the mcHF. Switch ON/OFF here. If OFF, it takes the following menu values for compensating the imbalance. The automatic algorithm achieves up to 60dB mirror rejection. See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_80M_RX_IQ_GAIN_BAL, &ts.display_rx_iq, "RX IQ Balance (80m)", UiMenuDesc("IQ Balance Adjust for all receive if frequency translation is NOT OFF. Requires USB/LSB/CW mode to be changeable.See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_80M_RX_IQ_PHASE_BAL, &ts.display_rx_iq, "RX IQ Phase   (80m)", UiMenuDesc("IQ Phase Adjust for all receive if frequency translation is NOT OFF. Requires USB/LSB/CW mode to be changeable.See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_RX_IQ_GAIN_BAL, &ts.display_rx_iq, "RX IQ Balance (10m)", UiMenuDesc("IQ Balance Adjust for all receive if frequency translation is NOT OFF. Requires USB/LSB/CW mode to be changeable.See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_RX_IQ_PHASE_BAL, &ts.display_rx_iq, "RX IQ Phase   (10m)", UiMenuDesc("IQ Phase Adjust for all receive if frequency translation is NOT OFF. Requires USB/LSB/CW mode to be changeable. See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_80M_TX_IQ_GAIN_BAL, NULL, "TX IQ Balance (80m)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Requires USB or LSB mode to be changeable. See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_80M_TX_IQ_PHASE_BAL, NULL, "TX IQ Phase   (80m)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Requires USB or LSB mode to be changeable. See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_TX_IQ_GAIN_BAL, NULL, "TX IQ Balance (10m)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Requires USB or LSB mode to be changeable. See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_TX_IQ_PHASE_BAL, NULL, "TX IQ Phase   (10m)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Requires USB or LSB mode to be changeable. See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_80M_TX_IQ_GAIN_BAL_TRANS_OFF, NULL, "TX IQ Balance (80m,CW)", UiMenuDesc("IQ Balance Adjust for CW transmissions (and all transmission if frequency translation is OFF). See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_80M_TX_IQ_PHASE_BAL_TRANS_OFF, NULL, "TX IQ Phase   (80m,CW)", UiMenuDesc("IQ Phase Adjust for CW transmissions (and all transmission if frequency translation is OFF).See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_TX_IQ_GAIN_BAL_TRANS_OFF, NULL, "TX IQ Balance (10m,CW)", UiMenuDesc("IQ Balance Adjust for CW transmissions (and all transmission if frequency translation is OFF).See Wiki Adjustments and Calibration.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_TX_IQ_PHASE_BAL_TRANS_OFF, NULL, "TX IQ Phase   (10m,CW)", UiMenuDesc("IQ Phase Adjust for CW transmissions (and all transmission if frequency translation is OFF).See Wiki Adjustments and Calibration.") },

    { MENU_CONF, MENU_ITEM, CONFIG_20M_TX_IQ_GAIN_BAL, NULL, "TX IQ Balance (20m)", UiMenuDesc("IQ Balance Adjust for all transmission if frequency translation is NOT OFF. Calibrate on 14.100 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_20M_TX_IQ_PHASE_BAL, NULL, "TX IQ Phase   (20m)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Calibrate on 14.100 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_15M_TX_IQ_GAIN_BAL, NULL, "TX IQ Balance (15m)", UiMenuDesc("IQ Balance Adjust for all transmission if frequency translation is NOT OFF. Calibrate on 21.100 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_15M_TX_IQ_PHASE_BAL, NULL, "TX IQ Phase   (15m)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Calibrate on 21.100 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_UP_TX_IQ_GAIN_BAL, NULL, "TX IQ Balance (10mUp)", UiMenuDesc("IQ Balance Adjust for all transmission if frequency translation is NOT OFF. Calibrate on 29.650 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_UP_TX_IQ_PHASE_BAL, NULL, "TX IQ Phase   (10mUp)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Calibrate on 29.650 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_20M_TX_IQ_GAIN_BAL_TRANS_OFF, NULL, "TX IQ Balance (20m,CW)", UiMenuDesc("IQ Balance Adjust for all transmission if frequency translation is OFF. Calibrate on 14.100 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_20M_TX_IQ_PHASE_BAL_TRANS_OFF, NULL, "TX IQ Phase   (20m,CW)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is OFF. Calibrate on 14.100 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_15M_TX_IQ_GAIN_BAL_TRANS_OFF, NULL, "TX IQ Balance (15m,CW)", UiMenuDesc("IQ Balance Adjust for all transmission if frequency translation is OFF. Calibrate on 21.100 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_15M_TX_IQ_PHASE_BAL_TRANS_OFF, NULL, "TX IQ Phase   (15m,CW)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is OFF. Calibrate on 21.100 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_UP_TX_IQ_GAIN_BAL_TRANS_OFF, NULL, "TX IQ Balance (10mUp,CW)", UiMenuDesc("IQ Balance Adjust for all transmission if frequency translation is OFF. Calibrate on 29.650 MHz.") },
    { MENU_CONF, MENU_ITEM, CONFIG_10M_UP_TX_IQ_PHASE_BAL_TRANS_OFF, NULL, "TX IQ Phase   (10mUp,CW)", UiMenuDesc("IQ Phase Adjust for all transmission if frequency translation is OFF. Calibrate on 29.650 MHz.") },
    // { MENU_CONF, MENU_ITEM, CONFIG_AM_RX_GAIN_BAL,"244","AM  RX IQ Bal.", UiMenuDesc(":soon:") },
    // { MENU_CONF, MENU_ITEM, CONFIG_AM_RX_PHASE_BAL,"244b","AM  RX IQ Phase", UiMenuDesc(":soon:") },
    // { MENU_CONF, MENU_ITEM, CONFIG_FM_RX_GAIN_BAL,"245","FM  RX IQ Bal.", UiMenuDesc(":soon:") },
    //{ MENU_CONF, MENU_ITEM, CONFIG_AM_TX_GAIN_BAL,"254","AM  TX IQ Bal.", UiMenuDesc(":soon:") },
    //{ MENU_CONF, MENU_ITEM, CONFIG_FM_TX_GAIN_BAL,"255","FM  TX IQ Bal.", UiMenuDesc(":soon:") },
#ifdef OBSOLETE_NR
    // DSP Configuration, probably never touched
	{ MENU_CONF, MENU_ITEM, CONFIG_DSP_NR_DECORRELATOR_BUFFER_LENGTH, NULL, "DSP NR BufLen", UiMenuDesc("DSP LMS noise reduction: length of the audio buffer that is used for simulation of a reference for the LMS algorithm. The longer the buffer, the better the performance, but this buffer length must always be larger than the number of taps in the FIR filter used. Thus, a larger buffer (and larger FIR filter) uses more MCU resources.") },
    { MENU_CONF, MENU_ITEM, CONFIG_DSP_NR_FFT_NUMTAPS, NULL, "DSP NR FIR NumTaps", UiMenuDesc("DSP LMS noise reduction: Number of taps in the DSP noise reduction FIR filter. The larger the number of taps in the filter, the better the performance, but the slower the performance of the filter and the mcHF.") },
    { MENU_CONF, MENU_ITEM, CONFIG_DSP_NR_POST_AGC_SELECT, NULL, "DSP NR Post-AGC", UiMenuDesc("DSP LMS noise reduction: Perform the DSP LMS noise reduction BEFORE or AFTER the AGC. NO = before AGC, YES = after AGC.") },
	{ MENU_CONF, MENU_ITEM, CONFIG_DSP_NOTCH_CONVERGE_RATE, NULL, "DSP Notch ConvRate", UiMenuDesc("DSP LMS automatic notch filter: ") },
    { MENU_CONF, MENU_ITEM, CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH, NULL, "DSP Notch BufLen", UiMenuDesc("DSP LMS automatic notch filter: length of the audio buffer that is used for simulation of a reference for the LMS algorithm. The longer the buffer, the better -and the slower- the performance, but this buffer length must always be larger than the number of taps in the FIR filter used. Thus, a larger buffer (and larger FIR filter) uses more MCU resources.") },
    { MENU_CONF, MENU_ITEM, CONFIG_DSP_NOTCH_FFT_NUMTAPS, NULL, "DSP Notch FIRNumTap", UiMenuDesc("DSP LMS automatic notch filter: Number of taps in the DSP automatic notch FIR filter. The larger the number of taps in the filter, the better the performance, but the slower the performance of the filter and the mcHF.") },
//    { MENU_CONF, MENU_ITEM, CONFIG_AGC_TIME_CONSTANT, NULL, "NB AGC T/C (<=Slow)", UiMenuDesc("Noise Blanker AGC time constant adjustment: Lower values are equivalent with slower Noise blanker AGC. While the menu is displayed, the noise blanker is switched OFF, so in order to test the effect of adjusting this parameter, leave the menu.") },
#endif
#ifdef USE_LMS_AUTONOTCH
	{ MENU_CONF, MENU_ITEM, CONFIG_DSP_NOTCH_CONVERGE_RATE, NULL, "DSP Notch ConvRate", UiMenuDesc("DSP LMS automatic notch filter: ") },
    { MENU_CONF, MENU_ITEM, CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH, NULL, "DSP Notch BufLen", UiMenuDesc("DSP LMS automatic notch filter: length of the audio buffer that is used for simulation of a reference for the LMS algorithm. The longer the buffer, the better -and the slower- the performance, but this buffer length must always be larger than the number of taps in the FIR filter used. Thus, a larger buffer (and larger FIR filter) uses more MCU resources.") },
    { MENU_CONF, MENU_ITEM, CONFIG_DSP_NOTCH_FFT_NUMTAPS, NULL, "DSP Notch FIRNumTap", UiMenuDesc("DSP LMS automatic notch filter: Number of taps in the DSP automatic notch FIR filter. The larger the number of taps in the filter, the better the performance, but the slower the performance of the filter and the mcHF.") },
#endif
//    { MENU_CONF, MENU_ITEM, CONFIG_SAM_PLL_TAUR, NULL, "SAM PLL tauR", UiMenuDesc(":soon:") },
//    { MENU_CONF, MENU_ITEM, CONFIG_SAM_PLL_TAUI, NULL, "SAM PLL tauI", UiMenuDesc(":soon:") },
//    { MENU_CONF, MENU_ITEM, CONFIG_SAM_SIDEBAND, NULL, "SAM Sideband", UiMenuDesc(":soon:") },

    // Reset I2C Config EEPROM to empty state
    { MENU_CONF, MENU_ITEM, CONFIG_RESET_SER_EEPROM, NULL, "Rst Conf EEPROM", UiMenuDesc("Clear the EEPROM so that at next start all stored configuration data is reset to the values stored in Flash (see Backup/Restore).") },

    { MENU_CONF, MENU_STOP, 0, NULL , NULL, UiMenuDesc("") }
};

const MenuDescriptor displayGroup[] =
{
    { MENU_DISPLAY, MENU_ITEM, CONFIG_LCD_AUTO_OFF_MODE, NULL, "LCD Auto Blank", UiMenuDesc("After x seconds LCD turns dark and LCD data sections stop. So power consumption is decreased and RX hum is decreased, too. LCD operation starts when using any button or the touchscreen.") },
    { MENU_DISPLAY, MENU_ITEM, CONFIG_FREQ_STEP_MARKER_LINE, NULL, "Step Size Marker", UiMenuDesc("If enabled, you'll see a line under the digit which is currently representing the selected tuning step size") },
    { MENU_DISPLAY, MENU_ITEM, CONFIG_DISP_FILTER_BANDWIDTH, NULL, "Filter BW Display", UiMenuDesc("Colour of the horizontal Filter Bandwidth indicator bar.") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SPECTRUM_SIZE, NULL, "Spectrum Size", UiMenuDesc("Change height of spectrum display") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SPECTRUM_FILTER_STRENGTH, NULL, "Spectrum Filter", UiMenuDesc("Lowpass filter for the spectrum FFT. Low values: fast and nervous spectrum; High values: slow and calm spectrum.") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SPECTRUM_FREQSCALE_COLOUR, NULL, "Spec FreqScale Colour", UiMenuDesc("Colour of the small frequency digits under the spectrum display.") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SPECTRUM_CENTER_LINE_COLOUR, NULL, "TX Carrier Colour", UiMenuDesc("Colour of the vertical line indicating the TX carrier frequency in the spectrum or waterdall display.") },
//    { MENU_DISPLAY, MENU_ITEM, CONFIG_SPECTRUM_FFT_WINDOW_TYPE, NULL, "Spectrum FFT Wind.", UiMenuDesc("Selects the window algorithm for the spectrum FFT. For low spectral leakage, Hann, Hamming or Blackman window is recommended.") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_LIGHT_ENABLE, NULL, "Scope Light", UiMenuDesc("The scope uses bars (NORMAL) or points (LIGHT) to represent data. LIGHT is a little less resource intensive.") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_SPEED, NULL, "Scope 1/Speed", UiMenuDesc("Lower Values: Higher refresh rate. Set to 0 to disable scope.") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_AGC_ADJUST, NULL, "Scope AGC Adj.", UiMenuDesc("Adjusting of scope / waterfall AGC for fitting graphs to screen") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_TRACE_COLOUR, NULL, "Scope Trace Colour", UiMenuDesc("Set colour of scope") },
	{ MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_TRACE_HL_COLOUR, NULL, "Scope BW Trace Colour", UiMenuDesc("Set colour of highlighted BW scope") },
	{ MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_BACKGROUND_HL_COLOUR, NULL, "Scope BW BCKgr Colour", UiMenuDesc("Set colour of highlighted BW background") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_GRID_COLOUR, NULL, "Scope Grid Colour", UiMenuDesc("Set colour of scope grid") },
    { MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_DB_DIVISION, NULL, "Scope Div.", UiMenuDesc("Set rf range for scope") },
    // { MENU_DISPLAY, MENU_ITEM, MENU_SCOPE_NOSIG_ADJUST, NULL, "Scope NoSig Adj.", UiMenuDesc("Set scope line corresponding to NO SIGNAL") },
    { MENU_DISPLAY, MENU_ITEM, MENU_WFALL_SPEED, NULL, "Wfall 1/Speed", UiMenuDesc("Lower Values: Higher refresh rate. Set to 0 to disable waterfall.") },
    { MENU_DISPLAY, MENU_ITEM, MENU_WFALL_COLOR_SCHEME, NULL, "Wfall Colours", UiMenuDesc("Select colour scheme for waterfall display.") },
    { MENU_DISPLAY, MENU_ITEM, MENU_WFALL_STEP_SIZE, NULL, "Wfall Step Size", UiMenuDesc("How many lines are moved in a single screen update") },
    // { MENU_DISPLAY, MENU_ITEM, MENU_WFALL_OFFSET, NULL, "Wfall Brightness", UiMenuDesc("Set to input level which waterfall uses for lowest level") },
    { MENU_DISPLAY, MENU_ITEM, MENU_WFALL_CONTRAST, NULL, "Wfall Contrast", UiMenuDesc("Adjust to fit your personal input level range to displayable colour range for waterfall") },
    // { MENU_DISPLAY, MENU_ITEM, MENU_WFALL_NOSIG_ADJUST, NULL, "Wfall NoSig Adj.", UiMenuDesc("Set NO SIGNAL state for waterfall") },
    { MENU_DISPLAY, MENU_ITEM, MENU_METER_COLOUR_UP, NULL, "Upper Meter Colour", UiMenuDesc("Set the colour of the scale of combined S/Power-Meter") },
    { MENU_DISPLAY, MENU_ITEM, MENU_METER_COLOUR_DOWN, NULL, "Lower Meter Colour", UiMenuDesc("Set the colour of the scale of combined SWR/AUD/ALC-Meter") },
    { MENU_DISPLAY, MENU_ITEM, MENU_DBM_DISPLAY, NULL, "dBm display", UiMenuDesc("RX signal power (measured within the filter bandwidth) can be displayed in dBm or normalized as dBm/Hz. This value is supposed to be quite accurate to +-3dB. Preferably use low spectrum display magnify settings. Accuracy is lower for very very weak and very very strong signals.")},
    { MENU_DISPLAY, MENU_ITEM, MENU_DBM_CALIBRATE, NULL, "dBm calibrate", UiMenuDesc("dBm display calibration. Just an offset (in dB) that is added to the internally calculated dBm or dBm/Hz value.")},
    { MENU_DISPLAY, MENU_ITEM, CONFIG_SMETER_ATTACK, NULL, "S-Meter Attack", UiMenuDesc("Attack controls how quickly the S-Meter reacts to rising signal levels, higher values represent quicker reaction") },
    { MENU_DISPLAY, MENU_ITEM, CONFIG_SMETER_DECAY, NULL, "S-Meter Decay", UiMenuDesc("Decay controls how quickly the S-Meter reacts to falling signal levels, higher values represent quicker reaction") },
#ifdef USE_8bit_FONT
	{ MENU_DISPLAY, MENU_ITEM, MENU_FREQ_FONT, NULL, "Freq display font", UiMenuDesc("Font selection for frequency display. Allows selection of old/modern fonts")},
#endif
    { MENU_DISPLAY, MENU_ITEM, MENU_UI_INVERSE_SCROLLING, NULL, "Menu Inverse Scrolling", UiMenuDesc("Inverts Enc2/Enc3 behavior in menu up/down and show/hide UI scrolling actions, used for side-mounted encoder dials.")},
	{ MENU_DISPLAY, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor cwGroup[] =
{
//    { MENU_CW, MENU_ITEM, MENU_CW_WIDE_FILT,"028","Wide Filt in CW Mode", UiMenuDesc(":soon:") },
    { MENU_CW, MENU_ITEM, MENU_KEYER_MODE, NULL, "CW Keyer Mode", UiMenuDesc("Select how the mcHF interprets the connected keyer signals. Supported modes: Iambic A and B Keyer (IAM A/B), Straight Key (STR_K), and Ultimatic Keyer (ULTIM)") },
    { MENU_CW, MENU_ITEM, MENU_KEYER_SPEED, NULL, "CW Keyer Speed", UiMenuDesc("Keyer Speed for the automatic keyer modes in WpM. Also changeable via Encoder 3 if in CW Mode.") },
    { MENU_CW, MENU_ITEM, MENU_KEYER_WEIGHT, NULL, "CW Keyer Weight", UiMenuDesc("Keyer Dit/Pause ratio for the automatic keyer modes. Higher values increase length of dit, decreases length of pause so that the total time is still according to the set WpM value.") },
    { MENU_CW, MENU_ITEM, MENU_SIDETONE_GAIN, NULL, "CW Sidetone Gain", UiMenuDesc("Audio volume for the monitor sidetone in CW TX. Also changeable via Encoder 1 if in CW Mode.") },
    { MENU_CW, MENU_ITEM, MENU_SIDETONE_FREQUENCY, NULL, "CW Side/Offset Freq", UiMenuDesc("Sidetone Frequency (also Offset frequency, see CW Freq. Offset below)") },
    { MENU_CW, MENU_ITEM, MENU_PADDLE_REVERSE, NULL, "CW Paddle Reverse", UiMenuDesc("Dit is Dah and Dah is Dit. Use if your keyer needs reverse meaning of the paddles.") },
    { MENU_CW, MENU_ITEM, MENU_CW_TX_RX_DELAY, NULL, "CW TX->RX Delay", UiMenuDesc("How long to stay in CW TX mode after stop sending a signal.") },
    { MENU_CW, MENU_ITEM, MENU_CW_OFFSET_MODE, NULL, "CW Freq. Offset", UiMenuDesc("TX: display is TX frequency if received frequency was zero-beated. DISP: display is RX frequency if received signal is matched to sidetone. SHIFT: LO shifts, display is RX frequency if signal is matched to sidetone.") },
    { MENU_CW, MENU_ITEM, MENU_CW_AUTO_MODE_SELECT, NULL, "CW LSB/USB Select", UiMenuDesc("Set appropriate sideband mode for CW. If AUTO, sideband is chosen for bands by its frequency. A long press on Mode button gets the other sideband mode")},
    { MENU_CW, MENU_ITEM, MENU_CW_DECODER, NULL,"CW decoder enable", UiMenuDesc("enable experimental CW decoding") },
    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_THRESH, NULL,"Signal threshold", UiMenuDesc("All signals above this threshold are intepreted as a dit or daah") },
    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_SNAP_ENABLE, NULL,"Tune helper", UiMenuDesc("graphical tune helper: adjust frequency until yellow vertical line is in centre of green box --> right on CW carrier frequency") },
//    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_AVERAGE, NULL,"Goertzel averager", UiMenuDesc("The CW tone is averaged over N Goertzel values") },
    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_BLOCKSIZE, NULL,"Blocksize for Goertzel", UiMenuDesc("How many samples are taken for the signal detection with the Goertzel algorithm?") },
//    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_AGC, NULL,"AGC for decoder", UiMenuDesc("Enable/disable AGC for CW decoder") },
    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_NOISECANCEL, NULL,"Noise cancel", UiMenuDesc("Enable/disable noise canceler for CW decoder") },
    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_SPIKECANCEL, NULL,"Spike cancel", UiMenuDesc("Enable/disable spike canceler or short cancel for CW decoder") },
    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_USE_3_GOERTZEL, NULL,"AGC for decoder", UiMenuDesc("Enable/disable AGC for CW decoder") },
    { MENU_CW, MENU_ITEM, MENU_CW_DECODER_SHOW_CW_LED, NULL,"show CW LED", UiMenuDesc("Enable/disable LED for CW decoder") },
	{ MENU_CW, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor filterGroup[] =
{
    { MENU_FILTER, MENU_ITEM, MENU_FP_SSB_01, NULL, "SSB Filter 1", UiMenuDesc("Filter bandwidth #1 when toggling with filter select button in LSB or USB.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_SSB_02, NULL, "SSB Filter 2", UiMenuDesc("Filter bandwidth #2 when toggling with filter select button in LSB or USB.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_SSB_03, NULL, "SSB Filter 3", UiMenuDesc("Filter bandwidth #3 when toggling with filter select button in LSB or USB.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_SSB_04, NULL, "SSB Filter 4", UiMenuDesc("Filter bandwidth #4 when toggling with filter select button in LSB or USB.") },

    { MENU_FILTER, MENU_ITEM, MENU_FP_CW_01, NULL, "CW Filter 1", UiMenuDesc("Filter bandwidth #1 when toggling with filter select button in CW.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_CW_02, NULL, "CW Filter 2", UiMenuDesc("Filter bandwidth #2 when toggling with filter select button in CW.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_CW_03, NULL, "CW Filter 3", UiMenuDesc("Filter bandwidth #3 when toggling with filter select button in CW.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_CW_04, NULL, "CW Filter 4", UiMenuDesc("Filter bandwidth #4 when toggling with filter select button in CW.") },

    { MENU_FILTER, MENU_ITEM, MENU_FP_AM_01, NULL, "AM/SAM Filter 1", UiMenuDesc("Filter bandwidth #1 when toggling with filter select button in AM & SAM.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_AM_02, NULL, "AM/SAM Filter 2", UiMenuDesc("Filter bandwidth #2 when toggling with filter select button in AM & SAM.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_AM_03, NULL, "AM/SAM Filter 3", UiMenuDesc("Filter bandwidth #3 when toggling with filter select button in AM & SAM.") },
    { MENU_FILTER, MENU_ITEM, MENU_FP_AM_04, NULL, "AM/SAM Filter 4", UiMenuDesc("Filter bandwidth #4 when toggling with filter select button in AM & SAM.") },

    // not needed any more: AM & SAM use exactly the same filters
//    { MENU_FILTER, MENU_ITEM, MENU_FP_SAM_01,"600", "SAM Filter 1", UiMenuDesc(":soon:") },
//    { MENU_FILTER, MENU_ITEM, MENU_FP_SAM_02,"600", "SAM Filter 2", UiMenuDesc(":soon:") },
//    { MENU_FILTER, MENU_ITEM, MENU_FP_SAM_03,"600", "SAM Filter 3", UiMenuDesc(":soon:") },
//    { MENU_FILTER, MENU_ITEM, MENU_FP_SAM_04,"600", "SAM Filter 4", UiMenuDesc(":soon:") },

    { MENU_FILTER, MENU_ITEM, CONFIG_AM_TX_FILTER_DISABLE, NULL,"AM  TX Audio Filter", UiMenuDesc("Select if AM-TX signal is filtered (strongly recommended to agree to regulations)") },
//    { MENU_FILTER, MENU_ITEM, CONFIG_SSB_TX_FILTER_DISABLE, NULL,"SSB TX Audio Filter", UiMenuDesc(":soon:") },
    { MENU_FILTER, MENU_ITEM, CONFIG_SSB_TX_FILTER, NULL,"SSB TX Audio Filter2", UiMenuDesc("Select if SSB-TX signal is filtered (strongly recommended to agree to regulations)") },

    { MENU_FILTER, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor powGroup[] =
{
    { MENU_POW, MENU_ITEM, CONFIG_TUNE_POWER_LEVEL, NULL, "Tune Power Level", UiMenuDesc("Select the power level for TUNE operation. May be set using the selected power level or have a fixed power level.") },
    { MENU_POW, MENU_ITEM, CONFIG_TUNE_TONE_MODE, NULL, "Tune Tone (SSB)", UiMenuDesc("Select if single tone or two tone is generated during TUNE operation. Not persistent.") },
    { MENU_POW, MENU_ITEM, CONFIG_CW_PA_BIAS, NULL, "CW PA Bias (If >0 )", UiMenuDesc("If set to a value above 0, this BIAS is used during CW transmission; otherwise normal BIAS is used during CW") },
    { MENU_POW, MENU_ITEM, CONFIG_REDUCE_POWER_ON_LOW_BANDS, NULL, "Reduce Power on Low Bands", UiMenuDesc("If set (recommended!)  frequencies below 8Mhz (40m or lower) require higher power adjust values (four times). This permits better control of generated power on these frequencies.") },
    { MENU_POW, MENU_ITEM, CONFIG_REDUCE_POWER_ON_HIGH_BANDS, NULL, "Reduce Power on High Bands", UiMenuDesc("If set frequencies above 8Mhz (30m or higher) require higher power adjust values (four times). This permits better control of generated power on these frequencies.") },
    { MENU_POW, MENU_ITEM, CONFIG_VSWR_PROTECTION_THRESHOLD, NULL, "VSWR Protect.threshold", UiMenuDesc("If not OFF, on TX/tune the bias of PA will be down to 0 etc when exceeding the specified value of VSWR") }, 
    { MENU_POW, MENU_ITEM, CONFIG_PA_BIAS, NULL, "PA Bias", UiMenuDesc("Defines the BIAS value of the PA. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_2200M_5W_ADJUST,&band_enabled[BAND_MODE_2200],"2200m 5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_630M_5W_ADJUST,&band_enabled[BAND_MODE_630],"630m  5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_160M_5W_ADJUST, NULL, "160m  5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_80M_5W_ADJUST, NULL, "80m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_60M_5W_ADJUST, NULL, "60m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_40M_5W_ADJUST, NULL, "40m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_30M_5W_ADJUST, NULL, "30m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_20M_5W_ADJUST, NULL, "20m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_17M_5W_ADJUST, NULL, "17m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_15M_5W_ADJUST, NULL, "15m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_12M_5W_ADJUST, NULL, "12m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_10M_5W_ADJUST, NULL, "10m   5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_6M_5W_ADJUST,&band_enabled[BAND_MODE_6],"6m    5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_4M_5W_ADJUST,&band_enabled[BAND_MODE_4],"4m    5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_2M_5W_ADJUST,&band_enabled[BAND_MODE_2],"2m    5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_70CM_5W_ADJUST,&band_enabled[BAND_MODE_70],"70cm  5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_23CM_5W_ADJUST,&band_enabled[BAND_MODE_23],"23cm  5W PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_2200M_FULL_POWER_ADJUST,&band_enabled[BAND_MODE_2200],"2200m Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_630M_FULL_POWER_ADJUST,&band_enabled[BAND_MODE_630],"630m  Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_160M_FULL_POWER_ADJUST, NULL, "160m  Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_80M_FULL_POWER_ADJUST, NULL, "80m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_60M_FULL_POWER_ADJUST, NULL, "60m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_40M_FULL_POWER_ADJUST, NULL, "40m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_30M_FULL_POWER_ADJUST, NULL, "30m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_20M_FULL_POWER_ADJUST, NULL, "20m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_17M_FULL_POWER_ADJUST, NULL, "17m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_15M_FULL_POWER_ADJUST, NULL, "15m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_12M_FULL_POWER_ADJUST, NULL, "12m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_10M_FULL_POWER_ADJUST, NULL, "10m   Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_6M_FULL_POWER_ADJUST,&band_enabled[BAND_MODE_6],"6m    Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_4M_FULL_POWER_ADJUST,&band_enabled[BAND_MODE_4],"4m    Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_2M_FULL_POWER_ADJUST,&band_enabled[BAND_MODE_2],"2m    Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_70CM_FULL_POWER_ADJUST,&band_enabled[BAND_MODE_70],"70cm  Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_23CM_FULL_POWER_ADJUST,&band_enabled[BAND_MODE_23],"23cm  Full PWR Adjust", UiMenuDesc("Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_2200M_ADJ,&band_enabled[BAND_MODE_2200],"2200m Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 2200m band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_630M_ADJ,&band_enabled[BAND_MODE_630],"630m Coupling Adj.", UiMenuDesc("Power Adjustment factor for the 630m band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_160M_ADJ, NULL, "160m Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 160m band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_80M_ADJ, NULL, "80m  Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 80m band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_40M_ADJ, NULL, "40m  Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 40m and 60m band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_20M_ADJ, NULL, "20m  Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 20m and 30m band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_15M_ADJ, NULL, "15m  Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 10m - 17m bands power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_6M_ADJ,&band_enabled[BAND_MODE_6], "6m   Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 6m band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_2M_ADJ,&band_enabled[BAND_MODE_2], "2m   Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 2m band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_70CM_ADJ,&band_enabled[BAND_MODE_70], "70cm Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 70cm band power values. See Wiki.") },
    { MENU_POW, MENU_ITEM, CONFIG_FWD_REV_COUPLING_23CM_ADJ,&band_enabled[BAND_MODE_23], "23cm Coupling Adj.", UiMenuDesc("Power Meter Adjustment factor for the 23cm band power values. See Wiki.") },

    { MENU_POW, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor men2touchGroup[] =
{
    { MENU_MEN2TOUCH, MENU_ITEM, MENU_DYNAMICTUNE, NULL, "Dynamic Tune", UiMenuDesc("Toggles dynamic tune mode") },
    { MENU_MEN2TOUCH, MENU_ITEM, MENU_MIC_LINE_MODE, NULL, "Mic/Line Select", UiMenuDesc("Select the required signal input for transmit (except in CW). Also changeable via long press on M3") },
    { MENU_MEN2TOUCH, MENU_ITEM, MENU_SPECTRUM_MODE, NULL, "Spectrum Type", UiMenuDesc("Select if you want a scope-like or a waterfall-like (actually a fountain) display") },
    { MENU_MEN2TOUCH, MENU_ITEM, MENU_SPECTRUM_MAGNIFY, NULL, "Spectrum Magnify", UiMenuDesc("Select level of magnification (1x, 2x, 4x, 8x, 16x, 32x) of spectrum and waterfall display. Also changeable via touch screen. Refresh rate is much slower with high magnification settings. The dBm display has its maximum accuracy in magnify 1x setting.") },
    { MENU_MEN2TOUCH, MENU_ITEM, MENU_RESTART_CODEC, NULL, "Restart Codec", UiMenuDesc("Sometimes there is a problem with the I2S IQ signal stream from the Codec, resulting in mirrored signal reception. Restarting the CODEC Stream will cure that problem. Try more than once, if first call did not help.") },
    { MENU_MEN2TOUCH, MENU_ITEM, MENU_DIGITAL_MODE_SELECT, NULL, "Digital Mode", UiMenuDesc("Select the active digital mode (FreeDV,RTTY, ...).") },
    { MENU_MEN2TOUCH, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor infoGroup[] =
{
    { MENU_SYSINFO, MENU_INFO, INFO_DISPLAY, NULL,"Display", UiMenuDesc("Displays working mode (SPI/parallel") },
    { MENU_SYSINFO, MENU_INFO, INFO_DISPLAY_CTRL, NULL,"Disp. Controller", UiMenuDesc("identified LCD controller chip") },
    { MENU_SYSINFO, MENU_INFO, INFO_OSC_NAME, NULL,"Oscillator", UiMenuDesc("Local oscillator type") },
#ifdef USE_OSC_SI570
    { MENU_SYSINFO, MENU_INFO, INFO_SI570, NULL,"SI570", UiMenuDesc("Startup frequency and I2C address of local oscillator Type SI570") },
#endif
    { MENU_SYSINFO, MENU_INFO, INFO_EEPROM, NULL,"EEPROM", UiMenuDesc("type of serial EEPROM and its capacity") },
    { MENU_SYSINFO, MENU_INFO, INFO_TP, NULL,"Touchscreen", UiMenuDesc("touchscreen state") },
    { MENU_SYSINFO, MENU_INFO, INFO_CPU, NULL,"CPU", UiMenuDesc("identification of fitted MCU") },
    { MENU_SYSINFO, MENU_INFO, INFO_FLASH, NULL,"Flash Size (kB)", UiMenuDesc("flash size of MCU") },
    { MENU_SYSINFO, MENU_INFO, INFO_RAM, NULL,"RAM Size (kB)", UiMenuDesc("RAM size of MCU") },
    { MENU_SYSINFO, MENU_INFO, INFO_FW_VERSION, NULL,"Firmware", UiMenuDesc("firmware version") },
    { MENU_SYSINFO, MENU_INFO, INFO_BUILD, NULL,"Build", UiMenuDesc("firmware: timestamp of building") },
    { MENU_SYSINFO, MENU_INFO, INFO_BL_VERSION, NULL,"Bootloader", UiMenuDesc("bootloader version") },
    { MENU_SYSINFO, MENU_INFO, INFO_RFBOARD, NULL,"RF Board", UiMenuDesc("Displays the detected RF Board hardware identification.") },
    { MENU_SYSINFO, MENU_INFO, INFO_CODEC, NULL,"Audio Codec Presence", UiMenuDesc("Audio Codec I2C communication successfully tested? This is not a full test of the Audio Codec functionality, it only reports if I2C communication reported no problem talking to the codec.") },
    { MENU_SYSINFO, MENU_INFO, INFO_CODEC_TWINPEAKS, NULL,"Audio Codec Twinpeaks Corr.", UiMenuDesc("In some cases the audio codec needs to be restarted to produce correct IQ. The IQ auto correction detects this. If this fixes the problem, Done is displayed, Failed otherwise") },
    { MENU_SYSINFO, MENU_INFO, INFO_VBAT, NULL,"Backup RAM Battery", UiMenuDesc("Battery Support for Backup RAM present?") },
    { MENU_SYSINFO, MENU_INFO, INFO_RTC, NULL,"Real Time Clock", UiMenuDesc("Battery Supported Real Time Clock present?") },
    { MENU_SYSINFO, MENU_INFO, INFO_LICENCE, NULL,"FW license", UiMenuDesc("Display license of firmware") },
    { MENU_SYSINFO, MENU_INFO, INFO_HWLICENCE, NULL,"HW license", UiMenuDesc("Display license of hardware") },

    { MENU_SYSINFO, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor debugGroup[] =
{
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_ENABLE_INFO, NULL,"Enable Debug Info Display", UiMenuDesc("Enable debug outputs on LCD for testing purposes (touch screen coordinates, load) and audio interrupt duration indication via green led") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_CW_OFFSET_SHIFT_KEEP_SIGNAL, NULL,"CW Shift Keeps Signal", UiMenuDesc("Enable automatic sidetone correction for CW OFFSET MODE = SHIFT. If you tuned in SSB to a CW signal around the sidetone frequency, you'll keep that signal when going to CW. Even if you switch from USB to CW-LSB etc.") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_TX_AUDIO, NULL,"TX Audio via USB", UiMenuDesc("If enabled, send generated audio to PC during TX.") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_CLONEOUT, NULL,"FT817 Clone Transmit", UiMenuDesc("Will in future send out memory data to an FT817 Clone Info (to be used with CHIRP).") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_CLONEIN, NULL,"FT817 Clone Receive", UiMenuDesc("Will in future get memory data from an FT817 Clone Info (to be used with CHIRP).") },
//    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NEW_NB, NULL,"New Noiseblanker", UiMenuDesc("New noiseblanker for testing purposes") },
//    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_ENABLE, NULL,"Spectral NR", UiMenuDesc("enable spectral noise reduction for testing purposes") },
//    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_LONG_TONE_ENABLE, NULL,"Long tone", UiMenuDesc("enable long tone detection in spectral noise reduction for testing purposes") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_GAIN_SHOW, NULL,"Show gains", UiMenuDesc("Debugging: show gains of spectral noise reduction") },

//	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_LONG_TONE_ALPHA, NULL,"Long tone alpha", UiMenuDesc("time constant alpha for long tone detection in spectral noise reduction") },
//	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_LONG_TONE_THRESH, NULL,"Long tone thresh", UiMenuDesc("threshold for long tone detection in spectral noise reduction") },
//	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_GAIN_SMOOTH_ENABLE, NULL,"SNR gain smooth", UiMenuDesc("enable bin gain smoothing for spectral noise reduction for testing purposes") },
//    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_GAIN_SMOOTH_ALPHA, NULL,"SNR gain smooth alpha", UiMenuDesc("alpha = smoothing constant for spectral noise reduction for testing purposes") },
//	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_ALPHA, NULL,"NR alpha", UiMenuDesc("time constant alpha for spectral noise reduction") },
//    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_THRESH, NULL,"NR thresh", UiMenuDesc("threshold for spectral noise reduction voice activity detector") },
//    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_VAD_TYPE, NULL,"NR VAD type", UiMenuDesc("VAD type for spectral noise reduction voice activity detector") },
//	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_VAD_DELAY, NULL,"NR VAD delay", UiMenuDesc("delay for spectral noise reduction voice activity detector") },

	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_BETA, NULL,"NR beta", UiMenuDesc("time constant beta for spectral noise reduction, leave at 0.85") },
//	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_Mode, NULL,"NR Mode", UiMenuDesc("switch between the released NR and two development NRs") },
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_ASNR, NULL,"NR asnr", UiMenuDesc("Devel 2 NR: asnr") },
//#if defined(STM32F7) || defined(STM32H7)
//	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_FFT_SIZE, NULL,"NR FFT Size256", UiMenuDesc("enable FFT256 instead of FFT128 for devel2 NR") },
//	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_DEC_ENABLE, NULL,"NR decimation", UiMenuDesc("enable decimation-by-2 down to 6ksps for NR") },
//#endif
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_GAIN_SMOOTH_WIDTH, NULL,"NR smooth wd.", UiMenuDesc("Devel 2 NR: width of gain smoothing window") },
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_NR_GAIN_SMOOTH_THRESHOLD, NULL,"NR smooth thr.", UiMenuDesc("Devel 2 NR: threhold for gain smoothing") },

	//    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_RTTY_ATC, NULL,"RTTY ATC Enable", UiMenuDesc("Enable automatic threshold correction ATC for RTTY decoding") },
#ifdef USE_TWO_CHANNEL_AUDIO
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_ENABLE_STEREO, NULL,"STEREO Enable", UiMenuDesc("Enable stereo demodulation modes") },
#endif
#ifdef USE_LEAKY_LMS
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_LEAKY_LMS, NULL,"leaky LMS", UiMenuDesc("Use leaky LMS noise reduction instead of built-in CMSIS LMS algorithm") },
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_ANR_TAPS, NULL,"NR no taps", UiMenuDesc("Number of taps of leaky LMS noise reduction") },
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_ANR_DELAY, NULL,"NR delay", UiMenuDesc("Delay length of leaky LMS noise reduction") },
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_ANR_GAIN, NULL,"NR gain", UiMenuDesc("Gain of leaky LMS noise reduction") },
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_ANR_LEAK, NULL,"NR leak", UiMenuDesc("Leak of leaky LMS noise reduction") },
#endif
	{ MENU_DEBUG, MENU_ITEM, MENU_DEBUG_OSC_SI5351_PLLRESET, NULL,"Si5351a PLL Reset", UiMenuDesc("Debug Setting: Select when the Si5351a does a PLL RESET") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_HMC1023_COARSE, &hmc1023.present,"HMC1023 Coarse", UiMenuDesc("Debug Setting: Change LPF HMC1023LP5E coarse bandwidth") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_HMC1023_FINE, &hmc1023.present,"HMC1023 Fine", UiMenuDesc("Debug Setting: Change LPF HMC1023LP5E fine bandwidth") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_HMC1023_GAIN, &hmc1023.present,"HMC1023 Gain 10db", UiMenuDesc("Debug Setting: Switch LPF HMC1023LP5E +10db Amp on/off") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_HMC1023_BYPASS, &hmc1023.present,"HMC1023 Bypass", UiMenuDesc("Debug Setting: Set HMC1023 to bypass mode") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_HMC1023_OPAMP, &hmc1023.present,"HMC1023 Opamp Bias", UiMenuDesc("Debug Setting: Switch LPF HMC1023LP5E Opamp Bias") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_HMC1023_DRVR, &hmc1023.present,"HMC1023 Driver Bias", UiMenuDesc("Debug Setting: Set HMC1023 Driver Bias") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_TWINPEAKS_CORR_RUN, NULL,"Trigger Twinpeaks Corr.", UiMenuDesc("Trigger Twinpeaks Correction Manually if IQ Auto Correction is enabled, otherwise you will see 'Not Possible'") },
    { MENU_DEBUG, MENU_ITEM, CONFIG_RESET_SER_EEPROM_SIGNATURE, NULL, "Rst Conf EEPROM", UiMenuDesc("Clear the EEPROMi signature but keep all config values. This is mainly for debugging purposes).") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_FREEDV_MODE, NULL, "FreeDV Mode", UiMenuDesc("Change active FreeDV mode. Please note, you have to reboot to activate new mode") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_FREEDV_SQL_THRESHOLD, NULL, "FreeDV Squelch threshold", UiMenuDesc("If not OFF, FreeDV will squelch if detected SNR is below set value.") },
    { MENU_DEBUG, MENU_ITEM, MENU_DEBUG_SMOOTH_DYN_TUNE, NULL, "Smooth dynamic tune", UiMenuDesc("Activate smooth dynamic tune.") },

	{ MENU_DEBUG, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};

const MenuDescriptor hall_of_fameGroup[] =
{
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"M0NKA  (Chris) mcHF founder", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"KA7OEI (Clint) 1st great FW", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"DF8OE  (Andreas)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"DL4SAI (Harald)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"DB4PLE (Danilo)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"DD4WH  (Frank)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"DL2FW  (Michael)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"HB9OCQ (Stephan)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"       (Asbjorn)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"YL3AKE (Eriks)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"PA7N   (Erwin)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"UA9OLB (Dmitri)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"S53DZ  (Bojan)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"SP9BSL (Slawek)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"HB9GND (Dimce)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"MM0MZW (Mike)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"UB8JDC (Yuri)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"RV9YW  (Max)", UiMenuDesc("") },
    { MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"M0LNG  (Martin)", UiMenuDesc("") },
	{ MENU_HALL_OF_FAME, MENU_TEXT, 0, NULL,"KB3CS  (Chris)", UiMenuDesc("") },

    { MENU_HALL_OF_FAME, MENU_STOP, 0, NULL, NULL, UiMenuDesc("") }
};


MenuGroupState topGroupState;
MenuGroupState baseGroupState;
MenuGroupState confGroupState;
MenuGroupState displayGroupState;
MenuGroupState cwGroupState;
MenuGroupState filterGroupState;
MenuGroupState powGroupState;
MenuGroupState men2touchState;
MenuGroupState infoGroupState;
MenuGroupState debugGroupState;
MenuGroupState hall_of_fameGroupState;


const MenuGroupDescriptor groups[] =
{
    { topGroup, &topGroupState, NULL},  // Group 0
    { baseGroup, &baseGroupState, topGroup},  // Group 1
    { confGroup, &confGroupState, topGroup},  // Group 2
    { displayGroup, &displayGroupState, topGroup },  // Group 3
    { cwGroup, &cwGroupState, topGroup },  // Group 4
    { filterGroup, &filterGroupState, topGroup },  // Group 5
    { powGroup, &powGroupState, topGroup },  // Group 6
    { men2touchGroup, &men2touchState, topGroup },  // Group 7
    { infoGroup, &infoGroupState, topGroup },  // Group 8
    { debugGroup, &debugGroupState, topGroup },  // Group 9
    { hall_of_fameGroup, &hall_of_fameGroupState, topGroup },  // Group 10
};
