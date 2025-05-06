/**
 * @file main.c
 * @brief 智能垃圾桶主程序
 * @details 实现以下功能:
 *          1. 红外传感器检测垃圾桶空满状态，并通过LED和蜂鸣器指示
 *          2. 超声波检测人体靠近，自动开关盖
 *          3. 3分钟未清理报警提示
 *          4. 语音控制开关盖
 *          5. 烟雾检测报警
 *          6. OLED显示系统状态
 */

#include "DK_C8T6.h"

int main(void)
{
    /* 系统初始化 */
    Sys_Init();        // 初始化外设和GPIO
    InitTrashSystem(); // 初始化垃圾桶系统状态和状态变量
    // DS1302_SetTime(2025, 5, 6, 20, 25, 0, 2); // 年,月,日,时,分,秒,星期
    // 校准时间才使用，校准后注释掉，再重新编译下载
    while (1) {
        ProcessSerialCommands();  // 处理串口命令（如语音控制）
        HandleUltrasonicSensor(); // 处理超声波传感器和自动开关盖
        ProcessSensorData();      // 处理红外传感器数据
        CheckSmoke();             // 检测烟雾
        CheckCleanupTimeout();    // 检查清理超时
        UpdateStatusIndicators(); // 更新LED和蜂鸣器状态
        UpdateOLEDDisplay();      // 更新OLED显示（包括烟雾值和距离）
    }
}
