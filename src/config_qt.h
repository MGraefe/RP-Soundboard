#ifndef config_qt_H__
#define config_qt_H__

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QBoxLayout>
#include "ui_config_qt.h"

class ConfigModel;

namespace Ui {
	class ConfigQt;
}

class ConfigQt : public QDialog
{
	Q_OBJECT

public:
	explicit ConfigQt(ConfigModel *model, QWidget *parent = 0);
	~ConfigQt();

private slots:
	void onClickedPlay();
	void onClickedChoose();

private:
	void onUpdateModel();


	// layout
	//   | play
	//   | subLayout
	//      | choose
	struct button_element_t
	{
		QBoxLayout *layout;
		QPushButton *play;
		QBoxLayout *subLayout;
		QPushButton *choose;
	};

	Ui::ConfigQt *ui;
	QWidget *m_gridLayoutWidget;
	QGridLayout *m_gridLayout;
	std::vector<button_element_t> m_buttons;
	ConfigModel *m_model;
};

#endif // config_qt_H__
