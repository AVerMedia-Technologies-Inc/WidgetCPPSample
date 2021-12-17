#pragma once

#include <QtWidgets/QWidget>
#include "ui_WebClientDemo.h"
#include "Websocket2Client.h"
#include <QtCore/QJsonObject>
#include <QTimer>
#include <QTimeZone>

class WebClientDemo : public QWidget
{
    Q_OBJECT

public:
    WebClientDemo(QString uuid, QString port, QWidget *parent = Q_NULLPTR);
    ~WebClientDemo();

private slots:
    void btnSendMsg_clicked();
    void btnClose_clicked();
    void btnConnect_clicked();
    void showReceivedMessage(QString message);
    void btnManual_clicked(bool check);

private:
    Ui::WebClientDemo ui;
    Websocket2Client *m_webSocket2Client = NULL;
    QUrl m_url;
    QString m_uuid;
    QTimeZone m_timeZone;
    QJsonObject translate2Json(QString text);
    QString json2String(QJsonObject json);
    QString image2Base64(QImage image);
    void showWorldClockMessage(QImage image);
    QTimer* m_Timer;
    void drawingClock();
    bool m_isDigitalClock = true;
    void getPayload();
    void setPayload();
    QString m_jsonrpc = "2.0";
    QString m_city = "";
    QString m_clockType = "";
    QGraphicsScene* m_scene = new QGraphicsScene;
    void setPropertyUI();
};
