CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
SRC_DIR = src
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%)`

all: bin bin/server bin/client

bin:
	mkdir -p bin

bin/server: src/server.cpp
	g++ -std=c++17 -O2 -Wall -o bin/server src/server.cpp

bin/client: src/client.cpp
	g++ -std=c++17 -O2 -Wall -o bin/client src/client.cpp

clean:
	rm -rf bin

.PHONY: all clean
