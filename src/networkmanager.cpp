#include "networkmanager.h"

#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QUrl>
#include <QDebug>

#include "utils.h"

namespace tabor
{
const unsigned int requestTimeoutMSeconds = 1000;	
const unsigned int requestNewMessagesMSeconds = 5000;

NetworkManager::NetworkManager(const QString& user, const QString& password, QObject* parent)
	: QObject(parent), userLogin(user), userPassword(password)
{
	init();
}

NetworkManager::NetworkManager(QObject* parent)
	: QObject(parent)
{
	init();
}

void NetworkManager::init()
{	
	qnam = new QNetworkAccessManager(this);
	qnam->setCookieJar(new QNetworkCookieJar(this));

	connect(qnam, SIGNAL(finished ( QNetworkReply* )), SLOT(httpFinishedSlot(QNetworkReply*)));

	connect(&requestRepeatTimer, SIGNAL(timeout()), SLOT(repeatRequestSlot()));
	requestRepeatTimer.start(requestTimeoutMSeconds);
}

void NetworkManager::repeatRequestSlot()
{
	if (pendingQuery.isEmpty())
		return;

	qDebug() << "execute pending request";

	PendingQuery query = pendingQuery.front();
	pendingQuery.pop_front();

	if (query.isGet) {
		getHTTPRequest(query.url, query.callback, query.uuid);
	} else {
		postHTTPRequest(query.url, query.keyValue, query.callback, query.uuid);
	}
}

void NetworkManager::getHTTPRequest(const QUrl& url, ParserCallbackFunction callback, unsigned int uuid)
{
	if (latestRequest.isValid()) {
		if ((latestRequest.addMSecs(requestTimeoutMSeconds) >= QDateTime::currentDateTime())) {
			PendingQuery query;
			query.url = url;
			query.callback = callback;
			query.uuid = uuid;
			query.isGet = true;
		
			pendingQuery.push_back(query);

			qDebug() << "add query to request list";
		
			return;
		}
	}
	
	QNetworkRequest request(url);

	if (!cookies.isEmpty()) {
		QVariant var;
		var.setValue(cookies);
		request.setHeader(QNetworkRequest::CookieHeader, var);
	}

	QNetworkReply* reply = qnam->get(request);
	parserCallbackFunctions[reply].first = callback;
	parserCallbackFunctions[reply].second = uuid;

	latestRequest = QDateTime::currentDateTime();

	//connect(reply, SIGNAL(finished()), SLOT(httpFinished()));
	//connect(reply, SIGNAL(readyRead()), SLOT(httpReadyRead()));
	//connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(updateDataReadProgress(qint64,qint64)));
}

void NetworkManager::postHTTPRequest(const QUrl& url, const QMap<QString, QString>& keyValue, ParserCallbackFunction callback, unsigned int uuid)
{
	if (latestRequest.isValid()) {
		if ((latestRequest.addMSecs(requestTimeoutMSeconds) >= QDateTime::currentDateTime())) {
			PendingQuery query;
			query.url = url;
			query.callback = callback;
			query.uuid = uuid;
			query.isGet = false;
			query.keyValue = keyValue;

			pendingQuery.push_back(query);

			qDebug() << "add query to request list";

			return;
		}
	}
	
	QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    QUrl params;

	QMapIterator<QString, QString> i(keyValue);
	while (i.hasNext()) {
		i.next();
		params.addQueryItem(i.key(), i.value());
	}

	if (!cookies.isEmpty()) {
		QVariant var;
		var.setValue(cookies);
		request.setHeader(QNetworkRequest::CookieHeader, var);
	}
	
    QNetworkReply* reply = qnam->post(request, params.encodedQuery());	
	parserCallbackFunctions[reply].first = callback;
	parserCallbackFunctions[reply].second = uuid;
	
    //connect(reply, SIGNAL(finished()), SLOT(httpFinished()));
	//connect(reply, SIGNAL(readyRead()), SLOT(httpReadyRead()));
	//connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(updateDataReadProgress(qint64,qint64)));
}

void NetworkManager::cancelDownload()
{
}

void NetworkManager::httpFinishedSlot(QNetworkReply* reply)
{
	QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (reply->error()) {
		emit error(tr("Download failed: %1.").arg(reply->errorString()));
         //downloadButton->setEnabled(true);
		 
     } else if (!redirectionTarget.isNull()) {
         /*QUrl newUrl = url.resolved(redirectionTarget.toUrl());
         if (QMessageBox::question(this, tr("HTTP"),
                                   tr("Redirect to %1 ?").arg(newUrl.toString()),
                                   QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
             url = newUrl;
             reply->deleteLater();
		 
             return;
         }*/
		emit error(tr("Redirection not supported yet"));
		 
     } else {
		 QByteArray response = reply->readAll();
        (this->*parserCallbackFunctions[reply].first)(reply->url(), response, parserCallbackFunctions[reply].second);
     }
	 
	 parserCallbackFunctions.remove(reply);

     reply->deleteLater();
     reply = 0;
}

//void TaborNetworkManager::httpReadyRead()
//{
	// this slot gets called every time the QNetworkReply has new data.
	// We read all of its new data and write it into the file.
	// That way we use less RAM than when reading it at the finished()
	// signal of the QNetworkReply
	//qDebug() << reply->readAll();
//}

void NetworkManager::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{	
}

void NetworkManager::sslErrors(QNetworkReply*,const QList<QSslError> &errors)
{
	QString errorString;
	foreach (const QSslError &error, errors) {
		if (!errorString.isEmpty())
			errorString += ", ";
		errorString += error.errorString();
	}

	emit error(tr("One or more SSL errors has occurred: %1").arg(errorString));
}

// ----------------------------------------------------------------------------------
// request functions
// ----------------------------------------------------------------------------------

/*void NetworkManager::requestLogin()
{
    QMap<QString, QString> keyValue;
    keyValue["form_login"] = userLogin;
    keyValue["form_password"] = userPassword;
    keyValue["rem"] = "1";
    keyValue["submit"] = "enter";

    postHTTPRequest(loginPageUrl, keyValue, &NetworkManager::parsePostLogin);
}

void NetworkManager::requestFriendsList()
{
	QUrl url = friendsPageUrl + QString::number(1);

    getHTTPRequest(url, &NetworkManager::parseFriendsPage);
}

bool NetworkManager::isLogined() const
{
	return !cookies.isEmpty();
}

void NetworkManager::sendMessage(unsigned int uuid, const QString& message)
{
    QMap<QString, QString> keyValue;
    keyValue["chat_id"] = "";
    keyValue["msg_html"] = "";
    keyValue["msg_id"] = "0";
    keyValue["sent"] = "1";
    keyValue["user_id"] = uuid;
    keyValue["msg_text"] = message;

    postHTTPRequest(sendMesssagPageUrl, keyValue, &NetworkManager::parseSendMessage);

}

void NetworkManager::parseSendMessage(const QUrl& url, const QByteArray& response, unsigned int uuid)
{

    qDebug() << "response from sending message" << response;
}

// ----------------------------------------------------------------------------------
// parse functions
// ----------------------------------------------------------------------------------

void NetworkManager::parsePostLogin(const QUrl& url, const QByteArray& response, unsigned int uuid)
{
	cookies = qnam->cookieJar()->cookiesForUrl(url);
    qDebug() << cookies;

	emit recievedLogin();
}

void NetworkManager::parseFriendsPage(const QUrl& url, const QByteArray& response, unsigned int uuid)
{

}*/

}
