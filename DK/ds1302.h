#ifndef __DS1302_H
#define __DS1302_H

#include "dk_c8t6.h"

#define DS1302_CLK       RCC_APB2Periph_GPIOA
#define DS1302_CE_PORT   GPIOA
#define DS1302_CE_PIN    GPIO_Pin_5
#define DS1302_DATA_PORT GPIOA
#define DS1302_DATA_PIN  GPIO_Pin_6
#define DS1302_SCLK_PORT GPIOA
#define DS1302_SCLK_PIN  GPIO_Pin_7

#define CE_L             GPIO_ResetBits(DS1302_CE_PORT, DS1302_CE_PIN) // CE
#define CE_H             GPIO_SetBits(DS1302_CE_PORT, DS1302_CE_PIN)

#define DATA_L           GPIO_ResetBits(DS1302_SCLK_PORT, DS1302_DATA_PIN) // DATA
#define DATA_H           GPIO_SetBits(DS1302_SCLK_PORT, DS1302_DATA_PIN)

#define SCLK_L           GPIO_ResetBits(DS1302_DATA_PORT, DS1302_SCLK_PIN) // SCLK
#define SCLK_H           GPIO_SetBits(DS1302_DATA_PORT, DS1302_SCLK_PIN)

struct TIMEData {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 week;
}; // 创建TIMEData结构体方便存储时间日期数据

struct TIMERAM {
    u8 hour_kai;
    u8 minute_kai;
    u8 hour_guan;
    u8 minute_guan;
    u8 kai;
    u8 guan;
}; // 创建TIMEData结构体方便存储时间日期数据

extern struct TIMERAM TimeRAM;
extern struct TIMEData TimeData; // 全局变量

void DS1302_GPIO_Init(void);                // ds1302端口初始化
void DS1302_write_onebyte(u8 data);         // 向ds1302发送一字节数据
void DS1302_wirte_rig(u8 address, u8 data); // 向指定寄存器写一字节数据
u8 DS1302_read_rig(u8 address);             // 从指定寄存器读一字节数据
void DS1302_Init(void);                     // ds1302初始化函数
void DS1302_DATAOUT_init(void);             // IO端口配置为输出
void DS1302_DATAINPUT_init(void);           // IO端口配置为输入
void DS1302_read_time(void);                // 从ds1302读取实时时间（BCD码）
void DS1302_read_realTime(void);            // 将BCD码转化为十进制数据
void DS1302_wirteRAM(void);
void DS1302_readRAM(void);

#endif
