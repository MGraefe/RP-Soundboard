#ifndef UpdateChecker_H__
#define UpdateChecker_H__

#include <QtCore/QObject>
#include <QtCore/QXmlStreamReader>

class QNetworkReply;
class QNetworkAccessManager;

class UpdateChecker : public QObject
{
	Q_OBJECT

public:
	struct version_info_t
	{
		QString productName;
		int build;
		QString latestDownload;

		void reset();
		bool valid();
	};

public:
	explicit UpdateChecker(QObject *parent = NULL);
	void startCheck();

public slots:
	void onFinishDownloadXml(QNetworkReply *reply);
	void parseXml(QIODevice *device);
	void parseProduct(QXmlStreamReader &xml);
	void parseProductInner(QXmlStreamReader &xml);

private:
	QNetworkAccessManager *m_mgr;
	version_info_t m_verInfo;
};

#endif // UpdateChecker_H__
