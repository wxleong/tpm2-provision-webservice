# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Libraries
LIBS = -lzmq -lcrypto

# Source files
SENDER_SRC = send.c
RECEIVER_SRC = receive.c
TRANSCEIVER_SRC = transceiver.c

# Executable files
SENDER_EXE = sender
RECEIVER_EXE = receiver
TRANSCEIVER_EXE = transceiver

# Default target
all: $(SENDER_EXE) $(RECEIVER_EXE) $(TRANSCEIVER_EXE)

# Build the sender executable
$(SENDER_EXE): $(SENDER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Build the receiver executable
$(RECEIVER_EXE): $(RECEIVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Build the transceiver executable
$(TRANSCEIVER_EXE): $(TRANSCEIVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Clean up the build
clean:
	rm -f $(SENDER_EXE) $(RECEIVER_EXE) $(TRANSCEIVER_EXE)

# Phony targets
.PHONY: all clean

