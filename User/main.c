/**
 * @file main.c
 * @brief 智能垃圾桶主程序
 * @details 实现以下功能:
 *          1. 通过红外传感器检测垃圾桶空满状态
 *          2. 通过MQ2传感器检测烟雾
 *          3. 通过LED和蜂鸣器进行状态/警报提示
 *          4. 通过OLED显示垃圾桶状态和未清理时间
 */

#include "DK_C8T6.h"

int main(void)
{
    /* 系统初始化 */
    Sys_Init();           // 初始化外设和GPIO
    InitTrashSystem();    // 初始化垃圾桶系统状态
    uint8_t distance = 0; // 超声波测距值

    while (1) {

        Servo_SetAngle(0);  // 设置舵机角度为0度
        Delay_ms(1000);     // 延时1秒，等待舵机到达目标位置
        Servo_SetAngle(75);  // 设置舵机角度为75度
        Delay_ms(1000);     // 延时1秒，等待舵机到达目标位置
        ProcessSensorData();                         // 处理传感器数据
        CheckSmoke();                                // 检测烟雾
        CheckCleanupTimeout();                       // 检查清理超时
        UpdateStatusIndicators();                    // 更新LED和蜂鸣器状态
        UpdateOLEDDisplay();                         // 更新OLED显示
        distance = sonar_mm();                       // 获取测距值
        OLED_ShowNum(0, 32, distance, 3, OLED_8X16); // 显示测距值
        OLED_Update();                               // 更新OLED显示
        Delay_ms(50);                                // 短暂延时,降低CPU占用
    }
}
