/**
 * @file     LED.c
 * @brief    LED驱动程序
 * @details  实现三组LED的控制功能：
 *          1. LED1（PA8）
 *          2. LED2（PA12）
 *          3. 系统LED（PC13）
 * @author   DikiFive
 * @date     2025-04-30
 * @version  v1.1
 */

#include "stm32f10x.h" // STM32F10x外设库头文件
#include "dk_C8T6.h"   // 项目主头文件
#include "LED.h"       // LED驱动程序头文件

/**
 * @brief  初始化所有LED
 * @details 配置PA8, PA12, PC13为推挽输出，并设置默认状态为关闭
 * @param  无
 * @return 无
 */
void LED_All_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启时钟*/
    RCC_APB2PeriphClockCmd(LED1_GPIO_CLK | LED_SYS_GPIO_CLK, ENABLE); // 开启GPIOA和GPIOC的时钟

    /*GPIO初始化 LED1 (PA8)*/
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = LED1_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure); // 将PA8引脚初始化为推挽输出

    /*GPIO初始化 LED2 (PA12)*/
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = LED2_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure); // 将PA12引脚初始化为推挽输出

    /*GPIO初始化 系统LED (PC13)*/
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = LED_SYS_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_SYS_GPIO_PORT, &GPIO_InitStructure); // 将PC13引脚初始化为推挽输出

    /*设置GPIO初始化后的默认电平（关闭LED）*/
    GPIO_SetBits(LED1_GPIO_PORT, LED1_GPIO_PIN);       // 设置PA8引脚为高电平
    GPIO_SetBits(LED2_GPIO_PORT, LED2_GPIO_PIN);       // 设置PA12引脚为高电平
    GPIO_SetBits(LED_SYS_GPIO_PORT, LED_SYS_GPIO_PIN); // 设置PC13引脚为高电平
}

/**
 * @brief  打开LED1
 * @details 将PA8设置为低电平，点亮LED1
 * @param  无
 * @return 无
 */
void LED1_ON(void)
{
    GPIO_ResetBits(LED1_GPIO_PORT, LED1_GPIO_PIN); // 设置PA8引脚为低电平
}

/**
 * @brief  关闭LED1
 * @details 将PA8设置为高电平，熄灭LED1
 * @param  无
 * @return 无
 */
void LED1_OFF(void)
{
    GPIO_SetBits(LED1_GPIO_PORT, LED1_GPIO_PIN); // 设置PA8引脚为高电平
}

/**
 * @brief  翻转LED1状态
 * @details 如果LED1当前是亮的则熄灭，如果是熄灭的则点亮
 * @param  无
 * @return 无
 */
void LED1_Turn(void)
{
    if (GPIO_ReadOutputDataBit(LED1_GPIO_PORT, LED1_GPIO_PIN) == 0) // 获取输出寄存器的状态，如果当前引脚输出低电平
    {
        GPIO_SetBits(LED1_GPIO_PORT, LED1_GPIO_PIN); // 则设置PA8引脚为高电平
    } else                                           // 否则，即当前引脚输出高电平
    {
        GPIO_ResetBits(LED1_GPIO_PORT, LED1_GPIO_PIN); // 则设置PA8引脚为低电平
    }
}

/**
 * @brief  打开LED2
 * @details 将PA12设置为低电平，点亮LED2
 * @param  无
 * @return 无
 */
void LED2_ON(void)
{
    GPIO_ResetBits(LED2_GPIO_PORT, LED2_GPIO_PIN); // 设置PA12引脚为低电平
}

/**
 * @brief  关闭LED2
 * @details 将PA12设置为高电平，熄灭LED2
 * @param  无
 * @return 无
 */
void LED2_OFF(void)
{
    GPIO_SetBits(LED2_GPIO_PORT, LED2_GPIO_PIN); // 设置PA12引脚为高电平
}

/**
 * @brief  翻转LED2状态
 * @details 如果LED2当前是亮的则熄灭，如果是熄灭的则点亮
 * @param  无
 * @return 无
 */
void LED2_Turn(void)
{
    if (GPIO_ReadOutputDataBit(LED2_GPIO_PORT, LED2_GPIO_PIN) == 0) // 获取输出寄存器的状态，如果当前引脚输出低电平
    {
        GPIO_SetBits(LED2_GPIO_PORT, LED2_GPIO_PIN); // 则设置PA12引脚为高电平
    } else                                           // 否则，即当前引脚输出高电平
    {
        GPIO_ResetBits(LED2_GPIO_PORT, LED2_GPIO_PIN); // 则设置PA12引脚为低电平
    }
}

/**
 * @brief  打开系统LED
 * @details 将PC13设置为低电平，点亮系统LED
 * @param  无
 * @return 无
 */
void LED_Sys_ON(void)
{
    GPIO_ResetBits(LED_SYS_GPIO_PORT, LED_SYS_GPIO_PIN); // 设置PC13引脚为低电平
}

/**
 * @brief  关闭系统LED
 * @details 将PC13设置为高电平，熄灭系统LED
 * @param  无
 * @return 无
 */
void LED_Sys_OFF(void)
{
    GPIO_SetBits(LED_SYS_GPIO_PORT, LED_SYS_GPIO_PIN); // 设置PC13引脚为高电平
}

/**
 * @brief  翻转系统LED状态
 * @details 如果系统LED当前是亮的则熄灭，如果是熄灭的则点亮
 * @param  无
 * @return 无
 */
void LED_Sys_Turn(void)
{
    if (GPIO_ReadOutputDataBit(LED_SYS_GPIO_PORT, LED_SYS_GPIO_PIN) == 0) // 获取输出寄存器的状态，如果当前引脚输出低电平
    {
        GPIO_SetBits(LED_SYS_GPIO_PORT, LED_SYS_GPIO_PIN); // 则设置PC13引脚为高电平
    } else                                                 // 否则，即当前引脚输出高电平
    {
        GPIO_ResetBits(LED_SYS_GPIO_PORT, LED_SYS_GPIO_PIN); // 则设置PC13引脚为低电平
    }
}
