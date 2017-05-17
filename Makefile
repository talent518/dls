CC=gcc

INST_DIR = /usr/local
INC_DIR  = $(INST_DIR)/include
BIN_DIR  = $(INST_DIR)/bin
BUILD_DIR=$(PWD)/build

M_CFLAGS   = -I$(INC_DIR) `pkg-config --cflags glib-2.0` -g -O3 $(CFLAGS)
M_LFLAGS   = -lstdc++ -L$(INST_DIR)/lib -levent -Wl,-rpath,$(INST_DIR)/lib `pkg-config --libs glib-2.0` $(LFLAGS)

all: $(BIN_DIR) $(BUILD_DIR) $(BIN_DIR)/dls2

$(BIN_DIR):
	@-mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	@-mkdir -p $(BUILD_DIR)

$(BIN_DIR)/dls2: $(BUILD_DIR)/dls.o $(BUILD_DIR)/conn.o
	@echo -e "\E[34mbuild dls"
	@tput sgr0
	@$(CC) $(M_LFLAGS) -o $@ $?

$(BUILD_DIR)/%.o: %.c
	@echo -e "\E[32m"$?
	@tput sgr0
	@$(CC) $(M_CFLAGS) -c $? -o $@

test: kill clean all
	@echo -e "\E[34mbuild $@"
	@tput sgr0
	@$(BIN_DIR)/dls2

kill:
	@echo -e "\E[34mbuild $@"
	@tput sgr0
	@-killall -q -s SIGINT dls2

clean:
	@echo -e "\E[34mbuild $@"
	@tput sgr0
	@rm -rf $(BUILD_DIR)
	@rm -f $(BIN_DIR)/dls2

