################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../drivers/keyboard/usb/usbh_bsp.o \
../drivers/keyboard/usb/usbh_usr.o 

C_SRCS += \
../drivers/keyboard/usb/usbh_bsp.c \
../drivers/keyboard/usb/usbh_usr.c 

OBJS += \
./drivers/keyboard/usb/usbh_bsp.o \
./drivers/keyboard/usb/usbh_usr.o 

C_DEPS += \
./drivers/keyboard/usb/usbh_bsp.d \
./drivers/keyboard/usb/usbh_usr.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/keyboard/usb/%.o: ../drivers/keyboard/usb/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -D__FPU_USED -D__FPU_PRESENT -DUSE_STDPERIPH_DRIVER -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


