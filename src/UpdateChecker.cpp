
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QMessageBox>

#include "version/version.h"

#include "UpdateChecker.h"
#include "ts3log.h"

#define CHECK_URL "http://mgraefe.de/rpsb/version/version.xml"

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
UpdateChecker::UpdateChecker( QObject *parent /*= NULL*/ ) :
	QObject(parent)
{

}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void UpdateChecker::startCheck()
{
	m_mgr = new QNetworkAccessManager(this);
	connect(m_mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinishDownloadXml(QNetworkReply*)));

	QUrl url(CHECK_URL);
	QNetworkRequest request(url);
	m_mgr->get(request);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void UpdateChecker::onFinishDownloadXml(QNetworkReply *reply)
{
	if(reply->error() != QNetworkReply::NoError)
	{
		std::string err = reply->errorString().toUtf8();
		logError("UpdateChecker: Error requesting version document %s.\nError-String: %s", CHECK_URL, err.c_str());
	}
	else
	{
		parseXml(reply);
		if(m_verInfo.valid() && m_verInfo.build > TS3SB_VERSION_BUILD)
		{
			QMessageBox msgBox;
			msgBox.setTextFormat(Qt::RichText);
			msgBox.setText(QString("A new version of RP Soundboard is available (build %1).<br /><br />"\
				"You can download it here:<br /><a href=\"%2\">%2</a>")
				.arg(m_verInfo.build)
				.arg(m_verInfo.latestDownload));
			msgBox.setIcon(QMessageBox::Information);
			msgBox.setWindowTitle("New version!");
			msgBox.setStandardButtons(QMessageBox::Close);
			msgBox.setDefaultButton(QMessageBox::Close);
			msgBox.exec();
		}
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void UpdateChecker::parseXml(QIODevice *device)
{
	m_verInfo.reset();

	QXmlStreamReader xml;
	xml.setDevice(device);

	while(!xml.atEnd() && !xml.hasError())
	{
		QXmlStreamReader::TokenType token = xml.readNext();
		if(token == QXmlStreamReader::StartDocument)
			continue;
		
		if(token == QXmlStreamReader::StartElement)
		{
			if(xml.name() == "versionDescription")
				continue;
			if(xml.name() == "product")
			{
				parseProduct(xml);
			}
		}
	}

	xml.clear();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void UpdateChecker::parseProduct( QXmlStreamReader &xml )
{
	if (xml.attributes().value("descVersion") == "1" && 
		xml.attributes().value("name") == "rp_soundboard")
	{
		m_verInfo.productName = xml.attributes().value("name").toString();
		xml.readNext();
		while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "product"))
		{
			if(xml.tokenType() == QXmlStreamReader::StartElement)
				parseProductInner(xml);
			xml.readNext();
		}
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void UpdateChecker::parseProductInner( QXmlStreamReader &xml )
{
	if(xml.name() == "latestVersion")
	{
		xml.readNext();
		m_verInfo.build = xml.text().toInt();
	}
	else if(xml.name() == "latestDownload")
	{
		xml.readNext();
		while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "latestDownload"))
		{
			if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "url")
			{
				xml.readNext();
				m_verInfo.latestDownload = xml.text().toString();
			}
			xml.readNext();
		}
	}
}



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void UpdateChecker::version_info_t::reset()
{
	productName = QString();
	build = 0;
	latestDownload = QString();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool UpdateChecker::version_info_t::valid()
{
	return !productName.isNull() && !productName.isEmpty() &&
		!latestDownload.isNull() && !latestDownload.isEmpty() &&
		build > 0;
}
