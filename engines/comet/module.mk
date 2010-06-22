MODULE := engines/comet

MODULE_OBJS = \
	actor.o \
	animation.o \
	animationmgr.o \
	book.o \
	detection.o \
	dialog.o \
	font.o \
	comet.o \
	script.o \
	pak.o \
	puzzle.o \
	unpack.o \
	screen.o \
	speech.o \
	marche.o \
	saveload.o \
	shadow.o \
	inventory.o \
	map.o \
	music.o \
	sound.o \
	scene.o \
	text.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
