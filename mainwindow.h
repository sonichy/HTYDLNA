#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHostAddress>
#include <QUdpSocket>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QClipboard>
#include <QAction>
#include <QMenu>
#include "dlnaclient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QHostAddress groupAddress;
    QUdpSocket *socket;
    int c, intent;
    QString hostName, IP, uuid;
    QList<DLNAClient*> list_client;

private slots:
    void on_pushButton_discover_clicked();
    void on_pushButton_notify_clicked();
    void readMsg();
    void refresh();
    void tableWidget_contextMenu(const QPoint &point);

};

#endif // MAINWINDOW_H