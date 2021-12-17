#pragma once
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QtCore/QJsonObject>
class Websocket2Client : public QObject
{
	Q_OBJECT
public:
	//Websocket2Client(QString url, QObject* parent = Q_NULLPTR);
	Websocket2Client(QString url, QString uuid);
	~Websocket2Client();
	QWebSocket m_webSocket;
	QUrl m_url;
	QString m_uuid;
	void ShowClientInfo(QString info, QWebSocket* socket);
	QJsonObject translate2Json(QString text);
	QString json2String(QJsonObject json);

signals:
	void receivedMessage(QString message);
	void disconnected();
	void socketError(QAbstractSocket::SocketError error);
	void socketConnect();

public slots:
	void SendMsg(QString message);
	void Close();
	void Connect();
	void onTextMessageReceived(QString message);
	void onConnected();
	void offConnected();
	void error4Socket(QAbstractSocket::SocketError error);
};

