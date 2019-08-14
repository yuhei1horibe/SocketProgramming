CC     := gcc
INCDIR := include
CFLAGS := -g -O2 $(addprefix -I,$(INCDIR))
PROG   := main.out

OBJDIR := obj
SRCDIR := src
#DEPDIR := dep

SRCS   := network.c main.c
OBJS   := $(addprefix $(OBJDIR)/,$(SRCS:%.c=%.o))
DEPS   := $(addprefix $(OBJDIR)/,$(SRCS:%.c=%.d))

.PHONY: clean

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

all: $(OBJS)
	$(CC) -o $(PROG) $(OBJS)

clean:
	rm $(PROG) $(OBJDIR)/*

-include $(DEPS)

