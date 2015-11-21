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
INSTALL_DIR = $(BASE_DIR)install

ifeq ($(ARCH), x86_64)
CC=gcc
CXX=g++
NETLIBDIR=x86_64-linux-gnu
else
ifeq ($(ARCH), arm)
CC=arm-linux-gnueabi-gcc
CXX=arm-linux-gnueabi-g++
NETLIBDIR=
else
ifeq ($(ARCH), bbb)
CC=$(TOOLS_DIR)/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_linux/bin/arm-linux-gnueabihf-gcc
CXX=$(TOOLS_DIR)/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_linux/bin/arm-linux-gnueabihf-g++
NETLIBDIR=arm-linux-gnueabihf
else
$(error $(ARCH) is not supported yet.)
endif
endif
endif

SOURCE_DIR = $(BASE_DIR)src
CFLAGS = -I$(BOOST_DIR)/include -I/$(OPENSSL_DIR)/include
CFLAGS += -I$(SQLITE_DIR)/include -I$(CPP_NETLIB_DIR)/include

LDFLAGS = -L$(CPP_NETLIB_DIR)/lib/$(NETLIBDIR)
LDFLAGS += -L$(BOOST_DIR)/lib
LDFLAGS += -L$(OPENSSL_DIR)/lib -L$(SQLITE_DIR)/lib
LDFLAGS += -lcppnetlib-server-parsers -lcppnetlib-uri
LDFLAGS += -lcppnetlib-client-connections -lboost_system -lboost_thread
LDFLAGS += -lboost_chrono -lboost_timer
LDFLAGS += -lboost_program_options -lpthread -lssl -lcrypto -lsqlite3

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
	cp -a $(OPENSSL_DIR)/lib/lib* $(INSTALL_DIR)/lib/openssl
	cp -a $(BOOST_DIR)/lib/lib* $(INSTALL_DIR)/lib/boost
	cp -a $(SQLITE_DIR)/lib/lib* $(INSTALL_DIR)/lib/sqlite
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
