#ifndef SoundInfo_H__
#define SoundInfo_H__

#include <QtCore/QSettings>

class SoundInfo
{
public:
	SoundInfo();
	void readFromConfig(const QSettings &settings);
	void saveToConfig(QSettings &settings) const;

public:
	QString filename;
	int volume;
	bool cropEnabled;
	int cropStartValue;
	int cropStartUnit;
	int cropStopAfterAt;
	int cropStopValue;
	int cropStopUnit;
};

#endif // SoundInfo_H__
