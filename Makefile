# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall 

# Directories
ENV_DIR = env

# Source files
ENV_SOURCES = $(wildcard $(ENV_DIR)/*.cpp)
SHELL_SOURCE = shell.cpp

# Object files
ENV_OBJECTS = $(patsubst $(ENV_DIR)/%.cpp,$(ENV_DIR)/%.o,$(ENV_SOURCES))
SHELL_OBJECT = shell.o

# Executable name
SHELL_TARGET = shell

# All target
all: $(SHELL_TARGET)

# Rule to build shell executable
$(SHELL_TARGET): $(SHELL_OBJECT) $(ENV_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -lreadline -o $@

# Rule to compile shell object file
$(SHELL_OBJECT): $(SHELL_SOURCE)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile environment object files
$(ENV_DIR)/%.o: $(ENV_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(ENV_OBJECTS) $(SHELL_OBJECT) $(SHELL_TARGET)

run: 
	./build.sh

# Phony targets
.PHONY: all clean run
