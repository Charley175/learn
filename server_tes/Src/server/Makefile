CC = gcc
CXX = g++

C_OBJ = $(patsubst %.c,%.o,$(wildcard ./*.c))
CPP_OBJ = $(patsubst %.cpp,%.o,$(wildcard ./*.cpp))
OBJ = $(C_OBJ) $(CPP_OBJ)
INCLUDE = -I ../../ThreadPool/inc -I ../../Common/inc -I ./
LIB = -lpthread
LIB_SO = ./ThreadPool/libthread_pool.so
#CFLAGS=-Werror -g -fPIC -DDEBUG_LOG
CFLAGS = -Werror -g

target = server

$(target):$(OBJ) 
	$(CC) -L $(LIB_SO) $^ -o $@ $(LIB)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ $(INCLUDE)

PHONY +=clean
clean:
	rm -rf $(target)
	rm -rf $(OBJ)