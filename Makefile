CC ?= cc
CXX ?= c++
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic -Werror -std=c11
CXXFLAGS ?= -O2 -Wall -Wextra -Wpedantic -Werror -std=c++17
LDFLAGS ?=
CPPFLAGS := -Icore/include
BUILD := build
TARGET := $(BUILD)/dupliguard

.PHONY: all test sanitize java run-gui clean
all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/sha256.o: core/src/sha256.c core/include/dupliguard/sha256.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/scanner.o: cli/src/scanner.cpp cli/include/scanner.hpp core/include/dupliguard/sha256.h | $(BUILD)
	$(CXX) $(CPPFLAGS) -Icli/include $(CXXFLAGS) -c $< -o $@

$(BUILD)/main.o: cli/src/main.cpp cli/include/scanner.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) -Icli/include $(CXXFLAGS) -c $< -o $@

$(TARGET): $(BUILD)/sha256.o $(BUILD)/scanner.o $(BUILD)/main.o
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD)/test_sha256: tests/test_sha256.c $(BUILD)/sha256.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ -o $@

test: all $(BUILD)/test_sha256
	./$(BUILD)/test_sha256
	bash tests/test_cli.sh ./$(TARGET)

sanitize: CFLAGS := -O1 -g -Wall -Wextra -Wpedantic -Werror -std=c11 -fsanitize=address,undefined
sanitize: CXXFLAGS := -O1 -g -Wall -Wextra -Wpedantic -Werror -std=c++17 -fsanitize=address,undefined
sanitize: LDFLAGS := -fsanitize=address,undefined
sanitize: clean test

java: | $(BUILD)
	javac -d $(BUILD)/java desktop/src/main/java/io/dupliguard/DesktopApp.java

run-gui: all java
	java -cp $(BUILD)/java io.dupliguard.DesktopApp ./$(TARGET)

clean:
	rm -rf $(BUILD)
