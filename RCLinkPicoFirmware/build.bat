@echo off
:: Get the directory where the batch file is located
set BAT_DIR=%~dp0

:: Remove existing .uf2 file in the directory of the batch file
del "%BAT_DIR%nrf_rc_link.uf2" 2> nul

:: Open the developer console and set up Visual Studio 2022 environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

:: Change to the specific project directory
cd "C:\Users\aspen\Desktop\RCLink\RCLinkPicoFirmware\RCLPF"

:: Run cmake to generate NMake makefiles
cmake -G "NMake Makefiles"

:: Execute nmake to build the project
nmake

:: Copy the .uf2 file back to the batch file's directory
copy "nrf_rc_link\nrf_rc_link.uf2" "%BAT_DIR%"

:: Pause the script to view any output or errors
pause
