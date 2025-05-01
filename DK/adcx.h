#ifndef _ADCX_H_
#define _ADCX_H_
#include "stm32f10x.h" // Device header

// ADC 初始化函数，接受 ADC 外设指针作为参数
void ADCx_Init(ADC_TypeDef* ADCx_Instance);

// 获取 ADC 转换数据函数，接受 ADC 外设指针作为参数
u16 ADC_GetValue(ADC_TypeDef* ADCx_Instance, uint8_t ADC_Channel, uint8_t ADC_SampleTime);

#endif
