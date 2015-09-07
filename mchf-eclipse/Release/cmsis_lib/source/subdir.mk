################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../cmsis_lib/source/misc.o \
../cmsis_lib/source/stm32f4xx_adc.o \
../cmsis_lib/source/stm32f4xx_dac.o \
../cmsis_lib/source/stm32f4xx_dma.o \
../cmsis_lib/source/stm32f4xx_exti.o \
../cmsis_lib/source/stm32f4xx_flash.o \
../cmsis_lib/source/stm32f4xx_fsmc.o \
../cmsis_lib/source/stm32f4xx_gpio.o \
../cmsis_lib/source/stm32f4xx_i2c.o \
../cmsis_lib/source/stm32f4xx_pwr.o \
../cmsis_lib/source/stm32f4xx_rcc.o \
../cmsis_lib/source/stm32f4xx_rtc.o \
../cmsis_lib/source/stm32f4xx_spi.o \
../cmsis_lib/source/stm32f4xx_syscfg.o \
../cmsis_lib/source/stm32f4xx_tim.o \
../cmsis_lib/source/stm32f4xx_usart.o \
../cmsis_lib/source/stm32f4xx_wwdg.o 

C_SRCS += \
../cmsis_lib/source/misc.c \
../cmsis_lib/source/stm32f4xx_adc.c \
../cmsis_lib/source/stm32f4xx_dac.c \
../cmsis_lib/source/stm32f4xx_dma.c \
../cmsis_lib/source/stm32f4xx_exti.c \
../cmsis_lib/source/stm32f4xx_flash.c \
../cmsis_lib/source/stm32f4xx_fsmc.c \
../cmsis_lib/source/stm32f4xx_gpio.c \
../cmsis_lib/source/stm32f4xx_i2c.c \
../cmsis_lib/source/stm32f4xx_pwr.c \
../cmsis_lib/source/stm32f4xx_rcc.c \
../cmsis_lib/source/stm32f4xx_rtc.c \
../cmsis_lib/source/stm32f4xx_spi.c \
../cmsis_lib/source/stm32f4xx_syscfg.c \
../cmsis_lib/source/stm32f4xx_tim.c \
../cmsis_lib/source/stm32f4xx_usart.c \
../cmsis_lib/source/stm32f4xx_wwdg.c 

OBJS += \
./cmsis_lib/source/misc.o \
./cmsis_lib/source/stm32f4xx_adc.o \
./cmsis_lib/source/stm32f4xx_dac.o \
./cmsis_lib/source/stm32f4xx_dma.o \
./cmsis_lib/source/stm32f4xx_exti.o \
./cmsis_lib/source/stm32f4xx_flash.o \
./cmsis_lib/source/stm32f4xx_fsmc.o \
./cmsis_lib/source/stm32f4xx_gpio.o \
./cmsis_lib/source/stm32f4xx_i2c.o \
./cmsis_lib/source/stm32f4xx_pwr.o \
./cmsis_lib/source/stm32f4xx_rcc.o \
./cmsis_lib/source/stm32f4xx_rtc.o \
./cmsis_lib/source/stm32f4xx_spi.o \
./cmsis_lib/source/stm32f4xx_syscfg.o \
./cmsis_lib/source/stm32f4xx_tim.o \
./cmsis_lib/source/stm32f4xx_usart.o \
./cmsis_lib/source/stm32f4xx_wwdg.o 

C_DEPS += \
./cmsis_lib/source/misc.d \
./cmsis_lib/source/stm32f4xx_adc.d \
./cmsis_lib/source/stm32f4xx_dac.d \
./cmsis_lib/source/stm32f4xx_dma.d \
./cmsis_lib/source/stm32f4xx_exti.d \
./cmsis_lib/source/stm32f4xx_flash.d \
./cmsis_lib/source/stm32f4xx_fsmc.d \
./cmsis_lib/source/stm32f4xx_gpio.d \
./cmsis_lib/source/stm32f4xx_i2c.d \
./cmsis_lib/source/stm32f4xx_pwr.d \
./cmsis_lib/source/stm32f4xx_rcc.d \
./cmsis_lib/source/stm32f4xx_rtc.d \
./cmsis_lib/source/stm32f4xx_spi.d \
./cmsis_lib/source/stm32f4xx_syscfg.d \
./cmsis_lib/source/stm32f4xx_tim.d \
./cmsis_lib/source/stm32f4xx_usart.d \
./cmsis_lib/source/stm32f4xx_wwdg.d 


# Each subdirectory must supply rules for building sources it contributes
cmsis_lib/source/%.o: ../cmsis_lib/source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -D__FPU_USED -D__FPU_PRESENT -DUSE_STDPERIPH_DRIVER -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


