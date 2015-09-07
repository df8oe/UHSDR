################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../cmsis_boot/startup/startup_stm32f4xx.o 

C_SRCS += \
../cmsis_boot/startup/startup_stm32f4xx.c 

OBJS += \
./cmsis_boot/startup/startup_stm32f4xx.o 

C_DEPS += \
./cmsis_boot/startup/startup_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
cmsis_boot/startup/%.o: ../cmsis_boot/startup/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -D__FPU_USED -D__FPU_PRESENT -DUSE_STDPERIPH_DRIVER -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


