MODULE := engines/comet

MODULE_OBJS = \
	actor.o \
	animation.o \
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
	saveload.o \
	screen.o \
	scene.o \
	script.o \
	shadow.o \
	speech.o \
	text.o \
	unpack.o \
	resource.o \
	resourcemgr.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
