# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Libraries
LIBS = -lzmq

# Source files
SENDER_SRC = send.c
RECEIVER_SRC = receive.c

# Executable files
SENDER_EXE = zmq_sender
RECEIVER_EXE = zmq_receiver

# Default target
all: $(SENDER_EXE) $(RECEIVER_EXE)

# Build the sender executable
$(SENDER_EXE): $(SENDER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Build the receiver executable
$(RECEIVER_EXE): $(RECEIVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Clean up the build
clean:
	rm -f $(SENDER_EXE) $(RECEIVER_EXE)

# Phony targets
.PHONY: all clean

