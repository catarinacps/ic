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
#	and extra stuff through here
LIB_EXTRA :=
INC_EXTRA :=

#	- Compilation flags:
#	Compiler and language version
CC := gcc -std=gnu11
CXX := g++ -std=c++17
#	If DEBUG is defined, we'll turn on the debug flag and attach address
#	sanitizer on the executables.
DEBUGF := $(if $(DEBUG),-g -fsanitize=address)
CFLAGS :=\
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wshadow \
	-Wunreachable-code \
	-lpthread
FUN := -fopenmp
OPT := $(if $(DEBUG),-O0,-O2 -march=native)
LIB := -L$(LIB_DIR) $(LIB_EXTRA)\
	$(shell pkg-config starpu-1.3 --libs) \
	-ltbb \
	-lm
INC := -I$(INC_DIR) -I$(SRC_DIR) $(INC_EXTRA)\
	$(shell pkg-config starpu-1.3 --cflags)

################################################################################
#	Files:

#	- Main source files:
#	Presumes that all "main" source files are in the root of SRC_DIR
MAIN := $(wildcard $(SRC_DIR)/*.c)

MAIN_CPP := $(wildcard $(SRC_DIR)/*.cpp)

#	- Path to all final binaries:
TARGET_EXE := $(patsubst %.c, $(OUT_DIR)/%, $(notdir $(MAIN)))

TARGET_EXE_CPP := $(patsubst %.cpp, $(OUT_DIR)/%, $(notdir $(MAIN_CPP)))

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

################################################################################
#	Targets:

.DEFAULT_GOAL = all

all: $(TARGET_EXE) $(TARGET_EXE_CPP)

clean:
	rm -f $(OBJ_DIR)/*.o $(INC_DIR)/*~ $(TARGET_EXE) *~ *.o

redo: clean all

################################################################################
#	Debugging and etc.:

#	Debug of the Make variables
print-%:
	@echo $* = $($*)

.PHONY: all clean redo print-%
