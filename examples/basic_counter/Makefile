ROOT_PATH := ../..
include $(ROOT_PATH)/Common.mk

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

TARGET := counter.exe

all: $(TARGET)

$(TARGET): $(OBJS) BUILD_LIB
	$(GCC) $(OBJS) $(LIBS) $(INCLUDES) -o $@c

%.o: %.cpp
	$(GCC) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@ -MMD -MP

BUILD_LIB:
	$(MAKE) -C $(LIB_ROOT_PATH) -f Makefile

clean:
	rm -rf *.o *.d $(TARGET)
	$(MAKE) -C $(LIB_ROOT_PATH) -f Makefile clean

-include $(DEPS)