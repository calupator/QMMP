HEADERS += srconverter.h \
 effectsrconverterfactory.h \
 settingsdialog.h

SOURCES += srconverter.cpp \
 effectsrconverterfactory.cpp \
 settingsdialog.cpp

DESTDIR = ../
QMAKE_CLEAN = ../libsrconverter.so
INCLUDEPATH += ../../../
CONFIG += release \
warn_on \
plugin \
link_pkgconfig

PKGCONFIG += samplerate
TEMPLATE = lib
QMAKE_LIBDIR += ../../../
LIBS += -lqmmp -L/usr/lib -I/usr/include

#TRANSLATIONS = translations/ffmpeg_plugin_ru.ts
#RESOURCES = translations/translations.qrc

isEmpty(LIB_DIR){
    LIB_DIR = /lib
}
target.path = $$LIB_DIR/qmmp/Effect
INSTALLS += target
#FORMS += settingsdialog.ui

FORMS += settingsdialog.ui

