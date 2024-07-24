#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TOPIC_SERVER "server"

char *bytes_to_hexstr(const uint8_t *buffer, size_t len)
{
    char *hex_str = (char *)malloc(len * 2 + 1);
    if (!hex_str) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 2, "%02x", buffer[i]);
    }

    hex_str[len * 2] = '\0';
    return hex_str;
}

int hex_digit_to_int(char hex)
{
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    } else if (hex >= 'A' && hex <= 'F') {
        return hex - 'A' + 10;
    } else if (hex >= 'a' && hex <= 'f') {
        return hex - 'a' + 10;
    } else {
        return -1;
    }
}

uint8_t *hexstr_to_bytes(const char *hex_str, size_t *out_len)
{
    size_t len = strlen(hex_str);
    if (len % 2 != 0) {
        return NULL;
    }

    *out_len = len / 2;
    unsigned char *bytes = (unsigned char *)malloc(*out_len);
    if (!bytes) {
        return NULL;
    }

    for (size_t i = 0; i < *out_len; i++) {
        int high = hex_digit_to_int(hex_str[i * 2]);
        int low = hex_digit_to_int(hex_str[i * 2 + 1]);

        if (high == -1 || low == -1) {
            free(bytes);
            return NULL;
        }

        bytes[i] = (unsigned char)((high << 4) | low);
    }

    return bytes;
}

uint32_t bytes_to_uint32(const unsigned char *bytes)
{
    uint32_t result = 0;
    result |= bytes[0] << 24;
    result |= bytes[1] << 16;
    result |= bytes[2] << 8;
    result |= bytes[3];
    return result;
}

char *concat_zmq_message(const char *dest_topic, const char *src_topic, const char *message)
{
    size_t sum = 0;
    size_t size = 0;
    char *concat = NULL;

    size = strlen(dest_topic) + 1;

    if (src_topic) {
        size += strlen(src_topic) + 1;
    }

    if (message) {
        size += strlen(message) + 1;
    }

    concat = (char *) calloc(1, size);
    if (!concat) {
        fprintf(stderr, "Unable to alloc memory.\n");
        return NULL;
    }

    memcpy(concat, dest_topic, strlen(dest_topic));
    sum = strlen(dest_topic);

    if (src_topic) {
        concat[sum++] = ' ';

        memcpy(concat + sum, src_topic, strlen(src_topic));
        sum += strlen(src_topic);
    }

    if (message) {
        concat[sum++] = ' ';

        memcpy(concat + sum, message, strlen(message));
        sum += strlen(message);
    }

    concat[sum] = '\0';

    return concat;
}

int process_command(const char *topic, void *zmq_pub_sock, void *zmq_sub_sock)
{
    int ret = 1;
    uint8_t stdin_buffer[4096] = {0}; /* TPM2_MAX_COMMAND_SIZE */
    size_t bytes_read = 0;
    size_t total_bytes_read = 0;
    size_t bytes_to_read = 10; /* Header size */
    uint32_t command_size = 0;
    char *command_string = NULL;
    char *concat_string = NULL;
    char *polling_string = NULL;

    /* Read the bytes from stdin */

    if (!(bytes_read = fread(stdin_buffer, 1, bytes_to_read, stdin))) {
        fprintf(stderr, "Unable to read the command header.\n");
        goto exit;
    }

    command_size = bytes_to_uint32(stdin_buffer + 2);

    if (command_size < 10 || command_size > 4096) {
        fprintf(stderr, "Unable to decode the command size or invalid command size.\n");
        goto exit;
    }

    total_bytes_read = bytes_read;
    bytes_to_read = command_size - total_bytes_read;

    if (!(bytes_read = fread(stdin_buffer + total_bytes_read, 1, bytes_to_read, stdin))) {
        fprintf(stderr, "Unable to read the command payload.\n");
        goto exit;
    }

    total_bytes_read += bytes_read;

    /* Convert the received bytes to hex string */
    command_string = bytes_to_hexstr(stdin_buffer, total_bytes_read);
    if (!command_string) {
        fprintf(stderr, "Unable to convert command bytes to hex string.\n");
        goto exit;
    }

    /* Construct the polling message  */
    polling_string = concat_zmq_message(topic, topic, NULL);
    if (!polling_string) {
        fprintf(stderr, "Unable to alloc memory.\n");
        goto exit;
    }

    /* Poll for connection readiness by sending an empty message with only topic set */
    while (1) {
        char buffer[256];

        if (zmq_send(zmq_pub_sock, polling_string, strlen(polling_string), 0) != strlen(polling_string)) {
            fprintf(stderr, "Message publication to ZeroMQ has failed.\n");
            goto exit;
        }

        /* Retry if the polling message is not received */
        if (zmq_recv(zmq_sub_sock, buffer, sizeof(buffer), ZMQ_DONTWAIT) > 0) {
            break;
        }

        usleep(1000);
    }

    /* Concat topics and command strings, separated by spaces and ends with NULL byte */
    concat_string = concat_zmq_message(TOPIC_SERVER, topic, command_string);
    if (!concat_string) {
        fprintf(stderr, "Unable to alloc memory.\n");
        goto exit;
    }

    /* Finally, publish the message here */
    if (zmq_send(zmq_pub_sock, concat_string, strlen(concat_string), 0) != strlen(concat_string)) {
        fprintf(stderr, "Message publication to ZeroMQ has failed.\n");
        goto exit;
    }

    ret = 0;

exit:
    /* Clean up */
    free(command_string);
    free(concat_string);
    free(polling_string);

    return ret;
}

int process_response(const char *topic, void *zmq_pub_sock, void *zmq_sub_sock)
{
    char *desc_topic_string = NULL;
    char *src_topic_string = NULL;
    char raw_string[256 + 256 + (4096*2)]; // desc_topic + src_topic + TPM2_MAX_COMMAND_SIZE
    int zmq_ret = 0;
    char *response_string = NULL;
    uint8_t *response_bytes = NULL;
    size_t response_bytes_size = 0;


    /* Receive the response string from ZeroMQ. */

    while (1) {
        if ((zmq_ret = zmq_recv(zmq_sub_sock, raw_string, sizeof(raw_string), 0)) < 0) {
            fprintf(stderr, "zmq_recv has failed.\n");
            return 1;
        }

        raw_string[zmq_ret] = '\0';

        /* Split the topics and message */

        if (!(desc_topic_string = strtok(raw_string, " ")) ||
            !(src_topic_string = strtok(NULL, " "))) {
            fprintf(stderr, "Received malformed message from ZeroMQ.\n");
            return 1;
        }

        /* Check if this is not the message from previous polling operation */
        if (!strcmp(src_topic_string, topic)) {
            continue;
        }

        if (!(response_string = strtok(NULL, " "))) {
            fprintf(stderr, "Received malformed message from ZeroMQ.\n");
            return 1;
        }

        break;
    }

    /* Convert hex string back to byte array */

    response_bytes = hexstr_to_bytes(response_string, &response_bytes_size);
    if (response_bytes == NULL) {
        fprintf(stderr, "Unable to convert response hex string to bytes.\n");
        free(response_string);
        return 1;
    }

    /* Output the bytes to stdout */

    fwrite(response_bytes, sizeof(unsigned char), response_bytes_size, stdout);

    /* Clean up */

    free(response_bytes);

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

