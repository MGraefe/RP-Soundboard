// src/UpdateChecker.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__UpdateChecker_H__
#define rpsbsrc__UpdateChecker_H__

#include <QtCore/QObject>
#include <QtCore/QXmlStreamReader>

class QNetworkReply;
class QNetworkAccessManager;
class UpdaterWindow;

class UpdateChecker : public QObject
{
	Q_OBJECT

public:
	struct version_info_t
	{
		QString productName;
		int build;
		QString latestDownload;
		QString version;

		void reset();
		bool valid();
	};

public:
	explicit UpdateChecker(QObject *parent = NULL);
	void startCheck();
	

public slots:
	void onFinishedUpdate();
	void onFinishDownloadXml(QNetworkReply *reply);
	void parseXml(QIODevice *device);
	void parseProduct(QXmlStreamReader &xml);
	void parseProductInner(QXmlStreamReader &xml);

private:
	void askUserForUpdate();

private:
	QNetworkAccessManager *m_mgr;
	version_info_t m_verInfo;
	UpdaterWindow *m_updater;
};

#endif // rpsbsrc__UpdateChecker_H__
