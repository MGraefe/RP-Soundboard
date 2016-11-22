@echo off
copy ..\bin\Release_win_x86\rp_soundboard_win32.dll plugins\
copy ..\bin\Release_win_x64\rp_soundboard_win64.dll plugins\
set /p build="Build number: "
mkdir releases\%build%
"C:\Program Files\7-Zip\7z.exe" a releases\%build%\rp_soundboard_%build%.ts3_plugin -tzip -mx=9 -mm=Deflate ./package.ini plugins/*
pause