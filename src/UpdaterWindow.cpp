// src/UpdaterWindow.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "buildinfo.h"
#include "UpdaterWindow.h"
#include "ts3log.h"
#include <QMessageBox>
#include <QProcess>



UpdaterWindow::UpdaterWindow( QWidget *parent /*= 0*/ ) :
	QDialog(parent),
	ui(new Ui::updaterWindow),
	m_file(nullptr),
	m_manager(nullptr),
	m_reply(nullptr),
	m_redirects(0),
	m_execute(false),
	m_canceled(false),
	m_success(false)
{
	ui->setupUi(this);
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onClickedCancel(QAbstractButton*)));
}


void UpdaterWindow::startDownload(const QUrl &url, const QFileInfo &fileInfo, bool execute /*= false*/)
{
	m_url = url;
	m_fileinfo = fileInfo;
	m_execute = execute;

	if(m_fileinfo.fileName().isEmpty())
	{
		QFileInfo info(url.path());
		m_fileinfo.setFile(info.fileName());
	}

	logInfo("Downloading update from '%s' to '%s'", m_url.toString().toUtf8().data(), m_fileinfo.absoluteFilePath().toUtf8().data());

	m_file = new QFile(m_fileinfo.filePath());
	if(!m_file->open(QIODevice::WriteOnly))
	{
		logError("Unable to write to file %s", m_fileinfo.absoluteFilePath().toUtf8().data());
		QMessageBox::information(this, "Error", "Unable to save the file.");
		delete m_file;
		m_file = nullptr;
		return;
	}

	ui->statusLabel->setText("Downloading Update...");
	m_manager = new QNetworkAccessManager(this);
	startRequest(url);	
}


void UpdaterWindow::startRequest(const QUrl &url)
{
	QNetworkRequest request(url);
	request.setRawHeader("User-Agent", QByteArray("RP Soundboard Updater, ") + buildinfo_getPluginVersion());
	m_reply = m_manager->get(QNetworkRequest(url));
	connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, 
		SLOT(onDownloadProgress(qint64, qint64)));
	connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));
}


void UpdaterWindow::onReadyRead()
{
	if(m_file)
		m_file->write(m_reply->readAll());
}


void UpdaterWindow::onDownloadProgress(qint64 bytes, qint64 total)
{
	if(m_canceled)
		return;

	ui->progressBar->setMaximum(total);
	ui->progressBar->setValue(bytes);
}


void UpdaterWindow::onFinished()
{
	if(m_canceled)
	{
		logDebug("Canceled download of update");
		m_canceled = false;
		if(m_file)
		{
			m_file->close();
			m_file->remove();
			delete m_file;
			m_file = nullptr;
		}
		m_reply->deleteLater();
		m_reply = nullptr;
		this->hide();
	}
	else
	{
		m_file->flush();
		m_file->close();

		QVariant redirect = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
		if(m_reply->error())
		{
			m_file->remove();
			QMessageBox::information(this, "Error", QString("Could not download file: ") + m_reply->errorString());
		}
		else if(redirect.isValid())
		{
			if(m_redirects < 10)
			{
				m_redirects++;
				QUrl url = m_url.resolved(redirect.toUrl());
				logInfo("Download of update redirected to %s", url.toString().toUtf8().data());
				m_url = url;
				m_reply->deleteLater();
				m_reply = nullptr;
				m_file->open(QIODevice::WriteOnly);
				m_file->resize(0);
				startRequest(url);
				return;
			}
			else
			{
				logError("Download of update redirected more than 10 times, maybe a redirection loop?");
				m_success = false;
			}
		}
		else
		{
			m_success = true;
			if(m_execute)
				m_success = executeFile();
			this->hide();
		}

		m_reply->deleteLater();
		m_reply = nullptr;
		delete m_file;
		m_file = nullptr;
		m_manager = nullptr;
		m_redirects = 0;
	}

	emit finished();
}


bool UpdaterWindow::executeFile()
{
	bool status = QProcess::startDetached("package_inst.exe",
		QStringList(m_fileinfo.absoluteFilePath()));
	if(!status)
		logError("Error starting the update process package_inst.exe with cmd line \"%s\"",
			m_fileinfo.absoluteFilePath().toUtf8().data());
	return status;
}


void UpdaterWindow::onClickedCancel(QAbstractButton*)
{
	if(!m_canceled)
	{
		logDebug("Cancelling download of update...");
		m_canceled = true;
		ui->statusLabel->setText("Canceling Download...");
		m_reply->abort();
	}
}


