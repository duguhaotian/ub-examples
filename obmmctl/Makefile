# OBMM Control Tool Makefile
# Supports two build modes:
#   - make        : Production with real libobmm (requires -DUSE_REAL_LIBOBMM)
#   - make test   : Unit tests with stubs

CC = gcc
CFLAGS_COMMON = -Wall -Wextra -Werror -std=c11 -I./include
CFLAGS_PROD = $(CFLAGS_COMMON) -DUSE_REAL_LIBOBMM
CFLAGS_TEST = $(CFLAGS_COMMON)

# Directories
SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
STUB_DIR = stubs
BUILD_DIR = build

# Source files (excluding obmm_wrapper which is deleted)
MAIN_SRCS = $(SRC_DIR)/main.c \
            $(SRC_DIR)/cli_parser.c \
            $(SRC_DIR)/log.c \
            $(SRC_DIR)/sysfs_reader.c \
            $(SRC_DIR)/libobmm_wrap.c \
            $(SRC_DIR)/cmd_lend.c \
            $(SRC_DIR)/cmd_borrow.c \
            $(SRC_DIR)/cmd_owner.c \
            $(SRC_DIR)/cmd_status.c \
            $(SRC_DIR)/cmd_help.c

# Source files for test build (excludes main.c)
LIB_SRCS = $(SRC_DIR)/cli_parser.c \
           $(SRC_DIR)/log.c \
           $(SRC_DIR)/sysfs_reader.c \
           $(SRC_DIR)/libobmm_wrap.c \
           $(SRC_DIR)/cmd_lend.c \
           $(SRC_DIR)/cmd_borrow.c \
           $(SRC_DIR)/cmd_owner.c \
           $(SRC_DIR)/cmd_status.c \
           $(SRC_DIR)/cmd_help.c

STUB_SRCS = $(STUB_DIR)/libobmm_stubs.c

TEST_SRCS = $(TEST_DIR)/test_runner.c \
            $(TEST_DIR)/test_cli_parser.c \
            $(TEST_DIR)/test_sysfs_reader.c \
            $(TEST_DIR)/test_libobmm_wrap.c

# Object files
MAIN_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(MAIN_SRCS)))
LIB_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(LIB_SRCS)))
STUB_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(STUB_SRCS)))
TEST_OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(TEST_SRCS)))

# Targets
TARGET = obmmctl
TEST_TARGET = obmmctl_test

VPATH = $(SRC_DIR):$(STUB_DIR):$(TEST_DIR)

.PHONY: all clean test directories prod

# Default: production build (requires libobmm)
all: directories $(TARGET)

prod: all

directories:
	@mkdir -p $(BUILD_DIR)

# Production build with real libobmm
$(TARGET): $(MAIN_OBJS)
	$(CC) $(CFLAGS_PROD) -o $@ $^ -lobmm

# Test build with stubs (excludes main.o)
$(TEST_TARGET): $(TEST_OBJS) $(STUB_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS_TEST) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS_TEST) -c $< -o $@

test: directories $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)

# Dependencies
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/obmmctl.h $(INC_DIR)/cli_parser.h $(INC_DIR)/log.h
$(BUILD_DIR)/cli_parser.o: $(SRC_DIR)/cli_parser.c $(INC_DIR)/cli_parser.h $(INC_DIR)/obmmctl.h
$(BUILD_DIR)/log.o: $(SRC_DIR)/log.c $(INC_DIR)/log.h
$(BUILD_DIR)/sysfs_reader.o: $(SRC_DIR)/sysfs_reader.c $(INC_DIR)/sysfs_reader.h $(INC_DIR)/log.h
$(BUILD_DIR)/libobmm_wrap.o: $(SRC_DIR)/libobmm_wrap.c $(INC_DIR)/libobmm_wrap.h $(INC_DIR)/sysfs_reader.h $(INC_DIR)/obmm_stubs.h $(INC_DIR)/log.h
$(BUILD_DIR)/cmd_lend.o: $(SRC_DIR)/cmd_lend.c $(INC_DIR)/obmmctl.h $(INC_DIR)/cli_parser.h $(INC_DIR)/sysfs_reader.h $(INC_DIR)/libobmm_wrap.h $(INC_DIR)/log.h
$(BUILD_DIR)/cmd_borrow.o: $(SRC_DIR)/cmd_borrow.c $(INC_DIR)/obmmctl.h $(INC_DIR)/cli_parser.h $(INC_DIR)/sysfs_reader.h $(INC_DIR)/libobmm_wrap.h $(INC_DIR)/log.h
$(BUILD_DIR)/cmd_owner.o: $(SRC_DIR)/cmd_owner.c $(INC_DIR)/obmmctl.h $(INC_DIR)/cli_parser.h $(INC_DIR)/libobmm_wrap.h $(INC_DIR)/log.h
$(BUILD_DIR)/cmd_status.o: $(SRC_DIR)/cmd_status.c $(INC_DIR)/obmmctl.h $(INC_DIR)/cli_parser.h $(INC_DIR)/libobmm_wrap.h $(INC_DIR)/log.h
$(BUILD_DIR)/cmd_help.o: $(SRC_DIR)/cmd_help.c $(INC_DIR)/obmmctl.h $(INC_DIR)/cli_parser.h
$(BUILD_DIR)/libobmm_stubs.o: $(STUB_DIR)/libobmm_stubs.c $(INC_DIR)/obmm_stubs.h