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

        //		 flag++;     //此处是解析GPS的数据，大约就是25秒解析一下GPS的数据
        //			  if(flag==5)
        //				{
        //						flag=0;
        tempData += MQ2_ADC_Read();
        Delay_ms(2);
        //}
        // delay_ms(5);
    }
    tempData /= MQ2_READ_TIMES;

    float Vol = (tempData * 5 / 4096);
    float RS  = (5 - Vol) / (Vol * 0.5);
    float R0  = 6.64;

    float ppm = pow(11.5428 * R0 / RS, 0.6549f);

    return ppm;
#endif
}
