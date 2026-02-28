// src/UpdaterWindow.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <QDialog>
#include <QUrl>
#include <QFileInfo>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "ui_UpdaterWindow.h"

namespace Ui {
	class updaterWindow;
}


class UpdaterWindow : public QDialog
{
	Q_OBJECT

public:
	explicit UpdaterWindow(QWidget *parent = 0);
	void startDownload(const QUrl &url, const QFileInfo &fileInfo, bool execute = false);
	void startRequest(const QUrl & url);
	inline bool getSuccess() const {
		return m_success;
	}

public slots:
	void onReadyRead();
	void onDownloadProgress(qint64 bytes, qint64 total);
	void onClickedCancel(QAbstractButton*);
	void onFinished();

signals:
	void finished();
	
private:
	bool executeFile();
	Ui::updaterWindow *ui;
	QUrl m_url;
	QFileInfo m_fileinfo;
	QFile *m_file;
	QNetworkAccessManager *m_manager;
	QNetworkReply *m_reply;
	int m_redirects;
	bool m_execute;
	bool m_canceled;
	bool m_success;
};

