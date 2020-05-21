CPP = g++
CFLAGS = -g -Wall
TARGET = server client

all: $(TARGET)

server: server.cpp
	$(CPP) $(CFLAGS) -o server server.cpp

client: client.cpp
	$(CPP) $(CFLAGS) -o client client.cpp

clean: 
	rm -f $(TARGET)