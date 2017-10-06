CC=gcc

HEADER_DIR=/usr/local/include/RF24
LIB_DIR=/usr/local/lib
LIB=rf24

CFLAGS=-g
CFLAGS += $(shell pkg-config --cflags json-c)
CFLAGS += $(shell mysql_config --cflags)
LDFLAGS += -lmosquitto
LDFLAGS += -lpthread
LDFLAGS += $(shell pkg-config --libs json-c)
LDFLAGS += $(shell mysql_config --libs)

PROGRAMS = rf24_gateway

BINARY_PREFIX = rf24
SOURCES = $(PROGRAMS:=.cpp)

LIBS=-l$(LIB)

all: rf24_gateway

bcm2835.o: bcm2835.c
	$(CC) -fPIC $(CFLAGS) -c $^

rf24_gateway: rf24_gateway.o bcm2835.o
	@echo "[Linking]"
	$(CXX) -L$(LIB_DIR) $(LDFLAGS) $(LIBS) -o $@ $^

rf24_gateway.o: rf24_gateway.cpp
	$(CXX) -fPIC $(CFLAGS) -I$(HEADER_DIR)/.. -c $^

clean:
	@echo "[Cleaning]"
	rm -rf $(PROGRAMS)

install: install-bin

install-bin:
	@echo "[Installing to /opt/rf24_gateway]"
	@install rf24_gateway /opt/rf24_gateway/ 

.PHONY: install upload
