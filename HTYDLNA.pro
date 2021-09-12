QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYDLNA
TEMPLATE = app

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dlnaclient.cpp

HEADERS += \
        mainwindow.h \
    dlnaclient.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    res.qrc