/**
 * @file     UART3.h
 * @brief    串口3通信模块驱动程序头文件
 * @details  定义了串口3通信相关的函数接口
 * @author   DikiFive
 * @date     2025-05-06
 * @version  v1.0
 */

#ifndef __UART3_H
#define __UART3_H

#include "dk_C8T6.h"
#include <stdint.h>

/** @brief 接收缓冲区大小定义 */
#define UART3_RX_BUFFER_SIZE 64

/** @brief 串口3接收缓冲区 */
extern uint8_t UART3_RxBuffer[UART3_RX_BUFFER_SIZE];
/** @brief 串口3接收计数器 */
extern uint16_t UART3_RxCount;

/**
 * @brief  串口3初始化
 * @param  baudRate 波特率设置
 * @return 无
 */
void UART3_Init(uint32_t baudRate);

/**
 * @brief  通过串口3发送一个字节
 * @param  data 要发送的字节数据
 * @return 无
 */
void UART3_SendByte(uint8_t data);

/**
 * @brief  通过串口3发送字符串
 * @param  str 要发送的以'\0'结尾的字符串
 * @return 无
 */
void UART3_SendString(char *str);

#endif // __UART3_H
