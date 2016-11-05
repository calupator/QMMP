include(../../plugins.pri)

INCLUDEPATH += ../../../../src
CONFIG += warn_on \
plugin

TARGET =$$PLUGINS_PREFIX/General/notifier
unix:QMAKE_CLEAN =$$PLUGINS_PREFIX/General/libnotifier.so


TEMPLATE = lib
unix:QMAKE_LIBDIR += ../../../../lib
unix:LIBS += -lqmmpui -lqmmp
win32:QMAKE_LIBDIR += ../../../../bin
win32:LIBS += -lqmmpui0 -lqmmp0

RESOURCES = notifier_images.qrc \
            translations/translations.qrc

unix {
  isEmpty(LIB_DIR){
    LIB_DIR = /lib
  }
  qtHaveModule(x11extras) {
    DEFINES += Q_WS_X11
    QT += x11extras
  }
  target.path = $$LIB_DIR/qmmp/General
  INSTALLS += target
}

HEADERS += notifierfactory.h \
 notifier.h \
 popupwidget.h \
 settingsdialog.h
win32:HEADERS += ../../../../src/qmmpui/general.h
SOURCES += notifierfactory.cpp \
 notifier.cpp \
 popupwidget.cpp \
 settingsdialog.cpp

FORMS += settingsdialog.ui

