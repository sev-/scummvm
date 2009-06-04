MODULE := engines/comet

MODULE_OBJS = \
	animation.o \
	detection.o \
	dialog.o \
	font.o \
	comet.o \
	script.o \
	pak.o \
	unpack.o \
	sceneobjects.o \
	screen.o \
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
