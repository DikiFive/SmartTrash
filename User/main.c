/**
 * @file main.c
 * @brief 智能垃圾桶主程序
 * @details 实现以下功能:
 *          1. 通过红外传感器检测垃圾桶空满状态
 *          2. 通过MQ2传感器检测烟雾
 *          3. 通过LED和蜂鸣器进行状态/警报提示
 *          4. 通过OLED显示垃圾桶状态和未清理时间
 *          5. 显示串口1接收到的数据
 */

#include "DK_C8T6.h"
#include "usart1.h"
#include "OLED.h"
#include <stdint.h>

#define WINDOW_SIZE       5 // 滑动平均窗口大小
#define TRIGGER_THRESHOLD 3 // 连续检测次数阈值

uint8_t distance_readings[WINDOW_SIZE]; // 存储最近的测量值
uint8_t reading_index = 0;              // 当前存储位置索引
uint8_t trigger_count = 0;              // 连续触发计数器

// 函数：计算滑动平均距离
uint8_t calculate_average_distance(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sum += distance_readings[i];
    }
    return sum / WINDOW_SIZE;
}

int main(void)
{
    /* 系统初始化 */
    Sys_Init();        // 初始化外设和GPIO
    InitTrashSystem(); // 初始化垃圾桶系统状态
    usart1_Init(9600); // 初始化串口1，波特率9600
    OLED_Init();       // 初始化OLED显示

    // 初始化距离读取数组
    for (int i = 0; i < WINDOW_SIZE; i++) {
        distance_readings[i] = 0;
    }

    uint8_t raw_distance = 0; // 超声波原始测距值
    uint8_t avg_distance = 0; // 超声波平均测距值
    uint8_t smoke_value  = 0; // 烟雾传感器值

    // 清屏并显示初始提示信息
    OLED_Clear();+
    OLED_ShowString(0, 0, "UART1:", OLED_8X16);
    OLED_Update();

    while (1) {
        // 检查串口1是否接收到新数据
        if (USART1_RX_STA == 1)
        {
            // 在OLED上显示接收到的数据
            OLED_ShowChar(48, 0, USART1_RX_CMD, OLED_8X16); // 在"UART1:"后面显示接收到的字符
            OLED_Update(); // 更新OLED显示
            USART1_RX_STA = 0; // 清除接收标志，准备接收下一个数据
        }

        // raw_distance = sonar_mm(); // 获取超声波原始测距值

        // // 将新的原始数据存入滑动窗口
        // distance_readings[reading_index] = raw_distance;
        // reading_index                    = (reading_index + 1) % WINDOW_SIZE; // 更新索引，实现循环存储

        // // 计算滑动平均距离
        // avg_distance = calculate_average_distance();

        // // 使用平均距离进行逻辑判断，增加连续触发检测
        // if (avg_distance < 150) {
        //     if (trigger_count < TRIGGER_THRESHOLD) {
        //         trigger_count++;
        //     }
        //     // 只有连续触发次数达到阈值才打开盖子
        //     if (trigger_count >= TRIGGER_THRESHOLD) {
        //         Servo_SetAngle(75.0f); // 打开垃圾桶盖
        //     }
        // } else {
        //     trigger_count = 0;    // 重置触发计数器
        //     Servo_SetAngle(0.0f); // 关闭垃圾桶盖
        // }

        // ProcessSensorData();                             // 处理传感器数据
        // CheckSmoke();                                    // 检测烟雾
        // CheckCleanupTimeout();                           // 检查清理超时
        // UpdateStatusIndicators();                        // 更新LED和蜂鸣器状态
        // UpdateOLEDDisplay();                             // 更新OLED显示
        // smoke_value = MQ2_GetData_PPM();                 // 获取烟雾传感器值
        // OLED_ShowNum(0, 48, smoke_value, 4, OLED_8X16);  // 显示烟雾传感器值-
        // OLED_ShowNum(0, 32, avg_distance, 3, OLED_8X16); // 显示测距值
        // OLED_Update();                                   // 更新OLED显示
        // Delay_ms(50);
    }
}
