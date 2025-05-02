#ifndef __HC_SR04_H
#define __HC_SR04_H
#include "dk_C8T6.h" // Device header

/* 声速计算相关参数 */
#define ULTRASONIC_TEMPERATURE 25    // 默认温度25℃
#define ULTRASONIC_HUMIDITY    40    // 默认湿度40%
#define SOUND_SPEED_BASE       331.4 // 声速基准值(0℃时)

/* ULTRASONIC GPIO宏定义 */
#define ULTRASONIC_GPIO_CLK  RCC_APB2Periph_GPIOA
#define ULTRASONIC_GPIO_PORT GPIOA
#define TRIG_GPIO_PIN        GPIO_Pin_2
#define ECHO_GPIO_PIN        GPIO_Pin_3

#define TRIG_Send            PAout(2)
#define ECHO_Reci            PAin(3)

void HC_SR04_Init(void);
int16_t sonar_mm(void);
float sonar(void);

#endif /* __HC_SR04_H */
