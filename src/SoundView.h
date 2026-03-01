// src/SoundView.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <QWidget>
#include <QPainterPath>
#include <memory>

#include "SoundInfo.h"

class QTimer;

class SoundView : public QWidget
{
	Q_OBJECT

  public:
	SoundView(QWidget* parent = nullptr);
	void setSound(const SoundInfo& sound);

  protected:
	void paintEvent(QPaintEvent* evt);
	void resizeEvent(QResizeEvent* evt);

  private slots:
	void onTimer();

  private:
	void drawWaves(QPainter* painter);
	void preparePaths();

  private:
	SoundInfo m_soundInfo;
	QTimer* m_timer;
	size_t m_drawnBins;
	QPainterPath m_path[2];
};
