/**
 * @file ipkcpc.c
 * @author Oleksandr Turytsia (xturyt00@stud.fit.vutbr.cz)
 * @brief Client that can connect to any server using IPv4 ip, port and mode (TCP/UDP)
 * @date 2023-03-15
 *
 * @copyright read LICENSE
 *
 */

#include "./src/ipkcpc.h"

#define OPTION_HOST "-h"
#define OPTION_PORT "-p"
#define OPTION_MODE "-m"

#define ARG_SIZE 7
#define BUF_SIZE 1024
#define PORT_SIZE 65536

#ifdef _WIN32 // implementation for windows
 /**
  *@brief Event handler for UDP server connection
  *
  * @param signal
  * @return BOOL
  */
BOOL WINAPI interupt_udp(DWORD signal) {
    switch (signal) {
    case CTRL_C_EVENT:
        cleanup();
        exit(0);
        return TRUE;
    default:
        return FALSE;
    }
}

/**
 *@brief Event handler for TCP server connection
 *
 * @param signal
 * @return BOOL
 */
BOOL WINAPI interupt_tcp(DWORD signal) {
    switch (signal) {
    case CTRL_C_EVENT:
        if (sock.is_active) {
            send(sock.socketfd, "\n", 1, 0);
        }
        return interupt_udp(signal);
    default:
        return FALSE;
    }
}
#else
 /**
 *@brief Interupt handler for SIGINT (UDP)
 *
 */
void interupt_udp() {
    cleanup();
    exit(EXIT_SUCCESS);
}

/**
*@brief Interupt handler for SIGINT (TCP)
*
*/
void interupt_tcp() {
    if (sock.is_active) {
        send(sock.socketfd, "\n", 1, 0);
    }
    interupt_udp();
}
#endif

int main(int argc, char** argv) {

    //user's help in case if parameters are incorrect
    if (argc != ARG_SIZE) {
        exit_with_message("Usage %s -h <host> -p <port> -m <mode>", argv[0]);
    }

    struct args_t args;


    debug("Reading arguments...");

    for (int i = 1; i < ARG_SIZE; i += 2) {
        if (strcmp(argv[i], OPTION_HOST) == 0) {
            debug("host: %s", argv[i + 1]);
            strcpy(args.host, argv[i + 1]);
        }
        else if (strcmp(argv[i], OPTION_PORT) == 0) {
            debug("port: %s", argv[i + 1]);
            args.port = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], OPTION_MODE) == 0) {
            debug("mode: %s", argv[i + 1]);

            if (strcmp("tcp", argv[i + 1]) == 0) {
                args.mode = TCP;
            }
            else if (strcmp("udp", argv[i + 1]) == 0) {
                args.mode = UDP;
            }
            else {
                exit_with_message("Unknown mode %s", argv[i + 1]);
            }
        }
        else {
            exit_with_message("Unknown command %s", argv[i]);
        }
    }

    if (args.port < 0 || args.port > PORT_SIZE) {
        exit_with_message("Port must be in interval 0-%d", PORT_SIZE);
    }

    debug("Register SIGINT...");

#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &sock.wsa) != 0) {
        exit_with_message("Unable to startup windows socket API");
    }
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)(args.mode == TCP ? interupt_tcp : interupt_udp), TRUE) == FALSE) {
        exit_with_message("Could not set console control handler");
    }
#else
    if (signal(SIGINT, args.mode == TCP ? interupt_tcp : interupt_udp) == SIG_ERR) {
        exit_with_message("Unable to register SIGINT");
    }
#endif

    debug("Setting up server conection...");


    struct sockaddr_in server_addr = server_addr_setup(args);
    socklen_t serverlen = sizeof(server_addr);

    debug("Opening socket...");

    if (args.mode == TCP) {
        sock.socketfd = socket(AF_INET, SOCK_STREAM, 0);
    }
    else {
        sock.socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    }

    if (sock.socketfd < 0) {
        exit_with_message("Unable to open socket.");
    }
    else {
        sock.is_active = true;
    }

    debug("Connecting to the server...");

//timeout interval
#ifdef _WIN32
    DWORD timeout = 5000;
#else
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
#endif


    if (setsockopt(sock.socketfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        exit_with_message("setsockopt() failed");
    }


    if (connect(sock.socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        exit_with_message("Connection failed");
    }

    debug("Client has been connected successfuly. You can start typing");

    while (1) {

        char buf[BUF_SIZE] = { '\0' };
        char raw_request[BUF_SIZE] = { '\0' };
        char raw_response[BUF_SIZE] = { '\0' };

        fgets(raw_request, sizeof(raw_request), stdin);

        /**
         *@brief if user is using windows, when clicking ctrl+C will create another
         * thread that might invoke request logic. This if prevents it.
         */
        if (strlen(raw_request) == 0)
            continue;

        int send_bytes;
        struct req_t req;
        if (args.mode == TCP) {
            send_bytes = send(sock.socketfd, raw_request, strlen(raw_request), 0);
        }
        else {
            req = encode(buf, raw_request);
            send_bytes = sendto(sock.socketfd, buf, req.payload_size, 0, (struct sockaddr*)&server_addr, serverlen);
        }

        if (send_bytes < 0) {
            exit_with_message("Unable to send request");
        }


        int recv_bytes;
        struct res_t res;
        if (args.mode == TCP) {
            recv_bytes = recv(sock.socketfd, raw_response, BUF_SIZE, 0);
        }
        else {
            recv_bytes = recvfrom(sock.socketfd, raw_response, BUF_SIZE, 0, (struct sockaddr*)&server_addr, &serverlen);
            res = decode(buf, raw_response);
        }

        if (recv_bytes < 0) {
            exit_with_message("Unable to recieve response");
        }

        if (args.mode == TCP) {
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

    cleanup();

    return EXIT_SUCCESS;
}