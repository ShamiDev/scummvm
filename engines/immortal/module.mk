MODULE := engines/immortal

MODULE_OBJS = \
	metaengine.o \
	disk.o \
	immortal.o \
	kernal.o \
	logic.o \
	sprites.o \
	compression.o \
	misc.o \
	cycle.o \
	drawChr.o \
 	level.o

#	universe.o \
#	room.o \
#	object.o \
#	door.o \
#	flameset.o \
#	bullet.o \
#	monster.o \
#	motives.o

# This module can be built as a plugin
ifeq ($(ENABLE_IMMORTAL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
