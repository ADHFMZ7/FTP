# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20
LDLIBS = -lreadline

# Directories
SERVER_DIR = ./server
CLIENT_DIR = ./client
BIN_DIR = ./bin

# Source files
SERVER_SRC = $(SERVER_DIR)/main.cpp
CLIENT_SRC = $(CLIENT_DIR)/client.cpp $(CLIENT_DIR)/main.cpp

# Targets
SERVER_TARGET = $(BIN_DIR)/server
CLIENT_TARGET = $(BIN_DIR)/client

# Default target
all: $(BIN_DIR) $(SERVER_TARGET) $(CLIENT_TARGET)

# Ensure the bin directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build the server
$(SERVER_TARGET): $(SERVER_SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the client
$(CLIENT_TARGET): $(CLIENT_SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

# Clean up build artifacts
clean:
	rm -rf $(BIN_DIR)

