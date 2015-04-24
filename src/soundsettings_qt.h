#ifndef about_qt_H__
#define about_qt_H__


#include <QtWidgets/QDialog>
#include <QtGui/QCloseEvent>
#include "SoundInfo.h"

class ConfigModel;

namespace Ui {
	class SoundSettingsQt;
}


class SoundSettingsQt: public QDialog
{
	Q_OBJECT

public:
	explicit SoundSettingsQt(const SoundInfo &soundInfo, QWidget *parent = 0);
	const SoundInfo &getSoundInfo() const { return m_soundInfo; }

protected:
	void done(int r);

private slots:
	void onVolumeChanged(int value);
	void onBrowsePressed();

private:
	void initGui(const SoundInfo &sound);
	void fillFromGui(SoundInfo &sound);
private:
	Ui::SoundSettingsQt *ui;
	ConfigModel *m_model;
	size_t m_soundIndex;
	SoundInfo m_soundInfo;
};


#endif