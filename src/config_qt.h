// src/config_qt.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__config_qt_H__
#define rpsbsrc__config_qt_H__

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenu>
#include <QtCore/QPointer>
#include <QTimer>
#include <QList>
#include <QUrl>
#include <QRadioButton>


#include "ui_config_qt.h"
#include "ConfigModel.h"

class SpeechBubble;
class ExpandableSection;
class SoundButton;

namespace Ui {
	class ConfigQt;
}

class ConfigQt : public QWidget
{
	Q_OBJECT

public:
	explicit ConfigQt(ConfigModel *model, QWidget *parent = 0);

	void createBubbles();

	virtual ~ConfigQt();

	static QString getShortcutString(const char *internalName);
	static QString getShortcutString(size_t buttonId);
    static QString getConfigShortcutString(int cfg);
	static void openHotkeySetDialog(size_t buttonId, QWidget *parent);
	void onHotkeyRecordedEvent(const char *keyword, const char *key);

    void setConfiguration(int cfg);
    bool hotkeysEnabled();

protected:
	virtual void closeEvent(QCloseEvent * evt) override;
	virtual void showEvent(QShowEvent *evt) override;

private slots:
	void onClickedPlay();
	void onClickedStop();
	void onUpdateVolume(int val);
	void onUpdateMuteLocally(bool val);
	void onUpdateCols(int val);
	void onUpdateRows(int val);
	void onUpdateMuteMyself(bool val);
	void showButtonContextMenu(const QPoint &point);
	void onStopBubbleFinished();
	void onButtonBubbleFinished();
	void onColsBubbleFinished();
	void showStopButtonContextMenu(const QPoint &point);
	void showPauseButtonContextMenu(const QPoint &point);
	void onStartPlayingSound(bool preview, QString filename);
	void onStopPlayingSound();
	void onPausePlayingSound();
	void onUnpausePlayingSound();
	void onPlayingIconTimer();
	void onUpdateShowHotkeysOnButtons(bool val);
	void onButtonFileDropped(const QList<QUrl> &urls);
	void onButtonPausePressed();
	void onButtonDroppedOnButton(SoundButton *button);

    void onSetConfig();
    void onConfigHotkey();


    void onSaveModel();
    void onLoadModel();

signals:
	void hotkeyRecordedEvent(QString keyword, QString key);

private:
	void showSetHotkeyMenu(const char *hotkeyName, const QPoint &point);
	void setPlayingLabelIcon(int index);
	void playSound(size_t buttonId);
	void chooseFile(size_t buttonId);

	void setButtonFile(size_t buttonId, const QString &fn, bool askForDisablingCrop = true);

	void openAdvanced(size_t buttonId);
	void deleteButton(size_t buttonId);
	void createButtons();
	void updateButtonText(int i);
	void openHotkeySetDialog(size_t buttonId);
	// layout
	//   | play
	//   | subLayout
	//      | choose
	struct button_element_t
	{
		QBoxLayout *layout;
		SoundButton *play;
	};


	class ModelObserver : public ConfigModel::Observer
	{
	public:
		ModelObserver(ConfigQt &parent) : p(parent) {}
		void notify(ConfigModel &model, ConfigModel::notifications_e what, int data) override;
	private:
		ConfigQt &p;
	};

	Ui::ConfigQt *ui;
	std::vector<button_element_t> m_buttons;
	ConfigModel *m_model;
	QBoxLayout *m_configArea;
	ModelObserver m_modelObserver;
	QMenu m_buttonContextMenu;
	QPointer<SpeechBubble> m_buttonBubble;
	QAction *actSetHotkey;
    ExpandableSection *settingsSection;
    ExpandableSection *fileSection;
    QTimer *playingIconTimer;
	int playingIconIndex;
	QIcon m_pauseIcon;
	QIcon m_playIcon;
	std::array<QRadioButton*, NUM_CONFIGS> m_configRadioButtons;
	std::array<QPushButton*, NUM_CONFIGS> m_configHotkeyButtons;
};

#endif // rpsbsrc__config_qt_H__
