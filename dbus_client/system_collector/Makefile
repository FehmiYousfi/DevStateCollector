CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 `pkg-config --cflags dbus-1`  -rdynamic -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = `pkg-config --libs dbus-1 tinyxml2` 
# Find all .cpp files in the directory
SOURCES = $(wildcard *.cpp)

# Generate a list of executable names by removing .cpp extension from each source file
EXECUTABLES = $(SOURCES:.cpp=)

# Build targets
all: $(EXECUTABLES)

# Rule to compile each .cpp file into an executable
%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Clean target to remove the executables
clean:
	rm -f $(EXECUTABLES) $(wildcard *.o)
