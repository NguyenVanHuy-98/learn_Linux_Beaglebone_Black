# Cross compiler prefix
CROSS_COMPILE := /home/huy/work_space/ti_sdk_board/external-toolchain-dir/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-

# Compiler
CC := $(CROSS_COMPILE)gcc

# Source & output
TARGET := read_can.o
SRCS   := read_can.c

# Flags (không cần flags đặc biệt cho SocketCAN)
CFLAGS := -Wall -O2

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)