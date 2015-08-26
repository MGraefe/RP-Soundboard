
#include "about_qt.h"
#include "buildinfo.h"

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
AboutQt::AboutQt(QWidget *parent) :
	QWidget(parent, Qt::Window | Qt::WindowTitleHint /*| Qt::CustomizeWindowHint*/ | Qt::WindowCloseButtonHint),
	ui(new Ui::AboutQt)
{
	ui->setupUi(this);
	ui->l_version->setText(buildinfo_getPluginVersion());
	setFixedSize(size());
}
