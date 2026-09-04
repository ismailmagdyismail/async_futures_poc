ROOT_PATH := ../..
include $(ROOT_PATH)/Common.mk

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

SERVER_TARGET := server.exe
CLIENT_TARGET := client.exe
SERVER_MAIN := server
CLIENT_MAIN := client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_MAIN).o BUILD_LIB
	$(GCC) $(SERVER_MAIN).o $(LIBS) -o $@

$(CLIENT_TARGET): $(CLIENT_MAIN).o BUILD_LIB
	$(GCC) $(CLIENT_MAIN).o $(LIBS) -o $@

BUILD_LIB:
	$(MAKE) -C $(LIB_ROOT_PATH) -f Makefile

$(SERVER_MAIN).o: $(SERVER_MAIN).cpp
	$(GCC) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

$(CLIENT_MAIN).o: $(CLIENT_MAIN).cpp
	$(GCC) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf *.o *.d $(TARGET)
	$(MAKE) -C $(LIB_ROOT_PATH) -f Makefile clean

-include $(DEPS)