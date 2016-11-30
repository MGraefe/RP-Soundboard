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
	setStyleSheet("");
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::dropEvent(QDropEvent *evt)
{
	pressing = false;
	dragging = false;
	setStyleSheet("");
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
void SoundButton::mouseReleaseEvent(QMouseEvent *evt)
{
	pressing = false;
	dragging = false;
	QPushButton::mouseReleaseEvent(evt);
}
