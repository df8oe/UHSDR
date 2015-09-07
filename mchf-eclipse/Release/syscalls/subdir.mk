################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../syscalls/syscalls.o 

C_SRCS += \
../syscalls/syscalls.c 

OBJS += \
./syscalls/syscalls.o 

C_DEPS += \
./syscalls/syscalls.d 


# Each subdirectory must supply rules for building sources it contributes
syscalls/%.o: ../syscalls/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -D__FPU_USED -D__FPU_PRESENT -DUSE_STDPERIPH_DRIVER -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


