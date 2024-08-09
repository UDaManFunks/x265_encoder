OS_TYPE := $(shell uname -s)
BASE_DIR = ./
OBJ_DIR = ./build
BUILD_DIR = ./bin
WRAPPER_DIR = ./wrapper
X265_DIR = ../x265
CFLAGS = -O3 -fPIC -Iinclude -Iwrapper -I$(X265_DIR)/source -I$(X265_DIR)/build/linux
HEADERS = plugin.h x265_encoder.h
SRCS = plugin.cpp x265_encoder.cpp 
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)

ifeq ($(OS_TYPE), Linux)
LDFLAGS = -fPIC -shared -lpthread -Wl,-Bsymbolic -L$(X265_DIR)/build/linux
else
LDFLAGS = -dynamiclib
endif

TARGET = $(BUILD_DIR)/x265_encoder.dvcp
LDFLAGS += -lx265

.PHONY: all

all: prereq make-subdirs $(HEADERS) $(SRCS) $(OBJS) $(TARGET)

prereq:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET):
	$(CC) $(WRAPPER_DIR)/build/*.o $(OBJ_DIR)/*.o $(LDFLAGS) -o $(TARGET)

clean: clean-subdirs
	rm -rf $(OBJ_DIR)
	rm -rf $(BUILD_DIR)

make-subdirs:
	(cd wrapper; make; cd ..)

clean-subdirs:
	(cd wrapper; make clean; cd ..)
