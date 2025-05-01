#ifndef __USART1_H
#define __USART1_H

#include "dk_C8T6.h" // Device header

#define EN_USART1_RX 1 // 使能（1）/禁止（0）串口1接收

extern u8 USART1_RX_STA; // 接收状态标记
extern u8 USART1_RX_CMD;

void usart1_Init(u32 bound);

void USART1_IRQHandler(void);

void USART1_SendByte(uint8_t Data);
void Uart1_SendCMD(int CMD, int feedback, int dat);
void Uart1_SendCMD2(int CMD, int dat1, int dat2, int dat3);

void USART1_DoSum(int *Str, int len);
void USART1_SendCmd(int len);

#endif
