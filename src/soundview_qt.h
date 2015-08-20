#ifndef soundview_qt_H__
#define soundview_qt_H__

#include <QtWidgets/QWidget>
#include <memory>

#include "SoundInfo.h"

class QTimer;

class SoundView : public QWidget
{
	Q_OBJECT

public:
	SoundView(QWidget *parent = NULL);
	void setSound(const SoundInfo &sound);

protected:
	void paintEvent(QPaintEvent *evt);
	void resizeEvent(QResizeEvent *evt);

private slots:
	void onTimer();

private:
	void drawWaves(QPainter *painter);
	void preparePaths();

private:
	SoundInfo m_soundInfo;
	QTimer *m_timer;
	size_t m_drawnBins;
	QPainterPath m_path[2];
};

#endif // soundview_qt_H__
