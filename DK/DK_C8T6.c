#include "DK_C8T6.h"

/* 常量定义 */
#define SMOKE_THRESHOLD_ADC 3722 // 烟雾传感器阈值(3V对应的ADC值 = 3.0/3.3 * 4095)
#define CLEANUP_TIMEOUT_S   180  // 清理超时时间(3分钟)

/* 全局变量 */
static uint32_t last_cleanup_time   = 0; // 上次清理时间(系统运行秒数)
static uint8_t trash_status         = 0; // 垃圾桶状态: 0-空, 1-有垃圾, 2-已满
static uint8_t display_needs_update = 1; // OLED显示更新标志
static uint8_t time_overflow        = 0; // 时间溢出标志

void Sys_Init(void)
{
    ADCx_Init(ADC1); // Initialize ADC1
    OLED_Init();     // Initialize OLED display
    LED_All_Init();  // Initialize LED
    Servo_Init();    // Initialize servo motor
    // Buzzer_Init();     // Initialize buzzer
    usart1_Init(9600);  // Initialize USART1 with baud rate 9600
    CountSensor_Init(); // Initialize red infrared sensors

    MQ2_Init();     // Initialize MQ2 smoke sensor
    Timer_Init();   // Initialize timer for tracking
    HC_SR04_Init(); // Initialize ultrasonic sensor
}

void InitTrashSystem(void)
{
    last_cleanup_time    = system_runtime_s;
    trash_status         = 0;
    display_needs_update = 1;
    time_overflow        = 0;

    OLED_Clear();
    OLED_Update();
}

void ProcessSensorData(void)
{
    uint8_t bottom_sensor = Sensor_GetValue();  // 底部传感器
    uint8_t top_sensor    = Sensor_GetValue2(); // 顶部传感器
    uint8_t old_status    = trash_status;

    if (bottom_sensor && top_sensor) {
        trash_status      = 0;                // 垃圾桶空
        last_cleanup_time = system_runtime_s; // 重置清理计时器
    } else if (!bottom_sensor && top_sensor) {
        trash_status = 1; // 有垃圾
    } else if (!bottom_sensor && !top_sensor) {
        trash_status = 2; // 垃圾桶已满
    }

    if (old_status != trash_status) {
        display_needs_update = 1;
    }
}

void CheckSmoke(void)
{
    uint16_t smoke_value = MQ2_GetData();
    if (smoke_value >= SMOKE_THRESHOLD_ADC) {
        LED_Sys_ON(); // 红灯亮-表示检测到烟雾
        Buzzer_ON();  // 蜂鸣器报警
    }
}

void CheckCleanupTimeout(void)
{
    uint32_t current_time = system_runtime_s;
    uint32_t time_since_cleanup;

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
}

void UpdateStatusIndicators(void)
{
    switch (trash_status) {
        case 0:            // 空
            LED1_ON();     // 绿灯亮
            LED2_OFF();    // 黄灯灭
            LED_Sys_OFF(); // 红灯灭
            Buzzer_OFF();  // 蜂鸣器关闭
            break;
        case 1:            // 有垃圾
            LED1_OFF();    // 绿灯灭
            LED2_ON();     // 黄灯亮
            LED_Sys_OFF(); // 红灯灭
            Buzzer_OFF();  // 蜂鸣器关闭
            break;
        case 2:           // 已满
            LED1_OFF();   // 绿灯灭
            LED2_OFF();   // 黄灯灭
            LED_Sys_ON(); // 红灯亮
            Buzzer_ON();  // 蜂鸣器报警
            break;
    }
}

void UpdateOLEDDisplay(void)
{
    static uint32_t last_display_time = 0;
    if (system_runtime_s != last_display_time) {
        display_needs_update = 1;
        last_display_time    = system_runtime_s;
    }

    if (display_needs_update) {
        uint32_t current_time = system_runtime_s;
        uint32_t time_since_cleanup;

        if (time_overflow) {
            time_since_cleanup = (UINT32_MAX - last_cleanup_time) + current_time;
        } else {
            time_since_cleanup = current_time - last_cleanup_time;
        }

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
        display_needs_update = 0;
    }
}
