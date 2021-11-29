#include "dlnaclient.h"

DLNAClient::DLNAClient(QString s)
{

    socket = new QUdpSocket;
    QStringList SL = s.split("\r\n");

    foreach (QString s, SL) {
        if (s.contains("LOCATION:", Qt::CaseInsensitive)) {
            QString surl = s.remove("LOCATION:", Qt::CaseInsensitive).trimmed();
            qDebug() << surl;
            scheme = surl.left(surl.indexOf("://") + 3);
            qDebug() << scheme;
            QString s0 = surl.mid(s.indexOf("://") + 2);
            qDebug() << s0;
//            QRegularExpression RE("([^:]*):([^/]*)/(.*)");
//            QRegularExpressionMatch REM = RE.match(s0);
//            IP = REM.captured(1);
//            port = REM.captured(2);
//            description = REM.captured(3);
            IP = s0.left(s0.indexOf(":"));
            port = s0.mid(s0.indexOf(":") + 1, s0.indexOf("/") - s0.indexOf(":") - 1);
            description = s0.right(s0.length() - s0.indexOf("/") - 1);
            qDebug() << IP << port << description;
            QByteArray BA = getReply(surl);
            QFile file;
            if (description == "")
                file.setFileName("description.xml");
            else
                file.setFileName(description);
            if (file.open(QFile::WriteOnly)) {
                file.write(BA);
                file.close();
            }
            qDebug() << getDescription(BA);
        }
    }
}

QByteArray DLNAClient::getReply(QString surl)
{
    QNetworkAccessManager *NAM = new QNetworkAccessManager;
    QNetworkRequest request;
    request.setUrl(QUrl(surl));
    QNetworkReply *reply = NAM->get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->deleteLater();
    return reply->readAll();
}

QByteArray DLNAClient::postReply(QString surl, QString spost)
{
    QNetworkAccessManager *NAM = new QNetworkAccessManager;
    QNetworkRequest request;
    request.setUrl(QUrl(surl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; charset=\"utf-8\"");
    request.setRawHeader("SOAPAction", "\"urn:schemas-upnp-org:service:AVTransport:1#SetAVTransportURI\"");
    QByteArray BA_post;
    BA_post.append(spost.toUtf8());
    QNetworkReply *reply = NAM->post(request, BA_post);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->deleteLater();
    return reply->readAll();
}

bool DLNAClient::getDescription(QByteArray BA)
{
    QDomDocument doc;
    bool b = doc.setContent(BA);
    if (b) {
        QDomElement root = doc.documentElement();
        friendlyName = root.elementsByTagName("friendlyName").at(0).firstChild().nodeValue();
        //qDebug() << friendlyName;
        QDomNodeList service = root.elementsByTagName("service");        
        for (int i=0; i<service.length(); i++) {
            QDomNodeList child = service.at(i).childNodes();
            //QList<QMap> https://blog.csdn.net/imxiangzi/article/details/52685604
            QMap<QString,QString> map;
            for (int j=0; j<child.length(); j++) {
                map.insert(child.at(j).nodeName(), child.at(j).firstChild().nodeValue());
            }
            //qDebug() << map;
            list_service.append(map);
        }
        //qDebug() << list_service.length() << list_service;
    }
    emit finish();
    return b;
}

QString DLNAClient::Desc()
{
    QString XML = "<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:r=\"urn:schemas-rinconnetworks-com:metadata-1-0/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">\n";
    XML+="<item>\n";
    XML+="<dc:title>Capital Edinburgh "  + QString::number(QDateTime::currentDateTime().time().msec()) + "</dc:title>\n";
    XML+="<upnp:class>object.item.audioItem.audioBroadcast</upnp:class>\n";
    XML+="<desc id=\"cdudn\" nameSpace=\"urn:schemas-rinconnetworks-com:metadata-1-0/\">SA_RINCON65031_</desc>\n";
    XML+="</item>\n";
    XML += "</DIDL-Lite>\n";
    return XML;
}

QString DLNAClient::UploadFileToPlay(QString ControlURL, QString UrlToPlay)
{    
    qDebug() << UrlToPlay;
    QString XML = "<?xml version=\"1.0\"?>\n"
                  "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n"
                  "<SOAP-ENV:Body>\n"
                  "<u:SetAVTransportURI xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\n"
                  "<InstanceID>0</InstanceID>\n";
    XML += "<CurrentURI>" + UrlToPlay + "</CurrentURI>\n";
    XML += "<CurrentURIMetaData></CurrentURIMetaData>\n";//" + Desc() + "
    XML += "</u:SetAVTransportURI>\n";
    XML += "</SOAP-ENV:Body>\n"
           "</SOAP-ENV:Envelope>";
    QString surl = scheme + IP + ":" + port;
    if (!ControlURL.startsWith("/"))
        surl += "/";
    surl += ControlURL;
    qDebug() << surl;
    return postReply(surl, XML);
}