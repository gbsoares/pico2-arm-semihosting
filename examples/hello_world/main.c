#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "semihosting/semihosting.h"

void semihost_printf(const char* format, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Ensure null termination
    if (len >= sizeof(buffer)) {
        buffer[sizeof(buffer) - 1] = '\0';
    }

    // Send the formatted string via semihosting
    semihosting_write(1, buffer, strlen(buffer));
}

int main()
{
    semihost_printf("=== ARM Semihosting Test ===\n");
    semihost_printf("This message appears in the debugger console.\n");
    semihost_printf("Counter test: ");
    for (int i = 0; i < 5; i++) {
        semihost_printf("%d", i);
    }
    semihost_printf("\n");

    return 0;
}
