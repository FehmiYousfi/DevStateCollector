CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 `pkg-config --cflags dbus-1 dbus-glib-1 glib-2.0 glibmm-2.4` -rdynamic -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function
LDFLAGS = `pkg-config --libs dbus-1 dbus-glib-1 glibmm-2.4`

# Find all .cpp files in the directory
SOURCES = $(wildcard *.cpp)

# Generate a list of executable names by removing .cpp extension from each source file
EXECUTABLES = $(SOURCES:.cpp=)

# Build targets
all: $(EXECUTABLES)

# Rule to compile each .cpp file into an executable
%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Clean target to remove the executables and object files
clean:
	rm -f $(EXECUTABLES) $(wildcard *.o)
