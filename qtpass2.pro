#-------------------------------------------------
#
# Project created by QtCreator 2016-02-10T10:23:41
#
#-------------------------------------------------

QT       += core gui concurrent

CONFIG += link_pkgconfig
PKGCONFIG += keepass2pp libssl

QMAKE_CXXFLAGS += -std=c++1y

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtpass2
TEMPLATE = app

SOURCES += main.cpp\
        qtpasswindow.cpp \
    qkdbxdatabase.cpp \
    qkdbxgroup.cpp \
    passwordchangedialog.cpp \
    sslentropy.cpp \
    passwordoptions.cpp \
    passwordgenerator.cpp \
    utils.cpp \
    undocommands.cpp \
    qkdbxview.cpp \
    databasesettings.cpp \
    executor.cpp \
    localclipboard.cpp \
    ui/grouppickerdialog.cpp \
    ui/partial/groupeditor.cpp \
    ui/groupeditordialog.cpp \
    ui/partial/notificationframe.cpp \
    ui/entryversionsdialog.cpp \
    ui/partial/versioneditor.cpp \
    ui/entryeditdialog.cpp \
    ui/newentrydialog.cpp \
    ui/newgroupdialog.cpp \
    ui/partial/iconseditor.cpp \
    ui/icondialog.cpp \
    ui/iconbutton.cpp \
    qmdiaction.cpp \
    ui/opendialog.cpp \
    ui/newdialog.cpp

HEADERS  += qtpasswindow.h \
    qkdbxdatabase.h \
    qkdbxgroup.h \
    passwordchangedialog.h \
    passwordoptions.h \
    passwordgenerator.h \
    utils.h \
    databaseviewwidget.h \
    undocommands.h \
    qkdbxview.h \
    databasesettings.h \
    executor.h \
    localclipboard.h \
    ui/grouppickerdialog.h \
    ui/partial/groupeditor.h \
    ui/groupeditordialog.h \
    ui/partial/notificationframe.h \
    ui/entryversionsdialog.h \
    ui/partial/versioneditor.h \
    ui/entryeditdialog.h \
    ui/newentrydialog.h \
    ui/newgroupdialog.h \
    ui/partial/iconseditor.h \
    ui/icondialog.h \
    ui/iconbutton.h \
    qmdiaction.h \
    ui/opendialog.h \
    ui/newdialog.h

FORMS    += qtpasswindow.ui \
    passwordchangedialog.ui \
    passwordoptions.ui \
    passwordgenerator.ui \
    qkdbxview.ui \
    databasesettings.ui \
    ui/grouppickerdialog.ui \
    ui/partial/groupeditor.ui \
    ui/groupeditordialog.ui \
    ui/entryversionsdialog.ui \
    ui/partial/versioneditor.ui \
    ui/entryeditdialog.ui \
    ui/newentrydialog.ui \
    ui/newgroupdialog.ui \
    ui/partial/iconseditor.ui \
    ui/icondialog.ui \
    ui/opendialog.ui \
    ui/newdialog.ui

QMAKE_RESOURCE_FLAGS += -no-compress

RESOURCES += \
    icons/16x16/16x16.qrc \
    icons/HighRes/HighRes.qrc \
    icons/32x32/32x32.qrc

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
