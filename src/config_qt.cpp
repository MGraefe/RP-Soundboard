#include "common.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QFileInfo>

#include "config_qt.h"
#include "ConfigModel.h"
#include "device.h"

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

	createButtons();
	
	connect(ui->b_stop, SIGNAL(released()), this, SLOT(onClickedStop()));
	connect(ui->sl_volume, SIGNAL(valueChanged(int)), this, SLOT(onUpdateVolume(int)));
	connect(ui->cb_playback_locally, SIGNAL(valueChanged(int)), this, SLOT(onUpdatePlaybackLocal(int)));

	ui->sl_volume->setValue(m_model->getVolume());
	ui->cb_playback_locally->setChecked(m_model->getPlaybackLocal() == 1);
	
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


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onClickedStop()
{
	sb_stopPlayback();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateVolume(int val)
{
	sb_setVolume(val);
	m_model->setVolume(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdatePlaybackLocal( int val )
{
	sb_setLocalPlayback(val);
	m_model->setPlaybackLocal(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateCols( int val )
{
	m_model->setRows(val);
	createButtons();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateRows( int val )
{
	m_model->setCols(val);
	createButtons();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::createButtons()
{
	for(button_element_t &elem : m_buttons)
	{
		delete elem.choose;
		delete elem.subLayout;
		delete elem.play;
		delete elem.layout;
	}
	
	m_buttons.clear();

	int numRows = m_model->getRows();
	int numCols = m_model->getCols();

	for(int i = 0; i < numRows; i++)
	{
		for(int j = 0; j < numCols; j++)
		{
			button_element_t elem;
			elem.layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
			elem.layout->setSpacing(0);
			ui->gridLayout->addLayout(elem.layout, i, j);

			elem.play = new QPushButton(this);
			elem.play->setText(QString("Play ") + QString::number(i*numCols+j+1));
			elem.play->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			elem.layout->addWidget(elem.play);
			connect(elem.play, SIGNAL(released()), this, SLOT(onClickedPlay()));

			elem.subLayout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
			elem.layout->addLayout(elem.subLayout);

			elem.choose = new QPushButton(this);
			elem.choose->setText("...");
			elem.subLayout->addWidget(elem.choose);
			connect(elem.choose, SIGNAL(released()), this, SLOT(onClickedChoose()));

			elem.play->updateGeometry();
			elem.choose->updateGeometry();

			m_buttons.push_back(elem);
		}
	}
}

