cmake_minimum_required(VERSION 3.15)

set(CMAKE_GENERATOR "Unix Makefiles")

set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CROSS_TARGET_TRIPLET "arm-none-eabi-")

set(CMAKE_EXECUTABLE_SUFFIX_C   .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf)

# This should be safe to set for a bare-metal cross-compiler
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Путь к директории с вашими исходниками
set(SOURCE_DIR C:/Users/Afhib/Desktop/Projects/Examples_JZ-STM32F407VET6-V1.0)
set(ARM_NONE_EABI_DIR C:/Users/Afhib/.vscode/extensions/metalcode-eu.windows-arm-none-eabi-0.1.6/bin)

# Настройка кросс-компиляции
set(CMAKE_C_COMPILER ${ARM_NONE_EABI_DIR}/arm-none-eabi-gcc.exe)
set(CMAKE_CXX_COMPILER ${ARM_NONE_EABI_DIR}/arm-none-eabi-g++.exe)
set(CMAKE_ASM_COMPILER ${ARM_NONE_EABI_DIR}/arm-none-eabi-gcc.exe)

# Флаги компиляции
set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -Og -ffunction-sections -fdata-sections -Wall -fstack-usage --specs=nano.specs -mfloat-abi=hard -mfpu=fpv4-sp-d16 -std=gnu18"
    "-DDEBUG"    
    "-DUSE_HAL_DRIVER"
    "-DSTM32F407xx"
    "-DconfigAPPLICATION_ALLOCATED_HEAP=1"
    "-DHOST_TRANSFER_LAYER_COMPONENT_ENABLE"
    "-DHOST_REQUEST_CONTROL_COMPONENT_ENABLE"
    "-DHOST_DEVICE_COMPONENT_ENABLE")

# Преобразуем строку в список аргументов
string(REPLACE ";" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(CMAKE_CXX_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -std=gnu18"
    "-DDEBUG"
    "-DUSE_HAL_DRIVER"
    "-DSTM32F407xx")

# Преобразуем строку в список аргументов
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

project(Examples_JZ-STM32F407VET6-V1.0 C ASM)

set(xLIB_PATH "C:/Users/Afhib/Desktop/Projects/xLibs")
set(COMPONENTS_PATH "C:/Users/Afhib/Desktop/Projects/Examples_JZ-STM32F407VET6-V1.0/Components")
set(SINTEZ_ELECTRO_SOURCES_PATH "C:/Users/Afhib/Desktop/Projects/Repositories-SintezElectro/Sources")

# Добавление директорий с заголовочными файлами
include_directories(
    ${SOURCE_DIR}/Core/Inc
    ${SOURCE_DIR}/LWIP/App
    ${SOURCE_DIR}/LWIP/Target
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/system
    ${SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc
    ${SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ${SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${SOURCE_DIR}/Drivers/BSP/Components/dp83848
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/netif/ppp
    ${SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/apps
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/priv
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/lwip/prot
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/netif
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/net
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/posix/sys
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/include/compat/stdc
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/system/arch
    ${SOURCE_DIR}/Drivers/CMSIS/Include
    ${xLIB_PATH}
    ${xLIB_PATH}/Components
    ${COMPONENTS_PATH}
    ${COMPONENTS_PATH}/Configurations
    ${SINTEZ_ELECTRO_SOURCES_PATH}
    ${SINTEZ_ELECTRO_SOURCES_PATH}/CAN_Local
)

# Добавление исходников
file(GLOB SOURCES
    ${SOURCE_DIR}/STM32CubeIDE/Application/User/Startup/*.s
    ${SOURCE_DIR}/STM32CubeIDE/Application/User/Core/*.c
    ${SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/*.c
    ${SOURCE_DIR}/Drivers/BSP/Components/dp83848/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/api/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/apps/mqtt/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/core/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/core/ipv4/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/core/ipv6/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/netif/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/src/netif/ppp/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/system/arch/*.c
    ${SOURCE_DIR}/Middlewares/Third_Party/LwIP/system/OS/*.c
    ${SOURCE_DIR}/Core/Src/*.c
    ${SOURCE_DIR}/LWIP/Target/*.c
    ${SOURCE_DIR}/LWIP/App/*.c
    ${SOURCE_DIR}/Components/*.c
    ${SOURCE_DIR}/Components/Interfaces/*.c
    ${xLIB_PATH}/Common/*.c
    ${xLIB_PATH}/Peripherals/xUSART/*.c
    ${xLIB_PATH}/Peripherals/xTimer/*.c
    ${xLIB_PATH}/Peripherals/CAN/*.c
    ${xLIB_PATH}/Abstractions/xTransferLayer/*.c
    ${xLIB_PATH}/Abstractions/xTerminal/*.c
    ${xLIB_PATH}/Abstractions/xSystem/*.c
    ${xLIB_PATH}/Abstractions/xPort/*.c
    ${xLIB_PATH}/Components/Terminal/*.c
    ${xLIB_PATH}/Components/Terminal/Adapters/*.c
    ${xLIB_PATH}/Components/Terminal/Communication/*.c
    ${xLIB_PATH}/Components/USART-Ports/*.c
    ${xLIB_PATH}/Components/USART-Ports/Adapters/STM32F4xx/*.c
)

# Создание исполняемого файла
add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Указание скрипта линкера
#set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/STM32CubeIDE/STM32F407VETX_FLASH.ld")
#set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_SOURCE_DIR}/STM32CubeIDE/STM32F407VETX_FLASH.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_CURRENT_SOURCE_DIR}/STM32CubeIDE/STM32F407VETX_FLASH.ld -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map -Wl,--gc-sections")

#set(CMAKE_GNULD_IMAGE_VERSION "")

# Указание библиотек для линковки
target_link_libraries(${PROJECT_NAME}.elf
    --specs=nosys.specs
    -Wl,--gc-sections
    -static
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -mthumb
    -Wl,--start-group
    -lm
    -Wl,--end-group
)

# Генерация файла формата HEX
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
)
