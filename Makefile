HEADER_DIR=/usr/local/include/RF24
LIB_DIR=/usr/local/lib
LIB=rf24

CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)
LDFLAGS += -lmosquitto

PROGRAMS = rf24_gateway

BINARY_PREFIX = rf24
SOURCES = $(PROGRAMS:=.cpp)

LIBS=-l$(LIB)

all: $(PROGRAMS)

$(PROGRAMS): $(SOURCES)
	$(CXX) $(CFLAGS) -I$(HEADER_DIR)/.. -I.. -L$(LIB_DIR) $(LDFLAGS) $@.cpp $(LIBS) -o $@

clean:
	@echo "[Cleaning]"
	rm -rf $(PROGRAMS)

install: all
	@echo "[Installing examples to $(EXAMPLES_DIR)]"
	@mkdir -p $(EXAMPLES_DIR)
	@for prog in $(PROGRAMS); do \
		install -m 0755 $${prog} $(EXAMPLES_DIR)/$(BINARY_PREFIX)-$${prog}; \
	done

.PHONY: install upload
