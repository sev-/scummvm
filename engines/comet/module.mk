MODULE := engines/comet

MODULE_OBJS = \
	actor.o \
	animationmgr.o \
	book.o \
	comet.o \
	detection.o \
	dialog.o \
	font.o \
	inventory.o \
	map.o \
	music.o \
	pak.o \
	puzzle.o \
	resource.o \
	resourcemgr.o \
	saveload.o \
	screen.o \
	scene.o \
	script.o \
	shadow.o \
	speech.o \
	text.o \
	unpack.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
