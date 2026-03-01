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

const QString& getButtonMime()
{
	static QString uuid = QUuid::createUuid().toString();
	return uuid;
}


SoundButton::SoundButton(QWidget* parent) :
	QPushButton(parent),
	pressing(false),
	dragging(false)
{
	setAcceptDrops(true);
}


SoundButton::~SoundButton() {}


void SoundButton::dragEnterEvent(QDragEnterEvent* evt)
{
	if (evt->mimeData()->hasUrls() || evt->mimeData()->hasFormat(getButtonMime()))
	{
		applyBackgroundColor(QColor(153, 204, 255));
		evt->acceptProposedAction();
	}
}


void SoundButton::dragMoveEvent(QDragMoveEvent* evt)
{
	if (evt->mimeData()->hasUrls() || evt->mimeData()->hasFormat(getButtonMime()))
		evt->acceptProposedAction();
}


void SoundButton::dragLeaveEvent(QDragLeaveEvent*)
{
	sb_disableHotkeysTemporarily(false);

	pressing = false;
	dragging = false;
	applyBackgroundColor(backgroundColor);
}


void SoundButton::dropEvent(QDropEvent* evt)
{
	pressing = false;
	dragging = false;
	applyBackgroundColor(backgroundColor);
	SoundButton* button = nullptr;
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


void SoundButton::mousePressEvent(QMouseEvent* evt)
{
	// If the user has a hotkey 'switch config' bound to mouse 1 then
	// this button is actually deleted before mouseReleaseEvent is called
	// hence the clicked() signal is never emitted, resulting in silence
	// instead of a lovely sound. Disabling hotkeys while a button is pressed
	// is a workaround for this.
	sb_disableHotkeysTemporarily(true);

	pressing = true;
	dragStart = evt->pos();
	QPushButton::mousePressEvent(evt);
}


void SoundButton::mouseMoveEvent(QMouseEvent* evt)
{
	if (pressing && !dragging && (evt->pos() - dragStart).manhattanLength() > 5)
	{
		dragging = true;

		QMimeData* mimeData = new QMimeData;
		mimeData->setData(getButtonMime(), QByteArray());
		mimeData->setProperty("sourceButton", qVariantFromValue(this));

		QDrag* drag = new QDrag(this);
		drag->setMimeData(mimeData);
		drag->setPixmap(QPixmap(":/icon/img/speaker_icon_3_64.png"));
		drag->exec();
	}

	QPushButton::mouseMoveEvent(evt);
}


void SoundButton::setBackgroundColor(const QColor& color)
{
	backgroundColor = color;
	applyBackgroundColor(color);
}


void SoundButton::applyBackgroundColor(const QColor& color)
{
	if (color.alpha() != 0)
	{
		float brightness = 0.2126f * color.redF() + 0.7152f * color.greenF() + 0.0722f * color.blueF();
		QColor textColor = brightness < 0.5f ? Qt::white : Qt::black;
		setStyleSheet(QString("color: %1; background-color: %2;").arg(textColor.name(), color.name()));
	}
	else
		setStyleSheet(QString());
}


void SoundButton::mouseReleaseEvent(QMouseEvent* evt)
{
	sb_disableHotkeysTemporarily(false);

	pressing = false;
	dragging = false;
	QPushButton::mouseReleaseEvent(evt);
}
