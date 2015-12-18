#-------------------------------------------------
#
# Project created by QtCreator 2011-05-28T10:47:46
#
#-------------------------------------------------

QT       += core gui

TARGET = UsbSVUD
TEMPLATE = app
QMAKE_CXXFLAGS_RELEASE=-O3 -Wextra
QMAKE_CXXFLAGS_DEBUG=-Wextra -g

RC_FILE=icon.rc

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

LIBS += -L../src -lqledplugin -L../../libusb -lusb

INCLUDEPATH += ../../libusb

RESOURCES += \
    UsbSVUD.qrc
