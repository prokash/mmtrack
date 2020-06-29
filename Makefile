# Makefile template for a shared library in C
# # https://www.topbug.net/blog/2019/10/28/makefile-template-for-a-shared-library-in-c-with-explanations/
#
CC = gcc  # C compiler
CFLAGS = -fPIC -Wall -Wextra -g -fno-inline-small-functions  # C flags
##CFLAGS = -fPIC -Wall -Wextra -O2  # C flags
LDFLAGS = -shared   # linking flags
RM = rm -f   # rm command
TARGET_LIB = libmemstats.so  # target lib
#
SRCS = memapihook.c hashsep.c  # source files
#SRCS = memapihook2.c mmstat.c # source files
OBJS = $(SRCS:.c=.o)
#
.PHONY: all
all: ${TARGET_LIB}
#
$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)
