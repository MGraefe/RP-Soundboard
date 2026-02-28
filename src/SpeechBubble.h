// src/SpeechBubble.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#pragma once

#include <QDialog>
#include <QColor>

class QLayout;
class QLabel;

class SpeechBubble : public QDialog
{
	Q_OBJECT
	typedef QDialog BaseClass;

public:
	explicit SpeechBubble(QWidget *parent = 0);
	void setText(const QString &text);
	void attachTo(QWidget *widget);
	void setBackgroundColor(const QColor &color);
	void setClosable(bool closable);
	void setBubbleStyle(bool bubbleStyle);

protected:
	void paintEvent(QPaintEvent *evt) override;
	bool eventFilter(QObject *object, QEvent *evt) override;
	void mouseReleaseEvent(QMouseEvent *evt) override;
	void mouseMoveEvent(QMouseEvent *evt) override;

signals:
	void closePressed();

private:
	void recalcPos();
	QRect getCloseButtonRect();

	QString m_text;
	QWidget *m_attach;
	int m_tipHeight;
	int m_tipWidth;
	int m_tipDistLeft;
	bool m_mouseOverCloseButton;
	bool m_closable;
	bool m_bubbleStyle;
	QColor m_backgroundColor;
};

