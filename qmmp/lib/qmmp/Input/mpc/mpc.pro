FORMS += detailsdialog.ui 
HEADERS += decodermpcfactory.h \
           decoder_mpc.h \
	   detailsdialog.h
SOURCES += decoder_mpc.cpp \
           decodermpcfactory.cpp \
	   detailsdialog.cpp
DESTDIR = ../
QMAKE_CLEAN += ../libmpc.so
INCLUDEPATH += ../../../
CONFIG += release \
warn_on \
plugin \
link_pkgconfig
TEMPLATE = lib
QMAKE_LIBDIR += ../../../
LIBS += -lqmmp -L/usr/lib -lmpcdec -I/usr/include
PKGCONFIG += taglib 
#TRANSLATIONS = translations/mpc_plugin_ru.ts
#RESOURCES = translations/translations.qrc

isEmpty (LIB_DIR){
LIB_DIR = /lib
}

target.path = $$LIB_DIR/qmmp/Input
INSTALLS += target
