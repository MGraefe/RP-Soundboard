// src/Theme.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <QString>

enum class ThemeMode
{
	System = 0, // Follow OS (Windows only; falls back to Light on other platforms)
	Light = 1,
	Dark = 2,
};

const char* themeModeToString(ThemeMode mode);
ThemeMode themeModeFromString(const char* str); // unknown strings fall back to System

// Returns true if the given mode resolves to dark (queries the OS for System mode).
bool themeIsDark(ThemeMode mode);

// Returns the stylesheet to apply to the top-level window widget.
QString lightThemeStylesheet();
QString darkThemeStylesheet();

// Returns the stylesheet for the sound button grid (gridWidget).
QString lightGridStylesheet();
QString darkGridStylesheet();
