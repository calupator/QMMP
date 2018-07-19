#Common settings for Qmmp Plugin Pack build

#Extra clean target

unix: QMAKE_DISTCLEAN += -r .build

#Some conf to redirect intermediate stuff in separate dirs
UI_DIR=./.build/ui
MOC_DIR=./.build/moc
OBJECTS_DIR=./.build/obj
RCC_DIR=./.build/rcc

unix{
INCLUDEPATH += /home/user/qmmp-1.2/include
QMAKE_LIBDIR += /home/user/qmmp-1.2/lib
INCLUDEPATH += /usr/local/include
}else{
INCLUDEPATH += C:/projects/qmmp-svn-stable/qmmp-1.2/src
QMAKE_LIBDIR += C:/projects/qmmp-svn-stable/qmmp-1.2/bin
}

QT += widgets

#Version

QMMP_PLUGIN_PACK_VERSION = 1.2.3
CONFIG += SVN_VERSION


#Comment/uncomment this if you want to change plugins list

CONFIG += SRC_PLUGIN
CONFIG += MPG123_PLUGIN
CONFIG += FFAP_PLUGIN
CONFIG += FFAP_YASM #assembler optimizations
CONFIG += XMP_PLUGIN
CONFIG += GOOM_PLUGIN
CONFIG += GOOM_ASM #assembler optimizations
CONFIG += HISTORY_PLUGIN
CONFIG += FFVIDEO_PLUGIN

CONFIG -= $$DISABLED_PLUGINS
