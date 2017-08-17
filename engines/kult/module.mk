MODULE := engines/kult

MODULE_OBJS = \
	detection.o \
	kult.o \
	resource.o \
	script.o

# This module can be built as a plugin
ifeq ($(ENABLE_KULT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
