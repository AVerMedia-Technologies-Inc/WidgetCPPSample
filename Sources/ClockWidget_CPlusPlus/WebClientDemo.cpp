#include "WebClientDemo.h"
#include <QJsonParseError>
#include <QKeyEvent>
#include <QEvent>
#include <QtWidgets/qmessagebox.h>
//#include "windows.h" 
#include <QBuffer>
#include <QImage>
#include <QTimeZone>
#include <QPixmap>
#include <QPainter>

WebClientDemo::WebClientDemo(QString uuid, QString port, QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_url = QUrl("ws://127.0.0.1:" + port);
    ui.editURL->setText(m_url.toString());
    m_uuid = uuid;
    m_webSocket2Client = new Websocket2Client(ui.editURL->text(),uuid);
    connect(m_webSocket2Client, &Websocket2Client::receivedMessage, this, &WebClientDemo::showReceivedMessage);
    connect(m_webSocket2Client, &Websocket2Client::disconnected, this, &WebClientDemo::btnClose_clicked);
    connect(m_webSocket2Client, &Websocket2Client::socketConnect, this, &WebClientDemo::getPayload);

    m_Timer = new QTimer(this);
    connect(m_Timer, &QTimer::timeout, this, &WebClientDemo::drawingClock);
    //m_Timer->start(1000);
    btnConnect_clicked();
    QList<QByteArray> timeZoneIds = QTimeZone::availableTimeZoneIds();
    QDateTime localDateTime(QDate::currentDate(), QTime::currentTime());
    QString tempZone("America/New_York");
}

WebClientDemo::~WebClientDemo()
{
    if (&m_webSocket2Client)
    {
        m_webSocket2Client->Close();
        m_webSocket2Client = NULL;
    }
}

void WebClientDemo::btnSendMsg_clicked()
{
    m_webSocket2Client->SendMsg(ui.editSendMsg->text());
}

void WebClientDemo::btnClose_clicked()
{
    m_Timer->stop();
    ui.txtInfo->append("Socket Disconnected");
    m_webSocket2Client->Close();
    ui.btnConnect->setEnabled(true);
    ui.editSendMsg->setEnabled(true);
}

void WebClientDemo::btnConnect_clicked()
{    
    m_webSocket2Client->Connect();
    ui.btnConnect->setEnabled(false);
    ui.editSendMsg->setEnabled(false);
}

void WebClientDemo::showReceivedMessage(QString message)
{
    QJsonObject jsonMsg = translate2Json(message);
    ui.txtInfo->append(message); 

    if (jsonMsg.contains("result"))
    {
        QString registerResult = jsonMsg.value("result").toString();
        if (registerResult == "ax.register.widget")
        {
            getPayload();
            return;
        }
    }

    QString id = jsonMsg.value("params").toObject().value("id").toString();
    QString action = jsonMsg.value("params").toObject().value("payload").toObject().value("action").toString();    
    QString method = jsonMsg.value("method").toString();
    

    //QMessageBox::information(this, "action", runType + ";" + id);
    if (action == "set_type_val")
    {
        QString clockType = jsonMsg.value("params").toObject().value("payload").toObject().value("type").toString();
        if (clockType == "analog")
        {
            m_isDigitalClock = false;
        }
        else
        {
            m_isDigitalClock = true;
        }
        m_clockType = clockType;
        setPayload();
    }
    else if(method == "ax.property.connected")
    {
        setPropertyUI();
    }
    else if (action == "set_city_val" || method == "ax.update.payload")
    {
        QString timeZone = jsonMsg.value("params").toObject().value("payload").toObject().value("city").toString();
        QString currentTime = "";

        QList<QByteArray> timeZoneIds = QTimeZone::availableTimeZoneIds();
        QDateTime localDateTime(QDate::currentDate(), QTime::currentTime());
        m_Timer->start(1000);
#pragma region GetCurrentTime

        m_city = timeZone;
        if (timeZone == "new_york")
        {            
            QTimeZone timeZoneTemp("America/New_York");
            m_timeZone = timeZoneTemp;
            QString displayNameTimeZone = m_timeZone.displayName(QTimeZone::GenericTime);
            //QMessageBox::information(this, "Information", "TimeZone: " + displayNameTimeZone);
            
        }
        else if (timeZone == "california")
        {
            QTimeZone timeZoneTemp("America/Los_Angeles");
            m_timeZone = timeZoneTemp;
            QString displayNameTimeZone = m_timeZone.displayName(QTimeZone::GenericTime);
            //QMessageBox::information(this, "Information", "TimeZone: " + displayNameTimeZone);
        }
        else if (timeZone == "australia")
        {
            QTimeZone timeZoneTemp(timeZoneIds[295]);
            m_timeZone = timeZoneTemp;
            QString displayNameTimeZone = m_timeZone.displayName(QTimeZone::GenericTime);
            //QMessageBox::information(this, "Information", "TimeZone: " + displayNameTimeZone);
        }
        else if (timeZone == "taipei")
        {            
            QTimeZone timeZoneTemp("Asia/Taipei");
            m_timeZone = timeZoneTemp;
            QString displayNameTimeZone = m_timeZone.displayName(QTimeZone::GenericTime);
            //QMessageBox::information(this, "Information", "TimeZone: " + displayNameTimeZone);
        }



#pragma endregion

        if (method == "ax.update.payload")
        {
            QString clockType = jsonMsg.value("params").toObject().value("payload").toObject().value("type").toString();
            if (clockType == "analog")
            {
                m_isDigitalClock = false;
            }
            else
            {
                m_isDigitalClock = true;
            }
            m_clockType = clockType;
            setPayload();
            setPropertyUI();
        }

    }
}

QJsonObject WebClientDemo::translate2Json(QString messageData)
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

QString WebClientDemo::json2String(QJsonObject json)
{
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

QString WebClientDemo::image2Base64(QImage image)
{
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    image.save(&buf, "JPG");
    QByteArray ba2 = ba.toBase64();
    QString b64str = QString::fromLatin1(ba2);
    buf.aboutToClose();
    ba.clear();
    ba2.clear();
    return b64str;
}

void WebClientDemo::showWorldClockMessage(QImage image)
{
    QString imageMsg = image2Base64(image);

    QJsonObject jsonWorldClock;
    jsonWorldClock.insert("id", 0);
    jsonWorldClock.insert("jsonrpc", "2.0");
    jsonWorldClock.insert("method", "ax.set.image");
    QJsonObject jsonParameters;
    jsonParameters.insert("id", m_uuid);
    QJsonObject jsonPayload;
    jsonPayload.insert("image", imageMsg);
    jsonParameters.insert("payload", jsonPayload);
    jsonWorldClock.insert("params", jsonParameters);

    QString msg4WorldClock = json2String(jsonWorldClock);
    ui.editSendMsg->setText(msg4WorldClock);
    btnSendMsg_clicked();
}

void WebClientDemo::drawingClock()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime targetTime;
    if (m_timeZone.isValid())
    {
        targetTime = currentTime.toTimeZone(m_timeZone);
    }
    else
    {
        targetTime = currentTime;
    }
    QPixmap pixmap;
    if (m_isDigitalClock)
    {
        QString timeFormat = targetTime.toString("HH:mm:ss");
        pixmap = QPixmap(":/WebClientDemo/Resource/Blank.png", "PNG");
        QPainter painter(&pixmap);
        painter.setPen(QPen(Qt::red, 8, Qt::SolidLine));
        QFont font = ui.editURL->font();
        font.setPixelSize(12);
        painter.setFont(font);
        painter.drawText(0, 0, pixmap.width(), pixmap.height(), Qt::AlignCenter, timeFormat);
        painter.end();
    }
    else
    {
        int second = targetTime.time().second();
        int minute = targetTime.time().minute();
        int hour = targetTime.time().hour();

        pixmap = QPixmap(":/WebClientDemo/Resource/ClockJPG.jpg", "JPG");
        QPainter painter(&pixmap);

#pragma region Drawing Clock
        painter.translate(pixmap.width() / 2, pixmap.height() / 2);
        float radius;
        if (pixmap.width() > pixmap.height())
        {
            radius = (float)(pixmap.height() - 8) / 2;
        }
        else
        {
            radius = (float)(pixmap.width() - 8) / 2;
        }

        //Draw the second hand
        painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
        painter.rotate((6 * second));
        painter.drawLine(QPoint(0, 0), QPoint(0, (-1) * (float)(radius / 1.5)));
        //Draw minute hand
        painter.setPen(QPen(Qt::blue, 4, Qt::SolidLine));
        painter.rotate(((-6) * second));
        painter.rotate((float)(0.1 * second + 6 * minute));
        painter.drawLine(QPoint(0, 0), QPoint(0, (-1) * (float)(radius / 2)));
        //Draw hour hand
        painter.setPen(QPen(Qt::green, 8, Qt::SolidLine));
        painter.rotate((float)(0.1 * second + 6 * minute) * (-1));
        painter.rotate((float)(30.0 / 3600.0 * second + 30.0 / 60.0 * minute + hour * 30.0));
        painter.drawLine(QPoint(0, 0), QPoint(0, (-1) * (float)(radius / 2.5)));

        painter.end();
#pragma endregion
    }
    m_scene->clear();
    m_scene->destroyed();
    m_scene->deleteLater();
    m_scene = new QGraphicsScene;
    m_scene->addPixmap(pixmap);
    ui.graphicsView->setScene(m_scene);
    ui.graphicsView->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui.graphicsView->show();
    showWorldClockMessage(pixmap.toImage());
    pixmap.scaled(0,0);
}

void WebClientDemo::btnManual_clicked(bool manual)
{
    //Manual
    if (manual)
    {
        m_Timer->stop();
        ui.editSendMsg->setEnabled(true);
        ui.btnCalifornia->setText("Auto");
    }
    else
    {
        m_Timer->start(1000);
        ui.editSendMsg->setEnabled(false);
        ui.btnCalifornia->setText("Manual");
    }
}

void WebClientDemo::getPayload()
{
    QJsonObject jsonAppTriggered;
    jsonAppTriggered.insert("jsonrpc", m_jsonrpc);
    jsonAppTriggered.insert("method", "ax.get.payload");
    QJsonObject jsonParameters;
    jsonParameters.insert("id", m_uuid);
    jsonAppTriggered.insert("params", jsonParameters);

    QString msg4AppTriggered = json2String(jsonAppTriggered);

    ui.editSendMsg->setText(msg4AppTriggered);
    btnSendMsg_clicked();
}

void WebClientDemo::setPayload()
{
    QJsonObject jsonSetPayload;
    jsonSetPayload.insert("jsonrpc", "2.0");
    jsonSetPayload.insert("method", "ax.set.payload");
    QJsonObject jsonParameters;
    jsonParameters.insert("id", m_uuid);
    QJsonObject jsonPayload;
    jsonPayload.insert("city", m_city);
    jsonPayload.insert("type", m_clockType);
    jsonParameters.insert("payload", jsonPayload);
    jsonSetPayload.insert("params", jsonParameters);

    QString msg4WorldClock = json2String(jsonSetPayload);
    ui.editSendMsg->setText(msg4WorldClock);
    btnSendMsg_clicked();
}

void WebClientDemo:: setPropertyUI()
{
    QJsonObject jsonSetPayload;
    QString msg4WorldClock;
    jsonSetPayload.insert("id",0);
    jsonSetPayload.insert("jsonrpc", "2.0");
    jsonSetPayload.insert("method", "ax.send.to.property");
    if(m_city != "")
    {
        QJsonObject jsonParameters;
        jsonParameters.insert("id", m_uuid);
        QJsonObject jsonPayload;
        jsonPayload.insert("action", "send_city_val");
        jsonPayload.insert("city", m_city);
        jsonParameters.insert("payload", jsonPayload);
        jsonSetPayload.insert("params", jsonParameters);

        msg4WorldClock = json2String(jsonSetPayload);
        ui.txtInfo->append("[Send] " + msg4WorldClock);
        ui.editSendMsg->setText(msg4WorldClock);
        btnSendMsg_clicked();
    }

    if(m_clockType != "")
    {
        jsonSetPayload.remove("params");
        QJsonObject jsonParametersType;
        jsonParametersType.insert("id", m_uuid);
        QJsonObject jsonPayloadType;
        jsonPayloadType.insert("action", "send_type_val");
        jsonPayloadType.insert("type", m_clockType);
        jsonParametersType.insert("payload", jsonPayloadType);
        jsonSetPayload.insert("params", jsonParametersType);

        msg4WorldClock = json2String(jsonSetPayload);
        ui.txtInfo->append("[Send] " + msg4WorldClock);
        ui.editSendMsg->setText(msg4WorldClock);
        btnSendMsg_clicked();
    }
}
