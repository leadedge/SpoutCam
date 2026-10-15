@echo off
cd /d "%~dp0"
:: -----------------------------
:: Elevate to Administrator if needed
:: -----------------------------
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Requesting Administrator privileges...
    powershell -Command "Start-Process '%~f0' -Verb runAs"
    exit /b
)
echo.
echo REGISTER SpoutCam 64 bit
echo.
echo. 1 - Register
echo. 2 - Un-register
echo. 3 - Re-register
echo. Enter to cancel
echo.
:: set /P input= Select option : 
:: This complicated mtohod makes a leading space
for /f %%A in ('echo prompt $E ^| cmd') do set "ESC=%%A"
<nul set /p "=%ESC%[1CChoose option: "
set /p input=
if "%input%"=="" goto Finish

if "%input%"=="1" goto Register
if "%input%"=="2" goto Unregister
if "%input%"=="3" goto Reregister
goto Finish

:Reregister
echo UnRegistering 64-bit DLL...
regsvr32.exe /u SpoutCam64.ax
echo Registering 64-bit DLL...
regsvr32.exe SpoutCam64.ax
goto Finish

:Unregister
echo UnRegistering 64-bit DLL...
regsvr32.exe /u SpoutCam64.ax
goto Finish

:Register
echo Registering 64-bit DLL...
regsvr32.exe SpoutCam64.ax

:Finish
