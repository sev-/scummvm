MODULE := engines/comet

MODULE_OBJS = \
	detection.o \
	font.o \
	anim.o \
	comet.o \
	script.o \
	pak.o \
	unpack.o \
	sceneobjects.o \
	marche.o \
	shadow.o \
	music.o \
	sound.o \

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
