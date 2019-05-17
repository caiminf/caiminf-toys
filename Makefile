
LIB=-lstdc++ -lrt -lpthread
CC=g++
FLAGS=-Wall -Werror -g
OBJS=$(patsubst %.c,%.o,$(wildcard ./src/*.c)) $(patsubst %.cpp,%.o,$(wildcard ./src/*.cpp))  
INC=-I./include/


all: server

server: ${OBJS} ./program/server.cpp
		$(CC) $^ $(LIB) $(FLAGS) -o $@
		
%.o: %.c
		$(CC) $(FLAGS) $(INCLUDE) -o $@ -c $<

clean:
		rm -rf $(TARGET)
		rm -f $(OBJS)

install: