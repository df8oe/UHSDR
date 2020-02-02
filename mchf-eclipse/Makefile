#
# Author: Harald Baumgart DL4SAI
# everybody may  copy, use or modify this file
# there is no guarantee for anything by the author
#
# HB 20.8.2015
#
# Rev. 09/08/2015 corrected by Andreas Richter DF8OE 
# Rev. 2016-04-06 cleanup - Stephan HB9ocq
# Rev. 06/12/2016 possibility of choosing individual toolchain - Andreas Richter DF8OE
# Rev. 2017-01-06 HB9ocq - added versioning of build: extracted from source, propagated to env.vars
# Rev. 20/11/2018 RV9YW (m-chichikalov) - fixed limitation of parameter length on cmd.exe windows by passing all obj. as one file to linker

#  -*- makefile -*-
# if you want to build in a different directory than this one
# go to the directory, figure out the path from the new diretory you are in to this one and call make like this
# make -f replace_with_pfad_to_makefile/Makefile ROOTLOC=replace_with_pfad_to_makefile desired_maketarget
# for example to build the bootloader in the build-bl folder inside this directory
# make -f ../Makefile ROOTLOC=".." bootloader 

# to invoke asserts into the build use make with DEBUG=1; -> make -f "../Makefile" ROOTLOC=".." DEBUG=1 all

# you can set the MCU you want to build for here or via environment variable (F4 or F4-512KB or F7 or H7)
ifndef BUILDFOR
  BUILDFOR=F4
endif

# default value for building the mcHF UI board.
ifndef CONFIGFLAGS
  CONFIGFLAGS=-DUI_BRD_MCHF -DRF_BRD_MCHF
endif

# We have to use "override" since we pass some of these things from
# the commandline but then change / append to it
# see https://www.gnu.org/software/make/manual/html_node/Override-Directive.html#Override-Directive
# BUILDFOR, CONFIGFLAGS are affected by this


# F4-512KB results in "small build for F4"
ifeq ($(BUILDFOR),F4-512KB)
  FW_LINKERFILE_F4=arm-gcc-link_f4_flash512k.ld
  override BUILDFOR=F4
  override CONFIGFLAGS +=-DIS_SMALL_BUILD
else
  FW_LINKERFILE_F4=arm-gcc-link_f4_flash1024k.ld
endif

ifdef DEBUG
  override CONFIGFLAGS += -DDEBUG -DUSE_FULL_ASSERT -DTRACE
else
  override CONFIGFLAGS += -DNDEBUG
endif

ifeq ($(BUILDFOR),F7)
  TRX_NAME="OVI40 F7"
endif
ifeq ($(BUILDFOR),H7)
  TRX_NAME="OVI40 H7"
endif

ROOTLOC=.
vpath %.c $(ROOTLOC)
vpath %.cpp $(ROOTLOC)
vpath %.S $(ROOTLOC)

# you can set these environment to your individual values here or via environment variables
# TRX_ID maximum lenght is 5 characters!!
ifndef TRX_ID
  TRX_ID=mchf
endif
ifeq ($(TRX_NAME),mcHF)
  TRX_NAME='mcHF QRP'
endif
ifndef TRX_NAME
  TRX_NAME='mcHF QRP'
endif

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
  PREFIX = $(OPT_GCC_ARM)/bin/
else
  PREFIX = 
endif

# Under MacOS we have to use gsed instead of sed
# This mechanism can be used also for other flavours
OS := $(shell uname)
SED = sed
ifeq ($(OS),Darwin)
  SED = gsed
endif

COMPILEFLAGS := -D_GNU_SOURCE -DTRX_ID=\"$(TRX_ID)\" -DTRX_NAME=\"$(TRX_NAME)\" $(CONFIGFLAGS) -DUSE_HAL_DRIVER\
	-DFDV_ARM_MATH -DFREEDV_MODE_EN_DEFAULT=0 -DFREEDV_MODE_1600_EN=1 -DCODEC2_MODE_EN_DEFAULT=0 -DCODEC2_MODE_1300_EN=1\
	-ffunction-sections -fdata-sections -flto -Wall -Wuninitialized -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-sign-compare -g3

# identifying of "official builds by DF8OE"
ifneq (,$(wildcard ../DF8OE))
  COMPILEFLAGS += -DOFFICIAL_BUILD=\"1\"
endif

# compilation options
MACHFLAGS_F4 := -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -DARM_MATH_CM4 -DCORTEX_M4 -DSTM32F407xx -D__FPU_PRESENT=1U
MACHFLAGS_F7 := -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16    -mthumb -DARM_MATH_CM7 -DCORTEX_M7 -DSTM32F767xx -D__FPU_PRESENT=1
MACHFLAGS_H7 := -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16    -mthumb -DARM_MATH_CM7 -DCORTEX_M7 -DSTM32H743xx -D__FPU_PRESENT=1

BASECFLAGS_F4  = $(MACHFLAGS_F4)  $(COMPILEFLAGS) $(EXTRACFLAGS)
BASECFLAGS_F7  = $(MACHFLAGS_F7)  $(COMPILEFLAGS) $(EXTRACFLAGS) -DFREEDV_MODE_700D_EN=1 -DCODEC2_MODE_700C_EN=1
BASECFLAGS_H7  = $(MACHFLAGS_H7)  $(COMPILEFLAGS) $(EXTRACFLAGS) -DFREEDV_MODE_700D_EN=1 -DCODEC2_MODE_700C_EN=1

LINKERLOC=$(ROOTLOC)/linker

LDFLAGS_BASE = -L$(LINKERLOC) -flto -g3 $(CFLAGS) $(CXXFLAGS)
LDFLAGS_F4 = $(LDFLAGS_BASE) $(MACHFLAGS_F4)
LDFLAGS_F7 = $(LDFLAGS_BASE) $(MACHFLAGS_F7) 
LDFLAGS_H7 = $(LDFLAGS_BASE) $(MACHFLAGS_H7) 

LIBS :=--specs=nosys.specs --specs=nano.specs

# ------------- nothing to change below this line ---------------------- 

# propagate version info from source to environment variables
PROJECT_VERSION_FILE := $(ROOTLOC)/src/uhsdr_version.h

ifdef MSYSTEM
$(eval  $(shell $(SED) -n -e \'s/\"//g\' -e '/[^-]UHSDR_VER_MAJOR/{s!\#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e \'s/\"//g\' -e '/[^.]UHSDR_VER_MINOR/{s!\#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e \'s/\"//g\' -e '/[^.]UHSDR_VER_RELEASE/{s!\#define\s*!!; s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
else
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/UHSDR_VER_MAJOR/{s!#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/UHSDR_VER_MINOR/{s!#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/UHSDR_VER_RELEASE/{s!#define\s*!!; s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
endif

UHSDR_VER_TAINT := $(shell  git status . | grep --quiet 'working directory clean' && echo "" || echo "+")

BOOTLOADER=bl-$(TRX_ID)
FIRMWARE=fw-$(TRX_ID)

# Every subdirectory with header files must be mentioned here
include $(ROOTLOC)/include.mak

# every source-file has to be mentioned here 
include $(ROOTLOC)/files.mak

include $(ROOTLOC)/bootloader.mak

ifeq ($(BUILDFOR),F4)

$(BOOTLOADER).elf : CFLAGS = ${BASECFLAGS_F4} -Os -DBOOTLOADER_BUILD
$(FIRMWARE).elf : CFLAGS = ${BASECFLAGS_F4} -O2
$(BOOTLOADER).elf : LDFLAGS = ${LDFLAGS_F4} -T$(LINKERLOC)/arm-gcc-link-bootloader_f4.ld
$(FIRMWARE).elf : LDFLAGS = ${LDFLAGS_F4} -T$(LINKERLOC)/$(FW_LINKERFILE_F4)

	# Every subdirectory with header files must be mentioned here
	include $(ROOTLOC)/f4-include.mak

	# every source-file has to be mentioned here 
	include $(ROOTLOC)/f4-files.mak

	include $(ROOTLOC)/f4-bootloader.mak

endif
ifeq ($(BUILDFOR),F7)

$(BOOTLOADER).elf: CFLAGS = ${BASECFLAGS_F7} -Os -DBOOTLOADER_BUILD
$(FIRMWARE).elf : CFLAGS = ${BASECFLAGS_F7} -O2
$(BOOTLOADER).elf : LDFLAGS = ${LDFLAGS_F7} -T$(LINKERLOC)/arm-gcc-link-bootloader_f7.ld
$(FIRMWARE).elf : LDFLAGS = ${LDFLAGS_F7} -T$(LINKERLOC)/arm-gcc-link_f7.ld

	# Every subdirectory with header files must be mentioned here
	include $(ROOTLOC)/f7-include.mak

	# every source-file has to be mentioned here 
	include $(ROOTLOC)/f7-files.mak

	include $(ROOTLOC)/f7-bootloader.mak

endif
ifeq ($(BUILDFOR),H7)

$(BOOTLOADER).elf: CFLAGS = ${BASECFLAGS_H7} -Os -DBOOTLOADER_BUILD
$(FIRMWARE).elf : CFLAGS = ${BASECFLAGS_H7} -O2
$(BOOTLOADER).elf : LDFLAGS = ${LDFLAGS_H7} -T$(LINKERLOC)/arm-gcc-link-bootloader_h7.ld
$(FIRMWARE).elf : LDFLAGS = ${LDFLAGS_H7} -T$(LINKERLOC)/arm-gcc-link_h7.ld

	# Every subdirectory with header files must be mentioned here
	include $(ROOTLOC)/h7-include.mak

	# every source-file has to be mentioned here 
	include $(ROOTLOC)/h7-files.mak

	include $(ROOTLOC)/h7-bootloader.mak

endif

INC_DIRS = $(foreach d, $(SUBDIRS) $(HAL_SUBDIRS), -I$(ROOTLOC)/$d)

ifdef VERBOSE
	VPRE:=
else
	VPRE:=@
endif

CC      = $(VPRE)$(PREFIX)arm-none-eabi-gcc
CXX     = $(VPRE)$(PREFIX)arm-none-eabi-g++
OC      = $(VPRE)$(PREFIX)arm-none-eabi-objcopy
SZ      = $(VPRE)$(PREFIX)arm-none-eabi-size
HEX2DFU = $(VPRE)python3 $(ROOTLOC)/support/hex2dfu/hex2dfu.py

POSTCOMPILE = @mv -f $*.Td $*.d && touch $@
DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td


ECHO = @echo

ifdef SystemRoot  # WINxx
    RM = del /Q
    FixPath = $(subst /,\,$1)
else
    RM = rm -f
    FixPath = $1
endif

# how to compile individual object files
ALL_SRC:=$(SRC) $(DSPLIB_SRC) $(HAL_SRC) $(BL_SRC) $(BL_HAL_SRC)

OBJS := $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRC:.cpp=.o)))
DSPLIB_OBJS := $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(DSPLIB_SRC:.cpp=.o)))
HAL_OBJS := $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(HAL_SRC:.cpp=.o)))
BL_OBJS := $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(BL_SRC:.c=.o)))
BL_HAL_OBJS := $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(BL_HAL_SRC:.c=.o)))

ALL_OBJS := $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(ALL_SRC:.cpp=.o)))


$(DSPLIB_OBJS): EXTRA_CFLAGS:= -Wno-strict-aliasing
basesw/%/usbh_mtp.o: CFLAGS += -Wno-implicit-fallthrough
basesw/%/usbh_msc.o: CFLAGS += -Wno-implicit-fallthrough
drivers/usb/%/usbh_hid.o: CFLAGS += -Wno-implicit-fallthrough
basesw/%stm32h7xx_hal_spi.o: CFLAGS += -Wno-strict-aliasing
basesw/%stm32f4xx_ll_usb.o: CFLAGS += -Wno-attributes
basesw/%stm32f7xx_ll_usb.o: CFLAGS += -Wno-attributes
basesw/%stm32h7xx_ll_usb.o: CFLAGS += -Wno-attributes

.S.o: %.d
	$(ECHO) "  [CC] $@"
	@mkdir -p $(subst $(ROOTLOC)/,,$(shell dirname $<))
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEPFLAGS) -std=gnu11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

.c.o: %.d
	$(ECHO) "  [CC] $@"
	@mkdir -p $(subst $(ROOTLOC)/,,$(shell dirname $<))
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(DEPFLAGS) -std=gnu11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

.cxx.o: %.d
	$(ECHO) "  [CXX] $@"
	@mkdir -p $(subst $(ROOTLOC)/,,$(shell dirname $<))
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPFLAGS) -std=gnu++11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

.cpp.o: %.d
	$(ECHO) "  [CXX] $@"
	@mkdir -p $(subst $(ROOTLOC)/,,$(shell dirname $<))
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPFLAGS) -std=gnu++11 -c ${INC_DIRS} $< -o $@
	$(POSTCOMPILE)

%.bin: %.elf
	$(ECHO) "  [OBJC] $@"
	$(SZ) $<
	$(OC) -v -O binary $< $@

%.hex: %.elf
	$(ECHO) "  [BIN] $@"
	$(SZ) $<
	$(OC) -v -O ihex $< $@

%.dfu: %.hex
	$(ECHO) "  [H2D] $@"
	$(SZ) $<
	$(HEX2DFU) $< $@

%.d: ;

.PRECIOUS: %.d

include $(wildcard $(patsubst %,%.d,$(basename $(ALL_SRC))))

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


all:  firmware $(TRX_ID).handbook
	# compile the firmware ARM-executables .bin / .elf and .dfu, generate .map 

firmware:  $(FIRMWARE) 
	# compile the firmware ARM-executables .bin / .elf and .dfu, generate .map 
	@echo "using \c"
	$(CC) --version | grep gcc

bootloader:  $(BOOTLOADER)
	# compile the bootloader ARM-executables .bin / .elf and .dfu, generate .map 
	@echo "using \c"
	$(CC) --version | grep gcc

$(FIRMWARE): $(FIRMWARE).elf $(FIRMWARE).dfu $(FIRMWARE).bin

$(BOOTLOADER):  $(BOOTLOADER).bin $(BOOTLOADER).dfu
	# compile the bootloader ARM-executables .bin / .elf and .dfu for mcHF SDR TRx, generate .map and .dmp

# compilation
$(FIRMWARE).elf:  $(HAL_OBJS) $(DSPLIB_OBJS) $(OBJS)
	$(ECHO) "  [LD] $@"
	@$(file > ./firmware_obj_list.lst,$^)
	$(CXX) $(LDFLAGS) -Xlinker --gc-sections -Llibs -Wl,-Map,$(FIRMWARE).map -o$@ @./firmware_obj_list.lst $(LIBS)
	$(RM) $(call FixPath,firmware_obj_list.lst)

# compilation
$(BOOTLOADER).elf:  $(BL_HAL_OBJS) $(BL_OBJS)
	$(ECHO) "  [LD] $@"
	@$(file > ./bootloader_obj_list.lst,$^)
	$(CXX) $(LDFLAGS) -Xlinker --gc-sections -Llibs -Wl,-Map,$(BOOTLOADER).map -o$@ @./bootloader_obj_list.lst $(LIBS)
	$(RM) $(call FixPath,bootloader_obj_list.lst)

$(TRX_ID).handbook:
	@$(ROOTLOC)/support/ui/menu/mk-menu-handbook auto

$(TRX_ID).version:
	# the build artifacts SHOULD identify as
	@printf "Version %s.%s.%s%s\n" $(UHSDR_VER_MAJOR) $(UHSDR_VER_MINOR) $(UHSDR_VER_RELEASE) $(UHSDR_VER_TAINT)

# cleaning rule
clean-firmware:  
	# remove the firmware executables, map, dmp and all object files (.o) (except libs object files)
	$(RM) $(call FixPath,$(OBJS))	
	$(RM) $(call FixPath,$(FIRMWARE).elf)
	$(RM) $(call FixPath,$(FIRMWARE).dfu)
	$(RM) $(call FixPath,$(FIRMWARE).bin)
	$(RM) $(call FixPath,$(FIRMWARE).map)

clean-bootloader:  
	# remove the bootloader executables, map, dmp and all object files (.o) (except libs object files)
	$(RM) $(call FixPath,$(BL_OBJS))
	$(RM) $(call FixPath,$(BL_HAL_OBJS))
	$(RM) $(call FixPath,$(BOOTLOADER).elf)
	$(RM) $(call FixPath,$(BOOTLOADER).dfu)
	$(RM) $(call FixPath,$(BOOTLOADER).bin)
	$(RM) $(call FixPath,$(BOOTLOADER).map)

clean-libs:  
	# remove libs object files (.o)
	$(RM) $(call FixPath,$(DSPLIB_OBJS))
	$(RM) $(call FixPath,$(HAL_OBJS))

clean:  clean-firmware clean-bootloader clean-libs
	# remove the executables, map, dmp and all object files (.o)
	$(RM) $(call FixPath,*~)

docs:  
	# generate source docs as per "Doxyfile"
	doxygen Doxyfile

docs-clean:  
	# remove docs
	# as defined in file "Doxyfile" OUTPUT_DIRECTORY
	$(RM) --recursive --verbose $(call FixPath,$(ROOTLOC)/../docs)

gcc-version:  
	# the build will be done using
	$(CC) --version | grep gcc

handbook-test:  
	# extract UI Menu Descriptor data from source code and generate graph + table for handbook in different directory for test purposes
	@$(ROOTLOC)/support/ui/menu/mk-menu-handbook test

handbook-ui-menu:  
	# extract UI Menu Descriptor data from source code and generate graph + table for handbook
	@$(ROOTLOC)/support/ui/menu/mk-menu-handbook

handbook-ui-menu-clean:  
	# remove generated UI Menu files
	$(RM) $(ROOTLOC)/support/ui/menu/uhsdr-logo.png
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure.py*
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure_graph.gv
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure_graph.svg
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure_graph.png
	$(RM) $(ROOTLOC)/support/ui/menu/ui_menu_structure_mdtable.md
	$(RM) $(ROOTLOC)/support/ui/menu/menu-handbook-build.timestamp

handy:  
	# rm all .o (but not executables, .map and .dmp)
	$(RM) $(call FixPath,$(ALL_OBJS))
	$(RM) $(call FixPath,*~)

release:  
	# generate quick operating guide
	/@inkscape --export-png=$(ROOTLOC)/useful_manuals/mcHF-quick-manual.png $(ROOTLOC)/useful_manuals/mcHF-quick-manual.svg
	@inkscape --export-pdf=$(ROOTLOC)/useful_manuals/mcHF-quick-manual.pdf $(ROOTLOC)/useful_manuals/mcHF-quick-manual.svg

# EOFILE
