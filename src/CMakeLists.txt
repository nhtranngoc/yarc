cmake_minimum_required(VERSION 3.10)

set(LDSCRIPT "${CMAKE_SOURCE_DIR}/stm32f429i-discovery.ld")
set(LIBNAME "opencm3_stm32f4")
add_definitions(-DSTM32F4)

set(FP_FLAGS "-mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(ARCH_FLAGS "-mthumb -mcpu=cortex-m4 ${FP_FLAGS}")

set(OPENCM3_DIR "${CMAKE_SOURCE_DIR}/libopencm3")

include_directories(${OPENCM3_DIR}/include)

link_directories(${OPENCM3_DIR}/lib)

set(CMAKE_ASM_FLAGS "-D__ASSEMBLY__ -mcpu=cortex-m4 -mthumb")
set(CMAKE_C_FLAGS " \
-lc \
-lgcc \
-Os -g -Wall -Werror -Wredundant-decls -Wstrict-prototypes \
-fno-common -ffunction-sections -fdata-sections \
-MD -Wundef \
${ARCH_FLAGS}")

set(CMAKE_CXX_FLAGS " \
-lc \
-lgcc \
-Os \
-lm \
-fsingle-precision-constant -Wdouble-promotion -Wfloat-conversion \
-Wall -std=c++14 ${ARCH_FLAGS}")

set(CMAKE_EXE_LINKER_FLAGS " \
-T${LDSCRIPT} \
--static \
-nostartfiles \
--specs=nano.specs \
--specs=nosys.specs \
-Wl,--start-group -lc -lgcc -Wl,--end-group")

add_custom_target(libopencm3 make WORKING_DIRECTORY ${OPENCM3_DIR})

# SRC
include_directories(../include)
include_directories(../lcd)
include_directories(../assets)

file(GLOB USER_SOURCES 
    ./*.c 
    ./*.cpp
    ./lcd/*.c
    ${CMAKE_SOURCE_DIR}/assets/*.h)

add_executable(${PROJECT_NAME}.elf ${USER_SOURCES})
target_link_libraries(${PROJECT_NAME}.elf opencm3_stm32f4)
