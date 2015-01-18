#ifndef about_qt_H__
#define about_qt_H__

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QBoxLayout>

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