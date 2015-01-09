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
			QPushButton *playButton = new QPushButton(m_gridLayoutWidget);
			playButton->setText(QString("Play ") + QString::number(i*numCols+j+1));
			playButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			m_gridLayout->addWidget(playButton, i * 2, j);
			connect(playButton, SIGNAL(released()), this, SLOT(onClickedPlay()));
			m_playButtons.push_back(playButton);

			QPushButton *chooseButton = new QPushButton(m_gridLayoutWidget);
			chooseButton->setText("...");
			m_gridLayout->addWidget(chooseButton, i * 2 + 1, j);
			connect(chooseButton, SIGNAL(released()), this, SLOT(onClickedChoose()));
			m_chooseButtons.push_back(chooseButton);

			playButton->updateGeometry();
			chooseButton->updateGeometry();
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
	size_t buttonId = std::find(m_playButtons.begin(), m_playButtons.end(), button) - m_playButtons.begin();
	m_model->playFile(buttonId);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onClickedChoose()
{
	QPushButton *button = dynamic_cast<QPushButton*>(sender());
	size_t buttonId = std::find(m_chooseButtons.begin(), m_chooseButtons.end(), button) - m_chooseButtons.begin();

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
	for(int i = 0; i < m_playButtons.size(); i++)
	{
		const char *fn = m_model->getFileName(i);
		if(fn)
		{
			QFileInfo info = QFileInfo(QString(fn));
			m_playButtons[i]->setText(info.baseName());
		}
	}
}

