.PHONY: builddir library binary tests ctags
PRODUCT=gbaudio
BUILDDIR=$(PWD)/build
OUTPUTDIR=$(BUILDDIR)/output

BIN_DIR=bin
SRC_DIR=src
TEST_DIR=tests

INC_PATH=$(PWD)/inc
AR=ar
CC=gcc
SDL_CFLAGS=`sdl2-config --cflags`
CFLAGS=-std=c11 -g -Wall -Werror -I$(INC_PATH) -I. $(SDL_CFLAGS)
LFLAGS=-lSDL2

# Library

DEPS=\
	$(wildcard $(INC_PATH)/$(PRODUCT)/*.h) \
	$(wildcard $(SRC_DIR)/*.h) \
	$(NULL)

SRC=\
	$(notdir $(wildcard $(SRC_DIR)/*.c)) \
	$(NULL)

OBJ=\
	$(addprefix $(BUILDDIR)/$(SRC_DIR)/,$(SRC:.c=.o)) \
	$(NULL)

LIBRARY=$(OUTPUTDIR)/lib$(PRODUCT).a

# Binary

BIN_LFLAGS=

BIN_DEPS=\
	$(wildcard $(INC_PATH)/$(PRODUCT)/*.h) \
	$(NULL)

BIN_SRC=\
	$(notdir $(wildcard $(BIN_DIR)/*.c)) \
	$(NULL)

BIN_OBJ=\
	$(addprefix $(BUILDDIR)/$(BIN_DIR)/,$(BIN_SRC:.c=.o)) \
	$(NULL)

BIN_OUTPUTDIR=$(OUTPUTDIR)/bin

BIN_OUTPUT=\
	$(addprefix $(BIN_OUTPUTDIR)/,$(basename $(BIN_SRC))) \
	$(NULL)

# Tests

TEST_INC=$(PWD)/deps/tinyctest/inc

TEST_DEPS=\
	$(wildcard $(INC_PATH)/$(PRODUCT)/*.h) \
	$(wildcard $(SRC_DIR)/*.c) \
	$(wildcard $(SRC_DIR)/*.h) \
	$(NULL)

TEST_SRC=\
	$(notdir $(wildcard $(TEST_DIR)/*.c)) \
	$(NULL)

TEST_OBJ=\
	$(addprefix $(BUILDDIR)/$(TEST_DIR)/,$(TEST_SRC:.c=.o)) \
	$(NULL)

TEST_OUTPUT=$(OUTPUTDIR)/tests

# Build rules

all : builddir library binary tests ctags

builddir :
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/$(SRC_DIR)
	mkdir -p $(BUILDDIR)/$(BIN_DIR)
	mkdir -p $(BUILDDIR)/$(TEST_DIR)
	mkdir -p $(OUTPUTDIR)
	mkdir -p $(BIN_OUTPUTDIR)

$(BUILDDIR)/$(SRC_DIR)/%.o : $(SRC_DIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILDDIR)/$(BIN_DIR)/%.o : $(BIN_DIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILDDIR)/$(TEST_DIR)/%.o : $(TEST_DIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I$(SRC_DIR) -I$(TEST_INC)

# Library build

library : builddir $(LIBRARY)

$(LIBRARY) : $(OBJ)
	rm -f $(LIBRARY)
	$(AR) rcs $(LIBRARY) $^

# Binary build

binary : library $(BIN_OUTPUT)

$(BIN_OUTPUTDIR)/% : $(BUILDDIR)/$(BIN_DIR)/%.o $(LIBRARY)
	$(CC) -o $@ $< $(LIBRARY) $(CFLAGS) $(LFLAGS) $(BIN_LFLAGS)

# Test build

tests : library $(TEST_OUTPUT)

test : tests
	$(TEST_OUTPUT)

$(TEST_OUTPUT) : $(TEST_OBJ) $(LIBRARY)
	$(CC) -o $(TEST_OUTPUT) $^ $(LIBRARY) $(CFLAGS) $(LFLAGS)

# Ancillary rules

clean :
	rm -rf $(BUILDDIR)

ctags :
	/usr/local/bin/ctags -R --c-kinds=+p --fields=+S

# Layout directory structure based on the product name
setup :
	mkdir -p $(PWD)/bin
	mkdir -p $(PWD)/deps
	mkdir -p $(INC_PATH)/$(PRODUCT)
	mkdir -p $(PWD)/src
	mkdir -p $(PWD)/tests
