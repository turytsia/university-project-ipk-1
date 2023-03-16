/**
 *@file debug.h
 * @author Oleksandr Turytsia (xturyt00@stud.fit.vutbr.cz)
 * @brief Debug header file. Provides special functions for debugging and error throws
 * @version 0.1
 * @date 2023-03-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

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