################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Benchmark/bench_sqlite3.c 

OBJS += \
./Core/Src/Benchmark/bench_sqlite3.o 

C_DEPS += \
./Core/Src/Benchmark/bench_sqlite3.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Benchmark/%.o Core/Src/Benchmark/%.su: ../Core/Src/Benchmark/%.c Core/Src/Benchmark/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O3 -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -Wno-unused-function -Wno-comment -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Benchmark

clean-Core-2f-Src-2f-Benchmark:
	-$(RM) ./Core/Src/Benchmark/bench_sqlite3.d ./Core/Src/Benchmark/bench_sqlite3.o ./Core/Src/Benchmark/bench_sqlite3.su

.PHONY: clean-Core-2f-Src-2f-Benchmark

