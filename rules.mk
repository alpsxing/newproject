ifneq (,$(findstring x86_64, $(shell uname -a)))
ARCH ?= x86_64
HOSTARCH = x86_64
else
ARCH ?= x86
HOSTARCH = x86
endif
BASE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
MAKEFILE_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
EXTERNAL_DIR := $(BASE_DIR)external
EXTERNAL_LIB_DIR := $(EXTERNAL_DIR)/lib/$(ARCH)

BOOST_DIR := $(EXTERNAL_LIB_DIR)/boost
OPENSSL_DIR := $(EXTERNAL_LIB_DIR)/openssl
SQLITE_DIR := $(EXTERNAL_LIB_DIR)/sqlite
CPP_NETLIB_DIR := $(EXTERNAL_LIB_DIR)/cpp-netlib
TOOLS_DIR = $(BASE_DIR)tools
INSTALL_DIR = $(BASE_DIR)install
ARICRACK_DIR = $(EXTERNAL_LIB_DIR)/aircrack-ng
LIBNL_DIR = $(EXTERNAL_LIB_DIR)/libnl

CFLAGS=
LDFLAGS=
ifeq ($(ARCH), x86_64)
CC=gcc
CXX=g++
NETLIBDIR=x86_64-linux-gnu
ifeq ($(HOSTARCH), x86)
$(error Can not build x86_64 on x86 host))
endif
else
ifeq ($(ARCH), x86)
$(info Building x86 on x86_64 host)
CC=gcc
CXX=g++
ifeq ($(HOSTARCH), x86_64)
CFLAGS += -m32
LDFLAGS += -m32
endif
NETLIBDIR=i386-linux-gnu
else
ifeq ($(ARCH), bbb)
CC=$(TOOLS_DIR)/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_linux/bin/arm-linux-gnueabihf-gcc
CXX=$(TOOLS_DIR)/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_linux/bin/arm-linux-gnueabihf-g++
NETLIBDIR=arm-linux-gnueabihf
else
ifeq ($(ARCH), arm)
CC=arm-linux-gcc
CXX=arm-linux-g++
NETLIBDIR=arm
else
$(error $(ARCH) is not supported yet.)
endif
endif
endif
endif

SOURCE_DIR = $(BASE_DIR)src
CFLAGS += -I$(BOOST_DIR)/include -I/$(OPENSSL_DIR)/include -I/$(ARICRACK_DIR)/include
CFLAGS += -I$(SQLITE_DIR)/include -I$(CPP_NETLIB_DIR)/include
CFLAGS += -I$(LIBNL_DIR)/include
CFLAGS += -fpermissive

LDFLAGS += -L$(CPP_NETLIB_DIR)/lib/$(NETLIBDIR)
LDFLAGS += -L$(BOOST_DIR)/lib
LDFLAGS += -L$(OPENSSL_DIR)/lib -L$(SQLITE_DIR)/lib -L$(ARICRACK_DIR)/lib
LDFLAGS += -L$(LIBNL_DIR)/lib
LDFLAGS += -lcppnetlib-server-parsers -lcppnetlib-uri
LDFLAGS += -lcppnetlib-client-connections -lboost_system -lboost_thread
LDFLAGS += -lboost_chrono -lboost_timer
LDFLAGS += -lboost_program_options -lpthread -lssl -lcrypto -lsqlite3 -lairodump-ng -lnl-3 -lnl-genl-3

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

$(TARGET) : $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

install: $(TARGET)
	rm -fr $(INSTALL_DIR)/*
	mkdir -p $(INSTALL_DIR)/lib/openssl
	mkdir -p $(INSTALL_DIR)/lib/boost
	mkdir -p $(INSTALL_DIR)/lib/sqlite
	mkdir -p $(INSTALL_DIR)/lib/aircrack
	mkdir -p $(INSTALL_DIR)/lib/libnl
	cp -a $(OPENSSL_DIR)/lib/lib* $(INSTALL_DIR)/lib/openssl
	cp -a $(LIBNL_DIR)/lib/lib* $(INSTALL_DIR)/lib/libnl
	cp -a $(BOOST_DIR)/lib/lib* $(INSTALL_DIR)/lib/boost
	cp -a $(SQLITE_DIR)/lib/lib* $(INSTALL_DIR)/lib/sqlite
	cp -a $(ARICRACK_DIR)/lib/lib* $(INSTALL_DIR)/lib/aircrack 
	cp -a $(TARGET) $(INSTALL_DIR)
	cp -a $(BASE_DIR)run.sh $(INSTALL_DIR)

clean :
	@rm -fr $(OBJ_DIR) $(TARGET)

distclean:
	@rm -fr $(OBJ_DIR) $(TARGET)
	@rm -fr $(INSTALL_DIR)/*
endif

$(OBJ_DIR)/%.o : %.cpp $(HEADS)
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(OBJS) : | $(OBJ_DIR)

$(OBJ_DIR) :
	mkdir -p $(OBJ_DIR)
