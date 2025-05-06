#include "DK_C8T6.h"

/* 常量定义 */
#define SMOKE_THRESHOLD_PPM 300  // 烟雾浓度报警阈值(PPM)
#define CLEANUP_TIMEOUT_S   10   // 清理超时时间(3分钟)
#define WINDOW_SIZE         5    // 滑动平均窗口大小
#define TRIGGER_THRESHOLD   3    // 连续检测次数阈值
#define CLOSE_DISTANCE      50   // 垃圾桶盖打开距离(厘米)
#define CLOSE_DELAY_MS      1000 // 垃圾桶盖关闭延迟时间(毫秒)

/* 全局变量 */
static uint32_t last_cleanup_time   = 0; // 上次清理时间(系统运行秒数)
static uint8_t trash_status         = 0; // 垃圾桶状态: 0-空, 1-有垃圾, 2-已满
static uint8_t display_needs_update = 1; // OLED显示更新标志
static uint8_t time_overflow        = 0; // 时间溢出标志
static uint8_t smoke_alert_active   = 0; // 烟雾报警激活标志
static uint8_t cleanup_alert_active = 0; // 清理超时报警激活标志

/* 超声波相关变量 */
static uint8_t distance_readings[WINDOW_SIZE]; // 存储最近的测量值
static uint8_t reading_index         = 0;      // 当前存储位置索引
static uint8_t trigger_count         = 0;      // 连续触发计数器
static uint32_t lid_close_time       = 0;      // 垃圾桶盖预定关闭时间
static uint8_t lid_closing_scheduled = 0;      // 垃圾桶盖是否在等待关闭

// 函数：计算滑动平均距离
static uint8_t calculate_average_distance(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sum += distance_readings[i];
    }
    return sum / WINDOW_SIZE;
}

void HandleUltrasonicSensor(void)
{
    uint8_t raw_distance = sonar_mm(); // 获取超声波原始测距值

    // 将新的原始数据存入滑动窗口
    distance_readings[reading_index] = raw_distance;
    reading_index                    = (reading_index + 1) % WINDOW_SIZE; // 更新索引，实现循环存储

    // 计算滑动平均距离
    uint8_t avg_distance = calculate_average_distance();

    // 使用平均距离进行逻辑判断，增加连续触发检测
    if (avg_distance < CLOSE_DISTANCE) { // 调整触发距离
        if (trigger_count < TRIGGER_THRESHOLD) {

            trigger_count++;
        }
        // 只有连续触发次数达到阈值才打开盖子
        if (trigger_count >= TRIGGER_THRESHOLD) {
            Servo_SetAngle(75.0f);     // 打开垃圾桶盖
            lid_closing_scheduled = 0; // 取消已计划的关盖
        }
    } else {
        if (trigger_count >= TRIGGER_THRESHOLD) { // 之前是打开状态
            if (!lid_closing_scheduled) {
                // 设置延迟关闭时间
                lid_close_time        = system_runtime_ms + CLOSE_DELAY_MS;
                lid_closing_scheduled = 1;
            }
        }
        trigger_count = 0; // 重置触发计数器
    }

    // 检查是否需要关闭垃圾桶盖
    if (lid_closing_scheduled && system_runtime_ms >= lid_close_time) {
        Servo_SetAngle(0.0f); // 关闭垃圾桶盖
        lid_closing_scheduled = 0;
    }
}

void ProcessSerialCommands(void)
{
    if (USART1_NewCmd) {
        if (USART1_RX_CMD == 0x11) {
            Servo_SetAngle(75.0f); // 打开垃圾桶盖
        } else if (USART1_RX_CMD == 0x22) {
            Servo_SetAngle(0.0f); // 关闭垃圾桶盖
        }
        USART1_NewCmd = 0; // 清除命令标志位
    }
}

void Sys_Init(void)
{
    ADCx_Init(ADC1);    // Initialize ADC1
    OLED_Init();        // Initialize OLED display
    LED_All_Init();     // Initialize LED
    Servo_Init();       // Initialize servo motor
    Buzzer_Init();      // Initialize buzzer
    usart1_Init(9600);  // Initialize USART1 with baud rate 9600
    UART3_Init(9600);   // 初始化串口3
    CountSensor_Init(); // Initialize red infrared sensors
    MQ2_Init();         // Initialize MQ2 smoke sensor
    Timer_Init();       // Initialize timer for tracking
    HC_SR04_Init();     // Initialize ultrasonic sensor
    DS1302_GPIO_Init(); // Initialize DS1302 (time kept by backup battery)
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

    // 红外传感器返回值：0-被遮挡，1-未遮挡
    if (bottom_sensor && top_sensor) {
        trash_status      = 0;                // 两个传感器都未被遮挡，垃圾桶空
        last_cleanup_time = system_runtime_s; // 重置清理计时器
    } else if (!bottom_sensor && top_sensor) {
        trash_status = 1; // 底部被遮挡，有垃圾
    } else if (bottom_sensor && !top_sensor) {
        trash_status = 1; // 顶部被遮挡，有垃圾
    } else if (!bottom_sensor && !top_sensor) {
        trash_status = 2; // 两个传感器都被遮挡，垃圾桶已满
    }

    if (old_status != trash_status) {
        display_needs_update = 1;
    }
}

void CheckSmoke(void)
{
    uint16_t smoke_ppm_value = MQ2_GetData_PPM();                        // 获取PPM值
    smoke_alert_active       = (smoke_ppm_value >= SMOKE_THRESHOLD_PPM); // 与PPM阈值比较
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

    // 仅在垃圾桶非空时检查超时
    if (trash_status != 0) {
        cleanup_alert_active = (time_since_cleanup >= CLEANUP_TIMEOUT_S);
    } else {
        cleanup_alert_active = 0;
    }
    display_needs_update = 1; // 需要更新显示时间
}

void UpdateStatusIndicators(void)
{
    // 优先级处理：烟雾报警 > 清理超时报警 > 垃圾桶满报警 > 正常状态指示
    if (smoke_alert_active) {
        // 烟雾报警最高优先级
        LED1_OFF();   // 绿灯灭
        LED2_OFF();   // 黄灯灭
        LED_Sys_ON(); // 红灯亮
        Buzzer_ON();  // 蜂鸣器报警
    } else if (cleanup_alert_active) {
        // 清理超时报警次优先级
        LED1_OFF();   // 绿灯灭
        LED2_OFF();   // 黄灯灭
        LED_Sys_ON(); // 红灯亮
        Buzzer_ON();  // 蜂鸣器报警
    } else {
        // 正常状态指示
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

        /* 显示垃圾桶状态和距离 */
        OLED_ShowString(0, 0, "垃圾:", OLED_8X16);
        if (trash_status == 0) {
            OLED_ShowString(40, 0, "空", OLED_8X16);
        } else if (trash_status == 1) {
            OLED_ShowString(40, 0, "有", OLED_8X16);
        } else {
            OLED_ShowString(40, 0, "满", OLED_8X16);
        }

        /* 显示距离 */
        uint8_t avg_distance = calculate_average_distance();
        OLED_ShowString(80, 0, "D:", OLED_8X16);
        OLED_ShowNum(96, 0, avg_distance / 10, 3, OLED_8X16);

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

        /* 显示实时日期和时间 */
        DS1302_read_realTime();

        // 显示年月日
        OLED_ShowNum(0, 32, TimeData.year, 4, OLED_8X16);
        OLED_ShowString(32, 32, "/", OLED_8X16);
        OLED_ShowNum(40, 32, TimeData.month, 2, OLED_8X16);
        OLED_ShowString(56, 32, "/", OLED_8X16);
        OLED_ShowNum(64, 32, TimeData.day, 2, OLED_8X16);

        // 显示时分秒
        OLED_ShowNum(0, 48, TimeData.hour, 2, OLED_8X16);
        OLED_ShowString(16, 48, ":", OLED_8X16);
        OLED_ShowNum(24, 48, TimeData.minute, 2, OLED_8X16);
        OLED_ShowString(40, 48, ":", OLED_8X16);
        OLED_ShowNum(48, 48, TimeData.second, 2, OLED_8X16);

        /* 显示烟雾值 */
        OLED_ShowString(72, 48, "P:", OLED_8X16);
        uint16_t smoke_ppm = MQ2_GetData_PPM();
        OLED_ShowNum(88, 48, smoke_ppm, 4, OLED_8X16);

        OLED_Update();
        display_needs_update = 0;
    }
}
