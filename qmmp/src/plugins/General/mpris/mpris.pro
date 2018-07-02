include(../../plugins.pri)

TARGET = $$PLUGINS_PREFIX/General/mpris

QT += dbus

HEADERS += mprisfactory.h \
           mpris.h  \
           mpris2/root2object.h \
           mpris2/player2object.h

SOURCES += mprisfactory.cpp \
           mpris.cpp \
           mpris2/root2object.cpp \
           mpris2/player2object.cpp

RESOURCES = translations/translations.qrc

LIBS += $$QMMPUI_LIB

target.path = $$PLUGIN_DIR/General
INSTALLS += target
