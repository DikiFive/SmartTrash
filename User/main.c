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

/* 常量定义 */
#define SMOKE_THRESHOLD_ADC 3722 // 烟雾传感器阈值(3V对应的ADC值 = 3.0/3.3 * 4095)
#define CLEANUP_TIMEOUT_S   180  // 清理超时时间(3分钟)

/* 全局变量 */
static uint32_t last_cleanup_time   = 0; // 上次清理时间(系统运行秒数)
static uint8_t trash_status         = 0; // 垃圾桶状态: 0-空, 1-有垃圾, 2-已满
static uint8_t display_needs_update = 1; // OLED显示更新标志
static uint8_t time_overflow        = 0; // 时间溢出标志

int main(void)
{
    /* 系统初始化 */
    Sys_Init();                           // 初始化系统时钟、GPIO等
    last_cleanup_time = system_runtime_s; // 初始化清理时间为当前系统时间

    /* OLED显示初始化 */
    OLED_Clear(); // 清屏
    OLED_Update();
    while (1) {
        /* 读取红外传感器状态 */
        uint8_t bottom_sensor = Sensor_GetValue();  // 底部传感器
        uint8_t top_sensor    = Sensor_GetValue2(); // 顶部传感器

        /* 判断垃圾桶状态并设置LED/蜂鸣器 */
        uint8_t old_status = trash_status;
        if (bottom_sensor && top_sensor) {
            trash_status = 0;                     // 垃圾桶空
            LED1_ON();                            // 绿灯亮-表示空
            LED2_OFF();                           // 黄灯灭
            LED_Sys_OFF();                        // 红灯灭
            Buzzer_OFF();                         // 蜂鸣器关闭
            last_cleanup_time = system_runtime_s; // 重置清理计时器
        } else if (!bottom_sensor && top_sensor) {
            trash_status = 1; // 有垃圾
            LED1_OFF();       // 绿灯灭
            LED2_ON();        // 黄灯亮-表示有垃圾
            LED_Sys_OFF();    // 红灯灭
            Buzzer_OFF();     // 蜂鸣器关闭
        } else if (!bottom_sensor && !top_sensor) {
            trash_status = 2; // 垃圾桶已满
            LED1_OFF();       // 绿灯灭
            LED2_OFF();       // 黄灯灭
            LED_Sys_ON();     // 红灯亮-表示满
            Buzzer_ON();      // 蜂鸣器报警
        }

        // 状态改变时需要更新显示
        if (old_status != trash_status) {
            display_needs_update = 1;
        }

        /* 检测烟雾 */
        uint16_t smoke_value = MQ2_GetData();
        if (smoke_value >= SMOKE_THRESHOLD_ADC) {
            LED_Sys_ON(); // 红灯亮-表示检测到烟雾
            Buzzer_ON();  // 蜂鸣器报警
        }

        /* 检查清理超时 */
        uint32_t current_time = system_runtime_s;
        uint32_t time_since_cleanup;

        // 处理时间溢出情况
        if (current_time < last_cleanup_time) {
            time_overflow = 1;
        }

        if (time_overflow) {
            time_since_cleanup = (UINT32_MAX - last_cleanup_time) + current_time;
        } else {
            time_since_cleanup = current_time - last_cleanup_time;
        }

        if (time_since_cleanup >= CLEANUP_TIMEOUT_S && trash_status != 0) {
            LED_Sys_ON();             // 红灯亮-表示需要清理
            Buzzer_ON();              // 蜂鸣器报警
            display_needs_update = 1; // 需要更新显示时间
        }

        /* 更新OLED显示 */
        static uint32_t last_display_time = 0;
        // 每秒更新一次时间显示
        if (system_runtime_s != last_display_time) {
            display_needs_update = 1;
            last_display_time    = system_runtime_s;
        }

        // 仅在需要时更新显示
        if (display_needs_update) {
            OLED_Clear();

            /* 显示垃圾桶状态 */
            OLED_ShowString(0, 0, "垃圾:", OLED_8X16);
            if (trash_status == 0) {
                OLED_ShowString(40, 0, "空", OLED_8X16);
            } else if (trash_status == 1) {
                OLED_ShowString(40, 0, "有", OLED_8X16);
            } else {
                OLED_ShowString(40, 0, "满", OLED_8X16);
            }

            /* 显示未清理时间 */
            OLED_ShowString(0, 16, "Time:", OLED_8X16);
            uint16_t minutes = time_since_cleanup / 60;
            uint16_t seconds = time_since_cleanup % 60;

            // 显示分钟，确保至少显示两位数
            if (minutes < 10) {
                OLED_ShowString(40, 16, "0", OLED_8X16);
                OLED_ShowNum(48, 16, minutes, 1, OLED_8X16);
            } else {
                OLED_ShowNum(40, 16, minutes, 2, OLED_8X16);
            }
            OLED_ShowString(56, 16, ":", OLED_8X16);

            // 显示秒数，确保至少显示两位数
            if (seconds < 10) {
                OLED_ShowString(64, 16, "0", OLED_8X16);
                OLED_ShowNum(72, 16, seconds, 1, OLED_8X16);
            } else {
                OLED_ShowNum(64, 16, seconds, 2, OLED_8X16);
            }

            OLED_Update();
            display_needs_update = 0; // 重置更新标志
        }

        Delay_ms(50); // 短暂延时,降低CPU占用
    }
}
