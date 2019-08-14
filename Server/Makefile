CC     = gcc
CFLAGS = -g -Wall -O2

SRCS   = network.c main.c
OBJS   = $(SRCS:%.c=%.o)
DEPS   = $(SRCS:%.c=%.d)

.PHONY: clean

.c.o:
	$(CC) -MMD -o $@ -c $<

all: $(OBJS)
	$(CC) -o main.out $(OBJS)

clean:
	rm -rf ./*.d ./*.o ./*.out

-include $(DEPS)

