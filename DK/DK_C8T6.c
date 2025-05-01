#include "DK_C8T6.h"

void Sys_Init(void)
{
    OLED_Init();            // Initialize OLED display
    LED_All_Init();         // Initialize LED
    Buzzer_Init();          // Initialize buzzer
    usart1_Init(9600);      // Initialize USART1 with baud rate 9600
    CountSensor_Init();     // Initialize ultrasonic sensor
    MQ2_Init();             // Initialize MQ2 sensor
    DS1302_GPIO_Init();     // Initialize DS1302 GPIO
    DS1302_Init();          // Initialize DS1302
    DS1302_read_realTime(); // Read real-time data from DS1302
}