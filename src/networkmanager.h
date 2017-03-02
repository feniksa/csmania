#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QUrl>
#include <QString>
#include <QSslError>
#include <QNetworkCookie>
#include <QDateTime>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;

namespace tabor
{
	
class NetworkManager : public QObject
{
	Q_OBJECT

	struct PendingQuery;

	enum request_priority_t
	{
		REQUEST_PRIORITY_HIGHT,
		REQUEST_PRIORITY_LOW
	};
	
public:
    typedef void (NetworkManager::*ParserCallbackFunction)(const QUrl&, const QByteArray& response, unsigned int uuid);
    typedef QList<unsigned int> FriendsListType;

    explicit NetworkManager(QObject* parent = 0);
    explicit NetworkManager(const QString& user, const QString& password, QObject* parent = 0);

signals:
	void error(const QString&);
	
private slots:
	void httpFinishedSlot(QNetworkReply*);
	void repeatRequestSlot();
	
	void cancelDownload();
	//void httpReadyRead();
	void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes); 
	void sslErrors(QNetworkReply*,const QList<QSslError> &errors);
	
private:
	void init();
	bool isLogined() const;

	// network functions
	void getHTTPRequest(const QUrl& url, ParserCallbackFunction callback, unsigned int uuid = 0);
	void postHTTPRequest(const QUrl& url, const QMap<QString, QString>& keyValue, ParserCallbackFunction callback, unsigned int uuid = 0);

    // parser function
	void parsePostLogin(const QUrl&, const QByteArray& response, unsigned int uuid);
	void parseFriendsPage(const QUrl&, const QByteArray& response, unsigned int uuid);
    void parseSendMessage(const QUrl& url, const QByteArray& response, unsigned int uuid);

	// network variables
	QNetworkAccessManager* qnam;
	QList<QNetworkCookie>  cookies;

	QList<PendingQuery> pendingQuery;
	QMap<QNetworkReply*, QPair<ParserCallbackFunction, unsigned int> > parserCallbackFunctions;

	// connection data
	QString userLogin;
	QString userPassword;

	// requested data
	FriendsListType friendsIds;

	QDateTime latestRequest;
	QTimer requestRepeatTimer;
};

struct NetworkManager::PendingQuery
{
	ParserCallbackFunction callback;
	QUrl url;
	QMap<QString, QString> keyValue;
	unsigned int uuid;
	bool isGet;
};

}

#endif
