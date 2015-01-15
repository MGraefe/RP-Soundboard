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
	void onClickedStop();
	void onUpdateVolume(int val);
	void onUpdatePlaybackLocal(int val);
	void onUpdateCols(int val);
	void onUpdateRows(int val);

private:
	void onUpdateModel();
	void createButtons();

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
	std::vector<button_element_t> m_buttons;
	ConfigModel *m_model;
	QBoxLayout *m_configArea;
};

#endif // config_qt_H__
