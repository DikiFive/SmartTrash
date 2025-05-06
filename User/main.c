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
    Sys_Init();              // 初始化外设和GPIO
    InitTrashSystem();       // 初始化垃圾桶系统状态
    uint8_t distance    = 0; // 超声波测距值
    uint8_t smoke_value = 0; // 烟雾传感器值

    while (1) {

        distance = sonar_mm(); // 获取超声波测距值
        if (distance < 50) {
            Servo_SetAngle(75.0f); // 打开垃圾桶盖
        } else {
            Servo_SetAngle(0.0f); // 关闭垃圾桶盖
        }

        OLED_ShowNum(0, 0, distance, 3, OLED_8X16); // 显示测距值
        OLED_ShowString(0, 16, "mm", OLED_8X16);    // 显示单位
        OLED_Update();                              // 更新OLED显示
        Delay_ms(100);                              // 延时100毫秒
    }
}
