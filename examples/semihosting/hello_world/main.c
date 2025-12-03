#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "semihosting/semihosting.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

// Define LED pin - use board default or fallback to GPIO 25
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 25
#endif

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
    // Initialize stdio (includes UART)
    stdio_init_all();

#ifdef CYW43_WL_GPIO_LED_PIN
    // Initialize Pico W WiFi for LED control
    if (cyw43_arch_init()) {
        printf("Failed to initialise CYW43\n");
        return 1;
    }

    // Blink LED at start to indicate program is running (Pico W)
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(200);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(200);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
#else
    // Initialize LED (regular Pico)
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Blink LED at start to indicate program is running (regular Pico)
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(200);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(200);

    // UART output - Program start
    printf("UART: Program starting - ARM Semihosting Test\n");

    gpio_put(PICO_DEFAULT_LED_PIN, 1);
#endif

    semihost_printf("=== ARM Semihosting Test ===\n");
    semihost_printf("This message appears in the debugger console.\n");
    semihost_printf("Counter test: ");
    for (int i = 0; i < 5; i++) {
        semihost_printf("%d", i);
    }
    semihost_printf("\n");

    // UART output - Program end
    printf("UART: Program completed successfully\n");

    // Wait 5 seconds before turning off LED
    sleep_ms(5000);

#ifdef CYW43_WL_GPIO_LED_PIN
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    cyw43_arch_deinit();
#else
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
#endif

    return 0;
}
