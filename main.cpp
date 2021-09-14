#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qSetMessagePattern("[ %{file}: %{line} ] %{message}");
    QApplication a(argc, argv);
    a.setApplicationName("HTYSSDP");
    a.setApplicationVersion("0.1");
    MainWindow w;
    w.show();
    return a.exec();
}