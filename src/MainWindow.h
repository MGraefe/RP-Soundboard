// src/MainWindow.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QBoxLayout>
#include <QMenu>
#include <QPointer>
#include <QTimer>
#include <QList>
#include <QUrl>
#include <QRadioButton>


#include "ui_MainWindow.h"
#include "ConfigModel.h"

class SpeechBubble;
class ExpandableSection;
class SoundButton;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	explicit MainWindow(ConfigModel *model, QWidget *parent = 0);

	void createBubbles();

	virtual ~MainWindow();

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
	void onUpdateVolumeLocal(int val);
	void onUpdateVolumeRemote(int val);
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
	void onUpdateHotkeysDisabled(bool val);
	void onButtonFileDropped(const QList<QUrl> &urls);
	void onButtonPausePressed();
	void onButtonDroppedOnButton(SoundButton *button);
	void onFilterEditTextChanged(const QString &filter);
	void onVolumeSliderContextMenuLocal(const QPoint &point);
	void onVolumeSliderContextMenuRemote(const QPoint &point);

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
	void createConfigButtons();
	void updateButtonText(int i);
	void openHotkeySetDialog(size_t buttonId);
	void openButtonColorDialog(size_t buttonId);
	QString unescapeCustomText(const QString &text);

	class ModelObserver : public ConfigModel::Observer
	{
	public:
		ModelObserver(MainWindow &parent) : p(parent) {}
		void notify(ConfigModel &model, ConfigModel::notifications_e what, int data) override;
	private:
		MainWindow &p;
	};

	Ui::MainWindow *ui;
	std::vector<SoundButton*> m_buttons;
	ConfigModel *m_model;
	QBoxLayout *m_configArea;
	ModelObserver m_modelObserver;
	QMenu m_buttonContextMenu;
	QPointer<SpeechBubble> m_buttonBubble;
	QAction *actSetHotkey;
    ExpandableSection *settingsSection;
    ExpandableSection *configsSection;
    QTimer *playingIconTimer;
	int playingIconIndex;
	QIcon m_pauseIcon;
	QIcon m_playIcon;
	std::array<QRadioButton*, NUM_CONFIGS> m_configRadioButtons;
	std::array<QPushButton*, NUM_CONFIGS> m_configHotkeyButtons;
};

