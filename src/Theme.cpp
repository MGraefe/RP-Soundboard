// src/Theme.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#include "Theme.h"

#include <cstring>


const char* themeModeToString(ThemeMode mode)
{
	switch (mode)
	{
	case ThemeMode::Light:  return "light";
	case ThemeMode::Dark:   return "dark";
	case ThemeMode::System: return "system";
	}
	return "system";
}


ThemeMode themeModeFromString(const char* str)
{
	if (strcmp(str, "light") == 0)  return ThemeMode::Light;
	if (strcmp(str, "dark") == 0)   return ThemeMode::Dark;
	if (strcmp(str, "system") == 0) return ThemeMode::System;
	return ThemeMode::System;
}

#ifdef _WIN32
#include <windows.h>

static bool windowsIsDarkMode()
{
	HKEY key;
	if (RegOpenKeyExW(
			HKEY_CURRENT_USER,
			L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
			0, KEY_READ, &key) != ERROR_SUCCESS)
		return false;

	DWORD value = 1; // default: light
	DWORD size = sizeof(value);
	RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
	RegCloseKey(key);

	return value == 0; // 0 = dark mode
}
#endif


bool themeIsDark(ThemeMode mode)
{
	if (mode == ThemeMode::Dark)
		return true;
	if (mode == ThemeMode::Light)
		return false;

	// ThemeMode::System
#ifdef _WIN32
	return windowsIsDarkMode();
#else
	return false;
#endif
}


QString lightThemeStylesheet()
{
	return QString(); // Qt default light theme
}


QString darkThemeStylesheet()
{
	return
		"QWidget {"
		"  background-color: #1b1b1b;"
		"  color: #bbbbbb;"
		"}"
		"QPushButton {"
		"  background-color: #343638;"
		"  border: 1px solid #555555;"
		"  color: #bbbbbb;"
		"  padding: 4px;"
		"}"
		"QPushButton:hover {"
		"  background-color: #5c7a9e;"
		"  border-color: #7aaddc;"
		"}"
		"QPushButton:pressed {"
		"  background-color: #4a6fa5;"
		"  border-color: #6699cc;"
		"}"
		"QPushButton:disabled {"
		"  background-color: #3a3d3e;"
		"  border-color: #444444;"
		"  color: #666666;"
		"}"
		"QPushButton:flat {"
		"  background-color: transparent;"
		"  border: none;"
		"}"
		"QPushButton:flat:hover {"
		"  background-color: #444444;"
		"  border: none;"
		"}"
		"QCheckBox {"
		"  color: #bbbbbb;"
		"}"
		"QCheckBox::indicator {"
		"  background-color: #bbbbbb;"
		"  border: 1px solid #888888;"
		"  width: 12px;"
		"  height: 12px;"
		"}"
		"QCheckBox::indicator:hover {"
		"  background-color: #d0d0d0;"
		"  border-color: #aaaaaa;"
		"}"
		"QCheckBox::indicator:checked {"
		"  background-color: #7aaddc;"
		"  border-color: #7aaddc;"
		"}"
		"QLabel {"
		"  color: #bbbbbb;"
		"}"
		"QSpinBox {"
		"  background-color: #3c3f41;"
		"  border: 1px solid #555555;"
		"  color: #bbbbbb;"
		"}"
		"QSlider::groove:horizontal {"
		"  background: #555555;"
		"  height: 4px;"
		"  border-radius: 2px;"
		"}"
		"QSlider::handle:horizontal {"
		"  background: #7aaddc;"
		"  width: 12px;"
		"  height: 12px;"
		"  margin: -4px 0;"
		"  border-radius: 6px;"
		"}"
		"QLineEdit {"
		"  background-color: #3c3f41;"
		"  border: 1px solid #555555;"
		"  color: #bbbbbb;"
		"}"
		"QFrame#frame, QFrame#frame_2 {"
		"  color: #555555;"
		"}"
		"QToolButton {"
		"  background-color: transparent;"
		"  border: none;"
		"  color: #bbbbbb;"
		"}"
		"QScrollArea {"
		"  border: none;"
		"  background: transparent;"
		"}";
}


QString lightGridStylesheet()
{
	return
		"QPushButton {"
		"  padding: 4px;"
		"  border: 1px solid rgb(173, 173, 173);"
		"  color: black;"
		"  background-color: rgb(225, 225, 225);"
		"}"
		"QPushButton:disabled {"
		"  background-color: rgb(204, 204, 204);"
		"  border-color: rgb(191, 191, 191);"
		"  color: rgb(120, 120, 120);"
		"}"
		"QPushButton:hover {"
		"  background-color: rgb(228, 239, 249);"
		"  border-color: rgb(11, 123, 212);"
		"}"
		"QPushButton:pressed {"
		"  background-color: rgb(204, 228, 247);"
		"  border-color: rgb(0, 85, 155);"
		"}";
}


QString darkGridStylesheet()
{
	return
		"QPushButton {"
		"  padding: 4px;"
		"  border: 1px solid rgb(85, 85, 85);"
		"  color: rgb(187, 187, 187);"
		"  background-color: rgb(60, 63, 65);"
		"}"
		"QPushButton:disabled {"
		"  background-color: rgb(50, 52, 54);"
		"  border-color: rgb(65, 65, 65);"
		"  color: rgb(100, 100, 100);"
		"}"
		"QPushButton:hover {"
		"  background-color: rgb(70, 100, 140);"
		"  border-color: rgb(100, 155, 220);"
		"}"
		"QPushButton:pressed {"
		"  background-color: rgb(50, 80, 120);"
		"  border-color: rgb(80, 130, 200);"
		"}";
}
