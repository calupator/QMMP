include(../../plugins.pri)

HEADERS += outputwasapifactory.h \
           outputwasapi.h

SOURCES += outputwasapifactory.cpp \
           outputwasapi.cpp

HEADERS += ../../../../src/qmmp/output.h


TARGET=$$PLUGINS_PREFIX/Output/wasapi

CONFIG += warn_on \
thread \
plugin

TEMPLATE = lib
LIBS += -lqmmp0 -lstrmiids -ldmoguids -lmsdmo -lole32 -loleaut32 -luuid -lgdi32 -lksuser

RESOURCES = translations/translations.qrc

target.path = $$LIB_DIR/qmmp/Output
INSTALLS += target
