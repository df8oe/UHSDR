################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../misc/v_eprom/eeprom.o 

C_SRCS += \
../misc/v_eprom/eeprom.c 

OBJS += \
./misc/v_eprom/eeprom.o 

C_DEPS += \
./misc/v_eprom/eeprom.d 


# Each subdirectory must supply rules for building sources it contributes
misc/v_eprom/%.o: ../misc/v_eprom/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -D__FPU_USED -D__FPU_PRESENT -DUSE_STDPERIPH_DRIVER -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


