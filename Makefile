CC=gcc

INST_DIR = /usr/local
INC_DIR  = $(INST_DIR)/include
BIN_DIR  = $(INST_DIR)/bin
BUILD_DIR=$(PWD)/build

CFLAGS   = -I$(INC_DIR)
LFLAGS   = -lstdc++ -L$(INST_DIR)/lib -levent -Wl,-rpath,$(INST_DIR)/lib

all: $(BIN_DIR) $(BUILD_DIR) $(BIN_DIR)/dls

$(BIN_DIR):
	@-mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	@-mkdir -p $(BUILD_DIR)

$(BIN_DIR)/dls: $(BUILD_DIR)/dls.o $(BUILD_DIR)/conn.o
	@echo -e "\E[34mbuild dls"
	@tput sgr0
	@$(CC) $(LFLAGS) -o $@ $?

$(BUILD_DIR)/%.o: %.c
	@echo -e "\E[32m"$?
	@tput sgr0
	@$(CC) $(CFLAGS) -c $? -o $@

test: kill clean all
	@$(BIN_DIR)/dls

kill:
	@-killall -q -s SIGINT dls

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BIN_DIR)/dls
