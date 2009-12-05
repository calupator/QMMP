include(../../plugins.pri)

HEADERS += ladspaplugin.h \
           effectladspafactory.h \
           settingsdialog.h

SOURCES += ladspaplugin.cpp \
           effectladspafactory.cpp \
           settingsdialog.cpp

TARGET =$$PLUGINS_PREFIX/Effect/ladspa
QMAKE_CLEAN =$$PLUGINS_PREFIX/Effect/libladspa.so
INCLUDEPATH += ../../../
CONFIG += release \
warn_on \
plugin \
link_pkgconfig

TEMPLATE = lib
QMAKE_LIBDIR += ../../../../lib
LIBS += -lqmmp  -L/usr/lib -I/usr/include

TRANSLATIONS = translations/ladspa_plugin_cs.ts \
               translations/ladspa_plugin_de.ts \
               translations/ladspa_plugin_zh_CN.ts \
               translations/ladspa_plugin_zh_TW.ts \
               translations/ladspa_plugin_uk_UA.ts \
               translations/ladspa_plugin_pl.ts \
               translations/ladspa_plugin_ru.ts \
               translations/ladspa_plugin_it.ts \
               translations/ladspa_plugin_tr.ts \
               translations/ladspa_plugin_lt.ts
#RESOURCES = translations/translations.qrc

isEmpty(LIB_DIR){
    LIB_DIR = /lib
}
target.path = $$LIB_DIR/qmmp/Effect
INSTALLS += target


FORMS += settingsdialog.ui
