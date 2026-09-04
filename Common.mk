GCC := g++
CXX_FLAGS := -std=c++17 -g -O0 -Wall -Wextra
SHARED_LIB_FLAGS := -shared -fpic -Wl,-undefined,dynamic_lookup

SRC_PATH := $(ROOT_PATH)/src
LIB_ROOT_PATH := $(ROOT_PATH)/src
LIBS := -L$(LIB_ROOT_PATH) -lasync_futures

INCLUDES := -I$(SRC_PATH)