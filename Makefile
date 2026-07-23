#---------------------------------------------------------------------------------
# This Makefile is mostly boilerplate you won't need to touch. Think of it
# like a NES linker config / iNES header setup: it's declaring "here's my
# source files, here's the target chip (ARM11), here's what libraries to
# link against" rather than doing any logic itself.
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# TARGET   = name of the output file (TARGET.3dsx)
# BUILD    = folder for intermediate object files (like .o files from an assembler)
# SOURCES  = folders to look for .c/.cpp/.s files in
# INCLUDES = folders to look for header files in
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
# "source" = our own code. "stockfish-src/src" = Stockfish's code, cloned
# in by the GitHub Actions workflow before this Makefile ever runs (see
# build.yml) -- it won't exist if you try to build this locally without
# that step.
SOURCES		:=	source stockfish-src/src stockfish-src/src/syzygy
INCLUDES	:=	include stockfish-src/src stockfish-src/src/syzygy
# ROMFS: a folder whose contents get embedded directly into the .3dsx
# and are readable at runtime via paths like "romfs:/gfx/whatever.t3x".
# build.yml converts our piece PNGs into .t3x files and places them
# here before this Makefile ever runs.
ROMFS		:=	romfs
# The built-in %.3dsx:%.elf rule (inside 3ds_rules, part of devkitARM
# itself) already knows to embed this folder -- it just reads whatever
# is in _3DSXFLAGS. We don't need to rewrite that rule ourselves, just
# set this one variable. TOPDIR (not CURDIR) is used because this file
# gets re-parsed from inside the build/ subdirectory partway through
# the build, and TOPDIR reliably still points at the real project root.
#
# Turns out 3dsxtool also needs an .smdh (app title/author/icon
# metadata) any time --romfs is used -- it can't embed extras like
# RomFS without that basic metadata section too. 3ds_rules already has
# a ready-made rule for building one with sensible defaults; we just
# need to ask for it via _3DSXDEPS and point _3DSXFLAGS at it.
_3DSXDEPS	=	$(OUTPUT).smdh
_3DSXFLAGS	+=	--smdh=$(OUTPUT).smdh --romfs=$(TOPDIR)/$(ROMFS)

#---------------------------------------------------------------------------------
# options for code generation -- this is the "which CPU dialect" section,
# equivalent to telling an assembler "target 6502, not 65816"
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-g -Wall -O2 -mword-relocations \
			-fomit-frame-pointer -ffunction-sections \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -D__3DS__

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++17 -Wno-psabi

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

# -lctru = link against libctru (the "here's how to talk to 3DS hardware" library)
# -lm    = link against the standard math library
LIBS	:=	-lcitro2d -lcitro3d -lctru -lm

#---------------------------------------------------------------------------------
# CTRULIB points at libctru's install location; devkitPro's Makefiles set
# this env var for you automatically, you don't need to set it yourself.
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CTRULIB)

#---------------------------------------------------------------------------------
# You shouldn't need to edit anything below this line.
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD	:=	$(if $(strip $(CPPFILES)),$(CXX),$(CC))

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(TARGET).smdh $(TARGET).elf

else

DEPENDS	:=	$(OFILES:.o=.d)

$(OUTPUT).3dsx	:	$(OUTPUT).elf $(_3DSXDEPS)

$(OUTPUT).elf	:	$(OFILES)

-include $(DEPENDS)

endif
