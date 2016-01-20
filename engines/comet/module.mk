MODULE := engines/comet

MODULE_OBJS = \
	actor.o \
	animationmgr.o \
	console.o \
	comet.o \
	comet_gui.o \
	detection.o \
	dialog.o \
	music.o \
	resource.o \
	resourcemgr.o \
	saveload.o \
	screen.o \
	scene.o \
	script.o \
	shadow.o \
	talktext.o \
	unpack.o

# This module can be built as a plugin
ifeq ($(ENABLE_COMET), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
