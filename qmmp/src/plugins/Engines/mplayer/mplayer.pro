include(../../plugins.pri)
HEADERS += mplayerenginefactory.h \
    mplayerengine.h \
    settingsdialog.h \
    mplayermetadatamodel.h
SOURCES += mplayerengine.cpp \
    mplayerenginefactory.cpp \
    settingsdialog.cpp \
    mplayermetadatamodel.cpp
TARGET = $$PLUGINS_PREFIX/Engines/mplayer
QMAKE_CLEAN = $$PLUGINS_PREFIX/Engines/libmplayer.so

RESOURCES = translations/translations.qrc
target.path = $$LIB_DIR/qmmp/Engines
INSTALLS += target
FORMS += settingsdialog.ui
