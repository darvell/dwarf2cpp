# Makefile for dwarf2cpp
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Detect the operating system
UNAME_S := $(shell uname -s)

# Platform-specific settings
ifeq ($(UNAME_S),Darwin)
    # macOS
    CXXFLAGS += -DUSE_STD_FILESYSTEM
    LDFLAGS = 
else ifeq ($(UNAME_S),Linux)
    # Linux
    LDFLAGS = -lstdc++fs
else
    # Windows (MinGW)
    LDFLAGS = -lstdc++fs
endif

# Source files
SOURCES = main.cpp cpp.cpp
HEADERS = cpp.h dwarf.h elf.h
EXECUTABLE = dwarf2cpp

# Default target
all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(EXECUTABLE) $(LDFLAGS)

# Clean target
clean:
	rm -f $(EXECUTABLE)

# Install target (optional)
install: $(EXECUTABLE)
	cp $(EXECUTABLE) /usr/local/bin/

.PHONY: all clean install