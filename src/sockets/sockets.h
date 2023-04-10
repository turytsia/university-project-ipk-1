/**
 *@file sockets.h
 * @author Oleksandr Turytsia (xturyt00@stud.fit.vutbr.cz)
 * @brief Network logic header
 * @version 0.1
 * @date 2023-03-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef SOCKETS_H
#define SOCKETS_H

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
#include <sys/time.h>
#endif

#include "../common/common.h"

/**
 *@brief connection mode
 *
 */
enum mode_t {
    TCP,
    UDP
};

/**
 *@brief program argument data
 *
 */
struct args_t {
    char host[HOST_SIZE];
    int port;
    enum mode_t mode;
};

/**
 *@brief Socket data 
 * 
 */
struct sock_t {
    int socketfd;
    bool is_active;
#ifdef _WIN32
    WSADATA wsa;
#endif
};

/**
 *@brief UDP request data
 *
 */
struct req_t {
    int opcode;
    int msg_size;
    size_t payload_size;
};

/**
 *@brief UDP response data
 *
 */
struct res_t {
    int opcode;
    int status;
};

/**
 *@brief Global var for socket data.
 *@note This var is global because in windows there is no way to pass this into event handler. Issue can be solved using multiple threads which is a lot more complicated than creating 1 global variable that changes once in one place.
 *
 */
extern struct sock_t sock;

struct sockaddr_in server_addr_setup(struct args_t args);
struct req_t encode(char* buf, char* msg);
struct res_t decode(char* buf, char* msg);
void cleanup();

#endif
