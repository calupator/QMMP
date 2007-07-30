# ???? ?????? ? KDevelop ?????????? qmake.
# ------------------------------------------- 
# ?????????? ???????????? ???????? ???????? ???????: ./libs
# ???? - ??????????:  nnp

HEADERS += recycler.h \
           buffer.h \
           constants.h \
           decoder.h \
           output.h \
           filetag.h \
           outputfactory.h \
           equ\iir_cfs.h \
           equ\iir_fpu.h \
           equ\iir.h \
           decoderfactory.h \
           soundcore.h \
           visualization.h
SOURCES += recycler.cpp \
           decoder.cpp \
           output.cpp \
           equ\iir.c \
           equ\iir_cfs.c \
           equ\iir_fpu.c \
           soundcore.cpp
TARGET = qmmp
CONFIG += release \
warn_on \
qt \
thread

TEMPLATE = lib

isEmpty (LIB_DIR){
LIB_DIR = /lib
}

target.path = $$LIB_DIR/qmmp/Input
INSTALLS += target
