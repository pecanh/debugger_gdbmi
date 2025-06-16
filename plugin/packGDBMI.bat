@echo off
if %1.==. goto :ErrorExit
if %2.==. goto :ErrorExit
if %3.==. goto :ErrorExit

:: Example packgdbmi.bat $(#cb) $(project_name) $(target_name)
:: Parm1:    relative trunk\src location
:: parm2:    debugger_GDBmi
:: parme3:   target title debug
@echo on
SET CB_SRC=%1\devel32_64\share\codeblocks
SET PROJECT_NAME=%2
SET TARGET_NAME=%3

if exist %CB_SRC%\debugger_gdbmi.zip del /f /q %CB_SRC%\debugger_gdbmi.zip
copy bin\%PROJECT_NAME%\%TARGET_NAME%\debugger_gdbmi.dll %CB_SRC%\plugins\debugger_gdbmi.dll
::-zip -j9 %CB_SRC%\debugger_gdbmi.zip resource\manifest.xml resource\*.xrc
copy resource\GDBMImanifest.xml resource\manifest.xml
zip -j9 %CB_SRC%\debugger_gdbmi.zip resource\manifest.xml
goto :Exit

// ------------------------------------------------------------------------------
:ErrorExit
@echo %0 has a missing prameter
Exit /b 1

:Exit
Exit /b 0
