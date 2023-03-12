#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>

#define ARGC 7

#define OPTION_HOST "-h"
#define OPTION_PORT "-p"
#define OPTION_MODE "-m"

#define COLOR_CYAN "\033[0;36m"
#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

#define BUFFER_SIZE 1024

#define DEBUG 1

int socketfd;

void exit_with_message(const char* message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(stderr, COLOR_RED "[ERROR] " COLOR_RESET);
    vfprintf(stderr, message, args);
    printf("\n");
    va_end(args);
    send(socketfd, "\n", 1, 0);
    close(socketfd);
    exit(EXIT_FAILURE);
}

void debug(const char* message, ...) {
    if (!DEBUG)
        return;
    va_list args;
    va_start(args, message);
    printf(COLOR_CYAN "[DEBUG] " COLOR_RESET);
    vprintf(message, args);
    printf("\n");
    va_end(args);
}

void interupt_handler() {
    send(socketfd, "\n", 1, 0);
    close(socketfd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    if (argc != ARGC)
        exit_with_message("Usage %s -h <host> -p <port> -m <mode>", argv[0]);

    char host[255];
    char mode[255];
    unsigned port;

    debug("Reading arguments...");
    for (int i = 1; i < ARGC; i += 2) {
        if (strcmp(argv[i], OPTION_HOST) == 0) {
            debug("host: %s", argv[i + 1]);
            strcpy(host, argv[i + 1]);
        }
        else if (strcmp(argv[i], OPTION_PORT) == 0) {
            debug("port: %s", argv[i + 1]);
            port = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], OPTION_MODE) == 0) {
            debug("mode: %s", argv[i + 1]);
            strcpy(mode, argv[i + 1]);
        }
        else {
            exit_with_message("Unknown command %s", argv[i]);
        }
    }


    debug("Setting up server conection...");

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    debug("Opening socket...");
    
    if (strcmp(mode,"tcp") == 0) {
        socketfd = socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (strcmp(mode, "udp") == 0) {
        socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else {
        exit_with_message("Unknown mode: %s",mode);
    }

    if (socketfd < 0){
        exit_with_message("Unable to create socket.");
    }


    if(connect(socketfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0){
        exit_with_message("Connection failed");
    }

    debug("Register SIGINT...");
    if(signal(SIGINT, interupt_handler) == SIG_ERR){
        exit_with_message("Unable to register SIGINT");
    }

    debug("Client has been connected successfuly. You can start typing");
    

    while(1){
        char raw_request[BUFFER_SIZE] = {'\0'};
        char raw_response[BUFFER_SIZE] = {'\0'};


        char* user_input = fgets(raw_request, sizeof(raw_request), stdin);

        ssize_t send_bytes = send(socketfd, raw_request, strlen(raw_request), 0);

        if(send_bytes < 0){
            exit_with_message("Unable to send request");
        }


        ssize_t recv_bytes = recv(socketfd, raw_response, BUFFER_SIZE,0);

        if(recv_bytes < 0){
            exit_with_message("Unable to recieve response");
        }

        printf("%s",raw_response);

        // TODO react on ctrl + C

        if(strstr(raw_response, "BYE") != NULL){
            break;
        }

    }

    close(socketfd);

    return EXIT_SUCCESS;
}