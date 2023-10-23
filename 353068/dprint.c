#include "dprint.h"

void dprint_clog(char *color, FILE *stream, const char *str, ...)
{
    if (!DEBUG_PRINT)
    {
        return;
    }

    va_list args;
    va_start(args, str);
    vfprintf(stream, color, args);
    vfprintf(stream, str, args);
    vfprintf(stream, COLOR_RESET, args);
    va_end(args);
}

void dprint_log(char *color, FILE *stream, const char *str, ...)
{
    if (!DEBUG_PRINT)
    {
        return;
    }

    va_list args;
    va_start(args, str);
    vfprintf(stream, color, args);
    vfprintf(stream, str, args);
    vfprintf(stream, COLOR_RESET, args);
    va_end(args);
}


