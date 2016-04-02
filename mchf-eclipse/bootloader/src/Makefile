
SRCS=main.c command.c flash_if.c bootloader_it.c system_bootloader.c usb_bsp.c usbh_usr.c

# Library modules
SRCS += stm32f4xx_exti.c stm32f4xx_flash.c stm32f4xx_gpio.c stm32f4xx_rcc.c 
SRCS += stm32f4xx_syscfg.c stm32f4xx_tim.c misc.c
SRCS += mchf.c
SRCS += usb_core.c usb_hcd.c usb_hcd_int.c
SRCS += usbh_core.c usbh_hcs.c usbh_ioreq.c usbh_msc_bot.c usbh_msc_core.c 
SRCS += usbh_msc_fatfs.c usbh_msc_scsi.c usbh_stdreq.c
SRCS += fattime.c ff.c

# Binaries will be generated with this name (.elf, .bin, .hex, etc)
PROJ_NAME=bootloader

#######################################################################################

STM_COMMON=../

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS  = -Os -flto -ffunction-sections -fdata-sections -Wall -Tbootloader.ld -Wl,--gc-sections -Wl,--strip-all
CFLAGS += -DUSE_STDPERIPH_DRIVER
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -I../inc

# Choose fullspeed or highspeed (?)
CFLAGS += -DUSE_USB_OTG_FS
#CFLAGS += -DUSE_USB_OTG_HS -DUSE_EMBEDDED_PHY

# Include files from STM libraries
CFLAGS += -I$(STM_COMMON)/../drivers/fat_fs/inc
CFLAGS += -I$(STM_COMMON)/../cmsis 
CFLAGS += -I$(STM_COMMON)/../cmsis_lib/include
CFLAGS += -I$(STM_COMMON)/Libraries/stm32_usb_host_library/Core/inc
CFLAGS += -I$(STM_COMMON)/Libraries/stm32_usb_host_library/Class/MSC/inc
CFLAGS += -I$(STM_COMMON)/Libraries/stm32_usb_otg_driver/inc
#CFLAGS += -I$(STM_COMMON)/../usb/otg/inc
#CFLAGS += -I$(STM_COMMON)/../usb/usbh/core/inc
#CFLAGS += -I$(STM_COMMON)/../usb/usbh/class/MSC/inc


# add startup file to build
SRCS += startup_bootloader.s 

OBJS = $(SRCS:.c=.o)


vpath %.c $(STM_COMMON)/../cmsis_lib/source \
          $(STM_COMMON)/Libraries/stm32_usb_otg_driver/src \
          $(STM_COMMON)/Libraries/stm32_usb_host_library/Core/src \
          $(STM_COMMON)/Libraries/stm32_usb_host_library/Class/MSC/src \
          $(STM_COMMON)/../drivers/fat_fs/src
#          $(STM_COMMON)/../usb/otg/src \
#          $(STM_COMMON)/../usb/usbh/core/src \
#          $(STM_COMMON)/../usb/usbh/class/MSC/src \

.PHONY: proj

all: proj

proj: $(PROJ_NAME).bin $(PROJ_NAME).dfu

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@

$(PROJ_NAME).hex: $(PROJ_NAME).elf
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex

$(PROJ_NAME).bin: $(PROJ_NAME).elf
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

$(PROJ_NAME).dfu: $(PROJ_NAME).hex
	../hex2dfu/hex2dfu.py $(PROJ_NAME).hex

clean:
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).dfu

# Flash the STM32F4
burn: proj
	st-flash write $(PROJ_NAME).bin 0x8000000
#	$(STLINK)/st-flash write $(PROJ_NAME).bin 0x8000000
