CC      ?= gcc
CFLAGS  ?= -O2 -g -Wall -Wextra \
           -Iinclude \
           -Isrc/core/include \
           -Isrc/platform/include \
           -Isrc/app/include
LDFLAGS ?=

BUILD   ?= build
CORE_SRCS := src/core/chess.c src/core/move.c src/core/game.c
PLAT_SRCS := src/platform/platform_base.c src/platform/event_bus.c
APP_SRCS  := src/app/controller.c

CORE_OBJS := $(CORE_SRCS:%.c=$(BUILD)/%.o)
PLAT_OBJS := $(PLAT_SRCS:%.c=$(BUILD)/%.o)
APP_OBJS  := $(APP_SRCS:%.c=$(BUILD)/%.o)

TARGET ?= gtk

ifeq ($(TARGET),cli)
  include tools/make/cli.mk
endif

ifeq ($(TARGET),gtk)
  include tools/make/gtk.mk
endif

.PHONY: all clean run

all: $(BUILD)/$(TARGET)/chess_$(TARGET)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)

run: all
	$(BUILD)/$(TARGET)/chess_$(TARGET)
