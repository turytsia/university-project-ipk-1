#include "sockets.h"

/**
 *@brief Constructs server object based on program's arguments
 *
 * @param args 
 * @return struct sockaddr_in 
 */
struct sockaddr_in server_addr_setup(struct args_t args) {
    return (struct sockaddr_in) {
        .sin_family = AF_INET, .sin_port = htons(args.port), .sin_addr.s_addr = inet_addr(args.host)
    };
}

/**
 *@brief Encodes user's message for udp request and stores it in a buffer
 *
 * @param buf buffer
 * @param msg user's message
 * @return struct req_t
 */
struct req_t encode(char* buf, char* msg) {
    int opcode = 0;
    int msg_size = strlen(msg);
    int payload_size = msg_size + 1;

    memcpy(&buf[0], &opcode, sizeof(opcode));
    memcpy(&buf[1], &payload_size, sizeof(payload_size));
    memcpy(&buf[2], msg, msg_size + 1);

    return (struct req_t) { .payload_size = payload_size, .msg_size = msg_size, .opcode = opcode };
}

/**
 *@brief Decodes udp server's response into human-readable message
 *
 * @param buf buffer
 * @param msg response message
 * @return struct res_t
 */
struct res_t decode(char* buf, char* msg) {
    int opcode = msg[0];
    int status = msg[1];
    int  payload_size = msg[2];

    memcpy(buf, &msg[3], payload_size);
    buf[payload_size] = '\0';

    return (struct res_t) { .opcode = opcode, .status = status };
}

/**
 * @brief Closes sockets & terminates all the APIs
 *
 */
void cleanup() {
    if (sock.is_active) {
#ifdef _WIN32
        closesocket(sock.socketfd);
        WSACleanup(); //terminates Winsock 2 DLL (Dynamic Link Library)
#else
        close(sock.socketfd);
#endif   
    }
}