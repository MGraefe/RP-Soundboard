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

signals:
	void fileDropped(const QList<QUrl>&);
	void buttonDropped(SoundButton *button);

private:
	bool pressing;
	bool dragging;
	QPoint dragStart;
	
};

#endif // SOUNDBUTTON_H
