@echo off
copy ..\build32\release\rp_soundboard_win32.dll plugins\
copy ..\build64\release\rp_soundboard_win64.dll plugins\
set /p build="Build number: "
mkdir releases\%build%
"C:\Program Files\7-Zip\7z.exe" a releases\%build%\rp_soundboard_%build%.ts3_plugin -tzip -mx=9 -mm=Deflate ./package.ini plugins/*
pause
