
#include "common.h"

#include "ConfigModel.h"
#include "device.h"


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::readConfig(QSettings *settings)
{
	int numFiles = settings->value("num_files", 10).toInt();
	m_fns.resize(numFiles);

	int size = settings->beginReadArray("files");
	for(int i = 0; i < size && i < numFiles; i++)
	{
		settings->setArrayIndex(i);
		QByteArray ba = settings->value("path").toString().toLocal8Bit();
		m_fns[i] = ba.data();
	}
	settings->endArray();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::writeConfig(QSettings *settings)
{
	settings->setValue("num_files", m_fns.size());

	settings->beginWriteArray("files");
	for(int i = 0; i < m_fns.size(); i++)
	{
		settings->setArrayIndex(i);
		QString value = m_fns[i].c_str();
		settings->setValue("path", value);
	}
	settings->endArray();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
const char * ConfigModel::getFileName( int itemId )
{
	if(itemId >= 0 && itemId < m_fns.size())
		return m_fns[itemId].c_str();
	return NULL;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setFileName( int itemId, const char *fn )
{
	if(itemId >= 0 && itemId < m_fns.size())
	{
		m_fns[itemId] = fn;
		sb_saveConfig();
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::playFile( int itemId )
{
	if(itemId >= 0 && itemId < m_fns.size())
		sb_playFile(m_fns[itemId].c_str());
}
