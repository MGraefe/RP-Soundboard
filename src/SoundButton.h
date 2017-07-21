#ifndef SOUNDBUTTON_H
#define SOUNDBUTTON_H

#include <QPushButton>
#include <QList>
#include <QUrl>

class ConfigModel;

class SoundButton : public QPushButton
{
	Q_OBJECT

public:
	SoundButton(QWidget *parent);
	virtual ~SoundButton();

	virtual void dragEnterEvent(QDragEnterEvent *evt) override;
	virtual void dragMoveEvent(QDragMoveEvent *evt) override;
	virtual void dragLeaveEvent(QDragLeaveEvent *evt) override;
	virtual void dropEvent(QDropEvent *evt) override;
	virtual void mousePressEvent(QMouseEvent *evt) override;
	virtual void mouseReleaseEvent(QMouseEvent *evt) override;
	virtual void mouseMoveEvent(QMouseEvent *evt) override;

	void setBackgroundColor(const QColor &color);

signals:
	void fileDropped(const QList<QUrl>&);
	void buttonDropped(SoundButton *button);

private:
	void applyBackgroundColor(const QColor &color);

	bool pressing;
	bool dragging;
	QPoint dragStart;
	QColor backgroundColor;
	
};

#endif // SOUNDBUTTON_H
