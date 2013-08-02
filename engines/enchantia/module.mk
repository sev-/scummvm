MODULE := engines/enchantia

MODULE_OBJS = \
	datfile.o \
	decompress.o \
	detection.o \
	enchantia.o \
	logic.o \
	music.o \
	resource.o \
	saveload.o

# This module can be built as a plugin
#ifdef BUILD_PLUGINS
#PLUGIN := 1
#endif
ifeq ($(ENABLE_ENCHANTIA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
#
# Include common rules
include $(srcdir)/rules.mk
