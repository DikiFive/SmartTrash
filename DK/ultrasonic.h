#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H
#include "dk_C8T6.h" // Device header

/***************根据自己需求更改****************/
// ULTRASONIC GPIO宏定义

#define ULTRASONIC_GPIO_CLK      RCC_APB2Periph_GPIOA
#define ULTRASONIC_GPIO_PORT     GPIOA
#define ULTRASONIC_TRIG_GPIO_PIN GPIO_Pin_2
#define ULTRASONIC_ECHO_GPIO_PIN GPIO_Pin_3

#define TRIG_Send                PAout(2)
#define ECHO_Reci                PAin(3)

/*********************END**********************/

void Ultrasonic_Init(void);
float UltrasonicGetLength(void);

void OpenTimerForHc(void);
void CloseTimerForHc(void);
u32 GetEchoTimer(void);

#endif /* __ADC_H */
