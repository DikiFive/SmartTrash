#include "DK_C8T6.h" // Device header

// 状态定义
#define TRASH_EMPTY 0
#define TRASH_SOME  1
#define TRASH_FULL  2

// 阈值定义
#define DISTANCE_THRESHOLD 25.0f  // 人体检测距离阈值(cm)
#define SMOKE_THRESHOLD   100     // 烟雾报警阈值
#define CLEAN_TIMEOUT    180000   // 清理超时时间(3分钟 = 180000ms)

// 全局变量
static uint8_t trash_state = TRASH_EMPTY;  // 垃圾桶状态
static uint8_t lid_opened = 0;             // 垃圾桶盖状态
static uint32_t lid_close_timer = 0;       // 关盖延时计时器
static uint32_t clean_timer = 0;           // 清理计时器
static uint8_t clean_alarm = 0;            // 清理提醒标志

// 声明主要功能函数
void updateTrashState(void);               // 更新垃圾状态
void controlLidByDistance(void);           // 距离控制开关盖
void checkCleanTimeout(void);              // 检查清理超时
void checkSmokeLevel(void);               // 检查烟雾浓度
void updateDisplay(void);                  // 更新显示
void processVoiceCommand(void);            // 处理语音命令

// 功能实现：更新垃圾状态
void updateTrashState(void)
{
    uint8_t sensor_low = Sensor_GetValue();    // 下方传感器
    uint8_t sensor_high = Sensor_GetValue2();  // 上方传感器

    if(!sensor_low && !sensor_high) {          // 两个传感器都未被遮挡
        trash_state = TRASH_EMPTY;
        LED1_ON();    // 绿灯
        LED2_OFF();   // 黄灯关
        LED_Sys_OFF();// 红灯关
        Buzzer_OFF(); // 关闭蜂鸣器
    }
    else if(sensor_low && !sensor_high) {      // 只有下方传感器被遮挡
        trash_state = TRASH_SOME;
        LED1_OFF();   // 绿灯关
        LED2_ON();    // 黄灯
        LED_Sys_OFF();// 红灯关
        Buzzer_OFF(); // 关闭蜂鸣器
    }
    else if(sensor_low && sensor_high) {       // 两个传感器都被遮挡
        trash_state = TRASH_FULL;
        LED1_OFF();   // 绿灯关
        LED2_OFF();   // 黄灯关
        LED_Sys_ON(); // 红灯
        Buzzer_ON();  // 开启蜂鸣器报警
    }
}

// 功能实现：距离控制开关盖
void controlLidByDistance(void)
{
    float distance = UltrasonicGetLength();

    if(distance < DISTANCE_THRESHOLD && !lid_opened) {
        Servo_SetAngle(90.0f);  // 打开垃圾桶盖
        lid_opened = 1;
        lid_close_timer = 0;    // 重置关盖计时器
    }
    else if(distance >= DISTANCE_THRESHOLD && lid_opened) {
        lid_close_timer++;
        if(lid_close_timer >= 2000) {  // 2秒后关闭
            Servo_SetAngle(0.0f);   // 关闭垃圾桶盖
            lid_opened = 0;
            lid_close_timer = 0;
        }
    }
}

// 功能实现：检查清理超时
void checkCleanTimeout(void)
{
    if(trash_state != TRASH_EMPTY) {
        clean_timer++;
        if(clean_timer >= CLEAN_TIMEOUT) {
            clean_alarm = 1;
            Buzzer_ON();
            LED_Sys_ON();
        }
    }
    else {
        clean_timer = 0;
        clean_alarm = 0;
        Buzzer_OFF();
    }
}

// 功能实现：检查烟雾浓度
void checkSmokeLevel(void)
{
    uint16_t smoke_value = MQ2_GetData();
    if(smoke_value > SMOKE_THRESHOLD) {
        Buzzer_ON();
        LED_Sys_ON();
    }
}

// 功能实现：更新显示
void updateDisplay(void)
{
    OLED_Clear();

    // 显示"垃圾桶："标题
    OLED_ShowChinese(1, 1, "垃");
    OLED_ShowChinese(1, 2, "圾");
    OLED_ShowChinese(1, 3, "桶");
    OLED_ShowString(1, 7, ":");

    // 显示垃圾桶状态
    switch(trash_state) {
        case TRASH_EMPTY:
            OLED_ShowChinese(2, 1, "空");
            break;
        case TRASH_SOME:
            OLED_ShowChinese(2, 1, "有");
            OLED_ShowChinese(2, 2, "垃");
            OLED_ShowChinese(2, 3, "圾");
            break;
        case TRASH_FULL:
            OLED_ShowChinese(2, 1, "满");
            OLED_ShowChinese(2, 2, "了");
            break;
    }

    // 如果设置了清理提醒，显示需要清理
    if(clean_alarm) {
        OLED_ShowChinese(3, 1, "需");
        OLED_ShowChinese(3, 2, "要");
        OLED_ShowChinese(3, 3, "清");
        OLED_ShowChinese(3, 4, "理");
    }

    // 显示剩余清理时间
    if(trash_state != TRASH_EMPTY) {
        uint32_t time_left = (CLEAN_TIMEOUT - clean_timer) / 1000;  // 转换为秒
        OLED_ShowString(4, 1, "Time Left:");
        OLED_ShowNum(4, 11, time_left, 3);
        OLED_ShowString(4, 14, "s");
    }
}

// 功能实现：处理语音命令
void processVoiceCommand(void)
{
    if(USART1_RX_STA == 1) {  // 收到串口数据
        switch(USART1_RX_CMD) {
            case 0x01:  // 假设0x01是开盖命令
                Servo_SetAngle(90.0f);
                lid_opened = 1;
                lid_close_timer = 0;
                break;
            case 0x02:  // 假设0x02是关盖命令
                Servo_SetAngle(0.0f);
                lid_opened = 0;
                break;
        }
        USART1_RX_STA = 0;  // 清除接收标志
    }
}

int main(void)
{
    // 系统和外设初始化
    Sys_Init();           // 系统时钟初始化
    LED_All_Init();       // LED初始化
    Buzzer_Init();        // 蜂鸣器初始化
    CountSensor_Init();   // 红外传感器初始化
    Ultrasonic_Init();    // 超声波传感器初始化
    Servo_Init();         // 舵机初始化
    MQ2_Init();          // 烟雾传感器初始化
    Timer_Init();         // 定时器初始化
    OLED_Init();         // OLED显示屏初始化
    usart1_Init(9600);   // 串口1初始化(用于语音模块通信)

    // 初始状态设置
    Servo_SetAngle(0.0f);  // 确保垃圾桶盖关闭

    while (1)
    {
        // 检测垃圾状态并控制LED指示
        updateTrashState();

        // 检测距离控制开关盖
        controlLidByDistance();

        // 检查清理超时
        checkCleanTimeout();

        // 检查烟雾浓度
        checkSmokeLevel();

        // 更新OLED显示
        updateDisplay();

        // 处理语音命令
        processVoiceCommand();

        // 系统延时
        Delay_ms(10);  // 10ms循环间隔
    }
}
