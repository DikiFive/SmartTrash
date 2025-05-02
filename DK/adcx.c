#include "adcx.h"

/**
 * @brief  ADC初始化程序
 * @param  ADCx_Instance: 选择需要初始化的ADC外设，可以是ADC1, ADC2, 或ADC3
 * @retval 无
 */
void ADCx_Init(ADC_TypeDef *ADCx_Instance)
{
    // 开启ADC时钟
    if (ADCx_Instance == ADC1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    } else if (ADCx_Instance == ADC2) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
    } else if (ADCx_Instance == ADC3) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
    }

    // ADC频率进行6分频
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // 配置ADC结构体
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;      // 独立模式
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;       // 数据右对齐
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                   // 单次转换
    ADC_InitStructure.ADC_ScanConvMode       = DISABLE;                   // 非扫描模式
    ADC_InitStructure.ADC_NbrOfChannel       = 1;                         // 总通道数
    ADC_Init(ADCx_Instance, &ADC_InitStructure);                          // 初始化指定的ADC

    // 开启ADCx
    ADC_Cmd(ADCx_Instance, ENABLE);

    // 进行ADC校准
    ADC_ResetCalibration(ADCx_Instance);
    while (ADC_GetResetCalibrationStatus(ADCx_Instance) == SET);
    ADC_StartCalibration(ADCx_Instance);
    while (ADC_GetCalibrationStatus(ADCx_Instance) == SET);
}

/**
 * @brief  获取ADC转换后的数据
 * @param  ADCx_Instance: 选择需要获取数据的ADC外设，可以是ADC1, ADC2, 或ADC3
 * @param  ADC_Channel 	选择需要采集的ADC通道
 * @param  ADC_SampleTime  选择需要采样时间
 * @retval 返回转换后的模拟信号数值
 */
u16 ADC_GetValue(ADC_TypeDef *ADCx_Instance, uint8_t ADC_Channel, uint8_t ADC_SampleTime)
{
    // 配置ADC通道
    ADC_RegularChannelConfig(ADCx_Instance, ADC_Channel, 1, ADC_SampleTime);

    ADC_SoftwareStartConvCmd(ADCx_Instance, ENABLE);                 // 软件触发ADC转换
    while (ADC_GetFlagStatus(ADCx_Instance, ADC_FLAG_EOC) == RESET); // 读取ADC转换完成标志位
    return ADC_GetConversionValue(ADCx_Instance);
}
