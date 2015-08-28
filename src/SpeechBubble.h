
#ifndef SpeechBubble_H__
#define SpeechBubble_H__

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

#endif // SpeechBubble_H__