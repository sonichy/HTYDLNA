#ifndef DLNACLIENT_H
#define DLNACLIENT_H

#include <QString>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QDomDocument>

class DLNAClient: public QObject
{
    Q_OBJECT
public:
    DLNAClient(QString s);
    //~DLNAClient();
    QString scheme="", IP="", port="", description="", controlURL="", friendlyName="";
    QList<QMap<QString,QString>> list_service;
    QString UploadFileToPlay(QString ControlURL, QString UrlToPlay);

private:
    QUdpSocket *socket;
    QByteArray getReply(QString surl);
    QByteArray postReply(QString surl, QString spost);
    bool getDescription(QByteArray BA);
    QString Desc();

signals:
    void finish();

};

#endif // DLNACLIENT_H