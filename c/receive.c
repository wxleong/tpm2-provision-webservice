#include <zmq.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    // Create a new ZeroMQ context
    void *context = zmq_ctx_new();
    if (context == NULL) {
        printf("Error: Could not create ZeroMQ context\n");
        return 1;
    }

    // Create a new ZeroMQ socket of type SUB
    void *socket = zmq_socket(context, ZMQ_SUB);
    if (socket == NULL) {
        printf("Error: Could not create ZeroMQ socket\n");
        zmq_ctx_destroy(context);
        return 1;
    }

    // Connect the ZeroMQ socket to port 5555
    int rc = zmq_connect(socket, "tcp://localhost:6002");
    if (rc != 0) {
        printf("Error: Could not connect to ZeroMQ socket\n");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Subscribe to all messages
    rc = zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
    if (rc != 0) {
        printf("Error: Could not set socket option\n");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Receive messages
    while (1) {
        char buffer[2048];
        int size = zmq_recv(socket, buffer, sizeof(buffer), 0);
        if (size == -1) {
            printf("Error: Could not receive message\n");
            zmq_close(socket);
            zmq_ctx_destroy(context);
            return 1;
        }
        buffer[size] = '\0';
        printf("Received: %s\n", buffer);
    }

    // Clean up
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}

