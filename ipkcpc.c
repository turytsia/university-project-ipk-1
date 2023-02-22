#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>

#define ARGC 7

#define OPTION_HOST "-h"
#define OPTION_PORT "-p"
#define OPTION_MODE "-m"

#define COLOR_CYAN "\033[0;36m"
#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

#define DEBUG 1

void exit_with_message(const char* message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(stderr, COLOR_RED "[ERROR]" COLOR_RESET);
    vfprintf(stderr, message, args);
    printf("\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

void debug(const char* message, ...) {
    if (!DEBUG)
        return;
    va_list args;
    va_start(args, message);
    printf(COLOR_CYAN "[DEBUG]" COLOR_RESET);
    vprintf(message, args);
    printf("\n");
    va_end(args);
}

int main(int argc, char** argv) {
    if (argc != ARGC)
        exit_with_message("Usage %s -h <host> -p <port> -m <mode>", argv[0]);

    char host[255];
    char port[255];
    char mode[255];

    debug("Reading arguments...");
    for (int i = 1; i < ARGC; i += 2) {
        if (strcmp(argv[i], OPTION_HOST) == 0) {
            debug("host: %s", argv[i + 1]);
            strcpy(host, argv[i + 1]);
        }
        else if (strcmp(argv[i], OPTION_PORT) == 0) {
            debug("port: %s", argv[i + 1]);
            strcpy(port, argv[i + 1]);
        }
        else if (strcmp(argv[i], OPTION_MODE) == 0) {
            debug("mode: %s", argv[i + 1]);
            strcpy(mode, argv[i + 1]);
        }
        else {
            exit_with_message("Unknown command %s", argv[i]);
        }
    }

    debug("Opening socket...");
    int socket_desc;
    if (strcmp(mode,"tcp") == 0) {
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (strcmp(mode, "udp") == 0) {
        socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else {
        exit_with_message("Unknown mode: %s",mode);
    }
    debug("socket_desc: %d", socket_desc);

    if (socket_desc < 0)
        exit_with_message("Unable to create socket.");

    struct sockaddr_in server;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    // if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
    //     error("Invalid server address");
    // }


    return EXIT_SUCCESS;
}