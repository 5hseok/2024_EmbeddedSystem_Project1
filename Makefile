# Makefile

# For the kernel module
obj-m += dummy_driver.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# For the user space application
CC := gcc
CFLAGS := -Wall
TARGET_APP := application
SRC_APP := application.c

# Default target
all: dummy_driver application

dummy_driver:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

application: $(SRC_APP)
	$(CC) $(CFLAGS) -o $(TARGET_APP) $(SRC_APP)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(TARGET_APP)
