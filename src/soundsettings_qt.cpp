

#include "ui_soundsettings_qt.h"
#include "soundsettings_qt.h"
#include "ConfigModel.h"
#include <QtWidgets/QFileDialog>

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SoundSettingsQt::SoundSettingsQt(const SoundInfo &soundInfo, QWidget *parent /*= 0*/) :
	QDialog(parent),
	ui(new Ui::SoundSettingsQt),
	m_soundInfo(soundInfo)
{
	ui->setupUi(this);
	ui->previewSoundButton->hide();
	ui->startSoundUnitCombo->addItem("milliseconds");
	ui->startSoundUnitCombo->addItem("seconds");
	ui->stopSoundUnitCombo->addItem("milliseconds");
	ui->stopSoundUnitCombo->addItem("seconds");
	ui->stopSoundAtAfterCombo->addItem("after");
	ui->stopSoundAtAfterCombo->addItem("at");
	connect(ui->soundVolumeSlider, SIGNAL(valueChanged(int)), this, SLOT(onVolumeChanged(int)));
	connect(ui->filenameBrowseButton, SIGNAL(released()), this, SLOT(onBrowsePressed()));
	initGui(m_soundInfo);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::initGui(const SoundInfo &sound)
{
	ui->filenameEdit->setText(sound.filename);
	ui->soundVolumeSlider->setValue(sound.volume);
	ui->groupCrop->setChecked(sound.cropEnabled);
	ui->startSoundValueSpin->setValue(sound.cropStartValue);
	ui->startSoundUnitCombo->setCurrentIndex(sound.cropStartUnit);
	ui->stopSoundAtAfterCombo->setCurrentIndex(sound.cropStopAfterAt);
	ui->stopSoundValueSpin->setValue(sound.cropStopValue);
	ui->stopSoundUnitCombo->setCurrentIndex(sound.cropStopUnit);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::fillFromGui(SoundInfo &sound)
{
	sound.filename = ui->filenameEdit->text();
	sound.volume = ui->soundVolumeSlider->value();
	sound.cropEnabled = ui->groupCrop->isChecked();
	sound.cropStartValue = ui->startSoundValueSpin->value();
	sound.cropStartUnit = ui->startSoundUnitCombo->currentIndex();
	sound.cropStopAfterAt = ui->stopSoundAtAfterCombo->currentIndex();
	sound.cropStopValue = ui->stopSoundValueSpin->value();
	sound.cropStopUnit = ui->stopSoundUnitCombo->currentIndex();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundSettingsQt::done( int r )
{
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
		ui->filenameEdit->setText(fn);
}




