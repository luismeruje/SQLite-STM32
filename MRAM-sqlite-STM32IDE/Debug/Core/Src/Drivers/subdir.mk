################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Drivers/MRAM_driver.c \
../Core/Src/Drivers/flash_driver.c \
../Core/Src/Drivers/littlefs_driver.c 

OBJS += \
./Core/Src/Drivers/MRAM_driver.o \
./Core/Src/Drivers/flash_driver.o \
./Core/Src/Drivers/littlefs_driver.o 

C_DEPS += \
./Core/Src/Drivers/MRAM_driver.d \
./Core/Src/Drivers/flash_driver.d \
./Core/Src/Drivers/littlefs_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Drivers/%.o Core/Src/Drivers/%.su: ../Core/Src/Drivers/%.c Core/Src/Drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O3 -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -Wno-unused-function -Wno-comment -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Drivers

clean-Core-2f-Src-2f-Drivers:
	-$(RM) ./Core/Src/Drivers/MRAM_driver.d ./Core/Src/Drivers/MRAM_driver.o ./Core/Src/Drivers/MRAM_driver.su ./Core/Src/Drivers/flash_driver.d ./Core/Src/Drivers/flash_driver.o ./Core/Src/Drivers/flash_driver.su ./Core/Src/Drivers/littlefs_driver.d ./Core/Src/Drivers/littlefs_driver.o ./Core/Src/Drivers/littlefs_driver.su

.PHONY: clean-Core-2f-Src-2f-Drivers

