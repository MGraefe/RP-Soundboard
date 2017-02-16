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
#include <QPropertyAnimation>

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
#include "SoundButton.h"

#ifdef _WIN32
#include "Windows.h"
#endif

enum button_choices_e {
	BC_CHOOSE = 0,
	BC_ADVANCED,
	BC_SET_HOTKEY,
	BC_DELETE,
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
    /* Ensure resources are loaded */
    Q_INIT_RESOURCE(qtres);

    m_pauseIcon = QIcon(":/icon/img/pausebutton_32.png");
    m_playIcon = QIcon(":/icon/img/playarrow_32.png");

    ui->setupUi(this);
	//setAttribute(Qt::WA_DeleteOnClose);

	createConfigButtons();

    settingsSection = new ExpandableSection("Settings", 200, this);
    settingsSection->setContentLayout(*ui->settingsWidget->layout());
    layout()->addWidget(settingsSection);

    configsSection = new ExpandableSection("Configurations", 200, this);
    configsSection->setContentLayout(*ui->configsWidget->layout());
    layout()->addWidget(configsSection);

    QAction *actChooseFile = new QAction("Choose File", this);
	actChooseFile->setData((int)BC_CHOOSE);
	m_buttonContextMenu.addAction(actChooseFile);

	QAction *actAdvancedOpts = new QAction("Advanced Options", this);
	actAdvancedOpts->setData((int)BC_ADVANCED);
	m_buttonContextMenu.addAction(actAdvancedOpts);

	actSetHotkey = new QAction("Set hotkey", this);
	actSetHotkey->setData((int)BC_SET_HOTKEY);
	m_buttonContextMenu.addAction(actSetHotkey);

	QAction *actDeleteButton = new QAction("Make button great again (delete)", this);
	actDeleteButton->setData((int)BC_DELETE);
	m_buttonContextMenu.addAction(actDeleteButton);

	createButtons();

	ui->b_stop->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->b_pause->setContextMenuPolicy(Qt::CustomContextMenu);
	
	connect(ui->b_stop, SIGNAL(clicked()), this, SLOT(onClickedStop()));
	connect(ui->b_stop, SIGNAL(customContextMenuRequested(const QPoint&)), this,
		SLOT(showStopButtonContextMenu(const QPoint&)));
	connect(ui->b_pause, SIGNAL(clicked()), this, SLOT(onButtonPausePressed()));
	connect(ui->b_pause, SIGNAL(customContextMenuRequested(const QPoint&)), this,
		SLOT(showPauseButtonContextMenu(const QPoint&)));
	connect(ui->sl_volume, SIGNAL(valueChanged(int)), this, SLOT(onUpdateVolume(int)));
	connect(ui->cb_mute_locally, SIGNAL(clicked(bool)), this, SLOT(onUpdateMuteLocally(bool)));
	connect(ui->sb_rows, SIGNAL(valueChanged(int)), this, SLOT(onUpdateRows(int)));
	connect(ui->sb_cols, SIGNAL(valueChanged(int)), this, SLOT(onUpdateCols(int)));
	connect(ui->cb_mute_myself, SIGNAL(clicked(bool)), this, SLOT(onUpdateMuteMyself(bool)));
	connect(ui->cb_show_hotkeys_on_buttons, SIGNAL(clicked(bool)), this, SLOT(onUpdateShowHotkeysOnButtons(bool)));
	connect(ui->cb_disable_hotkeys, SIGNAL(clicked(bool)), this, SLOT(onUpdateHotkeysDisabled(bool)));

    /* Load/Save Model */
    connect(ui->pushLoad, SIGNAL(released()), this, SLOT(onLoadModel()));
    connect(ui->pushSave, SIGNAL(released()), this, SLOT(onSaveModel()));
    
    ui->playingIconLabel->hide();
	ui->playingLabel->setText("");
	playingIconTimer = new QTimer(this);
	playingIconTimer->setInterval(150);
	connect(playingIconTimer, SIGNAL(timeout()), this, SLOT(onPlayingIconTimer()));

	Sampler *sampler = sb_getSampler();
	connect(sampler, SIGNAL(onStartPlaying(bool, QString)), this, SLOT(onStartPlayingSound(bool, QString)), Qt::QueuedConnection);
	connect(sampler, SIGNAL(onStopPlaying()), this, SLOT(onStopPlayingSound()), Qt::QueuedConnection);
	connect(sampler, SIGNAL(onPausePlaying()), this, SLOT(onPausePlayingSound())); // No queued connection since signal is emitted from GUI Thread
	connect(sampler, SIGNAL(onUnpausePlaying()), this, SLOT(onUnpausePlayingSound())); // No queued connection since signal is emitted from GUI Thread

	createBubbles();

	m_model->addObserver(&m_modelObserver);

    /* Force configuration 0 */
    setConfiguration(0);
}

void ConfigQt::setConfiguration(int cfg)
{
	if (cfg < 0 || cfg >= NUM_CONFIGS)
	{
		logError("Invalid config id: %i", cfg);
		return;
	}

	m_configRadioButtons[cfg]->setChecked(true);
	m_model->setConfiguration(cfg);
	ui->labelStatus->setText(QString("Configuration %1").arg(cfg + 1));
}

void ConfigQt::onSetConfig()
{
	QRadioButton *button = qobject_cast<QRadioButton*>(sender());
	int configId = button->property("configId").toInt();
	setConfiguration(configId);
}

void ConfigQt::onConfigHotkey()
{
	QPushButton *button = qobject_cast<QPushButton*>(sender());
	int configId = button->property("configId").toInt();

	char buf[16];
	sb_getInternalConfigHotkeyName(configId, buf);
    ts3Functions.requestHotkeyInputDialog(getPluginID(), buf, configId, this);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigQt::~ConfigQt()
{
	m_model->remObserver(&m_modelObserver);
	delete ui;
}

void ConfigQt::onSaveModel()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Choose File to Save"), QString(), tr("Ini Files (*.ini)"));
    if (fn.isNull())
        return;
    m_model->writeConfig(fn);
}

void ConfigQt::onLoadModel()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Choose File to Load"), QString(), tr("Ini Files (*.ini)"));
    if (fn.isNull())
        return;
    m_model->readConfig(fn);
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
void ConfigQt::onUpdateMuteLocally(bool val)
{
	m_model->setPlaybackLocal(!val);
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

			elem.play = new SoundButton(this);
			elem.play->setProperty("buttonId", (int)m_buttons.size());
			elem.play->setText("(no file)");
			elem.play->setEnabled(true);
			elem.play->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
			elem.layout->addWidget(elem.play);
			connect(elem.play, SIGNAL(clicked()), this, SLOT(onClickedPlay()));
			elem.play->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(elem.play, SIGNAL(customContextMenuRequested(const QPoint&)), this,
				SLOT(showButtonContextMenu(const QPoint&)));
			connect(elem.play, SIGNAL(fileDropped(QList<QUrl>)), this, SLOT(onButtonFileDropped(QList<QUrl>)));
			connect(elem.play, SIGNAL(buttonDropped(SoundButton*)), this, SLOT(onButtonDroppedOnButton(SoundButton*)));

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
void ConfigQt::createConfigButtons()
{
	for (int i = 0; i < NUM_CONFIGS; i++)
	{
		m_configRadioButtons[i] = new QRadioButton(this);
		m_configRadioButtons[i]->setText(QString("Config %1").arg(i + 1));
		m_configRadioButtons[i]->setProperty("configId", i);
		m_configRadioButtons[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		ui->configsGrid->addWidget(m_configRadioButtons[i], 0, i, Qt::AlignCenter);
		connect(m_configRadioButtons[i], SIGNAL(clicked()), this, SLOT(onSetConfig()));

		m_configHotkeyButtons[i] = new QPushButton(this);
		m_configHotkeyButtons[i]->setText("Set hotkey");
		m_configHotkeyButtons[i]->setProperty("configId", i);
		m_configHotkeyButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_configHotkeyButtons[i]->setToolTipDuration(0);
		m_configHotkeyButtons[i]->setToolTip(getConfigShortcutString(i));
		ui->configsGrid->addWidget(m_configHotkeyButtons[i], 1, i, Qt::AlignCenter);
		connect(m_configHotkeyButtons[i], SIGNAL(clicked()), this, SLOT(onConfigHotkey()));

		ui->configsWidget->updateGeometry();
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool ConfigQt::hotkeysEnabled()
{
	return m_model->getHotkeysEnabled();
}

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::updateButtonText(int i)
{
	if(i >= m_buttons.size())
		return;

	QString text;
	const SoundInfo *info = m_model->getSoundInfo(i);
	if (info && !info->filename.isEmpty())
	{
		if (!info->customText.isEmpty())
			text = info->customText;
		else
			text = QFileInfo(info->filename).baseName();
	}
	else
		text = "(no file)";

	if (m_model->getShowHotkeysOnButtons())
	{
		QString shortcut = getShortcutString(i);
		if (shortcut.length() > 0)
			text = text + "\n" + shortcut;
	}
	m_buttons[i].play->setText(text);
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
			case BC_DELETE:
				deleteButton(buttonId);
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
	if (fn.isNull())
		return;
	setButtonFile(buttonId, fn);

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
void ConfigQt::deleteButton(size_t buttonId)
{
	const SoundInfo *info = m_model->getSoundInfo(buttonId);
	if (info)
		m_model->setSoundInfo(buttonId, SoundInfo());
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
		settingsSection->setExpanded(true);
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
	showSetHotkeyMenu("stop_all", ui->b_stop->mapToGlobal(point));
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::showPauseButtonContextMenu(const QPoint &point)
{
	showSetHotkeyMenu("pause_all", ui->b_pause->mapToGlobal(point));
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::showSetHotkeyMenu(const char *hotkeyName, const QPoint &point)
{
	QString hotkeyString = getShortcutString(hotkeyName);
	QString hotkeyText = "Set hotkey (Current: " +
		(hotkeyString.isEmpty() ? QString("None") : hotkeyString) + ")";

	QMenu menu;
	menu.addAction(hotkeyText);
	QAction *action = menu.exec(point);
	if (action)
		ts3Functions.requestHotkeyInputDialog(getPluginID(), hotkeyName, 0, this);
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
	playingIconTimer->start();
	ui->b_stop->setEnabled(true);
	ui->b_pause->setEnabled(true);
	ui->b_pause->setIcon(m_pauseIcon);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onStopPlayingSound()
{
	playingIconTimer->stop();
	ui->playingLabel->setText("");
	ui->playingIconLabel->hide();
	// You cannot set hotkeys for disabled buttons because their context menu doesn't open
	//ui->b_stop->setEnabled(false);
	//ui->b_pause->setEnabled(false);
	ui->b_pause->setIcon(m_pauseIcon);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onPausePlayingSound()
{
	playingIconTimer->stop();
	ui->b_pause->setIcon(m_playIcon);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUnpausePlayingSound()
{
	playingIconTimer->start();
	ui->b_pause->setIcon(m_pauseIcon);
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
    hotkeyName[0] = 0;
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

QString ConfigQt::getConfigShortcutString(int cfg)
{
	char buf[16];
	sb_getInternalConfigHotkeyName(cfg, buf);
    QString shortcut = getShortcutString(buf);
    if (!shortcut.isEmpty())
        return shortcut;

    return QString("no hotkey");
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onHotkeyRecordedEvent(const char *keyword, const char *key)
{
    QString sKey = key;

	int configId = -1;
	if (sscanf(keyword, "config_%i", &configId) == 1)
	{
		if (configId >= 0 && configId < NUM_CONFIGS)
			m_configHotkeyButtons[configId]->setToolTip(sKey);
		else
			logError("Invalid hotkey keyword: %s", keyword);
	}
    else
    {
        QString sKeyword = keyword;

        emit hotkeyRecordedEvent(sKey, sKeyword);

        if (m_model->getShowHotkeysOnButtons())
            for (size_t i = 0; i < m_buttons.size(); i++)
                updateButtonText(i);
    }
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateShowHotkeysOnButtons(bool val)
{
	m_model->setShowHotkeysOnButtons(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateHotkeysDisabled(bool val)
{
	m_model->setHotkeysEnabled(!val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::showEvent(QShowEvent *evt)
{
	QWidget::showEvent(evt);

	for(size_t i = 0; i < m_buttons.size(); i++)
		updateButtonText(i);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onButtonFileDropped(const QList<QUrl> &urls)
{
	int buttonId = sender()->property("buttonId").toInt();
	int buttonNr = 0;

	for(int i = 0; i < urls.size(); i++)
	{
		if (buttonNr == 1)
		{
			QMessageBox msgbox(QMessageBox::Icon::Question, "Fill buttons?",
				"You dropped multiple files. Consecutively apply them to the buttons following the one you dropped your files on?",
				QMessageBox::Yes | QMessageBox::No, this);
			if (msgbox.exec() == QMessageBox::No)
				break;
		}

		if (urls[i].isLocalFile())
		{
			QFileInfo info(urls[i].toLocalFile());
			if (info.isFile())
			{
				setButtonFile(buttonId + buttonNr, urls[i].toLocalFile(), buttonNr == 0);
				++buttonNr;
			}
			else if (buttonNr == 0)
			{
				QMessageBox msgBox(QMessageBox::Icon::Critical, "Unsupported drop type", "Some things could not be dropped here :(", QMessageBox::Ok, this);
				msgBox.exec();
			}
		}
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::setButtonFile(size_t buttonId, const QString &fn, bool askForDisablingCrop)
{
	const SoundInfo *info = m_model->getSoundInfo(buttonId);
	if (askForDisablingCrop && info && info->cropEnabled && info->filename != fn)
	{
		QMessageBox mb(QMessageBox::Question, "Keep crop settings?",
			"You selected a new file for a button that has 'crop sound' enabled.", QMessageBox::NoButton, this);
		QPushButton *btnDisable = mb.addButton("Disable cropping (recommended)", QMessageBox::YesRole);
		QPushButton *btnKeep = mb.addButton("Keep old crop settings", QMessageBox::NoRole);
		mb.setDefaultButton(btnDisable);
		mb.exec();
		if (mb.clickedButton() != btnKeep)
		{
			SoundInfo newInfo(*info);
			newInfo.cropEnabled = false;
			m_model->setSoundInfo(buttonId, newInfo);
		}
	}
	m_model->setFileName(buttonId, fn);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onButtonPausePressed()
{
	sb_pauseButtonPressed();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onButtonDroppedOnButton(SoundButton *button)
{
	SoundButton *btn0 = button;
	SoundButton *btn1 = qobject_cast<SoundButton*>(sender());
	int bid0 = btn0->property("buttonId").toInt();
	int bid1 = btn1->property("buttonId").toInt();
	const SoundInfo *info0 = m_model->getSoundInfo(bid0);
	const SoundInfo *info1 = m_model->getSoundInfo(bid1);

	// Copy sound info
	SoundInfo infoCopy0;
	SoundInfo infoCopy1;
	if (info0)
		infoCopy0 = *info0;
	if (info1)
		infoCopy1 = *info1;

	// And switch em
	m_model->setSoundInfo(bid0, infoCopy1);
	m_model->setSoundInfo(bid1, infoCopy0);

	// Switch button position and then animate the buttons to slide into place
	const int animDuration = 300;
	QPropertyAnimation *anim0 = new QPropertyAnimation(btn0, "pos");
	anim0->setStartValue(btn1->pos());
	anim0->setEndValue(btn0->pos());
	anim0->setDuration(animDuration);

	QPropertyAnimation *anim1 = new QPropertyAnimation(btn1, "pos");
	anim1->setStartValue(btn0->pos());
	anim1->setEndValue(btn1->pos());
	anim1->setDuration(animDuration);

	anim0->start();
	anim1->start();
	btn0->raise();
	btn1->raise();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::ModelObserver::notify(ConfigModel &model, ConfigModel::notifications_e what, int data)
{
    //p.ui->labelStatus->setText(QString("Notify Code: %1").arg((int)what));

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
		if (p.ui->cb_mute_locally->isChecked() != !model.getPlaybackLocal())
			p.ui->cb_mute_locally->setChecked(!model.getPlaybackLocal());
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
		break;
	case ConfigModel::NOTIFY_SET_SHOW_HOTKEYS_ON_BUTTONS:
		if (p.ui->cb_show_hotkeys_on_buttons->isChecked() != model.getShowHotkeysOnButtons())
			p.ui->cb_show_hotkeys_on_buttons->setChecked(model.getShowHotkeysOnButtons());
		for(size_t i = 0; i < p.m_buttons.size(); i++)
			p.updateButtonText(i);
		break;
	case ConfigModel::NOTIFY_SET_HOTKEYS_ENABLED:
		if (p.ui->cb_disable_hotkeys->isChecked() == model.getHotkeysEnabled())
			p.ui->cb_disable_hotkeys->setChecked(!model.getHotkeysEnabled());
		break;
	default:
		break;
	}
}
