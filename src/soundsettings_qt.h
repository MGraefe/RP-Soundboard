// src/soundsettings_qt.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__soundsettings_qt_H__
#define rpsbsrc__soundsettings_qt_H__


#include <QtWidgets/QDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QIcon>
#include <QtCore/QTimer>
#include "SoundInfo.h"

class ConfigModel;
class SoundView;

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
	void onPreviewPressed();
	void onTimer();

private:
	void initGui(const SoundInfo &sound);
	void fillFromGui(SoundInfo &sound);
	

private:
	Ui::SoundSettingsQt *ui;
	ConfigModel *m_model;
	size_t m_soundIndex;
	SoundInfo m_soundInfo;
	QIcon m_iconPlay;
	QIcon m_iconStop;
	QTimer *m_timer;
	SoundView *m_soundview;
};


#endif