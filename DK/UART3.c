/**
 * @file     UART3.c
 * @brief    串口3通信模块驱动程序
 * @details  实现基于USART3的串口通信功能，包括：
 *          - 串口初始化（支持可调波特率）
 *          - 数据发送功能
 *          - 数据接收及回显功能
 * @author   DikiFive
 * @date     2025-05-06
 * @version  v1.0
 */

#include "UART3.h"
#include <string.h>

/** @brief 串口3接收缓冲区 */
uint8_t UART3_RxBuffer[UART3_RX_BUFFER_SIZE];
/** @brief 串口3接收计数器 */
uint16_t UART3_RxCount = 0;

/**
 * @brief  串口3初始化
 * @param  baudRate 波特率设置
 * @return 无
 */
void UART3_Init(uint32_t baudRate)
{
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); // USART3在APB1总线
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // GPIOB时钟

    /* GPIO初始化 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10; // PB10作为TX
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11; // PB11作为RX
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* USART初始化 */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = baudRate;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_Init(USART3, &USART_InitStructure);

    /* 中断配置 */
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_Init(&NVIC_InitStructure);

    /* USART使能 */
    USART_Cmd(USART3, ENABLE);
}

/**
 * @brief  通过串口3发送一个字节
 * @param  data 要发送的字节数据
 * @return 无
 */
void UART3_SendByte(uint8_t data)
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, data);
}

/**
 * @brief  通过串口3发送字符串
 * @param  str 要发送的以'\0'结尾的字符串
 * @return 无
 */
void UART3_SendString(char *str)
{
    while (*str) {
        UART3_SendByte(*str++);
    }
}

/**
 * @brief  USART3中断服务函数
 * @details 实现接收数据的回显功能
 * @note   本函数为中断服务函数，由硬件自动调用
 * @param  无
 * @return 无
 */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
        /* 读取接收到的数据 */
        uint8_t RxData = USART_ReceiveData(USART3);

        /* 存储到接收缓冲区 */
        if (UART3_RxCount < UART3_RX_BUFFER_SIZE) {
            UART3_RxBuffer[UART3_RxCount++] = RxData;
        }

        /* 回显接收到的数据 */
        UART3_SendByte(RxData);

        /* 清除中断标志 */
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}
