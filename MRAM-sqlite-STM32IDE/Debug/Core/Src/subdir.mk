################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/FLASH_SECTOR_H7.c \
../Core/Src/lfs.c \
../Core/Src/lfs_util.c \
../Core/Src/little-fs-test.c \
../Core/Src/littlefs_sqlite_vfs.c \
../Core/Src/main.c \
../Core/Src/sqlite3.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32h7xx.c 

OBJS += \
./Core/Src/FLASH_SECTOR_H7.o \
./Core/Src/lfs.o \
./Core/Src/lfs_util.o \
./Core/Src/little-fs-test.o \
./Core/Src/littlefs_sqlite_vfs.o \
./Core/Src/main.o \
./Core/Src/sqlite3.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32h7xx.o 

C_DEPS += \
./Core/Src/FLASH_SECTOR_H7.d \
./Core/Src/lfs.d \
./Core/Src/lfs_util.d \
./Core/Src/little-fs-test.d \
./Core/Src/littlefs_sqlite_vfs.d \
./Core/Src/main.d \
./Core/Src/sqlite3.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32h7xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O3 -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -Wno-unused-function -Wno-comment -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/FLASH_SECTOR_H7.d ./Core/Src/FLASH_SECTOR_H7.o ./Core/Src/FLASH_SECTOR_H7.su ./Core/Src/lfs.d ./Core/Src/lfs.o ./Core/Src/lfs.su ./Core/Src/lfs_util.d ./Core/Src/lfs_util.o ./Core/Src/lfs_util.su ./Core/Src/little-fs-test.d ./Core/Src/little-fs-test.o ./Core/Src/little-fs-test.su ./Core/Src/littlefs_sqlite_vfs.d ./Core/Src/littlefs_sqlite_vfs.o ./Core/Src/littlefs_sqlite_vfs.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/sqlite3.d ./Core/Src/sqlite3.o ./Core/Src/sqlite3.su ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su

.PHONY: clean-Core-2f-Src

