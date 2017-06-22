// src/soundsettings_qt.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------



#include "ui_soundsettings_qt.h"
#include "soundsettings_qt.h"
#include "ConfigModel.h"
#include "main.h"
#include "samples.h"
#include "soundview_qt.h"
#include <QtWidgets/QFileDialog>
#include <QtGui/QPainter>
#include <QFileInfo>
#include "config_qt.h"
#include <QColorDialog>


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SoundSettingsQt::SoundSettingsQt(const SoundInfo &soundInfo, size_t buttonId, QWidget *parent /*= 0*/) :
	QDialog(parent),
	ui(new Ui::SoundSettingsQt),
	m_soundInfo(soundInfo),
	m_buttonId(buttonId),
	m_iconPlay(":/icon/img/playarrow_32.png"),
	m_iconStop(":/icon/img/stoparrow_32.png")
{
	m_soundview = new SoundView(this);
	m_soundview->setObjectName(QStringLiteral("soundview"));
	m_soundview->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
	m_soundview->setMinimumSize(QSize(0, 30));

	ui->setupUi(this);
	ui->startSoundUnitCombo->addItem("milliseconds");
	ui->startSoundUnitCombo->addItem("seconds");
	ui->stopSoundUnitCombo->addItem("milliseconds");
	ui->stopSoundUnitCombo->addItem("seconds");
	ui->stopSoundAtAfterCombo->addItem("after");
	ui->stopSoundAtAfterCombo->addItem("at");
	connect(ui->soundVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(onVolumeChanged(int)));
	connect(ui->filenameBrowseButton, SIGNAL(released()), this, SLOT(onBrowsePressed()));
	connect(ui->previewSoundButton, SIGNAL(released()), this, SLOT(onPreviewPressed()));
	connect(ui->hotkeyChangeButton, SIGNAL(clicked()), this, SLOT(onHotkeyChangePressed()));
	connect(ui->colorCheckBox, SIGNAL(clicked()), this, SLOT(onColorEnabledPressed()));
	connect(ui->colorButton, SIGNAL(clicked()), this, SLOT(onChooseColorPressed()));
	connect(parent, SIGNAL(hotkeyRecordedEvent(QString,QString)), this, SLOT(updateHotkeyText()));
	initGui(m_soundInfo);

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));

	ui->groupCrop->layout()->addWidget(m_soundview);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::initGui(const SoundInfo &sound)
{
	ui->filenameEdit->setText(sound.filename);
	ui->customTextEdit->setPlaceholderText(QFileInfo(sound.filename).baseName());
	ui->customTextEdit->setText(sound.customText);
	ui->soundVolumeSlider->setValue(sound.volume);
	ui->groupCrop->setChecked(sound.cropEnabled);
	ui->startSoundValueSpin->setValue(sound.cropStartValue);
	ui->startSoundUnitCombo->setCurrentIndex(sound.cropStartUnit);
	ui->stopSoundAtAfterCombo->setCurrentIndex(sound.cropStopAfterAt);
	ui->stopSoundValueSpin->setValue(sound.cropStopValue);
	ui->stopSoundUnitCombo->setCurrentIndex(sound.cropStopUnit);
	ui->colorCheckBox->setChecked(sound.customColorEnabled());
	ui->colorButton->setEnabled(sound.customColorEnabled());
	ui->colorButton->setStyleSheet(QString("background-color: %1").arg(sound.customColor.name()));
	this->customColor = sound.customColor;
	
	updateHotkeyText();

	m_soundview->setSound(sound);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::fillFromGui(SoundInfo &sound)
{
	sound.filename = ui->filenameEdit->text();
	sound.customText = ui->customTextEdit->text();
	sound.volume = ui->soundVolumeSlider->value();
	sound.cropEnabled = ui->groupCrop->isChecked();
	sound.cropStartValue = ui->startSoundValueSpin->value();
	sound.cropStartUnit = ui->startSoundUnitCombo->currentIndex();
	sound.cropStopAfterAt = ui->stopSoundAtAfterCombo->currentIndex();
	sound.cropStopValue = ui->stopSoundValueSpin->value();
	sound.cropStopUnit = ui->stopSoundUnitCombo->currentIndex();
	sound.customColor = this->customColor;
}

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SoundSettingsQt::~SoundSettingsQt()
{
	delete ui;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::done( int r )
{
	Sampler *sampler = sb_getSampler();
	if(sampler->getState() == Sampler::ePLAYING_PREVIEW)
		sampler->stopPlayback();
	fillFromGui(m_soundInfo);
	QDialog::done(r);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::onVolumeChanged(int value)
{
	ui->soundVolumeDbLabel->setText(QString("%1%2 dB").arg(value > 0 ? "+" : "", QString::number(value)));
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::onBrowsePressed()
{
	QString filePath = ui->filenameEdit->text();
	QString fn = QFileDialog::getOpenFileName(this, tr("Choose File"), filePath, tr("Files (*.*)"));
	if(!fn.isNull())
	{
		ui->filenameEdit->setText(fn);
		ui->customTextEdit->setPlaceholderText(QFileInfo(fn).baseName());

		SoundInfo info;
		fillFromGui(info);
		m_soundview->setSound(info);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::onPreviewPressed()
{
	Sampler *sampler = sb_getSampler();
	if(sampler->getState() != Sampler::ePLAYING_PREVIEW)
	{
		SoundInfo sound;
		fillFromGui(sound);
		if(sampler->playPreview(sound))
		{
			ui->previewSoundButton->setIcon(m_iconStop);
			m_timer->start(100);
		}
	}
	else
	{
		sampler->stopPlayback();
		ui->previewSoundButton->setIcon(m_iconPlay);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::onTimer()
{
	Sampler *sampler = sb_getSampler();
	if(sampler->getState() != Sampler::ePLAYING_PREVIEW)
	{
		ui->previewSoundButton->setIcon(m_iconPlay);
		m_timer->stop();
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::onHotkeyChangePressed()
{
	ConfigQt::openHotkeySetDialog(m_buttonId, this);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::updateHotkeyText()
{
	QString hotkeyText = ConfigQt::getShortcutString(m_buttonId);
	ui->hotkeyCurrentLabel->setText(QString("Current hotkey: ") + 
		(hotkeyText.isEmpty() ? QString("None") : hotkeyText));
}


void SoundSettingsQt::onColorEnabledPressed()
{
	customColor.setAlpha(ui->colorCheckBox->isChecked() ? 255 : 0);
	ui->colorButton->setEnabled(ui->colorCheckBox->isChecked());
}


void SoundSettingsQt::onChooseColorPressed()
{
	int alpha = customColor.alpha();
	customColor = QColorDialog::getColor(customColor, this, "Custom button color");
	customColor.setAlpha(alpha);
	ui->colorButton->setStyleSheet(QString("background-color: %1").arg(customColor.name()));
}





