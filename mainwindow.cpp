// https://blog.csdn.net/sinat_33859977/article/details/113567847

// https://github.com/sYCH3L/TwitchTVDLNAPlayer

// https://breezetemple.github.io/2019/03/01/dlna-and-upnp-protocol/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QStringList header;
    header << "IP" << "Description" << "FriendlyName";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableWidget_contextMenu(QPoint)));

    groupAddress = QHostAddress("239.255.255.250");
    socket = new QUdpSocket(this);
    auto ok = socket->bind(QHostAddress::AnyIPv4, 8081, QUdpSocket::ShareAddress);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readMsg()));
    if (!ok) {
        ui->statusBar->showMessage("Bind failed!");
        return;
    }

    ok = socket->joinMulticastGroup(groupAddress);
    if (!ok) {
        return;
    }

    hostName = QHostInfo::localHostName();
    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {
            IP = address.toString();
        }
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readMsg()
{    
    while (socket->hasPendingDatagrams()) {
        QByteArray reply;
        reply.resize(socket->pendingDatagramSize());
        socket->readDatagram(reply.data(), reply.size());
        qDebug() << c;
        qDebug() << reply.data();
        if (intent == 0) {
            DLNAClient *client = new DLNAClient(QString::fromUtf8(reply));
            connect(client, SIGNAL(finish()), this, SLOT(refresh()));
            list_client.append(client);
        }
    }
    if (intent == 0) {
        refresh();
    }
    c++;
}

void MainWindow::on_pushButton_discover_clicked()
{
    c = 0;
    intent = 0;
    ui->tableWidget->setRowCount(0);
    list_client.clear();
    QByteArray BA("M-SEARCH * HTTP/1.1\r\n"
                  "Host:239.255.255.250:1900\r\n"
                  //"ST: ssdp:all\r\n"
                  //"ST: upnp:rootdevice\r\n"
                  "ST: urn:schemas-upnp-org:service:AVTransport:1\r\n"  //投屏
                  "MAN: \"ssdp:discover\"\r\n"
                  "MX: 3\r\n"
                  "\r\n");

    auto writeOk = socket->writeDatagram(BA.data(), groupAddress, 1900);
    if (writeOk == -1) {
        qDebug() << "Writing Datagram failed";
    }
}

void MainWindow::on_pushButton_notify_clicked()
{
    c = 0;
    intent = 1;
    QString s = QString("NOTIFY * HTTP/1.1\r\n"
                "HOST: 239.255.255.250:1900\r\n"
                "NT: upnp:rootdevice\r\n"
                "NTS: ssdp:alive\r\n"
                "LOCATION: %1:8080/description.xml\r\n"
                "USN: uuid:8fa440b3-f312-4415-8640-f790a3845fc1::upnp:rootdevice\r\n"
                "CACHE-CONTROL: max-age=300\r\n"
                "SERVER: %2 UPnP/1.0 %3/%4\r\n"
                "\r\n").arg(IP).arg(hostName).arg(qApp->applicationName()).arg(qApp->applicationVersion());
    auto writeOk = socket->writeDatagram(s.toUtf8().data(), groupAddress, 1900);

    s = QString("NOTIFY * HTTP/1.1\r\n"
                "HOST: 239.255.255.250:1900\r\n"
                "NT: urn:schemas-upnp-org:service:AVTransport:1\r\n"
                "NTS: ssdp:alive\r\n"
                "LOCATION: %1:8080/description.xml\r\n"
                "USN: uuid:8fa440b3-f312-4415-8640-f790a3845fc1::urn:schemas-upnp-org:service:AVTransport:1\r\n"
                "CACHE-CONTROL: max-age=300\r\n"
                "SERVER: %2 UPnP/1.0 %3/%4\r\n"
                "\r\n").arg(IP).arg(hostName).arg(qApp->applicationName()).arg(qApp->applicationVersion());
    qDebug() << s;
    writeOk = socket->writeDatagram(s.toUtf8().data(), groupAddress, 1900);
    if (writeOk == -1) {
        qDebug() << "Writing Datagram failed";
    }
}

void MainWindow::refresh()
{
    ui->tableWidget->setRowCount(0);
    for (int i=0; i<list_client.length(); i++) {
        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(list_client.at(i)->scheme + list_client.at(i)->IP + ":" + list_client.at(i)->port));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(list_client.at(i)->description));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(list_client.at(i)->friendlyName));
    }
    ui->tableWidget->resizeColumnsToContents();
}

void MainWindow::tableWidget_contextMenu(const QPoint &point)
{
    QTableWidgetItem *item = ui->tableWidget->itemAt(point);
    if (item != nullptr) {
        int row = item->row();
        QList<QAction*> actions;
        QAction *action_copy = new QAction(this);
        action_copy->setText("Copy");
        actions.append(action_copy);
        QAction *action_play = new QAction(this);
        action_play->setText("Play");
        actions.append(action_play);
        QAction *result_action = QMenu::exec(actions, ui->tableWidget->mapToGlobal(point));
        if (result_action == action_copy) {
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(ui->tableWidget->item(row,0)->text() + "/" + ui->tableWidget->item(row,1)->text());         
        } else if (result_action == action_play) {            
            QString controlURL="";
            QList<QMap<QString,QString>> list_service = list_client.at(row)->list_service;
            for (int i=0; i<list_service.length(); i++) {
                QMap<QString,QString> map = list_service.at(i);
                qDebug() << map.value("serviceType");
                if (map.value("serviceType") == "urn:schemas-upnp-org:service:AVTransport:1") {
                    controlURL = map.value("controlURL");
                    break;
                }
            }
            QString s = list_client.at(row)->UploadFileToPlay(controlURL, ui->lineEdit->text());
            qDebug() << s;
            QFile file;
            if (controlURL == "")
                file.setFileName("controlURL.xml");
            else
                file.setFileName(QFileInfo(controlURL).fileName());
            if (file.open(QFile::WriteOnly)) {
                file.write(s.toUtf8());
                file.close();
            }
        }
    }
}