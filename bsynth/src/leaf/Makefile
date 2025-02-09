######################################
# Linux Makefile for libleaf
######################################

TARGET = libleaf
MODULE_DIR= Src
BUILD_DIR = build

######################################
# Source Files
######################################

C_SOURCES = \
Src/leaf-math.c \
Src/leaf-mempool.c \
Src/leaf-tables.c \
Src/leaf-distortion.c \
Src/leaf-dynamics.c \
Src/leaf-analysis.c \
Src/leaf-delay.c \
Src/leaf-oscillators.c \
Src/leaf-effects.c \
Src/leaf-electrical.c \
Src/leaf-envelopes.c \
Src/leaf-filters.c \
Src/leaf-instruments.c \
Src/leaf-reverb.c \
Src/leaf-midi.c \
Src/leaf-physical.c \
Src/leaf-sampling.c \
leaf.c \
Externals/d_fft_mayer.c

######################################
# Compiler and Flags
######################################

PREFIX = 
CC = gcc
CXX = g++
AR = ar

OPT = -O3
DEBUG = 0

# CFLAGS
C_INCLUDES = \
-I$(MODULE_DIR) \
-IInc/ \
-IExternals \
-I. \

WARNINGS = -Wall -Wno-attributes -Wno-strict-aliasing -Wno-maybe-uninitialized -Wno-missing-attributes -Wno-stringop-overflow

C_DEFS = -DHOST_BUILD -DNDEBUG=1 -DRELEASE=1
CFLAGS = $(C_INCLUDES) $(C_DEFS) -ggdb $(WARNINGS) $(OPT) -std=gnu11

CPP_WARNINGS = -Wno-register
CPPFLAGS = $(CFLAGS) $(CPP_WARNINGS) -std=gnu++14 -fno-exceptions -fno-rtti

######################################
# Build Rules
######################################

all: $(BUILD_DIR)/$(TARGET).a

OBJECTS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
vpath %.c $(sort $(dir $(C_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MP -MF $(BUILD_DIR)/$(notdir $(<:.c=.dep))

$(BUILD_DIR)/$(TARGET).a: $(OBJECTS) Makefile
	$(AR) -r $@ $(OBJECTS)

$(BUILD_DIR):
	mkdir -p $@

######################################
# Clean
######################################
clean:
	-rm -fR $(BUILD_DIR)

######################################
# Include Dependencies
######################################
-include $(wildcard $(BUILD_DIR)/*.dep)
