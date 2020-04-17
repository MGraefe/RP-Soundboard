// src/SoundInfo.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__SoundInfo_H__
#define rpsbsrc__SoundInfo_H__

#include <QSettings>
#include <QColor>

class SoundInfo
{
public:
	SoundInfo();
	void readFromConfig(const QSettings &settings);
	void saveToConfig(QSettings &settings) const;
	double getStartTime() const;
	double getPlayTime() const;

	static double getTimeUnitFactor(int unit);
	bool customColorEnabled() const { return customColor.alpha() != 0; }
	void setCustomColorEnabled(bool enabled) { customColor.setAlpha(enabled ? 255 : 0); }

public:
	QString filename;
	QString customText;
	QColor customColor;
	int volume;
	bool cropEnabled;
	int cropStartValue;
	int cropStartUnit;
	int cropStopAfterAt;
	int cropStopValue;
	int cropStopUnit;
};

#endif // rpsbsrc__SoundInfo_H__
