# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20
LDLIBS = -lreadline

# Directories
SERVER_DIR = ./server
CLIENT_DIR = ./client
BIN_DIR = ./bin
BIN_SERVER = $(BIN_DIR)/server
BIN_CLIENT = $(BIN_DIR)/client

# Source files
SERVER_SRC = $(SERVER_DIR)/main.cpp
CLIENT_SRC = $(CLIENT_DIR)/client.cpp $(CLIENT_DIR)/main.cpp

# Targets
SERVER_TARGET = $(BIN_SERVER)/server
CLIENT_TARGET = $(BIN_CLIENT)/client

# Default target
all: $(BIN_SERVER) $(BIN_CLIENT) $(SERVER_TARGET) $(CLIENT_TARGET)

# Ensure server and client bin directories exist
$(BIN_SERVER):
	mkdir -p $(BIN_SERVER)

$(BIN_CLIENT):
	mkdir -p $(BIN_CLIENT)

# Build the server
$(SERVER_TARGET): $(SERVER_SRC) | $(BIN_SERVER)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the client
$(CLIENT_TARGET): $(CLIENT_SRC) | $(BIN_CLIENT)
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

# Clean up build artifacts
clean:
	rm -rf $(BIN_DIR)
