ROOT_PATH := ../
include $(ROOT_PATH)/Common.mk

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

TARGET := libasync_futures.so

all: $(TARGET)

$(TARGET): $(OBJS)
	$(GCC) $(OBJS) $(SHARED_LIB_FLAGS) -o $(TARGET)

%.o: %.cpp
	$(GCC) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@ -MP -MMD

clean:
	rm -rf *.o *.d *.exe *.so *.a $(TARGET)

-include $(DEPS)