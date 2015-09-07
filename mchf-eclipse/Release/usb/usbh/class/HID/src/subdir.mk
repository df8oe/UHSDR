################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../usb/usbh/class/HID/src/usbh_hid_core.o \
../usb/usbh/class/HID/src/usbh_hid_keybd.o \
../usb/usbh/class/HID/src/usbh_hid_mouse.o 

C_SRCS += \
../usb/usbh/class/HID/src/usbh_hid_core.c \
../usb/usbh/class/HID/src/usbh_hid_keybd.c \
../usb/usbh/class/HID/src/usbh_hid_mouse.c 

OBJS += \
./usb/usbh/class/HID/src/usbh_hid_core.o \
./usb/usbh/class/HID/src/usbh_hid_keybd.o \
./usb/usbh/class/HID/src/usbh_hid_mouse.o 

C_DEPS += \
./usb/usbh/class/HID/src/usbh_hid_core.d \
./usb/usbh/class/HID/src/usbh_hid_keybd.d \
./usb/usbh/class/HID/src/usbh_hid_mouse.d 


# Each subdirectory must supply rules for building sources it contributes
usb/usbh/class/HID/src/%.o: ../usb/usbh/class/HID/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -D__FPU_USED -D__FPU_PRESENT -DUSE_STDPERIPH_DRIVER -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


