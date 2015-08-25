#ifndef updater_qt_H__
#define updater_qt_H__


#include <QtWidgets/QDialog>
#include <QtCore/QUrl>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "ui_updater_qt.h"

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


#endif // updater_qt_H__
