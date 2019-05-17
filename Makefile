
LIB=-lstdc++ -lrt -lpthread
CC=g++
FLAGS=-g --std=c++11
OBJS=$(patsubst %.c,%.o,$(wildcard ./src/*.c)) $(patsubst %.cpp,%.o,$(wildcard ./src/*.cpp))  
INC=-I./include/


all: server client

server: ${OBJS} ./program/server.cpp
		$(CC) $^ $(LIB) $(FLAGS) $(INC) -o $@

client: ${OBJS} ./program/client.cpp
		$(CC) $^ $(LIB) $(FLAGS) $(INC) -o $@
		
%.o: %.cpp
		$(CC) $(FLAGS) $(INC) -o $@ -c $<

clean:
		rm -rf $(TARGET)
		rm -f $(OBJS)
