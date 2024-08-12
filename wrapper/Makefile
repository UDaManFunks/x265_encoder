BASE_DIR = ../
OBJ_DIR = ./build
CFLAGS = -fPIC -O3 -I../include -Wall -Wno-multichar -Wno-unused-variable -std=c++20
CPP = g++

.PHONY: all

HEADERS = host_api.h plugin_api.h
SRCS = host_api.cpp plugin_api.cpp
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

all: prereq $(HEADERS) $(SRCS) $(OBJS)

prereq: 
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CPP) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS)
