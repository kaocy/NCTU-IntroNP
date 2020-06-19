CPP = g++
CFLAGS = -g -Wall -pthread
TARGET = client

all: $(TARGET)

client: client.cpp
	$(CPP) $(CFLAGS) -o client client.cpp

clean: 
	rm -f $(TARGET)