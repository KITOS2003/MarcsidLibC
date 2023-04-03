#ifndef LOG_H_
#define LOG_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum LOG_COLORS_ENUM
{
    TERM_BLACK = 0,
    TERM_RED,
    TERM_GREEN,
    TERM_YELLOW,
    TERM_BLUE,
    TERM_MAGENTA,
    TERM_CYAN,
    TERM_WHITE,
} LOG_COLORS_ENUM;

typedef enum LOG_COLORS_BY_TYPE
{
    CRITICAL_ERROR_COLOR = TERM_RED,
    WARNING_COLOR = TERM_YELLOW,
    DEBUG_MESSAGE_COLOR = TERM_BLUE,
    INFO_COLOR = TERM_MAGENTA,
} LOG_COLORS_BY_TYPE;

#define LOG_COLOR(foreground, background)                                      \
    printf("\x1B[%u;%um", 30 + foreground, 40 + background)
#define LOG_COLOR_CLEAN printf("\x1B[0m")

// common format used by all log macros
#define LOG_MESSAGE(color1, color2, head, message)                             \
    LOG_COLOR(color1, color2);                                                 \
    printf("%s:", head);                                                       \
    LOG_COLOR(color2, color1);                                                 \
    printf(" %s\n", message);                                                  \
    LOG_COLOR_CLEAN

// critical, unrecoverable error, terminates the program
#define LOG_CRITICAL_ERROR(message)                                            \
    LOG_MESSAGE(TERM_BLACK, CRITICAL_ERROR_COLOR, "CRITICAL", message)         \
    exit(1)
#define LOG_CRITICAL_ERROR_NOEXIT(message)                                     \
    LOG_MESSAGE(TERM_BLACK, CRITICAL_ERROR_COLOR, "CRITICAL", message)

#define LOG_WARNING(message)                                                   \
    LOG_MESSAGE(TERM_BLACK, WARNING_COLOR, "WARNING", message)

#define LOG_DEBUG_MESSAGE(message)                                             \
    LOG_MESSAGE(TERM_BLACK, DEBUG_MESSAGE_COLOR, "DEBUG", message)

#define LOG_INFO(message) LOG_MESSAGE(TERM_BLACK, INFO_COLOR, "INFO", message)

#define LOG_DATA_DUMP(message, color)                                          \
    LOG_COLOR(color, TERM_BLACK);                                              \
    printf("%s\n", message);                                                   \
    LOG_COLOR_CLEAN

void log_critical_error(char *header, size_t n, ...);
void log_warning(char *header, size_t n, ...);
void log_debug_message(char *header, size_t n, ...);
void log_info(char *header, size_t n, ...);

#endif // #ifndef LOG_H_

#ifndef LOG_INCLUDE_IMPLEMENTATION

void log_critical_error(char *header, size_t n, ...)
{
    LOG_CRITICAL_ERROR_NOEXIT(header);
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++)
    {
        LOG_DATA_DUMP(va_arg(args, char *), CRITICAL_ERROR_COLOR);
    }
    exit(1);
}

void log_warning(char *header, size_t n, ...)
{
    LOG_WARNING(header);
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++)
    {
        LOG_DATA_DUMP(va_arg(args, char *), WARNING_COLOR);
    }
}

void log_debug_message(char *header, size_t n, ...)
{
    LOG_DEBUG_MESSAGE(header);
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++)
    {
        LOG_DATA_DUMP(va_arg(args, char *), DEBUG_MESSAGE_COLOR);
    }
}

void log_info(char *header, size_t n, ...)
{
    LOG_INFO(header);
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++)
    {
        LOG_DATA_DUMP(va_arg(args, char *), INFO_COLOR);
    }
}

#endif // #ifndef LOG_INCLUDE_IMPLEMENTATION
