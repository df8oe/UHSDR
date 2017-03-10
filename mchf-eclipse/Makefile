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
# Rev. 2017-01-06 HB9ocq - added versioning of build: extracted from source, propagated to env.vars

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

# Under MacOS we have to use gsed instead of sed
# This mechanism can be used also for other flavours
OS := $(shell uname)
SED = sed
ifeq ($(OS),Darwin)
  SED = gsed
endif

# compilation options
MACHFLAGS := -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb
BASECFLAGS  = $(MACHFLAGS)  \
          -DARM_MATH_CM4 -D_GNU_SOURCE -DCORTEX_M4 -DSTM32F407xx -DUSE_HAL_DRIVER -DDEBUG -DUSE_FULL_ASSERT -DTRACE  \
          -ffunction-sections -fdata-sections -flto -O2 -Wall -Wno-unused-function -D__FPU_PRESENT=1U \
          $(EXTRACFLAGS)


# propagate version info from source to environment variables
PROJECT_VERSION_FILE := src/mchf_version.h
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/TRX4M_VER_MAJOR/{s!#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/TRX4M_VER_MINOR/{s!#define\s*!!;   s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
$(eval  $(shell $(SED) -n -e 's/"//g' -e '/TRX4M_VER_RELEASE/{s!#define\s*!!; s!\s\s*!=!p}' $(PROJECT_VERSION_FILE) ))
TRX4M_VER_TAINT := $(shell  git status . | grep --quiet 'working directory clean' && echo "" || echo "+")

LDFLAGS := $(MACHFLAGS) -flto --specs=nano.specs

LIBS := -lm -lc -lgcc -lnosys \
        -L$(PREFIX)/arm-none-eabi/lib/armv7e-m/fpu 

bootloader.elf : CFLAGS = ${BASECFLAGS} -DBOOTLOADER_BUILD
mchf.elf : CFLAGS = ${BASECFLAGS}

# Every subdirectory with header files must be mentioned here
include include.mak

# every source-file has to be mentioned here 


include files.mak

include bootloader.mak

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
else ifeq ($(shell uname), CYGWIN_NT-10.0)
    RM = rm -f
    FixPath = $1
endif

# how to compile individual object files
OBJS := $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRC:.cpp=.o)))
BLOBJS := $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(BLSRC:.c=.o)))

.c.o:
	$(ECHO) "  [CC] $@"
	$(CC) $(CFLAGS) -std=gnu11 -c ${INC_DIRS} $< -o $@

.cxx.o:
	$(ECHO) "  [CXX] $@"
	$(CXX) $(CFLAGS) $(CXXFLAGS) -std=gnu++11 -c ${INC_DIRS} $< -o $@

.cpp.o:
	$(ECHO) "  [CXX] $@"
	$(CXX) $(CFLAGS) $(CXXFLAGS) -std=gnu++11 -c ${INC_DIRS} $< -o $@

%.bin: %.elf
	$(ECHO) "  [OBJC] $@"
	$(OS) $<
	$(OC) -v -O binary $< $@

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

bootloader: bootloader.bin bootloader.elf


all:  $(LPRJ).elf $(PRJ).bin $(PRJ).handbook
	# compile the ARM-executables .bin and .elf for mcHF SDR TRx, generate .map and .dmp
	@echo "using \c"
	$(CC) --version | grep gcc

# compilation
$(LPRJ).elf:  $(OBJS) 
	$(ECHO) "  [LD] $@"
	$(CXX) $(LDFLAGS) -Tarm-gcc-link.ld -Xlinker --gc-sections -Llibs -Wl,-Map,${LPRJ}.map -o$@ $(OBJS) $(LIBS)

# compilation
bootloader.elf:  $(BLOBJS) 
	$(ECHO) "  [LD] $@"
	$(CXX) $(LDFLAGS) -Tarm-gcc-link-bootloader.ld -Xlinker --gc-sections -Llibs -Wl,-Map,$@.map -o$@ $(BLOBJS) $(LIBS)


$(PRJ).handbook:
	@support/ui/menu/mk-menu-handbook auto

$(PRJ).version:
	# the build artifacts SHOULD identify as
	@printf "Version %s.%s.%s%s\n" $(TRX4M_VER_MAJOR) $(TRX4M_VER_MINOR) $(TRX4M_VER_RELEASE) $(TRX4M_VER_TAINT)

# cleaning rule
clean:  
	# remove the executables, map, dmp and all object files (.o)
	$(RM) $(call FixPath,$(OBJS))
	$(RM) $(call FixPath,$(BLOBJS))
	$(RM) $(call FixPath,$(LPRJ).elf)
	$(RM) $(call FixPath,$(PRJ).bin)
	$(RM) $(call FixPath,$(LPRJ).map)
	$(RM) $(call FixPath,bootloader.elf)
	$(RM) $(call FixPath,bootloader.elf.map)
	$(RM) $(call FixPath,bootloader.bin)
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

handbook-ui-menu:  
	# extract UI Menu Descriptor data from source code and generate graph + table for handbook
	@support/ui/menu/mk-menu-handbook

handbook-test:  
	# extract UI Menu Descriptor data from source code and generate graph + table for handbook in different directory for test purposes
	@support/ui/menu/mk-menu-handbook test

handbook-ui-menu-clean:  
	# remove generated UI Menu files
	$(RM) support/ui/menu/mcHF-logo.png
	$(RM) support/ui/menu/ui_menu_structure.py*
	$(RM) support/ui/menu/ui_menu_structure_graph.gv
	$(RM) support/ui/menu/ui_menu_structure_graph.svg
	$(RM) support/ui/menu/ui_menu_structure_graph.png
	$(RM) support/ui/menu/ui_menu_structure_mdtable.md
	$(RM) support/ui/menu/menu-handbook-build.timestamp

release:  
	# generate quick operating guide
	@inkscape --export-png=useful_manuals/mcHF-quick-manual.png useful_manuals/mcHF-quick-manual.svg
	@inkscape --export-pdf=useful_manuals/mcHF-quick-manual.pdf useful_manuals/mcHF-quick-manual.svg

# EOFILE
