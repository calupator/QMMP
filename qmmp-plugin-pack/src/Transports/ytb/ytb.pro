include(../../plugins.pri)

TARGET = $$PLUGINS_PREFIX/Transports/ytb

QT += network

HEADERS += \
    ytbinputfactory.h \
    ytbinputsource.h \
    bufferdevice.h

SOURCES += \
    ytbinputfactory.cpp \
    ytbinputsource.cpp \
    bufferdevice.cpp

RESOURCES = translations/translations.qrc

unix {
    LIBS += -L/usr/lib
    PKGCONFIG += libcurl
    target.path = $$PLUGIN_DIR/Transports
    INSTALLS += target
}


