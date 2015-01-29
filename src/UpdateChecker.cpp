
//Parses XML Files from a server
//Example File:

// <?xml version="1.0" encoding="utf-8"?>
//
// <versionDescription>
//   <product descVersion="1" name="rp_soundboard">
//     <latestVersion>1101</latestVersion>
//     <latestDownload>
//       <url>http://mgraefe.de/rpsb/dl/rp_soundboard_1101.ts3_plugin</url>
//     </latestDownload>
//   </product>
// </versionDescription>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QMessageBox>

#include "version/version.h"
#include "buildinfo.h"

#include "UpdateChecker.h"
#include "ts3log.h"

#define CHECK_URL "http://mgraefe.de/rpsb/version/version.xml"


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool isValid(const QXmlStreamReader &xml)
{
	return !(xml.hasError() || xml.atEnd());
}


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
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", QByteArray("RP Soundboard Update Checker, ") + PLUGIN_VERSION);
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

	while(isValid(xml))
	{
		QXmlStreamReader::TokenType token = xml.readNext();
		if(token == QXmlStreamReader::StartElement && xml.name() == "product")
				parseProduct(xml);
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
		while(isValid(xml) && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "product"))
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
		while(isValid(xml) && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "latestDownload"))
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
