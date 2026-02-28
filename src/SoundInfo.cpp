// src/SoundInfo.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "SoundInfo.h"

#define NAME_PATH "path"
#define NAME_CUSTOM_TEXT "customText"
#define NAME_CUSTOM_COLOR "customColor"
#define NAME_VOLUME "volume"
#define NAME_CROP_ENABLED "cropEnabled"
#define NAME_CROP_START_VALUE "cropStartValue"
#define NAME_CROP_START_UNIT "cropStartUnit"
#define NAME_CROP_STOP_AFTER_AT "cropStopAfterAt"
#define NAME_CROP_STOP_VALUE "cropStopValue"
#define NAME_CROP_STOP_UNIT "cropStopUnit"

#define DEFAULT_PATH ""
#define DEFAULT_CUSTOM_TEXT ""
#define DEFAULT_CUSTOM_COLOR "00FFFFFF"
#define DEFAULT_VOLUME 0
#define DEFAULT_CROP_ENABLED false
#define DEFAULT_CROP_START_VALUE 1
#define DEFAULT_CROP_START_UNIT 1
#define DEFAULT_CROP_STOP_AFTER_AT 0
#define DEFAULT_CROP_STOP_VALUE 0
#define DEFAULT_CROP_STOP_UNIT 1


QColor stringToColor(const QString &str)
{
	QRgb rgb = str.toUInt(nullptr, 16);
	// don't construct directly from QRgb because alpha is ignored that way
	return QColor(qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb));
}

QString colorToString(const QColor &col)
{
	return QString::number(col.rgba(), 16);
}


SoundInfo::SoundInfo() :
	filename(DEFAULT_PATH),
	customText(DEFAULT_CUSTOM_TEXT),
	customColor(stringToColor(DEFAULT_CUSTOM_COLOR)),
	volume(DEFAULT_VOLUME),
	cropEnabled(DEFAULT_CROP_ENABLED),
	cropStartValue(DEFAULT_CROP_START_VALUE),
	cropStartUnit(DEFAULT_CROP_START_UNIT),
	cropStopAfterAt(DEFAULT_CROP_STOP_AFTER_AT),
	cropStopValue(DEFAULT_CROP_STOP_VALUE),
	cropStopUnit(DEFAULT_CROP_STOP_UNIT)
{

}


void SoundInfo::readFromConfig( const QSettings &settings )
{
	filename = settings.value(NAME_PATH, DEFAULT_PATH).toString();
	customText = settings.value(NAME_CUSTOM_TEXT, DEFAULT_CUSTOM_TEXT).toString();
	customColor = stringToColor(settings.value(NAME_CUSTOM_COLOR, DEFAULT_CUSTOM_COLOR).toString());
	volume = settings.value(NAME_VOLUME, DEFAULT_VOLUME).toInt();
	cropEnabled = settings.value(NAME_CROP_ENABLED, DEFAULT_CROP_ENABLED).toBool();
	cropStartValue = settings.value(NAME_CROP_START_VALUE, DEFAULT_CROP_START_VALUE).toInt();
	cropStartUnit = settings.value(NAME_CROP_START_UNIT, DEFAULT_CROP_START_UNIT).toInt();
	cropStopAfterAt = settings.value(NAME_CROP_STOP_AFTER_AT, DEFAULT_CROP_STOP_AFTER_AT).toInt();
	cropStopValue = settings.value(NAME_CROP_STOP_VALUE, DEFAULT_CROP_STOP_VALUE).toInt();
	cropStopUnit = settings.value(NAME_CROP_STOP_UNIT, DEFAULT_CROP_STOP_UNIT).toInt();
}


void SoundInfo::saveToConfig( QSettings &settings ) const
{
	settings.setValue(NAME_PATH, filename);
	settings.setValue(NAME_CUSTOM_TEXT, customText);
	settings.setValue(NAME_CUSTOM_COLOR, colorToString(customColor));
	settings.setValue(NAME_VOLUME, volume);
	settings.setValue(NAME_CROP_ENABLED, cropEnabled);
	settings.setValue(NAME_CROP_START_VALUE, cropStartValue);
	settings.setValue(NAME_CROP_START_UNIT, cropStartUnit);
	settings.setValue(NAME_CROP_STOP_AFTER_AT, cropStopAfterAt);
	settings.setValue(NAME_CROP_STOP_VALUE, cropStopValue);
	settings.setValue(NAME_CROP_STOP_UNIT, cropStopUnit);
}


double SoundInfo::getStartTime() const
{
	if(!cropEnabled)
		return 0.0;
	return (double)cropStartValue * getTimeUnitFactor(cropStartUnit);
}


double SoundInfo::getPlayTime() const
{
	if(!cropEnabled)
		return -1.0;
	double t = (double)cropStopValue * getTimeUnitFactor(cropStopUnit);
	if(cropStopAfterAt == 1) //stop AT x seconds instead of AFTER?
		t -= getStartTime();
	return std::max(t, 0.0);
}


double SoundInfo::getTimeUnitFactor(int unit)
{
	switch(unit)
	{
	case 0: return 0.001;
	case 1: return 1.0;
	default:
		throw std::logic_error("No such unit");
	}
}
