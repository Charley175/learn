CC = gcc
CXX = g++

C_OBJ = $(patsubst %.c,%.o,$(wildcard ./*.c))
CPP_OBJ = $(patsubst %.cpp,%.o,$(wildcard ./*.cpp))
OBJ = $(C_OBJ) $(CPP_OBJ)
INCLUDE = -I ../../ThreadPool/inc -I ../../Common/inc -I ./
LIB_SO = -lpthread -L../../ThreadPool/ -lthread_pool 

#CFLAGS=-Werror -g -fPIC -DDEBUG_LOG
CFLAGS = -g -ggdb -O0 -Wall -Werror 

target = server

$(target):$(OBJ) 
	$(CC) $^ -o $@ $(LIB_SO)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ $(INCLUDE)

PHONY +=clean
clean:
	rm -rf $(target)
	rm -rf $(OBJ)