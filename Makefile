SRC := ./src

CPPFLAGS = -MMD -MP
CFLAGS = -Wall

vpath %.c $(SRC)

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(SRCS:.c=.o)

clox: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

-include $(OBJS:.o=.d)

.PHONY: clean

clean:
	$(RM) -r clox $(OBJS)
