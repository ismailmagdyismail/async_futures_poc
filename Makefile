ROOT_PATH := .
include $(ROOT_PATH)/Common.mk

all:
	$(MAKE) -C $(SRC_PATH) -f Makefile


clean:
	$(MAKE) -C $(SRC_PATH) -f Makefile clean

