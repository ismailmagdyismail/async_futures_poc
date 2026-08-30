ROOT_PATH := ../
include $(ROOT_PATH)/Common.mk

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

TARGET := async_futures.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(GCC) $(OBJS) $(LIBS) -o $(TARGET)

%.o: %.cpp
	$(GCC) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf *.o *.d $(TARGET)

-include $(DEPS)