#include "filedownloader.h"

CFileDownloader::CFileDownloader(const QUrl& imageUrl, QObject *parent) :
	QObject(parent)
{
	QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
	sslConfig.setProtocol(QSsl::TlsV1_2);
	pNetworkAccessManager = new QNetworkAccessManager();
	connect(
		pNetworkAccessManager, SIGNAL (finished(QNetworkReply*)),
		this, SLOT (fileDownloaded(QNetworkReply*))
	);

	QNetworkRequest networkRequest;
	networkRequest.setSslConfiguration( sslConfig );
	networkRequest.setUrl(imageUrl);
	pNetworkAccessManager->get( networkRequest );
}

CFileDownloader::~CFileDownloader() { }

void CFileDownloader::fileDownloaded(QNetworkReply* pReply) {
	m_DownloadedData = pReply->readAll();
	pReply->deleteLater();
	emit downloaded();
}

QByteArray CFileDownloader::downloadedData() const {
	return m_DownloadedData;
}