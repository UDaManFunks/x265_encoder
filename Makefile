OS_TYPE := $(shell uname -s)
BASE_DIR = ./
OBJ_DIR = ./build
BUILD_DIR = ./bin
WRAPPER_DIR = ./wrapper
X265_DIR = ../x265_pkg
CFLAGS = -O3 -fPIC -Iinclude -Iwrapper -I$(X265_DIR)/include -I$(X265_DIR)/build/linux -Wall -Wno-multichar -Wno-unused-variable -std=c++20
HEADERS = plugin.h x265_encoder.h
SRCS = plugin.cpp x265_encoder.cpp 
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
CPP = g++

ifeq ($(OS_TYPE), Linux)
LDFLAGS = -fPIC -shared -lpthread -Wl,-Bsymbolic -Wl,--no-undefined -static-libstdc++ -static-libgcc -std=c++20 -lstdc++
else
LDFLAGS = -dynamiclib
endif

TARGET = $(BUILD_DIR)/x265_encoder.dvcp
LDFLAGS += -L$(X265_DIR)/lib -lx265 

.PHONY: all

all: prereq make-subdirs $(HEADERS) $(SRCS) $(OBJS) $(TARGET)

prereq:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CPP) -c -o $@ $< $(CFLAGS)

$(TARGET):
	$(CPP) $(WRAPPER_DIR)/build/*.o $(OBJ_DIR)/*.o $(LDFLAGS) -o $(TARGET)

clean: clean-subdirs
	rm -rf $(OBJ_DIR)
	rm -rf $(BUILD_DIR)

make-subdirs:
	(cd wrapper; make; cd ..)

clean-subdirs:
	(cd wrapper; make clean; cd ..)
