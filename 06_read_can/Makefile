# --- config ---
SRC := read_can.c
BIN := read_can.o

CROSS_COMPILE := /home/huy/work_space/ti_sdk_board/external-toolchain-dir/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-
CC := $(CROSS_COMPILE)gcc

CFLAGS  := -Wall -O2
LDFLAGS :=

.PHONY: all clean install

# --- build ---
all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

# copy binary lên board (đổi IP nếu cần)
install: $(BIN)
	scp $(BIN) root@192.168.8.8:/home/root/

# --- clean ---
clean:
	rm -f $(BIN) *.o