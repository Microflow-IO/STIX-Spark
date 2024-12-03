CC = gcc
SRC = cJSON key main socket stix util
INC = include -I/usr/local/modsecurity/include
LIB = -Llib -L/usr/lib64/ -L/usr/local/modsecurity/lib -ldl -lpthread -lm -lz -lpcre
OBJS = $(addsuffix .o, $(SRC))
TARGET = unistix

all: $(OBJS)
	$(CC) -g -o $(addprefix bin/, $(TARGET)) $(addprefix obj/, $(OBJS)) $(LIB)
origin: $(OBJS)
	$(CC) -g -o $(addprefix bin/, $(TARGET)) $(addprefix obj/, $(OBJS)) $(LIB)
%.o: $(addprefix src/, %.c)
	$(CC) -g -c -Wall $< -I$(INC) -o $(addprefix obj/, $@)
clean:
	rm -rf obj/* bin/* lib/*
install: all
	rm -rf /usr/local/bin/$(TARGET)
	cp bin/$(TARGET) /usr/local/bin/
