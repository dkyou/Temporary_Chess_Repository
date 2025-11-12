PORT_SRCS := src/ports/terminal/main_cli.c src/ports/terminal/terminal_platform.c
PORT_OBJS := $(PORT_SRCS:%.c=$(BUILD)/%.o)
LDFLAGS += -lm
$(BUILD)/$(TARGET)/chess_$(TARGET): $(CORE_OBJS) $(PLAT_OBJS) $(APP_OBJS) $(PORT_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
