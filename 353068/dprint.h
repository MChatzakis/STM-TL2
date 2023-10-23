#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "globals.h"

void dprint_clog(char *color, FILE *stream, const char *str, ...);