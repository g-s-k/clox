BIN_DIR = ./bin
BUILD_DIR = ./build
SRC_DIR = ./src

EPHEMERAL = $(BIN_DIR) $(BUILD_DIR)

SRCS = $(wildcard $(SRC_DIR)/*.c)
BASE = $(notdir $(SRCS:.c=.o))
OBJS = $(addprefix $(BUILD_DIR)/, $(BASE))

CFLAGS = -Wall

$(BIN_DIR)/clox: $(OBJS) $(BIN_DIR)
	$(CC) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(EPHEMERAL):
	mkdir -p $@

.PHONY: clean

clean:
	$(RM) -r $(EPHEMERAL)
