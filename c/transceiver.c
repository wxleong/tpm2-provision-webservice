#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/buffer.h>

uint32_t bytes_to_int(const unsigned char* bytes)
{
    uint32_t result = 0;
    result |= bytes[0] << 24;
    result |= bytes[1] << 16;
    result |= bytes[2] << 8;
    result |= bytes[3];
    return result;
}

int process_command(const char *topic, void *zmq_pub_sock, void *zmq_sub_sock)
{
    int ret = 1;
    unsigned char buffer[4096] = {0}; /* TPM2_MAX_COMMAND_SIZE */
    size_t bytes_read = 0;
    size_t total_bytes_read = 0;
    size_t bytes_to_read = 10; /* Header size */
    uint32_t command_size = 0;
    char *command_string = NULL;
    char *output_string = NULL;
    size_t output_string_size = 0;

    /* Copy the topic first*/

    if (!(bytes_read = fread(buffer, 1, bytes_to_read, stdin))) {
        fprintf(stderr, "Unable to read the command header.\n");
        goto exit;
    }

    command_size = bytes_to_int(buffer + 2);

    if (command_size < 10 || command_size > 4096) {
        fprintf(stderr, "Unable to decode the command size.\n");
        goto exit;
    }

    total_bytes_read = bytes_read;
    bytes_to_read = command_size - total_bytes_read;

    if (!(bytes_read = fread(buffer + total_bytes_read, 1, bytes_to_read, stdin))) {
        fprintf(stderr, "Unable to read the command payload.\n");
        goto exit;
    }

    total_bytes_read += bytes_read;

    /* Send the command String to ZeroMQ. */

    command_string = OPENSSL_buf2hexstr(buffer, total_bytes_read);
    if (command_string == NULL) {
        fprintf(stderr, "Unable to convert command bytes to hex string.\n");
        goto exit;
    }

    /* Concat topic and command strings, separated by a space */
    output_string_size = strlen(topic) + strlen(command_string) + 1;
    output_string = (char *) calloc(1, output_string_size);
    if (!output_string) {
        fprintf(stderr, "Unable to alloc memory.\n");
        goto exit;
    }
    memcpy(output_string, topic, strlen(topic));
    output_string[strlen(topic)] = ' ';
    memcpy(output_string + strlen(topic) + 1, command_string, strlen(command_string));

    /* Poll for connection readiness by sending an empty message with only topic set */
    while (1) {
        char buffer[1];
        char *tmp = "server abc";

        if (zmq_send(zmq_pub_sock, tmp, strlen(tmp), 0) != strlen(tmp)) {
            fprintf(stderr, "Message publication to ZeroMQ has failed.\n");
            goto exit;
        }

        /* Retry if the empty message is not received */
        if (zmq_recv(zmq_sub_sock, buffer, 1, ZMQ_DONTWAIT) > 0) {
            break;
        }
        usleep(1000);
    }

    /* Finally, publish the message here */
    char *tmp2 = "server abc 000102030405";
    //if (zmq_send(zmq_pub_sock, output_string, strlen(output_string), 0) != strlen(output_string)) {
    if (zmq_send(zmq_pub_sock, tmp2, strlen(tmp2), 0) != strlen(tmp2)) {
        fprintf(stderr, "Message publication to ZeroMQ has failed.\n");
        goto exit;
    }

    ret = 0;

exit:
    /* Clean up */
    free(output_string);
    OPENSSL_free(command_string);

    return ret;
}

int process_response(const char *topic, void *zmq_pub_sock, void *zmq_sub_sock)
{
    char *response_string = NULL;
    unsigned char *response_bytes = NULL;
    long response_bytes_size = 0;

#if 1 /* For testing only */
    /* tpm2_startup response raw byte array */
    unsigned char sample_response_bytes[] = {0x80, 0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x01, 0x00};
    size_t sample_response_bytes_size = sizeof(sample_response_bytes);

    /* Convert byte array to hex string */
    response_string = OPENSSL_buf2hexstr(sample_response_bytes, sample_response_bytes_size);
    if (response_string == NULL) {
        fprintf(stderr, "Unable to convert response bytes to hex string.\n");
        return 1;
    }
#else
    /* Receive the response string from ZeroMQ. */

    /* ... */
#endif

    /* Convert hex string back to byte array */
    response_bytes = OPENSSL_hexstr2buf(response_string, &response_bytes_size);
    if (response_bytes == NULL) {
        perror("Error converting hex string to byte array");
        fprintf(stderr, "Unable to convert response hex string to bytes.\n");
        OPENSSL_free(response_string);
        return 1;
    }

    fwrite(response_bytes, sizeof(unsigned char), response_bytes_size, stdout);

    /* Clean up */
#if 1
    OPENSSL_free(response_string);
#endif
    OPENSSL_free(response_bytes);

    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 1;
    int zmq_rc;
    void *zmq_pub_ctx, *zmq_sub_ctx;
    void *zmq_pub_sock, *zmq_sub_sock;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        goto exit;
    }

    /* Create a new ZeroMQ context */

    zmq_pub_ctx = zmq_ctx_new();
    if (zmq_pub_ctx == NULL) {
        fprintf(stderr, "Unable to create ZeroMQ context.\n");
        goto exit;
    }

    zmq_sub_ctx = zmq_ctx_new();
    if (zmq_sub_ctx == NULL) {
        fprintf(stderr, "Unable to create ZeroMQ context.\n");
        goto exit;
    }

    /* Create a new ZeroMQ socket */

    zmq_pub_sock = zmq_socket(zmq_pub_ctx, ZMQ_PUB);
    if (zmq_pub_sock == NULL) {
        fprintf(stderr, "Unable to create ZeroMQ socket.\n");
        goto exit;
    }

    zmq_sub_sock = zmq_socket(zmq_sub_ctx, ZMQ_SUB);
    if (zmq_sub_sock == NULL) {
        fprintf(stderr, "Unable to create ZeroMQ socket.\n");
        goto exit;
    }

    /* Connect the ZeroMQ socket to a known port */

    zmq_rc = zmq_connect(zmq_pub_sock, "tcp://127.0.0.1:6001");
    if (zmq_rc != 0) {
        fprintf(stderr, "Unable to connect to ZeroMQ socket.\n");
        goto exit;
    }

    zmq_rc = zmq_connect(zmq_sub_sock, "tcp://127.0.0.1:6002");
    if (zmq_rc != 0) {
        fprintf(stderr, "Unable to connect to ZeroMQ socket.\n");
        goto exit;
    }

    if (zmq_setsockopt(zmq_sub_sock, ZMQ_SUBSCRIBE, argv[1], strlen(argv[1]))) {
        fprintf(stderr, "Unable to set ZeroMQ socket option.\n");
        return 1;
    }

    if (process_command(argv[1], zmq_pub_sock, zmq_sub_sock)
        || process_response(argv[1], zmq_pub_sock, zmq_sub_sock)) {
        return 1;
    }

    ret = 0;

exit:
    zmq_close(zmq_pub_sock);
    zmq_close(zmq_sub_sock);
    zmq_ctx_destroy(zmq_pub_ctx);
    zmq_ctx_destroy(zmq_sub_ctx);

    return ret;
}

