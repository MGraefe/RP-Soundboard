#include "SoundButton.h"
#include "main.h"
#include "ConfigModel.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SoundButton::SoundButton(QWidget *parent) : 
	QPushButton(parent)
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
	if (evt->mimeData()->hasUrls())
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
	if (evt->mimeData()->hasUrls())
		evt->acceptProposedAction();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::dragLeaveEvent(QDragLeaveEvent *evt)
{
	setStyleSheet("");
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SoundButton::dropEvent(QDropEvent *evt)
{
	if (!evt->mimeData()->hasUrls())
		return;
	setStyleSheet("");
	emit fileDropped(evt->mimeData()->urls());
}
