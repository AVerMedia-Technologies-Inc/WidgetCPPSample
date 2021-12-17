#include "Websocket2Client.h"
#include <QJsonParseError>

/*
Websocket2Client::Websocket2Client(QString url, QObject *parent)
		:QObject(parent)
{
	m_url = url;
}
*/
Websocket2Client::Websocket2Client(QString url, QString uuid)
{
	m_url = url;
	m_uuid = uuid;
}

Websocket2Client::~Websocket2Client()
{
	if (&m_webSocket)
	{
		m_webSocket.close();
		m_webSocket.abort();
	}
}

void Websocket2Client::SendMsg(QString message)
{
	m_webSocket.sendTextMessage(message);
	ShowClientInfo("[sendTextMessage] " + message, &m_webSocket);
}

void Websocket2Client::Close()
{
	ShowClientInfo("[Socket Close] ", &m_webSocket);
	m_webSocket.close();
}

void Websocket2Client::Connect()
{
	QObject::connect(&m_webSocket, &QWebSocket::connected, this, &Websocket2Client::onConnected);	
	m_webSocket.open(m_url);
}

void Websocket2Client::onTextMessageReceived(QString message)
{
	ShowClientInfo("[onTextMessageReceived] " + message, &m_webSocket);
	emit receivedMessage(message);
}

void Websocket2Client::onConnected()
{
	QObject::connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &Websocket2Client::onTextMessageReceived);
	QObject::connect(&m_webSocket, &QWebSocket::disconnected, this, &Websocket2Client::offConnected);
	QObject::connect(&m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &Websocket2Client::error4Socket);

	emit socketConnect();

	QJsonObject jsonAppTriggered;
	jsonAppTriggered.insert("method", "ax.register.widget");
	QJsonObject jsonParameters;
	jsonParameters.insert("id", m_uuid);
	jsonAppTriggered.insert("params", jsonParameters);

	QString msg4AppTriggered = json2String(jsonAppTriggered);

	SendMsg(msg4AppTriggered);
}

void Websocket2Client::offConnected()
{
	emit disconnected();
}

void Websocket2Client::ShowClientInfo(QString info, QWebSocket* socket)
{

}

QString Websocket2Client::json2String(QJsonObject json)
{
	QJsonDocument doc(json);
	QString strJson(doc.toJson(QJsonDocument::Compact));
	return strJson;
}

QJsonObject Websocket2Client::translate2Json(QString messageData)
{
	QJsonObject message2json;
	QJsonParseError error;
	QJsonDocument message = QJsonDocument::fromJson(messageData.toUtf8(), &error);
	if (error.error)
	{
		qWarning() << "Failed to parse text message as JSON object:" << messageData
			<< "Error is:" << error.errorString();
	}
	else if (!message.isObject())
	{
		qWarning() << "Received JSON message that is not an object: " << messageData;
	}
	else
	{
		message2json = message.object();
	}
	return message2json;
}

void Websocket2Client::error4Socket(QAbstractSocket::SocketError error)
{
	emit socketError(error);
}
