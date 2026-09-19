CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2

TARGET := mini-cc

.PHONY: all clean test examples

all: $(TARGET)

$(TARGET): src/main.c
	$(CC) $(CFLAGS) $< -o $@

test: $(TARGET)
	bash tests/test.sh

examples: $(TARGET)
	@mkdir -p build
	./$(TARGET) < examples/control_flow.c > build/control_flow.s
	$(CC) build/control_flow.s -o build/control_flow
	./build/control_flow; printf 'exit code: %s\n' "$$?"

clean:
	rm -f $(TARGET)
	rm -rf build