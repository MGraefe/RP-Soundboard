#ifndef config_qt_H__
#define config_qt_H__

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
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

	Ui::ConfigQt *ui;
	QWidget *m_gridLayoutWidget;
	QGridLayout *m_gridLayout;
	std::vector<QPushButton*> m_playButtons;
	std::vector<QPushButton*> m_chooseButtons;
	ConfigModel *m_model;
};

#endif // config_qt_H__
