#ifndef about_qt_H__
#define about_qt_H__

#include <QtWidgets/QWidget>

#include "ui_about_qt.h"

namespace Ui {
	class AboutQt;
}


class AboutQt : public QWidget
{
	Q_OBJECT

public:
	explicit AboutQt(QWidget *parent = 0);

private:
	Ui::AboutQt *ui;
};


#endif