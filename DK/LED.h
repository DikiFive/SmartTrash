/**
 * @file     LED.h
 * @brief    LED驱动程序头文件
 * @details  声明了所有LED控制函数接口，包括：
 *          1. LED1 (PA8)
 *          2. LED2 (PA12)
 *          3. 系统LED (PC13)
 * @author   DikiFive
 * @date     2025-04-30
 * @version  v1.1
 */

#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h" // STM32F10x外设库头文件

// LED1 (PA8) 定义
#define LED1_GPIO_PORT GPIOA
#define LED1_GPIO_PIN  GPIO_Pin_8
#define LED1_GPIO_CLK  RCC_APB2Periph_GPIOA

// LED2 (PA12) 定义
#define LED2_GPIO_PORT GPIOA
#define LED2_GPIO_PIN  GPIO_Pin_12
#define LED2_GPIO_CLK  RCC_APB2Periph_GPIOA

// 系统LED (PC13) 定义
#define LED_SYS_GPIO_PORT GPIOC
#define LED_SYS_GPIO_PIN  GPIO_Pin_13
#define LED_SYS_GPIO_CLK  RCC_APB2Periph_GPIOC

/**
 * @brief  初始化所有LED
 * @details 配置PA8, PA12, PC13为推挽输出模式
 * @param  无
 * @return 无
 */
void LED_All_Init(void);

/**
 * @brief  LED1控制函数组
 * @note   控制PA8引脚
 */
void LED1_ON(void);   /**< 打开LED1 */
void LED1_OFF(void);  /**< 关闭LED1 */
void LED1_Turn(void); /**< 翻转LED1状态 */

/**
 * @brief  LED2控制函数组
 * @note   控制PA12引脚
 */
void LED2_ON(void);   /**< 打开LED2 */
void LED2_OFF(void);  /**< 关闭LED2 */
void LED2_Turn(void); /**< 翻转LED2状态 */

/**
 * @brief  系统LED控制函数组
 * @note   控制PC13引脚
 */
void LED_Sys_ON(void);   /**< 打开系统LED */
void LED_Sys_OFF(void);  /**< 关闭系统LED */
void LED_Sys_Turn(void); /**< 翻转系统LED状态 */

#endif /* __LED_H */
