# Makefile for re-dfa
#

DFA_TARGET = re-dfa
DFA_INCLUDES = -I$(DFA_DIR)/$(INCLUDE_DIR) -I$(DFA_DIR)
DFA_FLAGS = -O3 -lpthread 
DFA_CFLAGS = $(CFLAGS) $(DFA_INCLUDES) $(DFA_FLAGS)
DFA_SOURCES = $(wildcard $(DFA_DIR)/$(SRC_DIR)/*.cpp)
DFA_OBJECTS = $(patsubst %,$(BUILD_DIR)/$(DFA_DIR)/%.o, $(subst $(DFA_DIR)/$(SRC_DIR)/,,$(subst .cpp,,$(DFA_SOURCES))))

re-dfa: directories
re-dfa: dfa_directories
re-dfa: $(BIN_DIR)/$(DFA_TARGET)

dfa_directories: 
	@mkdir -p $(BUILD_DIR)/$(DFA_DIR)
	@mkdir -p $(BUILD_DIR)/$(DFA_DIR)/$(TESTS_DIR)

$(BUILD_DIR)/$(DFA_DIR)/%.o: $(DFA_DIR)/$(SRC_DIR)/%.cpp
	$(CC) -Wall $(DFA_CFLAGS) -c $< -o $@

$(BIN_DIR)/$(DFA_TARGET): $(DFA_OBJECTS)
	$(CC) $(DFA_OBJECTS) -Wall $(LIBS) -o $@
