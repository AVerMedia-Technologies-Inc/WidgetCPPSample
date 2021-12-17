#include "WebClientDemo.h"
#include <QtWidgets/QApplication>
#include <QUuid>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString pluginUuid;
    QString port;
    if (argc >= 3)
    {
        pluginUuid = QString(argv[1]);
        port = QString(argv[2]);
    }
    else
    {
        pluginUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    WebClientDemo frmClient(pluginUuid, port);
    frmClient.setWindowTitle("PluginQt.Exe_UUID:[ " + pluginUuid + " ]_PORT:" + port);
    //frmClient.show();
    return a.exec();
}
