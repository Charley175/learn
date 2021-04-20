CC = gcc
CXX = g++

OBJ = $(patsubst %.c,%.o,$(wildcard ./src/*.c))
THREAD_INCLUDE = -I ./inc -I ../Common/inc
LIB= -lpthread

#CFLAGS=-Werror -g -fPIC -DDEBUG_LOG
CFLAGS = -Werror -g -fPIC

target = libthread_pool.so

$(target):$(OBJ)
	$(CC) -shared $^ -o $@ $(LIB)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(THREAD_INCLUDE)

PHONY +=clean
clean:
	rm -rf $(target)
	rm -rf $(OBJ)
