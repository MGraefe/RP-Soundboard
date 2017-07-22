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
#include <QtNetwork/QNetworkRequest>

class QNetworkReply;
class QNetworkAccessManager;
class UpdaterWindow;
class ConfigModel;

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
		QString featuresUrl;
		QString features;

		void reset();
		bool valid();
	};

public:
	explicit UpdateChecker(QObject *parent = NULL);
	void startCheck(bool explicitCheck = true, ConfigModel *config = NULL);
	static QByteArray getUserAgent();
	static void setUserAgent(QNetworkRequest &request);

public slots:
	void onFinishedUpdate();
	void onFinishDownload(QNetworkReply *reply);

private:
	void parseXml(QIODevice *device);
	void parseProduct(QXmlStreamReader &xml);
	void parseProductInner(QXmlStreamReader &xml);
	void onFinishDownloadXml(QNetworkReply *reply);
	void onFinishDownloadFeatures(QNetworkReply * reply);
	void askUserForUpdate();

private:
	enum class Loading
	{
		mainXml,
		features,
	} loading;

	QNetworkAccessManager *m_mgr;
	version_info_t m_verInfo;
	UpdaterWindow *m_updater;
	ConfigModel *m_config;
	bool m_explicitCheck;
};

#endif // rpsbsrc__UpdateChecker_H__
