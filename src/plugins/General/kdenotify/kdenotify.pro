# -------------------------------------------------
# Project created by QtCreator 2009-10-14T17:55:38
# -------------------------------------------------
include(../../plugins.pri)
QT += dbus
CONFIG += warn_on \
    plugin
unix:TARGET = $$PLUGINS_PREFIX/General/kdenotify
unix:QMAKE_CLEAN = $$PLUGINS_PREFIX/General/libkdenotify.so
TEMPLATE = lib
unix {
    LIBS += -lqmmpui \
        -lqmmp
}
unix {
    target.path = $$LIB_DIR/qmmp/General
    INSTALLS += target
}
SOURCES += kdenotifyfactory.cpp \
    kdenotify.cpp \
    settingsdialog.cpp
HEADERS += kdenotifyfactory.h \
    kdenotify.h \
    settingsdialog.h
RESOURCES += translations/translations.qrc
FORMS += settingsdialog.ui

images.files = images/app_icon.png \
               images/empty_cover.png
images.path = /share/qmmp/images/
INSTALLS += images
