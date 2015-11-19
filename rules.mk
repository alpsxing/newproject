ARCH ?= x86_64
BASE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
MAKEFILE_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
EXTERNAL_DIR := $(BASE_DIR)external
EXTERNAL_LIB_DIR := $(EXTERNAL_DIR)/lib/$(ARCH)
BOOST_DIR := $(EXTERNAL_LIB_DIR)/boost
OPENSSL_DIR := $(EXTERNAL_LIB_DIR)/openssl
SQLITE_DIR := $(EXTERNAL_LIB_DIR)/sqlite
CPP_NETLIB_DIR := $(EXTERNAL_LIB_DIR)/cpp-netlib
TOOLS_DIR = $(BASE_DIR)tools

ifeq ($(ARCH), x86_64)
CC=gcc
else
ifeq ($(ARCH), arm)
CC=arm-linux-gnueabi-gcc
else
ifeq ($(ARCH), bbb)
CC=$(TOOLS_DIR)/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_linux/bin/arm-linux-gnueabihf-gcc
else
$(error $(ARCH) is not supported yet.)
endif
endif
endif

SOURCE_DIR = $(BASE_DIR)src
CFLAGS = -I$(BOOST_DIR)/include -I/$(OPENSSL_DIR)/include
CFLAGS += -I$(SQLITE_DIR)/include -I$(CPP_NETLIB_DIR)/include

SRCS = $(SRCS_CXX)
OBJ_DIR = obj/$(ARCH)
OBJS := $(SRCS_CXX:.cpp=.o)
OBJS := $(addprefix $(OBJ_DIR)/, $(OBJS))

ifeq ($(TARGET),)
all : $(OBJS)

clean :
	@rm -fr $(OBJ_DIR)
else
all : $(TARGET)

$(TARGET) :
	@echo "OBJS=$(OBJS)"
endif

$(OBJ_DIR)/%.o : %.cpp %.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(OBJS) : | $(OBJ_DIR)

$(OBJ_DIR) :
	mkdir -p $(OBJ_DIR)

