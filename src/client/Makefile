CC = gcc
CXX = g++

C_OBJ = $(patsubst %.c,%.o,$(wildcard ./*.c))
CPP_OBJ = $(patsubst %.cpp,%.o,$(wildcard ./*.cpp))
OBJ = $(C_OBJ) $(CPP_OBJ)
INCLUDE = -I ./ThreadPool/inc -I ./Common/inc -I ./
LIB = -lefence -lpthread
#CFLAGS=-Werror -g -fPIC -DDEBUG_LOG
CFLAGS = -Werror -g -ggdb -O0
target = client

$(target):$(OBJ) 
	$(CC)  $^ -o $@ $(LIB)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

%.o:%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ $(INCLUDE)

PHONY +=clean
clean:
	rm -rf $(target)
	rm -rf $(OBJ)