#include "HC_SR04.h"
#include "Timer.h"
#include "Delay.h"

uint64_t time     = 0; // 声明变量，用来计时
uint64_t time_end = 0; // 声明变量，存储回波信号时间

void HC_SR04_Init(void)
{
    RCC_APB2PeriphClockCmd(ULTRASONIC_GPIO_CLK, ENABLE);  // 启用GPIOA外设时钟
    GPIO_InitTypeDef GPIO_InitStructure;                  // 定义结构体
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;     // 设置GPIO口为推挽输出
    GPIO_InitStructure.GPIO_Pin   = TRIG_GPIO_PIN;        // 设置GPIO口5
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     // 设置GPIO口速度50Mhz
    GPIO_Init(ULTRASONIC_GPIO_PORT, &GPIO_InitStructure); // 初始化GPIOB

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;                  // 设置GPIO口为下拉输入模式
    GPIO_InitStructure.GPIO_Pin  = ECHO_GPIO_PIN;                  // 设置GPIO口6
    GPIO_Init(ULTRASONIC_GPIO_PORT, &GPIO_InitStructure);          // 初始化GPIOA
    GPIO_WriteBit(ULTRASONIC_GPIO_PORT, TRIG_GPIO_PIN, Bit_RESET); // 输出低电平
    Delay_us(15);                                                  // 延时15微秒
}

int16_t sonar_mm(void) // 测距并返回单位为毫米的距离结果
{
    uint32_t Distance, Distance_mm = 0;
    GPIO_WriteBit(ULTRASONIC_GPIO_PORT, TRIG_GPIO_PIN, Bit_SET);             // 输出高电平
    Delay_us(15);                                                            // 延时15微秒
    GPIO_WriteBit(ULTRASONIC_GPIO_PORT, TRIG_GPIO_PIN, Bit_RESET);           // 输出低电平
    while (GPIO_ReadInputDataBit(ULTRASONIC_GPIO_PORT, ECHO_GPIO_PIN) == 0); // 等待低电平结束
    time = 0;                                                                // 计时清零
    while (GPIO_ReadInputDataBit(ULTRASONIC_GPIO_PORT, ECHO_GPIO_PIN) == 1); // 等待高电平结束
    time_end = time;                                                         // 记录结束时的时间
    if (time_end / 100 < 38)                                                 // 判断是否小于38毫秒，大于38毫秒的就是超时，直接调到下面返回0
    {
        Distance    = (time_end * 346) / 2; // 计算距离，25°C空气中的音速为346m/s
        Distance_mm = Distance / 100;       // 因为上面的time_end的单位是10微秒，所以要得出单位为毫米的距离结果，还得除以100
    }
    return Distance_mm; // 返回测距结果
}

float sonar(void) // 测距并返回单位为米的距离结果
{
    uint32_t Distance, Distance_mm = 0;
    float Distance_m = 0;
    GPIO_WriteBit(ULTRASONIC_GPIO_PORT, TRIG_GPIO_PIN, Bit_SET); // 输出高电平
    Delay_us(15);
    GPIO_WriteBit(ULTRASONIC_GPIO_PORT, TRIG_GPIO_PIN, Bit_RESET); // 输出低电平
    while (GPIO_ReadInputDataBit(ULTRASONIC_GPIO_PORT, ECHO_GPIO_PIN) == 0);
    time = 0;
    while (GPIO_ReadInputDataBit(ULTRASONIC_GPIO_PORT, ECHO_GPIO_PIN) == 1);
    time_end = time;
    if (time_end / 100 < 38) {
        Distance    = (time_end * 346) / 2;
        Distance_mm = Distance / 100;
        Distance_m  = Distance_mm / 1000;
    }
    return Distance_m;
}