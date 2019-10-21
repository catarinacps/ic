#	-- c-template --
#
#	c-template's project Makefile.
#
#	Utilization example:
#		make <TARGET> ["DEBUG=true"]
#
#	@param TARGET
#		Can be any of the following:
#		all - builds the project (DEFAULT TARGET)
#		clean - cleans up all binaries generated during compilation
#		redo - cleans up and then builds
#		help - shows the utilization example
#
#	@param "DEBUG=true"
#		When present, the build will happen in debug mode.
#
#	@author
#		@hcpsilva - Henrique Silva
#
#	Make's default action is "all" when no parameters are provided.


################################################################################
#	Definitions:

#	- Project's directories:
INC_DIR := include
OBJ_DIR := bin
OUT_DIR := build
SRC_DIR := src
LIB_DIR := lib

DEBUG :=

#	Add defines through this variable in the command line
DEF :=

#	- Compilation flags:
#	Compiler and language version
CC := gcc -std=c17
CXX := g++ -std=c++20
#	If DEBUG is defined, we'll turn on the debug flag and attach address
#	sanitizer on the executables.
DEBUGF := $(if $(DEBUG),-g -fsanitize=address)
CFLAGS :=\
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wshadow \
	-Wunreachable-code
LFLAGS :=\
	-shared \
	-fPIC
FUN := -fopenmp
OPT := $(if $(DEBUG),-O0,-O2 -march=native)
LIB := -L$(LIB_DIR)\
	$(shell pkg-config starpu --libs)
INC := -I$(INC_DIR) -I$(SRC_DIR)\
	$(shell pkg-config starpu --cflags)

################################################################################
#	Files:

#	- Main source files:
#	Presumes that all "main" source files are in the root of SRC_DIR
MAIN := $(notdir $(wildcard $(SRC_DIR)/*.c))

MAIN_CPP := $(notdir $(wildcard $(SRC_DIR)/*.cpp))

#	- Path to all final binaries:
TARGET_EXE := $(patsubst %.c, $(OUT_DIR)/%, $(MAIN))

TARGET_EXE_CPP := $(patsubst %.cpp, $(OUT_DIR)/%, $(MAIN_CPP))

#	- Library files:
LIBS := $(shell basename $(wildcard $(LIB_DIR)/*/))

#	- Path to all final libraries:
TARGET_LIB := $(patsubst %, $(LIB_DIR)/lib%.so, $(LIBS))

#	- Other source files:
SRC := $(filter-out $(MAIN), $(shell find $(SRC_DIR) -name '*.c'))

#	- Objects to be created:
OBJ := $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

################################################################################
#	Rules:

#	- Executables:
$(TARGET_EXE): $(OUT_DIR)/%: $(SRC_DIR)/%.c $(OBJ)
	$(CC) -o $@ $^ $(INC) $(LIB) $(DEBUGF) $(OPT) $(DEF) $(FUN)

$(TARGET_EXE_CPP): $(OUT_DIR)/%: $(SRC_DIR)/%.cpp $(OBJ)
	$(CXX) -o $@ $^ $(INC) $(LIB) $(DEBUGF) $(OPT) $(DEF) $(FUN)

#	- Objects:
$(OBJ_DIR)/%.o:
	$(CC) -c -o $@ $(filter %/$*.c, $(SRC)) $(INC) $(CFLAGS) $(DEBUGF) $(OPT) $(DEF) $(FUN)

#	- Shared Libraries:
$(TARGET_LIB): $(LIB_DIR)/lib%.so:
	$(CC) -o $@ $(wildcard $(LIB_DIR)/$*/*.c) $(LFLAGS) $(FUN) $(INC) $(CFLAGS) $(OPT)

################################################################################
#	Targets:

.DEFAULT_GOAL = all

all: $(TARGET_LIB) $(TARGET_EXE)

clean:
	rm -f $(OBJ_DIR)/*.o $(INC_DIR)/*~ $(TARGET_EXE) $(TARGET_LIB) *~ *.o

redo: clean all

################################################################################
#	Debugging and etc.:

#	Debug of the Make variables
print-%:
	@echo $* = $($*)

.PHONY: all clean redo print-%
