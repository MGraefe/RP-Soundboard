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


#include "ui_config_qt.h"
#include "ConfigModel.h"

class SpeechBubble;

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

protected:
	virtual void closeEvent(QCloseEvent * evt) override;

private slots:
	void onClickedPlay();
	void onClickedStop();
	void onUpdateVolume(int val);
	void onUpdatePlaybackLocal(bool val);
	void onUpdateCols(int val);
	void onUpdateRows(int val);
	void onUpdateMuteMyself(bool val);
	void showButtonContextMenu(const QPoint &point);
	void onStopBubbleFinished();
	void onButtonBubbleFinished();
	void onColsBubbleFinished();

private:
	void playSound(size_t buttonId);
	void chooseFile(size_t buttonId);
	void openAdvanced(size_t buttonId);
	void createButtons();
	void updateButtonText(int i);

	// layout
	//   | play
	//   | subLayout
	//      | choose
	struct button_element_t
	{
		QBoxLayout *layout;
		QPushButton *play;
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
};

#endif // rpsbsrc__config_qt_H__
