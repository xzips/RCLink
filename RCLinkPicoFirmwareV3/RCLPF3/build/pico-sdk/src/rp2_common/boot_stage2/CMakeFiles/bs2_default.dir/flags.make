﻿# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.27

# compile ASM with C:/Program Files (x86)/GNU Arm Embedded Toolchain/10 2021.10/bin/arm-none-eabi-gcc.exe
ASM_DEFINES = -DPICO_BOARD=\"pico\" -DPICO_BUILD=1 -DPICO_NO_HARDWARE=0 -DPICO_ON_DEVICE=1

ASM_INCLUDES = -IC:\Users\aspen\Code\pico-sdk\src\rp2_common\boot_stage2\asminclude -IC:\Users\aspen\Code\pico-sdk\src\rp2040\hardware_regs\include -IC:\Users\aspen\Code\pico-sdk\src\rp2_common\hardware_base\include -IC:\Users\aspen\Code\pico-sdk\src\common\pico_base\include -IC:\Users\aspen\Desktop\RCLink\RCLinkPicoFirmwareV3\RCLPF3\build\generated\pico_base -IC:\Users\aspen\Code\pico-sdk\src\boards\include -IC:\Users\aspen\Code\pico-sdk\src\rp2_common\pico_platform\include -IC:\Users\aspen\Code\pico-sdk\src\rp2_common\boot_stage2\include

ASM_FLAGS = -mcpu=cortex-m0plus -mthumb -O3 -DNDEBUG
