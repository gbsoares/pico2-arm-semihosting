#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdio_semihosting.h"

int main() 
{
    stdio_semihosting_init();

    printf("=== ARM Semihosting Test ===\n");
    printf("This message appears in the debugger console.\n");
    printf("Counter test: ");
    for (int i = 0; i < 5; i++)
    {
        printf("%d", i);
    }
    printf("\n");
    
    return 0;
}
