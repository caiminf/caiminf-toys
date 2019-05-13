TARGET=server
LIB=-lstdc++ -lrt -lpthread
LIBA=
FLAGS=-Wall -Werror -g
OBJS=$(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.cpp,%.o,$(wildcard *.cpp))  

all:$(TARGET)

$(TARGET):${OBJS}
		g++ $^ $(LIBA) $(LIB) -g $(FLAGS) -o $@
		
.cpp.o:
		$(CC) $(FLAGS) $(INCLUDE) -g -o $@ -c $<

clean:
		rm -rf $(TARGET)
		rm -f $(OBJS)

install: