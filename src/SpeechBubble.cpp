// src/SpeechBubble.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "SpeechBubble.h"
#include <QtGui/QPainter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtCore/QEvent>

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SpeechBubble::SpeechBubble( QWidget *parent /*= 0*/ ) :
	QDialog(parent, Qt::FramelessWindowHint)
{

}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::setText( const QString &text )
{
	m_text = text;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::paintEvent(QPaintEvent *evt)
{
	QPainter painter(this);
	//painter.setRenderHint(QPainter::Antialiasing, false);

	painter.setPen(QColor(0, 0, 0));
	painter.setBrush(QColor(255, 222, 147));
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

	QTextOption option;
	option.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	painter.drawText(QRect(0, 0, width() - 1, height() - 1), m_text, option);

	QDialog::paintEvent(evt);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::attachTo( QWidget *widget )
{
	m_attach = widget;
	m_attach->installEventFilter(this);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool SpeechBubble::eventFilter(QObject *object, QEvent *evt)
{
	if(object == m_attach)
	{
		if(evt->type() == QEvent::Resize)
		{
			//TODO: recalc position
		}
		else if(evt->type() == QEvent::Move)
		{
			//TODO: recalc position
		}
	}

	return QDialog::eventFilter(object, evt);
}
