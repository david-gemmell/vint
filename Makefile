TARGET=vint
SRC_DIR=src
BUILD_DIR=build
TEST_DIR=test
PREFIX=/usr/local/bin

MAIN=main
SOURCES=$(shell find $(SRC_DIR) -maxdepth 1 -name '*.c' ! -name '$(MAIN).c')
OBJECTS=$(subst $(SRC_DIR), $(BUILD_DIR), $(SOURCES:.c=.o))

MAIN_TESTS=$(MAIN)_tests
TEST_SOURCES=$(shell find $(TEST_DIR) -maxdepth 1 -name '*.c' ! -name '$(MAIN)_tests.c')
TEST_OBJECTS=$(subst $(TEST_DIR)/, $(BUILD_DIR)/, $(TEST_SOURCES:.c=.o))

.PHONY: all clean install uninstall test

all: $(TARGET)

test: $(TARGET)_tests
	./$(TARGET)_tests

clean:
	rm -rf $(TARGET) $(TARGET)_tests $(BUILD_DIR)/*.o

install: $(TARGET)
	install $(TARGET) $(PREFIX)

uninstall:
	rm -rf $(PREFIX)/$(TARGET)

$(TARGET)_tests: $(BUILD_DIR) $(OBJECTS) $(TEST_OBJECTS) $(BUILD_DIR)/$(MAIN)_tests.o
	$(CC) $(CFLAGS) -o $(TARGET)_tests $(OBJECTS) $(TEST_OBJECTS) $(BUILD_DIR)/$(MAIN)_tests.o

$(TARGET): $(BUILD_DIR) $(OBJECTS) $(BUILD_DIR)/$(MAIN).o
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(BUILD_DIR)/$(MAIN).o

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o : $(TEST_DIR)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@ -iquote $(SRC_DIR)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@
