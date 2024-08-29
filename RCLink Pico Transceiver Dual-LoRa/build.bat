@echo off
:: Get the directory where the batch file is located
set BAT_DIR=%~dp0

:: Remove existing .uf2 file in the directory of the batch file
del "%BAT_DIR%nrf_rc_link.uf2" 2> nul

:: Open the developer console and set up Visual Studio 2022 environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

:: Change to the specific project directory
cd "C:\Users\aspen\Desktop\RCLink\RCLink Pico Transceiver Dual-LoRa\Pico-LoRa-Transceiver"

:: Remove and recreate the build directory
if exist build rmdir /s /q build
mkdir build
cd build

:: Configure the build with CMake
cmake -G "NMake Makefiles" ../pico_rc_link -DCMAKE_BUILD_TYPE=Release -DPICO_BOARD=pico

:: Build the project
cmake --build . --config Release

:: Copy the .uf2 file back to the batch file's directory
copy "pico_rc_link.uf2" "%BAT_DIR%"

:: Change directory back to the original directory that the batch file started in
cd "%BAT_DIR%"

:: Pause the script to view any output or errors
pause
