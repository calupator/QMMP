# ???? ?????? ? KDevelop ?????????? qmake.
# ------------------------------------------- 
# ?????????? ???????????? ???????? ???????? ???????: ./Plugins/Output
# ???? - ?????? ? ????????????? 

include(../../../qmmp.pri)

CONFIG += release warn_on 
TEMPLATE = subdirs

SUBDIRS += alsa

contains(CONFIG, JACK_PLUGIN){
    SUBDIRS += jack
    message(***********************)
    message(* JACK plugin enabled *)
    message(***********************)
}
