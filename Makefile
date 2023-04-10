SRC_DIR := src
EXT_DIR := extern
OBJ_DIR := obj
BIN_DIR := bin
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
LDFLAGS := -pthread
CPPFLAGS := -g 
CXXFLAGS := -I$(SRC_DIR) -I$(EXT_DIR)
TARGET	:= serverdemon

all: build

build: $(OBJ_FILES)
	g++ $(LDFLAGS) -o $(BIN_DIR)/$(TARGET) $^

run: build
	cd $(BIN_DIR)
	./$(BIN_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean: 
	rm $(OBJ_FILES)
	rm $(BIN_DIR)/$(TARGET)