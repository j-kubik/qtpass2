#-------------------------------------------------
#
# Project created by QtCreator 2016-02-10T10:23:41
#
#-------------------------------------------------

QT       += core gui

CONFIG += link_pkgconfig
PKGCONFIG += keepass2pp libssl

QMAKE_CXXFLAGS += -std=c++1y

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtpass2
TEMPLATE = app

SOURCES += main.cpp\
        qtpasswindow.cpp \
    opendialog.cpp \
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
    undocommands.cpp \
    versioneditor.cpp \
    qkdbxview.cpp \
    newentrydialog.cpp \
    databasesettings.cpp \
    callback.cpp \
    qgroupcombo.cpp

HEADERS  += qtpasswindow.h \
    opendialog.h \
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
    undocommands.h \
    versioneditor.h \
    qkdbxview.h \
    newentrydialog.h \
    databasesettings.h \
    callback.h \
    qgroupcombo.h

FORMS    += qtpasswindow.ui \
    opendialog.ui \
    entryeditdialog.ui \
    passwordchangedialog.ui \
    passwordoptions.ui \
    passwordgenerator.ui \
    entryversionsdialog.ui \
    icondialog.ui \
    versioneditor.ui \
    qkdbxview.ui \
    newentrydialog.ui \
    databasesettings.ui

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
