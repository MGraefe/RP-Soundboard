// src/soundview_qt.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------



#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include "soundview_qt.h"
#include "SampleVisualizerThread.h"


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SoundView::SoundView( QWidget *parent /*= NULL*/ ) :
	QWidget(parent),
	m_timer(new QTimer(this)),
	m_drawnBins(0)
{
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundView::paintEvent(QPaintEvent *evt)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);

	painter.setPen(QColor(70, 70, 70));
	painter.setBrush(QColor(30, 30, 30));
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

	painter.setPen(QColor(0, 130, 230));
	painter.setBrush(QColor(0, 130, 230));
	drawWaves(&painter);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundView::resizeEvent(QResizeEvent *evt)
{
	// Invalidate drawings
	m_drawnBins = 0;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundView::setSound( const SoundInfo &sound )
{
	m_soundInfo = sound;
	m_drawnBins = 0;
	if(!sound.filename.isEmpty())
	{
		SampleVisualizerThread::GetInstance().startAnalysis(sound.filename.toUtf8(), 1024);
		m_timer->start(250);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundView::onTimer()
{
	update();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundView::drawWaves(QPainter *painter)
{
	preparePaths();

	painter->drawPath(m_path[0]);
	painter->drawPath(m_path[1]);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
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

