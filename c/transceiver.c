#include <stdio.h>
#include <stdlib.h>
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

int process_command()
{
    unsigned char buffer[4096] = {0}; /* TPM2_MAX_COMMAND_SIZE */
    size_t bytes_read = 0;
    size_t total_bytes_read = 0;
    size_t bytes_to_read = 10; /* Header size */
    uint32_t command_size = 0;
    char *cmd_hex_string = NULL;

    if (!(bytes_read = fread(buffer, 1, bytes_to_read, stdin))) {
        fprintf(stderr, "Unable to read the command header.\n");
        return 1;
    }

    command_size = bytes_to_int(buffer + 2);

    if (command_size < 10 || command_size > sizeof(buffer)) {
        fprintf(stderr, "Unable to decode the command size.\n");
        return 1;
    }

    total_bytes_read = bytes_read;
    bytes_to_read = command_size - total_bytes_read;

    if (!(bytes_read = fread(buffer + total_bytes_read, 1, bytes_to_read, stdin))) {
        fprintf(stderr, "Unable to read the command payload.\n");
        return 1;
    }

    total_bytes_read += bytes_read;

    /* Send the command String to ZeroMQ. */

    cmd_hex_string = OPENSSL_buf2hexstr(buffer, total_bytes_read);
    if (cmd_hex_string == NULL) {
        fprintf(stderr, "Unable to convert command bytes to hex string.\n");
        return 1;
    }


    /* ... */

    return 0;
}

int process_response()
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

int main()
{
    if (process_command()
        || process_response()) {
        return 1;
    }

    return 0;
}

