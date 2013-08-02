MODULE := devtools/create_enchantia

MODULE_OBJS := \
	create_enchantia.o \
	md5.o \
	util.o

# Set the name of the executable
TOOL_EXECUTABLE := create_enchantia

# Include common rules
include $(srcdir)/rules.mk
