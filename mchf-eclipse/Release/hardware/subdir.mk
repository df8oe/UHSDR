################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../hardware/mchf_board.o \
../hardware/mchf_hw_i2c.o \
../hardware/mchf_hw_i2c2.o 

C_SRCS += \
../hardware/mchf_board.c \
../hardware/mchf_hw_i2c.c \
../hardware/mchf_hw_i2c2.c 

OBJS += \
./hardware/mchf_board.o \
./hardware/mchf_hw_i2c.o \
./hardware/mchf_hw_i2c2.o 

C_DEPS += \
./hardware/mchf_board.d \
./hardware/mchf_hw_i2c.d \
./hardware/mchf_hw_i2c2.d 


# Each subdirectory must supply rules for building sources it contributes
hardware/%.o: ../hardware/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -ffunction-sections  -g -DARM_MATH_CM4 -DSTM32F407VG -DSTM32F4XX -D__FPU_USED -D__FPU_PRESENT -DUSE_STDPERIPH_DRIVER -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


