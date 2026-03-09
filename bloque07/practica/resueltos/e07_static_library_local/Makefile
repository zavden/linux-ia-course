CC       = gcc
AR       = ar
CFLAGS   = -Wall -Wextra -Werror -pedantic -std=c17
LDFLAGS  =
LDLIBS   =

ifdef DEBUG
  CFLAGS += -g -O0 -fsanitize=address,undefined
  LDFLAGS += -fsanitize=address,undefined
else
  CFLAGS += -O2
endif

SRC_DIR   = src
BUILD_DIR = build
TARGET    = $(BUILD_DIR)/main
LIB       = $(BUILD_DIR)/libtextstats.a

OBJS_LIB  = $(BUILD_DIR)/text_stats.o
OBJS_MAIN = $(BUILD_DIR)/main.o

.PHONY: all clean run test debug

all: $(TARGET)

$(TARGET): $(OBJS_MAIN) $(LIB) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $(OBJS_MAIN) $(LIB) $(LDLIBS)

$(LIB): $(OBJS_LIB) | $(BUILD_DIR)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

debug:
	$(MAKE) DEBUG=1 all

test: $(TARGET)
	@echo "=== Ejecutando tests ==="
	@if [ -d tests ]; then \
		for t in tests/*.sh; do \
			echo "--- $$t ---"; \
			bash "$$t" || exit 1; \
		done; \
		echo "=== Todos los tests pasaron ==="; \
	else \
		echo "No hay directorio tests/"; \
	fi
