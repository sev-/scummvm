MODULE := engines/enchantia

MODULE_OBJS = \
	datfile.o \
	decompress.o \
	detection.o \
	enchantia.o \
	logic.o \
	metaengine.o \
	music.o \
	resource.o \
	saveload.o

# This module can be built as a plugin
ifeq ($(ENABLE_ENCHANTIA2), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
