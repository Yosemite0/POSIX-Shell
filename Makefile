CXX = g++
CXXFLAGS = -std=c++17 -Wall
SOURCES = shell.cpp env/ShellEnv.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = shell

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
