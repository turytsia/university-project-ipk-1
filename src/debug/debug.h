#ifndef DEBUG_H
#define DEBUG_H

#include "../common/common.h"
#include "../sockets/sockets.h"

#define COLOR_CYAN "\033[0;36m"
#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

//DEBUG mode
//#define DEBUG

void exit_with_message(const char* message, ...);
void debug(const char* message, ...);
#endif