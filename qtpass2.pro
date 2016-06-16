#-------------------------------------------------
#
# Project created by QtCreator 2016-02-10T10:23:41
#
#-------------------------------------------------

QT       += core gui

CONFIG += link_pkgconfig C++11
PKGCONFIG += keepass2pp libssl


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtpass2
TEMPLATE = app

SOURCES += main.cpp\
        qtpasswindow.cpp \
    opendialog.cpp \
    databaseview.cpp \
    qkdbxdatabase.cpp \
    qkdbxgroup.cpp \
    entryeditdialog.cpp \
    passwordchangedialog.cpp \
    sslentropy.cpp \
    passwordoptions.cpp \
    passwordgenerator.cpp \
    utils.cpp \
    entryversionsdialog.cpp \
    icondialog.cpp \
    undocommands.cpp

HEADERS  += qtpasswindow.h \
    opendialog.h \
    databaseview.h \
    qkdbxdatabase.h \
    qkdbxgroup.h \
    entryeditdialog.h \
    passwordchangedialog.h \
    passwordoptions.h \
    passwordgenerator.h \
    utils.h \
    entryversionsdialog.h \
    databaseviewwidget.h \
    icondialog.h \
    undocommands.h

FORMS    += qtpasswindow.ui \
    opendialog.ui \
    databaseview.ui \
    entryeditdialog.ui \
    passwordchangedialog.ui \
    passwordoptions.ui \
    passwordgenerator.ui \
    entryversionsdialog.ui \
    icondialog.ui

QMAKE_RESOURCE_FLAGS += -no-compress

RESOURCES += \
    icons/16x16/16x16.qrc \
    icons/HighRes/HighRes.qrc

win32{
    DEFINES += WIN32_EVENTS_SSL_ENTROPY_SOURCE
}else:packagesExist(xcb){
    DEFINES += XCB_EVENTS_SSL_ENTROPY_SOURCE
    PKGCONFIG += xcb
}else{
    message("You have no 'xcb' development package installed. Application will\
            be unable to use X11 events to seed it's random genrator with\
            entropy.");
}
