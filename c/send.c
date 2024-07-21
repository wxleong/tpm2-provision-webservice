#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    // Create a new ZeroMQ context
    void *context = zmq_ctx_new();
    if (context == NULL) {
        printf("Error: Could not create ZeroMQ context\n");
        return 1;
    }

    // Create a new ZeroMQ socket of type PUB
    void *socket = zmq_socket(context, ZMQ_PUB);
    if (socket == NULL) {
        printf("Error: Could not create ZeroMQ socket\n");
        zmq_ctx_destroy(context);
        return 1;
    }

    // Connect the ZeroMQ socket to port 5555
    int rc = zmq_connect(socket, "tcp://127.0.0.1:6001");
    if (rc != 0) {
        printf("Error: Could not connect ZeroMQ socket\n");
        zmq_close(socket);
        zmq_ctx_destroy(context);
        return 1;
    }

    // Wait for subscribers to connect
    sleep(1); // Sleep for a second to allow subscribers to connect

    // Publish messages
    for (int i = 0; i < 10; i++) {
        char message[256];
        snprintf(message, sizeof(message), "Hello, ZeroMQ! Message %d", i);
        printf("Publishing: %s\n", message);
        zmq_send(socket, message, strlen(message), 0);
        sleep(1); // Sleep for a second
    }

    // Clean up
    zmq_close(socket);
    zmq_ctx_destroy(context);

    return 0;
}

