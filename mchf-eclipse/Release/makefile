################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include usb/usbh/core/src/subdir.mk
-include usb/usbh/class/HID/src/subdir.mk
-include usb/usbd/core/src/subdir.mk
-include usb/usbd/class/cdc/src/subdir.mk
-include usb/usbd/class/audio/src/subdir.mk
-include usb/otg/src/subdir.mk
-include syscalls/subdir.mk
-include stdio/subdir.mk
-include misc/v_eprom/subdir.mk
-include hardware/subdir.mk
-include firmware_Target_Flash/Debug/obj/subdir.mk
-include firmware_Target_Flash/Debug/bin/subdir.mk
-include drivers/ui/oscillator/subdir.mk
-include drivers/ui/lcd/subdir.mk
-include drivers/ui/encoder/subdir.mk
-include drivers/ui/subdir.mk
-include drivers/keyboard/usb/subdir.mk
-include drivers/keyboard/subdir.mk
-include drivers/cat/usb/subdir.mk
-include drivers/cat/subdir.mk
-include drivers/audio/softdds/subdir.mk
-include drivers/audio/cw/subdir.mk
-include drivers/audio/codec/subdir.mk
-include drivers/audio/subdir.mk
-include cmsis_lib/source/subdir.mk
-include cmsis_boot/startup/subdir.mk
-include cmsis_boot/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
mchf-eclipse.hex \

SECONDARY_SIZE += \
mchf-eclipse.siz \


# All Target
all: mchf-eclipse.elf secondary-outputs

# Tool invocations
mchf-eclipse.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -Xlinker --gc-sections -Wl,-Map,"mchf-eclipse.map" -o "mchf-eclipse.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

mchf-eclipse.hex: mchf-eclipse.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex "mchf-eclipse.elf"  "mchf-eclipse.hex"
	@echo 'Finished building: $@'
	@echo ' '

mchf-eclipse.siz: mchf-eclipse.elf
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "mchf-eclipse.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SECONDARY_SIZE)$(OBJS)$(C_DEPS)$(ASM_DEPS)$(SECONDARY_FLASH)$(S_UPPER_DEPS) mchf-eclipse.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
