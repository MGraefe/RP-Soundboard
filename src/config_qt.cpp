// src/config_qt.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#include "common.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QFileInfo>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QMessageBox>

#include "config_qt.h"
#include "ConfigModel.h"
#include "main.h"
#include "soundsettings_qt.h"
#include "ts3log.h"
#include "SpeechBubble.h"
#include "buildinfo.h"
#include "plugin.h"
#include "ExpandableSection.h"
#include "samples.h"

#ifdef _WIN32
#include "Windows.h"
#endif

enum button_choices_e {
	BC_CHOOSE = 0,
	BC_ADVANCED,
	BC_SET_HOTKEY,
};


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigQt::ConfigQt( ConfigModel *model, QWidget *parent /*= 0*/ ) :
	QWidget(parent),
	ui(new Ui::ConfigQt),
	m_model(model),
	m_modelObserver(*this),
	m_buttonBubble(nullptr)
{
	ui->setupUi(this);
	//setAttribute(Qt::WA_DeleteOnClose);

	settingsSection = new ExpandableSection("Settings", 200, this);
	settingsSection->setContentLayout(*ui->settingsWidget->layout());
	layout()->addWidget(settingsSection);

	QAction *actChooseFile = new QAction("Choose File", this);
	actChooseFile->setData((int)BC_CHOOSE);
	m_buttonContextMenu.addAction(actChooseFile);

	QAction *actAdvancedOpts = new QAction("Advanced Options", this);
	actAdvancedOpts->setData((int)BC_ADVANCED);
	m_buttonContextMenu.addAction(actAdvancedOpts);

	actSetHotkey = new QAction("Set hotkey", this);
	actSetHotkey->setData((int)BC_SET_HOTKEY);
	m_buttonContextMenu.addAction(actSetHotkey);

	createButtons();

	ui->cb_advanced_config->hide();
	ui->b_stop->setContextMenuPolicy(Qt::CustomContextMenu);
	
	connect(ui->b_stop, SIGNAL(released()), this, SLOT(onClickedStop()));
	connect(ui->b_stop, SIGNAL(customContextMenuRequested(const QPoint&)), this,
		SLOT(showStopButtonContextMenu(const QPoint&)));
	connect(ui->sl_volume, SIGNAL(valueChanged(int)), this, SLOT(onUpdateVolume(int)));
	connect(ui->cb_playback_locally, SIGNAL(clicked(bool)), this, SLOT(onUpdatePlaybackLocal(bool)));
	connect(ui->sb_rows, SIGNAL(valueChanged(int)), this, SLOT(onUpdateRows(int)));
	connect(ui->sb_cols, SIGNAL(valueChanged(int)), this, SLOT(onUpdateCols(int)));
	connect(ui->cb_mute_myself, SIGNAL(clicked(bool)), this, SLOT(onUpdateMuteMyself(bool)));

	ui->playingIconLabel->hide();
	ui->playingLabel->setText("");
	playingIconTimer = new QTimer(this);
	connect(playingIconTimer, SIGNAL(timeout()), this, SLOT(onPlayingIconTimer()));

	Sampler *sampler = sb_getSampler();
	connect(sampler, SIGNAL(onStartPlaying(bool, QString)), this, SLOT(onStartPlayingSound(bool, QString)), Qt::QueuedConnection);
	connect(sampler, SIGNAL(onStopPlaying()), this, SLOT(onStopPlayingSound()), Qt::QueuedConnection);

	createBubbles();

	m_model->addObserver(&m_modelObserver);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigQt::~ConfigQt()
{
	m_model->remObserver(&m_modelObserver);
	delete ui;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::closeEvent(QCloseEvent * evt)
{
	m_model->setWindowSize(size().width(), size().height());
}

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onClickedPlay()
{
	QPushButton *button = dynamic_cast<QPushButton*>(sender());
	size_t buttonId = std::find_if(m_buttons.begin(), m_buttons.end(), [button](button_element_t &e){return e.play == button;}) - m_buttons.begin();

	playSound(buttonId);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onClickedStop()
{
	sb_stopPlayback();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateVolume(int val)
{
	m_model->setVolume(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdatePlaybackLocal(bool val)
{
	m_model->setPlaybackLocal(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateCols(int val)
{
	m_model->setCols(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateRows(int val)
{
	m_model->setRows(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateMuteMyself(bool val)
{
	m_model->setMuteMyselfDuringPb(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::createButtons()
{
	for(button_element_t &elem : m_buttons)
	{
		elem.layout->removeWidget(elem.play);
		delete elem.play;
		ui->gridLayout->removeItem(elem.layout);
		delete elem.layout;
	}
	
	m_buttons.clear();

	int numRows = m_model->getRows();
	int numCols = m_model->getCols();

	for(int i = 0; i < numRows; i++)
	{
		for(int j = 0; j < numCols; j++)
		{
			button_element_t elem = {0};
			elem.layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
			elem.layout->setSpacing(0);
			ui->gridLayout->addLayout(elem.layout, i, j);

			elem.play = new QPushButton(this);
			elem.play->setText("(no file)");
			elem.play->setEnabled(true);
			elem.play->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
			elem.layout->addWidget(elem.play);
			connect(elem.play, SIGNAL(released()), this, SLOT(onClickedPlay()));
			elem.play->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(elem.play, SIGNAL(customContextMenuRequested(const QPoint&)), this,
				SLOT(showButtonContextMenu(const QPoint&)));

			elem.play->updateGeometry();
			m_buttons.push_back(elem);
		}
	}

	for(int i = 0; i < m_buttons.size(); i++)
		updateButtonText(i);

	if(m_buttonBubble)
		m_buttonBubble->attachTo(m_buttons[0].play);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::updateButtonText(int i)
{
	if(i >= m_buttons.size())
		return;

	QString fn = m_model->getFileName(i);
	m_buttons[i].play->setText(fn.isEmpty() ? "(no file)" : QFileInfo(fn).baseName());
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::showButtonContextMenu( const QPoint &point )
{
	QPushButton *button = dynamic_cast<QPushButton*>(sender());
	size_t buttonId = std::find_if(m_buttons.begin(), m_buttons.end(), [button](button_element_t &e){return e.play == button;}) - m_buttons.begin();

	QString shortcutName = getShortcutString(buttonId);
	QString hotkeyText = "Set hotkey (Current: " +
		(shortcutName.isEmpty() ? QString("None") : shortcutName) + ")";
	actSetHotkey->setText(hotkeyText);

	QPoint globalPos = m_buttons[buttonId].play->mapToGlobal(point);
	QAction *action = m_buttonContextMenu.exec(globalPos);
	if(action)
	{
		bool ok = false;
		int choice = action->data().toInt(&ok);
		if(ok)
		{
			switch(choice)
			{
			case BC_CHOOSE: 
				chooseFile(buttonId); 
				break;
			case BC_ADVANCED: 
				openAdvanced(buttonId); 
				break;
			case BC_SET_HOTKEY:
				openHotkeySetDialog(buttonId);
				break;
			default: break;
			}
		}
		else
			logError("Invalid user data in context menu");
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::setPlayingLabelIcon(int index)
{
	ui->playingIconLabel->setPixmap(QPixmap(QString(":/icon/img/speaker_icon_%1_64.png").arg(index)));
}



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::playSound( size_t buttonId )
{
	const SoundInfo *info = m_model->getSoundInfo(buttonId);
	if(info)
		sb_playFile(*info);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::chooseFile( size_t buttonId )
{
	QString filePath = m_model->getFileName(buttonId);
	QString fn = QFileDialog::getOpenFileName(this, tr("Choose File"), filePath, tr("Files (*.*)"));
	if(!fn.isNull())
		m_model->setFileName(buttonId, fn);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::openAdvanced( size_t buttonId )
{
	const SoundInfo *buttonInfo = m_model->getSoundInfo(buttonId);
	SoundInfo defaultInfo;
	const SoundInfo &info = buttonInfo ? *buttonInfo : defaultInfo;
	SoundSettingsQt dlg(info, buttonId, this);
	dlg.setWindowTitle(QString("Sound %1 Settings").arg(QString::number(buttonId + 1)));
	if(dlg.exec() == QDialog::Accepted)
		m_model->setSoundInfo(buttonId, dlg.getSoundInfo());
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::createBubbles()
{
	if(m_model->getBubbleButtonsBuild() == 0)
	{
		m_buttonBubble = new SpeechBubble(this);
		m_buttonBubble->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		m_buttonBubble->setFixedSize(180, 80);
		m_buttonBubble->setText("Right click to choose sound file\nor open advanced options.");
		m_buttonBubble->attachTo(m_buttons[0].play);
		connect(m_buttonBubble, SIGNAL(closePressed()), this, SLOT(onButtonBubbleFinished()));
	}

	if(m_model->getBubbleStopBuild() == 0)
	{
		SpeechBubble *stopBubble = new SpeechBubble(this);
		stopBubble->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		stopBubble->setFixedSize(180, 60);
		stopBubble->setText("Stop the currently playing sound.");
		stopBubble->attachTo(ui->b_stop);
		connect(stopBubble, SIGNAL(closePressed()), this, SLOT(onStopBubbleFinished()));
	}

	if(m_model->getBubbleColsBuild() == 0)
	{
		SpeechBubble *colsBubble = new SpeechBubble(this);
		colsBubble->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		colsBubble->setFixedSize(180, 80);
		colsBubble->setText("Change the number of buttons\non the soundboard.");
		colsBubble->attachTo(ui->sb_cols);
		connect(colsBubble, SIGNAL(closePressed()), this, SLOT(onColsBubbleFinished()));
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onStopBubbleFinished()
{
	m_model->setBubbleStopBuild(buildinfo_getBuildNumber());
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onButtonBubbleFinished()
{
	m_model->setBubbleButtonsBuild(buildinfo_getBuildNumber());
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onColsBubbleFinished()
{
	m_model->setBubbleColsBuild(buildinfo_getBuildNumber());
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::showStopButtonContextMenu(const QPoint &point)
{
	QString shortcutName = getShortcutString("stop_all");
	QString hotkeyText = "Set hotkey (Current: " +
		(shortcutName.isEmpty() ? QString("None") : shortcutName) + ")";

	QMenu menu;
	menu.addAction(hotkeyText);

	QPoint globalPos = ui->b_stop->mapToGlobal(point);
	QAction *action = menu.exec(globalPos);
	if (action)
	{
		ts3Functions.requestHotkeyInputDialog(getPluginID(), "stop_all", 0, this);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onStartPlayingSound(bool preview, QString filename)
{
	QFileInfo info(filename);
	ui->playingLabel->setText(info.fileName());
	setPlayingLabelIcon(0);
	ui->playingIconLabel->show();
	playingIconIndex = 1;
	playingIconTimer->start(150);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onStopPlayingSound()
{
	playingIconTimer->stop();
	ui->playingLabel->setText("");
	ui->playingIconLabel->hide();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onPlayingIconTimer()
{
	setPlayingLabelIcon(playingIconIndex);
	++playingIconIndex %= 4;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::openHotkeySetDialog(size_t buttonId)
{
	openHotkeySetDialog(buttonId, this);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::openHotkeySetDialog( size_t buttonId, QWidget *parent )
{
	char intName[16];
	sb_getInternalHotkeyName((int)buttonId, intName);
	ts3Functions.requestHotkeyInputDialog(getPluginID(), intName, 0, parent);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString ConfigQt::getShortcutString(const char *internalName)
{
	char *hotkeyName = new char[128];
	unsigned int res = ts3Functions.getHotkeyFromKeyword(
		getPluginID(), &internalName, &hotkeyName, 1, 128);
	QString name = res == 0 ? QString(hotkeyName) : QString();
	delete[] hotkeyName;
	return name;
}



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString ConfigQt::getShortcutString(size_t buttonId)
{
	char intName[16];
	sb_getInternalHotkeyName((int)buttonId, intName);
	return getShortcutString(intName);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onHotkeyRecordedEvent(const char *keyword, const char *key)
{
	QString sKeyword = keyword;
	QString sKey = key;
	emit hotkeyRecordedEvent(sKey, sKeyword);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::ModelObserver::notify(ConfigModel &model, ConfigModel::notifications_e what, int data)
{
	switch(what)
	{
	case ConfigModel::NOTIFY_SET_ROWS:
		p.ui->sb_rows->setValue(model.getRows());
		p.createButtons();
		break;
	case ConfigModel::NOTIFY_SET_COLS:
		p.ui->sb_cols->setValue(model.getCols());
		p.createButtons();
		break;
	case ConfigModel::NOTIFY_SET_VOLUME:
		if (p.ui->sl_volume->value() != model.getVolume())
			p.ui->sl_volume->setValue(model.getVolume());
		break;
	case ConfigModel::NOTIFY_SET_PLAYBACK_LOCAL:
		if (p.ui->cb_playback_locally->isChecked() != model.getPlaybackLocal())
			p.ui->cb_playback_locally->setChecked(model.getPlaybackLocal());
		break;
	case ConfigModel::NOTIFY_SET_SOUND:
		p.updateButtonText(data);
		break;
	case ConfigModel::NOTIFY_SET_MUTE_MYSELF_DURING_PB:
		if (p.ui->cb_mute_myself->isChecked() != model.getMuteMyselfDuringPb())
			p.ui->cb_mute_myself->setChecked(model.getMuteMyselfDuringPb());
		break;
	case ConfigModel::NOTIFY_SET_WINDOW_SIZE:
		{
			QSize s = p.size();
			int w = 0, h = 0;
			model.getWindowSize(&w, &h);
			if(s.width() != w || s.height() != h)
				p.resize(w, h);
		}
	default:
		break;
	}
}
