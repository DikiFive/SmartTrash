#include "DK_C8T6.h" // Device header

int main(void)
{
    Sys_Init(); // Initialize system clock and peripherals
    while (1) {
        LED_Sys_ON(); // Turn on system LED
    }
}
