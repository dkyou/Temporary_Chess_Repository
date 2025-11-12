PKG ?= pkg-config
CFLAGS += $(shell $(PKG) --cflags gtk+-3.0 gdk-pixbuf-2.0) \
          -Isrc/ports/linux_gtk/include \
          -DPAL_IMAGE_EXTERNAL
LDFLAGS += $(shell $(PKG) --libs   gtk+-3.0 gdk-pixbuf-2.0) \
           -lm


PORT_SRCS := \
  src/ports/linux_gtk/main_gtk.c \
  src/ports/linux_gtk/gtk_platform.c \
  src/ports/linux_gtk/gtk_ui.c

PORT_OBJS := $(PORT_SRCS:%.c=$(BUILD)/%.o)

$(BUILD)/$(TARGET)/chess_$(TARGET): $(CORE_OBJS) $(PLAT_OBJS) $(APP_OBJS) $(PORT_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
