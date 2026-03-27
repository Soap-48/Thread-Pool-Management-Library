# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -O2

# Target library name
TARGET = libthreadpool.a

# Source files and object files
SRCS = thread_pool.cpp worker.cpp rng.cpp
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Rule to create the static library
$(TARGET): $(OBJS)
	ar rcs $@ $^

# Rule to compile source files into object files
%.o: %.cpp task.h thread_pool.h worker.h rng.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean