#include "ultrasonic.h"

// 超声波计数
u16 msHcCount;

void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(ULTRASONIC_GPIO_CLK, ENABLE);     // 打开连接 超声波传感器 的单片机引脚端口时钟
    GPIO_InitStructure.GPIO_Pin  = ULTRASONIC_TRIG_GPIO_PIN; // 配置连接 传感器TRIG 的单片机引脚模式
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         // 设置为推挽输出
    GPIO_Init(ULTRASONIC_GPIO_PORT, &GPIO_InitStructure);    // 初始化

    GPIO_InitStructure.GPIO_Pin  = ULTRASONIC_ECHO_GPIO_PIN; // 配置连接 传感器ECHO 的单片机引脚模式
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    // 设置为浮空输入输入
    GPIO_Init(ULTRASONIC_GPIO_PORT, &GPIO_InitStructure);    // 初始化
}

// 打开定时器4
static void OpenTimerForHc()
{
    TIM_SetCounter(TIM4, 0);
    msHcCount = 0;
    TIM_Cmd(TIM4, ENABLE);
}

// 关闭定时器4
static void CloseTimerForHc()
{
    TIM_Cmd(TIM4, DISABLE);
}

// 获取定时器4计数器值
u32 GetEchoTimer(void)
{
    u32 t = 0;
    t     = msHcCount * 1000;
    t += TIM_GetCounter(TIM4);
    TIM4->CNT = 0;
    Delay_ms(50);
    return t;
}

// 通过定时器4计数器值推算距离
float UltrasonicGetLength(void)
{
    u32 t            = 0;
    int i            = 0;
    float lengthTemp = 0;
    float sum        = 0;
    while (i != 5) {
        TRIG_Send = 1;
        Delay_us(20);
        TRIG_Send = 0;
        while (ECHO_Reci == 0);
        OpenTimerForHc();
        i = i + 1;
        while (ECHO_Reci == 1);
        CloseTimerForHc();
        t          = GetEchoTimer();
        lengthTemp = ((float)t / 58.0); // cm
        sum        = lengthTemp + sum;
    }
    lengthTemp = sum / 5.0;
    return lengthTemp;
}
