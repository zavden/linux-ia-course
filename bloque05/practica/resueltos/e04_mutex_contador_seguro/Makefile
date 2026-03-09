# ============================================================
# Linux-C Course — Makefile Template
# ============================================================
CC       = gcc
CFLAGS   = -Wall -Wextra -Werror -pedantic -std=c17 -pthread
LDFLAGS  = -pthread
LDLIBS   =

# Debug flags (activar con: make DEBUG=1)
ifdef DEBUG
  CFLAGS += -g -O0 -fsanitize=address,undefined
  LDFLAGS += -fsanitize=address,undefined
else
  CFLAGS += -O2
endif

SRC_DIR  = src
BUILD_DIR = build
SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET   = $(BUILD_DIR)/main

.PHONY: all clean run test debug

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

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
