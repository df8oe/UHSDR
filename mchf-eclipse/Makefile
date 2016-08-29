#  -*- makefile -*-

# Author: Harald Baumgart DL4SAI
# everybody may  copy, use or modify this file
# there is no guarantee for anything by the author
#
# HB 20.8.2015
#
# Rev. 09/08/2015 corrected by Andreas Richter DF8OE 
# Rev. 2016-04-06 cleanup - Stephan HB9ocq
# Rev. 06/12/2016 possibility of choosing individual toolchain - Andreas Richter DF8OE

# set these environent to your individual values
PRJ  = mchf
LPRJ = mchf

# If you want to hold different toolchains on Linux in /opt you can get them from
# https://launchpad.net/gcc-arm-embedded . Copy unpacked files as 'root' to /opt .
# If you want to use other toolchain than system-wide  installed proceed as following:
# Type on a terminal
# OPT_GCC_ARM=/opt/folder-of-your-toolchain
# export OPT_GCC_ARM
# Now 'make all' uses choosen toolchain instead of system wide installed.
# If yu want to switch back to system wide type
# OPT_GCC_ARM=

ifdef OPT_GCC_ARM
  PREFIX = $(OPT_GCC_ARM)
else
  PREFIX = /usr
endif

# choose what Keil-Library you want to use
#SCMSIS_LIB_VERSION = arm_cortexM4lf_math
CMSIS_LIB_VERSION = arm_cortexM4lf_math_r4p5

# compilation options
MACHFLAGS := -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb
CFLAGS  = $(MACHFLAGS)  \
          -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -DUSE_STDPERIPH_DRIVER  \
          -ffunction-sections -O2 -Wall -Wno-unused-function -D_GNU_SOURCE -DCORTEX_M4 \
          $(EXTRACFLAGS)


LDFLAGS := $(MACHFLAGS)

LIBS :=  	-lm -lg -lc -lgcc -l$(CMSIS_LIB_VERSION) \
        -L$(PREFIX)/arm-none-eabi/lib/armv7e-m/fpu  \
	-Llibs
	
# Every subdirectory with header files must be mentioned here
SUBDIRS :=  \
    .  \
    cmsis  \
    cmsis_boot  \
    cmsis_lib/include  \
    cmsis_lib/source  \
    \
    drivers/audio  \
    drivers/audio/codec  \
    drivers/audio/cw  \
    drivers/audio/filters  \
    drivers/audio/softdds  \
    drivers/cat  \
    drivers/cat/usb  \
    drivers/diag \
    drivers/diag/arm \
    drivers/keyboard  \
    drivers/keyboard/usb  \
    drivers/ui  \
    drivers/ui/encoder  \
    drivers/ui/lcd  \
    drivers/ui/misc  \
    drivers/ui/oscillator  \
    drivers/freedv  \
    \
    hardware  \
    \
    misc  \
    misc/v_eprom  \
    \
    usb  \
    usb/otg/inc  \
    usb/usbd/class/audio/inc  \
    usb/usbd/class/cdc/inc  \
    usb/usbd/core/inc  \
    usb/usbh/class/HID/inc  \
    usb/usbh/core/inc  \


# every source-file has to be mentioned here 

SRC =  \
    main.c  \
    \
    cmsis_boot/startup/startup_stm32f4xx.c  \
    cmsis_boot/system_stm32f4xx.c  \
    \
    cmsis_lib/source/misc.c  \
    cmsis_lib/source/stm32f4xx_adc.c  \
    cmsis_lib/source/stm32f4xx_dac.c  \
    cmsis_lib/source/stm32f4xx_dma.c  \
    cmsis_lib/source/stm32f4xx_exti.c  \
    cmsis_lib/source/stm32f4xx_flash.c  \
    cmsis_lib/source/stm32f4xx_fsmc.c  \
    cmsis_lib/source/stm32f4xx_gpio.c  \
    cmsis_lib/source/stm32f4xx_i2c.c  \
    cmsis_lib/source/stm32f4xx_pwr.c  \
    cmsis_lib/source/stm32f4xx_rcc.c  \
    cmsis_lib/source/stm32f4xx_rtc.c  \
    cmsis_lib/source/stm32f4xx_spi.c  \
    cmsis_lib/source/stm32f4xx_syscfg.c  \
    cmsis_lib/source/stm32f4xx_tim.c  \
    cmsis_lib/source/stm32f4xx_usart.c  \
    cmsis_lib/source/stm32f4xx_wwdg.c  \
    \
    drivers/audio/audio_driver.c  \
    drivers/audio/audio_filter.c  \
    drivers/audio/audio_management.c  \
    drivers/audio/freedv_mchf.c  \
    drivers/audio/freedv_test_data.c  \
    drivers/audio/codec/codec.c  \
    drivers/audio/codec/i2s.c  \
    drivers/audio/cw/cw_gen.c  \
    drivers/audio/filters/fir_rx_decimate_4.c  \
    drivers/audio/filters/fir_rx_decimate_4_min_lpf.c  \
    drivers/audio/filters/fir_rx_interpolate_16.c  \
    drivers/audio/filters/fir_rx_interpolate_16_10kHz.c  \
    drivers/audio/filters/iir_10k.c  \
    drivers/audio/filters/iir_10k_neu.c  \
    drivers/audio/filters/iir_15k_hpf_fm_squelch.c  \
    drivers/audio/filters/iir_1_4k.c  \
    drivers/audio/filters/iir_1_6k.c  \
    drivers/audio/filters/iir_1_8k.c  \
    drivers/audio/filters/iir_2_1k.c  \
    drivers/audio/filters/iir_2_3k.c  \
    drivers/audio/filters/iir_2_5k.c  \
    drivers/audio/filters/iir_2_7k.c  \
    drivers/audio/filters/iir_2_9k.c  \
    drivers/audio/filters/iir_2k7_tx_bpf.c  \
    drivers/audio/filters/iir_2k7_tx_bpf_fm.c  \
    drivers/audio/filters/iir_300hz.c  \
    drivers/audio/filters/iir_3_2k.c  \
    drivers/audio/filters/iir_3_4k.c  \
    drivers/audio/filters/iir_3_6k.c  \
    drivers/audio/filters/iir_3_8k.c  \
    drivers/audio/filters/iir_3k.c  \
    drivers/audio/filters/iir_4_2k.c  \
    drivers/audio/filters/iir_4_4k.c  \
    drivers/audio/filters/iir_4_6k.c  \
    drivers/audio/filters/iir_4_8k.c  \
    drivers/audio/filters/iir_4k.c  \
    drivers/audio/filters/iir_500hz.c  \
    drivers/audio/filters/iir_5_5k.c  \
    drivers/audio/filters/iir_5k.c  \
    drivers/audio/filters/iir_6_5k.c  \
    drivers/audio/filters/iir_6k.c  \
    drivers/audio/filters/iir_7_5k.c  \
    drivers/audio/filters/iir_7k.c  \
    drivers/audio/filters/iir_8_5k.c  \
    drivers/audio/filters/iir_8k.c  \
    drivers/audio/filters/iir_8k5_hpf_fm_squelch.c  \
    drivers/audio/filters/iir_9_5k.c  \
    drivers/audio/filters/iir_9k.c  \
    drivers/audio/filters/iir_antialias.c  \
    drivers/audio/filters/iq_rx_filter.c  \
    drivers/audio/filters/iq_rx_filter_am.c  \
    drivers/audio/filters/iq_tx_filter.c  \
    drivers/audio/softdds/softdds.c  \
    drivers/audio/softdds/dds_table.c  \
    \
    drivers/cat/cat_driver.c  \
    drivers/cat/usb/usbd_bsp.c  \
    drivers/cat/usb/usbd_cdc_vcp.c  \
    drivers/cat/usb/usbd_desc.c  \
    drivers/cat/usb/usbd_usr.c  \
    \
    drivers/diag/Trace.c  \
    drivers/diag/trace_impl.c  \
    \
    drivers/freedv/codebook.c  \
    drivers/freedv/codebookd.c  \
    drivers/freedv/codebookdt.c  \
    drivers/freedv/codebookge.c  \
    drivers/freedv/codebookjnd.c  \
    drivers/freedv/codebookjvm.c  \
    drivers/freedv/codebooklspmelvq.c  \
    drivers/freedv/codebookmel.c  \
    drivers/freedv/codebookres.c  \
    drivers/freedv/codebookvqanssi.c  \
    drivers/freedv/codebookvq.c  \
    drivers/freedv/codec2.c  \
    drivers/freedv/cohpsk.c  \
    drivers/freedv/dump.c  \
    drivers/freedv/fdmdv.c  \
    drivers/freedv/fifo.c  \
    drivers/freedv/fm.c  \
    drivers/freedv/freedv_api.c  \
    drivers/freedv/golay23.c  \
    drivers/freedv/interp.c  \
    drivers/freedv/kiss_fft.c  \
    drivers/freedv/linreg.c  \
    drivers/freedv/lpc.c  \
    drivers/freedv/lsp.c  \
    drivers/freedv/modem_stats.c  \
    drivers/freedv/nlp.c  \
    drivers/freedv/octave.c  \
    drivers/freedv/pack.c  \
    drivers/freedv/phase.c  \
    drivers/freedv/postfilter.c  \
    drivers/freedv/quantise.c  \
    drivers/freedv/sine.c  \
    drivers/freedv/varicode.c  \
    \
    drivers/keyboard/keyb_driver.c  \
    drivers/keyboard/usb/usbh_bsp.c  \
    drivers/keyboard/usb/usbh_usr.c  \
    \
    drivers/ui/encoder/ui_rotary.c  \
    drivers/ui/lcd/ui_lcd_hy28.c  \
    drivers/ui/lcd/ui_lcd_hy28_fonts.c  \
    drivers/ui/lcd/ui_spectrum.c  \
    drivers/ui/oscillator/ui_si570.c  \
    drivers/ui/ui_configuration.c  \
    drivers/ui/ui_driver.c  \
    drivers/ui/ui_menu.c  \
    \
    hardware/mchf_board.c  \
    hardware/mchf_hw_i2c.c  \
    hardware/mchf_hw_i2c2.c  \
    \
    misc/v_eprom/eeprom.c  \
    misc/config_storage.c  \
    misc/serial_eeprom.c  \
    misc/profiling.c  \
    \
    stdio/printf.c  \
    syscalls/syscalls.c  \
    \
    usb/otg/src/usb_core.c  \
    usb/otg/src/usb_dcd.c  \
    usb/otg/src/usb_dcd_int.c  \
    usb/otg/src/usb_hcd.c  \
    usb/otg/src/usb_hcd_int.c  \
    \
    usb/usbd/class/audio/src/usbd_audio_core.c  \
    usb/usbd/class/audio/src/usbd_audio_out_if.c  \
    usb/usbd/class/cdc/src/usbd_cdc_core.c  \
    usb/usbd/core/src/usbd_core.c  \
    usb/usbd/core/src/usbd_ioreq.c  \
    usb/usbd/core/src/usbd_req.c  \
    \
    usb/usbh/class/HID/src/usbh_hid_core.c  \
    usb/usbh/class/HID/src/usbh_hid_keybd.c  \
    usb/usbh/class/HID/src/usbh_hid_mouse.c  \
    usb/usbh/core/src/usbh_core.c  \
    usb/usbh/core/src/usbh_hcs.c  \
    usb/usbh/core/src/usbh_ioreq.c  \
    usb/usbh/core/src/usbh_stdreq.c  \

SRCCXX = \
    misc/TestCPlusPlusBuild.cpp  \

# ------------- nothing to change below this line ---------------------- 

INC_DIRS = $(foreach d, $(SUBDIRS), -I$d)

ifdef SystemRoot  # WINxx
    CC = arm-none-eabi-gcc
    CXX = arm-none-eabi-g++
    OC = arm-none-eabi-objcopy
    OS = arm-none-eabi-size
else
    CC = @${PREFIX}/bin/arm-none-eabi-gcc
    CXX = @${PREFIX}/bin/arm-none-eabi-g++
    OC = @${PREFIX}/bin/arm-none-eabi-objcopy
    OS = @${PREFIX}/bin/arm-none-eabi-size
endif

ECHO = @echo

ifdef SystemRoot  # WINxx
   RM = del /Q
   FixPath = $(subst /,\,$1)
else ifeq ($(shell uname), Linux)
      RM = rm --force
      FixPath = $1
else ifeq ($(shell uname), Darwin)
      RM = rm -f
      FixPath = $1
endif

# how to compile individual object files
OBJS :=
OBJS += $(SRC:.c=.o)
OBJS += $(SRCCXX:.cpp=.o)
.c.o:
	$(ECHO) "  [CC] $@"
	$(CC) $(CFLAGS) -std=gnu11 -c ${INC_DIRS} $< -o $@

%.o: %.cxx
	$(ECHO) "  [CXX] $@"
	$(CXX) $(CFLAGS) $(CXXFLAGS) -std=gnu++11 -c ${INC_DIRS} $< -o $@

%.o: %.cpp
	$(ECHO) "  [CXX] $@"
	$(CXX) $(CFLAGS) $(CXXFLAGS) -std=gnu++11 -c ${INC_DIRS} $< -o $@

# ---------------------------------------------------------
#  BUILT-IN HELP
#

define THISMAKEFILENAME
$(word 2,$Workfile: Makefile $ )
endef

# default (first) make goal
.PHONY: help
help:  
	# shows all make goals of this file (the text you are reading)
	@grep --after-context=1 --extended-regexp '^[[:alnum:]_-]+:[[:blank:]]{2,}' $(THISMAKEFILENAME)

# ---------------------------------------------------------

.PHONY: all clean docs docs-clean help

all:  $(LPRJ).elf $(PRJ).bin
	# compile the ARM-executables .bin and .elf for mcHF SDR TRx, generate .map and .dmp
	@echo "using \c"
	$(CC) --version | grep gcc

# compilation
$(LPRJ).elf:  $(OBJS) $(SRC)
	$(ECHO) "  [LD] $@"
	$(CXX) $(LDFLAGS) -Tarm-gcc-link.ld -Xlinker --gc-sections -Llibs -Wl,-Map,${LPRJ}.map -o${LPRJ}.elf $(OBJS) $(LIBS)

$(PRJ).bin:  $(LPRJ).elf
	$(ECHO) "  [OBJC] $@"
	$(OS) $<
	$(OC) -v -O binary $< $(PRJ).bin

# cleaning rule
clean:  
	# remove the executables, map, dmp and all object files (.o)
	$(RM) $(call FixPath,$(OBJS))
	$(RM) $(call FixPath,$(LPRJ).elf)
	$(RM) $(call FixPath,$(PRJ).bin)
	$(RM) $(call FixPath,$(LPRJ).map)
	$(RM) $(call FixPath,*~)

handy:  
	# rm all .o (but not executables, .map and .dmp)
	$(RM) $(call FixPath,$(OBJS))
	$(RM) $(call FixPath,*~)

gcc-version:  
	# the build will be done using
	$(CC) --version | grep gcc

docs:  
	# generate source docs as per "Doxyfile"
	doxygen Doxyfile

docs-clean:  
	# remove docs
	# as defined in file "Doxyfile" OUTPUT_DIRECTORY
	$(RM) --recursive --verbose $(call FixPath,../docs)

# EOFILE
