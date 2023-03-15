#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#endif

#define ARG_SIZE 7

#define OPTION_HOST "-h"
#define OPTION_PORT "-p"
#define OPTION_MODE "-m"

#define COLOR_CYAN "\033[0;36m"
#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

#define BUF_SIZE 1024
#define PORT_SIZE 65536

#define DEBUG 1

int socketfd;

struct req_t {
    int opcode;
    int msg_size;
    size_t payload_size;
};

struct res_t {
    int opcode;
    int status;
};

void exit_with_message(const char* message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(stderr, COLOR_RED "[ERROR] " COLOR_RESET);
    vfprintf(stderr, message, args);
    printf("\n");
    va_end(args);
    send(socketfd, "\n", 1, 0);
#ifdef _WIN32
    closesocket(socketfd);
    WSACleanup();
#else
    close(socketfd);
#endif
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

#ifdef _WIN32
BOOL WINAPI interupt_udp(DWORD signal) {
    switch (signal) {
    case CTRL_C_EVENT:
        closesocket(socketfd);
        WSACleanup();
        exit(0);
        return TRUE;
    default:
        return FALSE;
    }
}

BOOL WINAPI interupt_tcp(DWORD signal) {
    send(socketfd, "\n", 1, 0);
    return interupt_udp(signal);
}
#else
void interupt_udp() {
    close(socketfd);
    exit(EXIT_SUCCESS);
}

void interupt_tcp() {
    send(socketfd, "\n", 1, 0);

    interupt_udp();
}
#endif
struct req_t encode(char* buf, char* msg) {
    int opcode = 0;
    int len = strlen(msg);
    int payload_len = len + 1;

    memcpy(&buf[0], &opcode, sizeof(opcode));
    memcpy(&buf[1], &payload_len, sizeof(payload_len));
    memcpy(&buf[2], msg, len + 1);

    return (struct req_t) { .payload_size = payload_len, .msg_size = len, .opcode = opcode };
}

struct res_t decode(char* buf, char* msg) {
    int opcode = msg[0];
    int status = msg[1];
    int  payload_len = msg[2];

    memcpy(buf, &msg[3], payload_len);
    buf[payload_len] = '\0';

    return (struct res_t) { .opcode = opcode, .status = status };
}

int main(int argc, char** argv) {
    if (argc != ARG_SIZE) {
        exit_with_message("Usage %s -h <host> -p <port> -m <mode>", argv[0]);
    }

    char host[255];
    char mode[255];
    int port;

#ifdef _WIN32
    WSADATA wsa;
#endif

    debug("Reading arguments...");
    
    for (int i = 1; i < ARG_SIZE; i += 2) {
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

    if (port < 0 || port > PORT_SIZE) {
        exit_with_message("Port must be in interval 0-%d", PORT_SIZE);
    }


    debug("Register SIGINT...");

#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        exit_with_message("Unable to startup windows socket API");
    }
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)(strcmp(mode, "tcp") == 0 ? interupt_tcp : interupt_udp), TRUE) == FALSE) {
        exit_with_message("Could not set console control handler");
    }
#else
    if (signal(SIGINT, strcmp(mode, "tcp") == 0 ? interupt_tcp : interupt_udp) == SIG_ERR) {
        exit_with_message("Unable to register SIGINT");
    }
#endif

    debug("Setting up server conection...");

    struct sockaddr_in server_addr;
    socklen_t serverlen;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    serverlen = sizeof(server_addr);

    debug("Opening socket...");

    if (strcmp(mode, "tcp") == 0) {
        socketfd = socket(AF_INET, SOCK_STREAM, 0);
    }
    else if (strcmp(mode, "udp") == 0) {
        socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else {
        exit_with_message("Unknown mode: %s", mode);
    }

    if (socketfd < 0) {
        exit_with_message("Unable to open socket.");
    }

    debug("Connecting to the server...");

    if (connect(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        exit_with_message("Connection failed");
    }

    debug("Client has been connected successfuly. You can start typing");
    
    while (1) {

        char buf[BUF_SIZE] = { '\0' };
        char raw_request[BUF_SIZE] = { '\0' };
        char raw_response[BUF_SIZE] = { '\0' };

        fgets(raw_request, sizeof(raw_request), stdin);

#ifdef _WIN32
        Sleep(100);
#endif

        int send_bytes;
        struct req_t req;
        if (strcmp(mode, "tcp") == 0) {
            send_bytes = send(socketfd, raw_request, strlen(raw_request), 0);
        }
        else {
            req = encode(buf, raw_request);
            send_bytes = sendto(socketfd, buf, req.payload_size, 0, (struct sockaddr*)&server_addr, serverlen);
        }

        if (send_bytes < 0) {
            exit_with_message("Unable to send request");
        }

        int recv_bytes;
        struct res_t res;
        if (strcmp(mode, "tcp") == 0) {
            recv_bytes = recv(socketfd, raw_response, BUF_SIZE, 0);
        }
        else {
            recv_bytes = recvfrom(socketfd, raw_response, BUF_SIZE, 0, (struct sockaddr*)&server_addr, &serverlen);
            res = decode(buf, raw_response);
        }

        if (recv_bytes < 0) {
            exit_with_message("Unable to recieve response");
        }


        if (strcmp(mode, "tcp") == 0) {
            printf("%s\b", raw_response);

            if (strstr(raw_response, "BYE") != NULL) {
                break;
            }
        }
        else {
            if (res.opcode == 1 && res.status == 0) {
                printf("OK:%s\n", buf);
            }
            else if (res.opcode == 1 && res.status == 1) {
                printf("ERR:%s\n", buf);
            }
            else {
                printf("Unexpected response message\n");
            }
        }

    }

#ifdef _WIN32
    closesocket(socketfd);
    WSACleanup();
#else 
    close(socketfd);
#endif

    return EXIT_SUCCESS;
}