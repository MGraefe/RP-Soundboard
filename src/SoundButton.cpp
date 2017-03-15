#include "SoundButton.h"
#include "main.h"
#include "ConfigModel.h"
#include "ts3log.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUuid>
#include <QDrag>
#include <QStyle>

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
const QString &getButtonMime()
{
	static QString uuid = QUuid::createUuid().toString();
	return uuid;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SoundButton::SoundButton(QWidget *parent) : 
	QPushButton(parent),
	pressing(false),
	dragging(false)
{
	setAcceptDrops(true);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SoundButton::~SoundButton()
{}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::dragEnterEvent(QDragEnterEvent *evt)
{
	if (evt->mimeData()->hasUrls() || evt->mimeData()->hasFormat(getButtonMime()))
	{
		setStyleSheet("background-color: #99ccff;");
		evt->acceptProposedAction();
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::dragMoveEvent(QDragMoveEvent *evt)
{
	if (evt->mimeData()->hasUrls() || evt->mimeData()->hasFormat(getButtonMime()))
		evt->acceptProposedAction();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::dragLeaveEvent(QDragLeaveEvent *evt)
{
	pressing = false;
	dragging = false;
	setBackgroundColor(backgroundColor);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::dropEvent(QDropEvent *evt)
{
	pressing = false;
	dragging = false;
	setBackgroundColor(backgroundColor);
	SoundButton *button = nullptr;
	if (evt->mimeData()->hasUrls())
	{
		emit fileDropped(evt->mimeData()->urls());
	}
	else if (evt->mimeData()->hasFormat(getButtonMime()) && evt->source() != this)
	{
		button = evt->mimeData()->property("sourceButton").value<SoundButton*>();
		if (button)
		{
			button->adjustSize();
			emit buttonDropped(button);
		}
	}

	QPushButton::dropEvent(evt);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::mousePressEvent(QMouseEvent *evt)
{
	pressing = true;
	dragStart = evt->pos();
	QPushButton::mousePressEvent(evt);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::mouseMoveEvent(QMouseEvent *evt)
{
	if (pressing && !dragging &&
		(evt->pos() - dragStart).manhattanLength() > 5)
	{
		dragging = true;

		QMimeData *mimeData = new QMimeData;
		mimeData->setData(getButtonMime(), QByteArray());
		mimeData->setProperty("sourceButton", qVariantFromValue(this));

		QDrag *drag = new QDrag(this);
		drag->setMimeData(mimeData);
		drag->setPixmap(QPixmap(":/icon/img/speaker_icon_3_64.png"));
		drag->exec();
	}

	QPushButton::mouseMoveEvent(evt);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::setBackgroundColor(const QColor &color)
{
	backgroundColor = color;
	
	if (color.alpha() != 0)
	{
		float brightness = 0.2126f * color.redF() + 0.7152f * color.greenF() + 0.0722f * color.blueF();
		QColor textColor = brightness < 0.5f ? Qt::white : Qt::black;
		setStyleSheet(QString("color: %1; background-color: %2;").arg(textColor.name(), color.name()));
	}
	else
		setStyleSheet(QString());
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::mouseReleaseEvent(QMouseEvent *evt)
{
	pressing = false;
	dragging = false;
	QPushButton::mouseReleaseEvent(evt);
}
