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
#include <QtGui/QMouseEvent>

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SpeechBubble::SpeechBubble(QWidget *parent /*= 0*/) :
	BaseClass(parent, Qt::FramelessWindowHint),
	m_attach(nullptr),
	m_tipHeight(25),
	m_tipWidth(15),
	m_tipDistLeft(10),
	m_mouseOverCloseButton(false),
	m_closable(true),
	m_bubbleStyle(true),
	m_backgroundColor(QColor(255, 183, 59))
{
	setAttribute(Qt::WA_TranslucentBackground);
	if (parent)
		parent->installEventFilter(this);
	setMouseTracking(true);
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
	painter.setBrush(m_backgroundColor);

	// Draw outer appearance
	if (m_bubbleStyle)
	{
		QPoint polygonPoints[] = {
			QPoint(0, m_tipHeight), // top left
			QPoint(m_tipDistLeft, m_tipHeight),
			QPoint(m_tipDistLeft, 0),
			QPoint(m_tipDistLeft + m_tipWidth, m_tipHeight),
			QPoint(width() - 1, m_tipHeight), // top right
			QPoint(width() - 1, height() - 1), // bottom right
			QPoint(0, height() - 1) // bottom left
		};
		painter.drawPolygon(polygonPoints, 7);
	}
	else
	{
		QPoint polygonPoints[] = {
			QPoint(0, 0), // top left
			QPoint(width() - 1, 0), // top right
			QPoint(width() - 1, height() - 1), // bottom right
			QPoint(0, height() - 1) // bottom left
		};
		painter.drawPolygon(polygonPoints, 4);
	}

	// Draw text
	QTextOption option;
	option.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	int maxTextPos = m_bubbleStyle ? m_tipHeight : 0;
	painter.drawText(QRect(0, maxTextPos, width() - 1, height() - maxTextPos - 1), m_text, option);

	// Draw closing X
	if (m_closable)
	{
		QRect xRect = getCloseButtonRect();
		if (m_mouseOverCloseButton)
		{
			painter.drawRect(xRect + QMargins(2, 2, 1, 1));
			painter.setPen(QColor(100, 100, 100));
		}
		painter.drawLine(xRect.topLeft(), xRect.bottomRight());
		painter.drawLine(xRect.bottomLeft(), xRect.topRight());
	}

	BaseClass::paintEvent(evt);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::attachTo( QWidget *widget )
{
	m_attach = widget;
	m_attach->installEventFilter(this);
	recalcPos();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::setBackgroundColor(const QColor & color)
{
	m_backgroundColor = color;
	update();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::setClosable(bool closable)
{
	m_closable = closable;
	update();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::setBubbleStyle(bool bubbleStyle)
{
	m_bubbleStyle = bubbleStyle;
	update();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool SpeechBubble::eventFilter(QObject *object, QEvent *evt)
{
	if(object == parent())
	{
		switch(evt->type())
		{
			case QEvent::Move:
			case QEvent::Resize:
				recalcPos();
				break;
			case QEvent::Close:
				close();
				break;
			case QEvent::Hide:
				hide(); 
				break;
			case QEvent::Show:
				recalcPos();
				show(); 
				break;
			default:
				break;
		}
	}

	return BaseClass::eventFilter(object, evt);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::mouseReleaseEvent( QMouseEvent *evt )
{
	if (!m_closable)
		return;

	// Mouse inside closing x rect?
	if (getCloseButtonRect().contains(this->mapFromGlobal(QCursor::pos())))
	{
		emit closePressed();
		delete this;
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::mouseMoveEvent( QMouseEvent *evt )
{
	if (!m_closable)
		return;

	if ((getCloseButtonRect() + QMargins(1, 1, 1, 1)).contains(evt->pos()))
	{
		if(!m_mouseOverCloseButton)
		{
			m_mouseOverCloseButton = true;
			repaint();
		}
	}
	else
	{
		if(m_mouseOverCloseButton)
		{
			m_mouseOverCloseButton = false;
			repaint();
		}
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SpeechBubble::recalcPos()
{
	QPoint pos;
	if (m_bubbleStyle)
		pos = QPoint(m_attach->width() / 2 - 10, m_attach->height() / 2);
	else
		pos = QPoint(m_attach->width() / 2 - width() / 2, m_attach->height() / 2 - height() / 2);
	this->move(m_attach->mapToGlobal(pos));
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QRect SpeechBubble::getCloseButtonRect()
{
	int xsize = 8;
	int xborder = 4;
	return QRect(width() - xsize - xborder, m_tipHeight + xborder, xsize, xsize);
}


