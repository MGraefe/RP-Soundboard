
#include "SoundInfo.h"

#define NAME_PATH "path"
#define NAME_VOLUME "volume"
#define NAME_CROP_ENABLED "cropEnabled"
#define NAME_CROP_START_VALUE "cropStartValue"
#define NAME_CROP_START_UNIT "cropStartUnit"
#define NAME_CROP_STOP_AFTER_AT "cropStopAfterAt"
#define NAME_CROP_STOP_VALUE "cropStopValue"
#define NAME_CROP_STOP_UNIT "cropStopUnit"

#define DEFAULT_PATH ""
#define DEFAULT_VOLUME 0
#define DEFAULT_CROP_ENABLED false
#define DEFAULT_CROP_START_VALUE 1
#define DEFAULT_CROP_START_UNIT 1
#define DEFAULT_CROP_STOP_AFTER_AT 0
#define DEFAULT_CROP_STOP_VALUE 5
#define DEFAULT_CROP_STOP_UNIT 1


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SoundInfo::SoundInfo() :
	filename(DEFAULT_PATH),
	volume(DEFAULT_VOLUME),
	cropEnabled(DEFAULT_CROP_ENABLED),
	cropStartValue(DEFAULT_CROP_START_VALUE),
	cropStartUnit(DEFAULT_CROP_START_UNIT),
	cropStopAfterAt(DEFAULT_CROP_STOP_AFTER_AT),
	cropStopValue(DEFAULT_CROP_STOP_VALUE),
	cropStopUnit(DEFAULT_CROP_STOP_UNIT)
{

}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundInfo::readFromConfig( const QSettings &settings )
{
	filename = settings.value(NAME_PATH, DEFAULT_PATH).toString();
	volume = settings.value(NAME_VOLUME, DEFAULT_VOLUME).toInt();
	cropEnabled = settings.value(NAME_CROP_ENABLED, DEFAULT_CROP_ENABLED).toBool();
	cropStartValue = settings.value(NAME_CROP_START_VALUE, DEFAULT_CROP_START_VALUE).toInt();
	cropStartUnit = settings.value(NAME_CROP_START_UNIT, DEFAULT_CROP_START_UNIT).toInt();
	cropStopAfterAt = settings.value(NAME_CROP_STOP_AFTER_AT, DEFAULT_CROP_STOP_AFTER_AT).toInt();
	cropStopValue = settings.value(NAME_CROP_STOP_VALUE, DEFAULT_CROP_STOP_VALUE).toInt();
	cropStopUnit = settings.value(NAME_CROP_STOP_UNIT, DEFAULT_CROP_STOP_UNIT).toInt();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundInfo::saveToConfig( QSettings &settings ) const
{
	settings.setValue(NAME_PATH, filename);
	settings.setValue(NAME_VOLUME, volume);
	settings.setValue(NAME_CROP_ENABLED, cropEnabled);
	settings.setValue(NAME_CROP_START_VALUE, cropStartValue);
	settings.setValue(NAME_CROP_START_UNIT, cropStartUnit);
	settings.setValue(NAME_CROP_STOP_AFTER_AT, cropStopAfterAt);
	settings.setValue(NAME_CROP_STOP_VALUE, cropStopValue);
	settings.setValue(NAME_CROP_STOP_UNIT, cropStopUnit);
}
