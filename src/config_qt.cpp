#include "common.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QFileInfo>

#include "config_qt.h"
#include "ConfigModel.h"

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigQt::ConfigQt( ConfigModel *model, QWidget *parent /*= 0*/ ) :
	QDialog(parent),
	ui(new Ui::ConfigQt),
	m_model(model)
{
	ui->setupUi(this);
	//setAttribute(Qt::WA_DeleteOnClose);
	
	m_gridLayoutWidget = new QWidget();
	m_gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
	m_gridLayoutWidget->setGeometry(QRect(10, 10, 531, 281));
	m_gridLayout = new QGridLayout(m_gridLayoutWidget);
	m_gridLayout->setObjectName(QStringLiteral("gridLayout"));
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	ui->verticalLayout->addWidget(m_gridLayoutWidget);

	int numRows = 4;
	int numCols = 6;

	for(int i = 0; i < numRows; i++)
	{
		for(int j = 0; j < numCols; j++)
		{
			button_element_t elem;

			elem.layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
			elem.layout->setSpacing(0);
			m_gridLayout->addLayout(elem.layout, i, j);

			elem.play = new QPushButton(m_gridLayoutWidget);
			elem.play->setText(QString("Play ") + QString::number(i*numCols+j+1));
			elem.play->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			elem.layout->addWidget(elem.play);
			connect(elem.play, SIGNAL(released()), this, SLOT(onClickedPlay()));

			elem.subLayout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
			elem.layout->addLayout(elem.subLayout);

			elem.choose = new QPushButton(m_gridLayoutWidget);
			elem.choose->setText("...");
			elem.subLayout->addWidget(elem.choose);
			connect(elem.choose, SIGNAL(released()), this, SLOT(onClickedChoose()));

			elem.play->updateGeometry();
			elem.choose->updateGeometry();
			
			m_buttons.push_back(elem);
		}
	}

	onUpdateModel();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigQt::~ConfigQt()
{
	delete ui;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onClickedPlay()
{
	QPushButton *button = dynamic_cast<QPushButton*>(sender());
	size_t buttonId = std::find_if(m_buttons.begin(), m_buttons.end(), [button](button_element_t &e){return e.play == button;}) - m_buttons.begin();
	m_model->playFile(buttonId);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onClickedChoose()
{
	QPushButton *button = dynamic_cast<QPushButton*>(sender());
	size_t buttonId = std::find_if(m_buttons.begin(), m_buttons.end(), [button](button_element_t &e){return e.choose == button;}) - m_buttons.begin();

	QString filePath = m_model->getFileName(buttonId);
	QString fn = QFileDialog::getOpenFileName(this, tr("Choose File"), filePath, tr("Files (*.*)"));
	if(!fn.isNull())
	{
		QByteArray ba = fn.toLocal8Bit();
		m_model->setFileName(buttonId, ba.data());
		onUpdateModel();
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateModel()
{
	for(int i = 0; i < m_buttons.size(); i++)
	{
		const char *fn = m_model->getFileName(i);
		if(fn)
		{
			QFileInfo info = QFileInfo(QString(fn));
			m_buttons[i].play->setText(info.baseName());
		}
	}
}

