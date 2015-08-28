// src/SpeechBubble.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#pragma once
#ifndef rpsbsrc__SpeechBubble_H__
#define rpsbsrc__SpeechBubble_H__

#include <QtWidgets/QDialog>

class QLayout;
class QLabel;

class SpeechBubble : public QDialog
{
	Q_OBJECT

public:
	explicit SpeechBubble(QWidget *parent = 0);
	void setText(const QString &text);
	void attachTo(QWidget *widget);

protected:
	void paintEvent(QPaintEvent *evt) override;
	bool eventFilter(QObject *object, QEvent *evt) override;

private:
	QString m_text;
	QWidget *m_attach;
};

#endif // rpsbsrc__SpeechBubble_H__