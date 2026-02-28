// src/SoundView.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------



#include <QPainter>
#include <QTimer>
#include "SoundView.h"
#include "SampleVisualizerThread.h"


SoundView::SoundView( QWidget *parent /*= NULL*/ ) :
	QWidget(parent),
	m_timer(new QTimer(this)),
	m_drawnBins(0)
{
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}


void SoundView::paintEvent(QPaintEvent *evt)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);

	painter.setPen(QColor(70, 70, 70));
	painter.setBrush(QColor(30, 30, 30));
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

	painter.setPen(QColor(255, 255, 255));
	painter.setBrush(QColor(255, 255, 255));
	drawWaves(&painter);

	double songLength = SampleVisualizerThread::GetInstance().fileLength();
	double start = m_soundInfo.getStartTime();
	double playTime = m_soundInfo.getPlayTime();
	double end = (playTime > 0.0) ? (start + playTime) : songLength;
	int startPixel = int(start / songLength * (width() - 1));
	int endPixel = int(end / songLength * (width() - 1));

	painter.setPen(QColor(255, 174, 0));
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(startPixel, 0, endPixel - startPixel, height() - 1);

	painter.setCompositionMode(QPainter::CompositionMode_Multiply);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(0, 130, 230, 255));
	painter.drawRect(startPixel + 1, 1, endPixel - startPixel - 1, height() - 2);

	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(100, 100, 100));
	if (start > 0.0)
		painter.drawRect(0, 0, startPixel, height() - 1);
	if (end < songLength)
		painter.drawRect(endPixel + 1, 0, width() - 1 - (endPixel + 1), height() - 1);
}


void SoundView::resizeEvent(QResizeEvent *evt)
{
	// Invalidate drawings
	m_drawnBins = 0;
}


void SoundView::setSound( const SoundInfo &sound )
{
	bool filenameDiffers = m_soundInfo.filename != sound.filename;
	m_soundInfo = sound;
	m_drawnBins = 0;
	if(filenameDiffers && !sound.filename.isEmpty())
	{
		SampleVisualizerThread::GetInstance().startAnalysis(sound.filename.toUtf8(), 1024);
		m_timer->start(250);
	}
	else
	{
		update();
	}
}


void SoundView::onTimer()
{
	update();
}


void SoundView::drawWaves(QPainter *painter)
{
	preparePaths();

	painter->drawPath(m_path[0]);
	painter->drawPath(m_path[1]);
}


void SoundView::preparePaths()
{
	SampleVisualizerThread &t = SampleVisualizerThread::GetInstance();
	size_t bins = t.getBinsProcessed();
	if(m_drawnBins < bins)
	{
		double fhh = (double)height() * 0.5;
		double fw = (double)width();
		double fbins = (double)bins;
		double shortScale = 1.0 / ((double)std::numeric_limits<short>::max() * 1.1);
		m_path[0] = QPainterPath(QPointF(0.0, fhh));
		m_path[1] = QPainterPath(QPointF(0.0, fhh));
		for(size_t i = 0; i < bins; ++i)
		{
			double binValue0 = (double)t.getBins()[i * 2] * shortScale;
			double binValue1 = (double)t.getBins()[i * 2 + 1] * shortScale;
			double x = (double)i * (1.0 / 1024.0) * fw;
			m_path[0].lineTo(x, (1.0 + binValue0) * fhh);
			m_path[1].lineTo(x, (1.0 + binValue1) * fhh);
		}
		double endx = fw * fbins * (1.0 / 1024.0);
		m_path[0].lineTo(endx, fhh);
		m_path[1].lineTo(endx, fhh);
		m_path[0].closeSubpath();
		m_path[1].closeSubpath();
		m_drawnBins = bins;
	}
}

