#include "dprint.h"

void dprint_print_debug(char *color, FILE *stream, const char *str, ...) {
    if (!DEBUG_PRINT)
    {
        return;
    }

    va_list args;
    va_start(args, str);
    vfprintf(stream, color, args);
    vfprintf(stream, str, args);
    vfprintf(stream, color, args);
    va_end(args);
}
