#include "mq2.h"

void MQ2_Init(void)
{
#if MODE
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(MQ2_AO_GPIO_CLK, ENABLE); // 打开 ADC IO端口时钟
        GPIO_InitStructure.GPIO_Pin  = MQ2_AO_GPIO_PIN;  // 配置 ADC IO 引脚模式
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;    // 设置为模拟输入

        GPIO_Init(MQ2_AO_GPIO_PORT, &GPIO_InitStructure); // 初始化 ADC IO
    }
#else
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(MQ2_DO_GPIO_CLK, ENABLE); // 打开连接 传感器DO 的单片机引脚端口时钟
        GPIO_InitStructure.GPIO_Pin  = MQ2_DO_GPIO_PIN;  // 配置连接 传感器DO 的单片机引脚模式
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    // 设置为上拉输入

        GPIO_Init(MQ2_DO_GPIO_PORT, &GPIO_InitStructure); // 初始化
    }
#endif
}

#if MODE
uint16_t MQ2_ADC_Read(void)
{
    // 设置指定ADC的规则组通道，采样时间
    return ADC_GetValue(ADC1, ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}
#endif

uint16_t MQ2_GetData(void)
{

#if MODE
    uint32_t tempData = 0;
    for (uint8_t i = 0; i < MQ2_READ_TIMES; i++) {
        tempData += MQ2_ADC_Read();
        Delay_ms(5);
    }

    tempData /= MQ2_READ_TIMES;
    return tempData;

#else
    uint16_t tempData;
    tempData = !GPIO_ReadInputDataBit(MQ2_DO_GPIO_PORT, MQ2_DO_GPIO_PIN);
    return tempData;
#endif
}

float MQ2_GetData_PPM(void)
{
#if MODE
    float tempData = 0;
    // char flag=0;

    for (uint8_t i = 0; i < MQ2_READ_TIMES; i++) {
        tempData += MQ2_ADC_Read();
        Delay_ms(5);
    }
    tempData /= MQ2_READ_TIMES;

    // 使用3.3V作为参考电压
    float Vol = (tempData * 3.3f / 4096);
    if(Vol < 0.1f) return 1;  // 防止分母接近0导致计算错误

    float RS = (3.3f - Vol) / Vol;  // 传感器电阻，不需要0.5系数，因为已在电路中分压
    float R0 = 9.8f;  // 在洁净空气中测得的电阻值

    // 使用MQ2传感器的特性曲线进行计算
    // PPM = a * (RS/R0)^b
    // 其中 a = 658.9, b = -2.013 是根据MQ2数据手册的特性曲线拟合得到
    float ratio = RS / R0;
    if(ratio < 0.01f) return 9999;  // 防止比值过小导致计算错误

    float ppm = 658.9f * pow(ratio, -2.013f);

    // 限制返回值范围
    if(ppm > 9999.0f) return 9999;
    if(ppm < 1.0f) return 1;

    return (uint16_t)ppm;
#endif
}
