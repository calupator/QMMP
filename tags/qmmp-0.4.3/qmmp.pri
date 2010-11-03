#Common settings for Qmmp build

#Some conf to redirect intermediate stuff in separate dirs
UI_DIR=./.build/ui/
MOC_DIR=./.build/moc/
OBJECTS_DIR=./.build/obj

#Version

QMMP_VERSION = 0.4.3
CONFIG += SVN_VERSION


#Comment/uncomment this if you want to change plugins list

CONFIG += JACK_PLUGIN
CONFIG += FLAC_PLUGIN
CONFIG += MUSEPACK_PLUGIN
CONFIG += FFMPEG_PLUGIN
CONFIG += MODPLUG_PLUGIN
CONFIG += OSS_PLUGIN
CONFIG += PULSE_AUDIO_PLUGIN
CONFIG += ALSA_PLUGIN
CONFIG += AAC_PLUGIN
CONFIG += MPLAYER_PLUGIN
CONFIG += CDAUDIO_PLUGIN
CONFIG += BS2B_PLUGIN
CONFIG += LADSPA_PLUGIN
CONFIG += PROJECTM_PLUGIN

#additional features

CONFIG += WITH_ENCA
CONFIG += WITH_PROJECTM20
