# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2024
#
BLOCKSDS	?= /opt/blocksds/core
BLOCKSDSEXT	?= /opt/blocksds/external

# User config

NAME			:= libdsf_user_settings
GAME_TITLE		:= Firmware user settings
GAME_SUBTITLE	:= LibDSF example

# Source code paths

GFXDIRS		:= graphics
BINDIRS		:= data

# Libraries

LIBS		:= -ldsf -lnds9
LIBDIRS		:= $(BLOCKSDS)/libs/libnds \
			   $(BLOCKSDSEXT)/libdsf

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile
