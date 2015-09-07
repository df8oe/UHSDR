################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../drivers/cat/usb/usbd_bsp.o \
../drivers/cat/usb/usbd_cdc_vcp.o \
../drivers/cat/usb/usbd_desc.o \
../drivers/cat/usb/usbd_usr.o 

C_SRCS += \
../drivers/cat/usb/usbd_bsp.c \
../drivers/cat/usb/usbd_cdc_vcp.c \
../drivers/cat/usb/usbd_desc.c \
../drivers/cat/usb/usbd_usr.c 

OBJS += \
./drivers/cat/usb/usbd_bsp.o \
./drivers/cat/usb/usbd_cdc_vcp.o \
./drivers/cat/usb/usbd_desc.o \
./drivers/cat/usb/usbd_usr.o 

C_DEPS += \
./drivers/cat/usb/usbd_bsp.d \
./drivers/cat/usb/usbd_cdc_vcp.d \
./drivers/cat/usb/usbd_desc.d \
./drivers/cat/usb/usbd_usr.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/cat/usb/%.o: ../drivers/cat/usb/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -D__FPU_USED -D__FPU_PRESENT -DUSE_STDPERIPH_DRIVER -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


