#include <stdio.h>
#include <stdarg.h>
#include "utils.h"

void log_info(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

