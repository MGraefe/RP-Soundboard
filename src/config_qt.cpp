#include "common.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QFileInfo>
#include <QtGui/QResizeEvent>

#include "config_qt.h"
#include "ConfigModel.h"
#include "device.h"
#include "soundsettings_qt.h"

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigQt::ConfigQt( ConfigModel *model, QWidget *parent /*= 0*/ ) :
	QWidget(parent),
	ui(new Ui::ConfigQt),
	m_model(model),
	m_modelObserver(*this)
{
	ui->setupUi(this);
	//setAttribute(Qt::WA_DeleteOnClose);

	createButtons();

	ui->cb_advanced_config->hide();
	
	connect(ui->b_stop, SIGNAL(released()), this, SLOT(onClickedStop()));
	connect(ui->sl_volume, SIGNAL(valueChanged(int)), this, SLOT(onUpdateVolume(int)));
	connect(ui->cb_playback_locally, SIGNAL(clicked(bool)), this, SLOT(onUpdatePlaybackLocal(bool)));
	connect(ui->sb_rows, SIGNAL(valueChanged(int)), this, SLOT(onUpdateRows(int)));
	connect(ui->sb_cols, SIGNAL(valueChanged(int)), this, SLOT(onUpdateCols(int)));
	connect(ui->cb_mute_myself, SIGNAL(clicked(bool)), this, SLOT(onUpdateMuteMyself(bool)));

	m_model->addObserver(&m_modelObserver);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigQt::~ConfigQt()
{
	m_model->remObserver(&m_modelObserver);
	delete ui;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::closeEvent(QCloseEvent * evt)
{
	m_model->setWindowSize(size().width(), size().height());
}

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onClickedPlay()
{
	QPushButton *button = dynamic_cast<QPushButton*>(sender());
	size_t buttonId = std::find_if(m_buttons.begin(), m_buttons.end(), [button](button_element_t &e){return e.play == button;}) - m_buttons.begin();
	const SoundInfo *info = m_model->getSoundInfo(buttonId);
	if(info)
		sb_playFile(*info);
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
		m_model->setFileName(buttonId, fn);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onClickedAdvanced()
{
	QPushButton *button = dynamic_cast<QPushButton*>(sender());
	size_t buttonId = std::find_if(m_buttons.begin(), m_buttons.end(), [button](button_element_t &e){return e.advanced == button;}) - m_buttons.begin();

	SoundSettingsQt dlg(*m_model->getSoundInfo(buttonId), this);
	dlg.setWindowTitle(QString("Sound %1 Settings").arg(QString::number(buttonId)));
	if(dlg.exec() == QDialog::Accepted)
		m_model->setSoundInfo(buttonId, dlg.getSoundInfo());
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
	m_model->setVolume(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdatePlaybackLocal(bool val)
{
	m_model->setPlaybackLocal(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateCols(int val)
{
	m_model->setCols(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateRows(int val)
{
	m_model->setRows(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::onUpdateMuteMyself(bool val)
{
	m_model->setMuteMyselfDuringPb(val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::createButtons()
{
	for(button_element_t &elem : m_buttons)
	{
		elem.subLayout->removeWidget(elem.choose);
		delete elem.choose;
		elem.subLayout->removeWidget(elem.advanced);
		delete elem.advanced;
		elem.layout->removeWidget(elem.play);
		delete elem.play;
		elem.layout->removeItem(elem.subLayout);
		delete elem.subLayout;
		ui->gridLayout->removeItem(elem.layout);
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
			elem.play->setText("(no file)");
			elem.play->setEnabled(false);
			elem.play->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
			elem.layout->addWidget(elem.play);
			connect(elem.play, SIGNAL(released()), this, SLOT(onClickedPlay()));

			elem.subLayout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
			elem.layout->addLayout(elem.subLayout);

			elem.choose = new QPushButton(this);
			elem.choose->setText("...");
			elem.subLayout->addWidget(elem.choose);
			connect(elem.choose, SIGNAL(released()), this, SLOT(onClickedChoose()));

			elem.advanced = new QPushButton(this);
			elem.advanced->setText("adv");
			elem.subLayout->addWidget(elem.advanced);
			connect(elem.advanced, SIGNAL(released()), this, SLOT(onClickedAdvanced()));

			elem.play->updateGeometry();
			elem.choose->updateGeometry();
			elem.advanced->updateGeometry();

			m_buttons.push_back(elem);
		}
	}

	for(int i = 0; i < m_buttons.size(); i++)
		updateButtonText(i);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::updateButtonText(int i)
{
	if(i >= m_buttons.size())
		return;

	QString fn = m_model->getFileName(i);
	if(!fn.isNull())
	{
		QFileInfo info = QFileInfo(fn);
		m_buttons[i].play->setText(info.baseName());
		m_buttons[i].play->setEnabled(true);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigQt::ModelObserver::notify(ConfigModel &model, ConfigModel::notifications_e what, int data)
{
	switch(what)
	{
	case ConfigModel::NOTIFY_SET_ROWS:
		p.ui->sb_rows->setValue(model.getRows());
		p.createButtons();
		break;
	case ConfigModel::NOTIFY_SET_COLS:
		p.ui->sb_cols->setValue(model.getCols());
		p.createButtons();
		break;
	case ConfigModel::NOTIFY_SET_VOLUME:
		if (p.ui->sl_volume->value() != model.getVolume())
			p.ui->sl_volume->setValue(model.getVolume());
		break;
	case ConfigModel::NOTIFY_SET_PLAYBACK_LOCAL:
		if (p.ui->cb_playback_locally->isChecked() != model.getPlaybackLocal())
			p.ui->cb_playback_locally->setChecked(model.getPlaybackLocal());
		break;
	case ConfigModel::NOTIFY_SET_SOUND:
		p.updateButtonText(data);
		break;
	case ConfigModel::NOTIFY_SET_MUTE_MYSELF_DURING_PB:
		if (p.ui->cb_mute_myself->isChecked() != model.getMuteMyselfDuringPb())
			p.ui->cb_mute_myself->setChecked(model.getMuteMyselfDuringPb());
		break;
	case ConfigModel::NOTIFY_SET_WINDOW_SIZE:
		{
			QSize s = p.size();
			int w = 0, h = 0;
			model.getWindowSize(&w, &h);
			if(s.width() != w || s.height() != h)
				p.resize(w, h);
		}
	default:
		break;
	}
}
