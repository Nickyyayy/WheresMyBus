CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Ibackend -MMD -MP

SRC_DIR = backend
BUILD_DIR = build
TARGET = wheresmybus

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
DEPS = $(OBJECTS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TARGET).exe output/*.html output/*.tmp

-include $(DEPS)

.PHONY: all run clean
