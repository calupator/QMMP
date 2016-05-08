#Common settings for Qmmp build

#Some conf to redirect intermediate stuff in separate dirs
UI_DIR=./.build/ui/
MOC_DIR=./.build/moc/
OBJECTS_DIR=./.build/obj
RCC_DIR=./.build/rcc

#Version

QMMP_VERSION = 0.9.10

#Comment/uncomment this if you want to change plugins list

CONFIG += JACK_PLUGIN
CONFIG += FLAC_PLUGIN
CONFIG += MUSEPACK_PLUGIN
CONFIG += FFMPEG_PLUGIN
CONFIG += MODPLUG_PLUGIN
CONFIG += WILDMIDI_PLUGIN
CONFIG += GME_PLUGIN
CONFIG += OPUS_PLUGIN
CONFIG += OSS_PLUGIN #deprecated
CONFIG += PULSE_AUDIO_PLUGIN
CONFIG += ALSA_PLUGIN
CONFIG += OSS4_PLUGIN
CONFIG += AAC_PLUGIN
CONFIG += MPLAYER_PLUGIN
CONFIG += CDAUDIO_PLUGIN
CONFIG += BS2B_PLUGIN
CONFIG += LADSPA_PLUGIN
CONFIG += PROJECTM_PLUGIN
CONFIG += UDISKS_PLUGIN #deprecated
CONFIG += UDISKS2_PLUGIN
CONFIG += HAL_PLUGIN
CONFIG += SID_PLUGIN

#additional features

CONFIG += WITH_ENCA
CONFIG += WITH_PROJECTM20
CONFIG += WITH_SKINNED
CONFIG += WITH_QSUI
CONFIG += WITH_NEW_JACK
#CONFIG += FFMPEG_LEGACY #uncomment for ffmpeg < 0.9 or libav < 0.8

CONFIG -= $$DISABLED_PLUGINS
