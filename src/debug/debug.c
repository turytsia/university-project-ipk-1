#include "debug.h"

/**
 * @brief terminates program with error code & message
 *
 * @param message
 * @param ...
 */
void exit_with_message(const char* message, ...) {
    va_list args;
    va_start(args, message);
    fprintf(stderr, COLOR_RED "[ERROR] " COLOR_RESET);
    vfprintf(stderr, message, args);
    printf("\n");
    va_end(args);
    cleanup();
    exit(EXIT_FAILURE);
}

/**
 *@brief prints actions that program has performed. (#define DEBUG)
 *
 * @param message
 * @param ...
 */
void debug(const char* message, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, message);
    printf(COLOR_CYAN "[DEBUG] " COLOR_RESET);
    vprintf(message, args);
    printf("\n");
    va_end(args);
#else
    message = message;
#endif
}