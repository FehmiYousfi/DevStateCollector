# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11

# Linker flags
LDFLAGS = -ludev -lubus -lubox

# Source files (all .cpp files in the directory)
SRC_FILES = $(wildcard *.cpp)

# Object files (one .o file per source file)
OBJ_FILES = $(SRC_FILES:.cpp=.o)

# Binaries (one binary per source file)
BINARIES = $(SRC_FILES:.cpp=)

# Default rule to build all binaries
all: $(BINARIES)

# Rule to build each binary from its corresponding .cpp file
$(BINARIES): % : %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Clean up object files and binaries
clean:
	rm -f $(OBJ_FILES) $(BINARIES)

