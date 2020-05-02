TARGET_EXEC ?= brill_tagger
CC=mpicc
CFLAGS=-g -O2 --std=c99 -Wall
BUILD_DIR ?= ./build
SRC_DIRS ?= ./src
LDFLAGS=-g -O2 -lpthread
SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ 

	rsync -rup ./files/* $(BUILD_DIR)

.PHONY:
	sync
	clean

sync:
	rsync -rup ./files/* $(BUILD_DIR)

All:
	make
	rsync -rup ./files/* $(BUILD_DIR)

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
