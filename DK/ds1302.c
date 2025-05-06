#include "ds1302.h"

struct TIMEData TimeData;
struct TIMERAM TimeRAM;
u8 read_time[7];

void DS1302_GPIO_Init(void) // CE,SCLK端口初始化
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(DS1302_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = DS1302_SCLK_PIN; // CE
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_Init(DS1302_SCLK_PORT, &GPIO_InitStructure); // 初始化
    GPIO_ResetBits(DS1302_SCLK_PORT, DS1302_SCLK_PIN);

    GPIO_InitStructure.GPIO_Pin   = DS1302_CE_PIN; // SCLK
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_Init(DS1302_CE_PORT, &GPIO_InitStructure);   // 初始化
    GPIO_ResetBits(DS1302_CE_PORT, DS1302_CE_PIN);
}

void DS1302_DATAOUT_init() // 配置双向I/O端口为输出态
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(DS1302_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = DS1302_DATA_PIN; // DATA
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(DS1302_DATA_PORT, &GPIO_InitStructure); // 初始化
    GPIO_ResetBits(DS1302_DATA_PORT, DS1302_DATA_PIN);
}

void DS1302_DATAINPUT_init() // 配置双向I/O端口为输入态
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(DS1302_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin  = DS1302_DATA_PIN; // DATA
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(DS1302_DATA_PORT, &GPIO_InitStructure); // 初始化
}

void DS1302_write_onebyte(u8 data) // 向DS1302发送一字节数据
{
    u8 count = 0;
    SCLK_L;
    DS1302_DATAOUT_init();

    for (count = 0; count < 8; count++) {
        SCLK_L;
        if (data & 0x01) {
            DATA_H;
        } else {
            DATA_L;
        } // 先准备好数据再发送
        SCLK_H; // 拉高时钟线，发送数据
        data >>= 1;
    }
}

void DS1302_wirte_rig(u8 address, u8 data) // 向指定寄存器地址发送数据
{
    u8 temp1 = address;
    u8 temp2 = data;
    CE_L;
    SCLK_L;
    Delay_us(1);
    CE_H;
    Delay_us(3);
    DS1302_write_onebyte(temp1);
    DS1302_write_onebyte(temp2);
    CE_L;
    SCLK_L;
    Delay_us(3);
}

u8 DS1302_read_rig(u8 address) // 从指定地址读取一字节数据
{
    u8 temp3       = address;
    u8 count       = 0;
    u8 return_data = 0x00;
    CE_L;
    SCLK_L;
    Delay_us(3);
    CE_H;
    Delay_us(3);
    DS1302_write_onebyte(temp3);
    DS1302_DATAINPUT_init(); // 配置I/O口为输入
    Delay_us(3);
    for (count = 0; count < 8; count++) {
        Delay_us(3); // 使电平持续一段时间
        return_data >>= 1;
        SCLK_H;
        Delay_us(5); // 使高电平持续一段时间
        SCLK_L;
        Delay_us(30); // 延时14us后再去读取电压，更加准确
        if (GPIO_ReadInputDataBit(DS1302_DATA_PORT, DS1302_DATA_PIN)) { return_data = return_data | 0x80; }
    }
    Delay_us(2);
    CE_L;
    DATA_L;
    return return_data;
}

void DS1302_Init(void)
{
    DS1302_wirte_rig(0x8e, 0x00); // 关闭写保护
    DS1302_wirte_rig(0x80, 0x00); // seconds 00秒 (BCD: 00)
    DS1302_wirte_rig(0x82, 0x17); // minutes 17分 (BCD: 0x17)
    DS1302_wirte_rig(0x84, 0x20); // hours 20时 (BCD: 0x20)
    DS1302_wirte_rig(0x86, 0x06); // date 6日 (BCD: 0x06)
    DS1302_wirte_rig(0x88, 0x05); // months 5月 (BCD: 0x05)
    DS1302_wirte_rig(0x8a, 0x02); // days 星期二 (BCD: 0x02)
    DS1302_wirte_rig(0x8c, 0x25); // year 25年 (BCD: 0x25)
    DS1302_wirte_rig(0x8e, 0x80); // 开启写保护
}

void DS1302_read_time(void)
{
    read_time[0] = DS1302_read_rig(0x81); // 读秒
    read_time[1] = DS1302_read_rig(0x83); // 读分
    read_time[2] = DS1302_read_rig(0x85); // 读时
    read_time[3] = DS1302_read_rig(0x87); // 读日
    read_time[4] = DS1302_read_rig(0x89); // 读月
    read_time[5] = DS1302_read_rig(0x8B); // 读星期
    read_time[6] = DS1302_read_rig(0x8D); // 读年
}

void DS1302_read_realTime(void)
{
    DS1302_read_time(); // BCD码转换为10进制
    TimeData.second = (read_time[0] >> 4) * 10 + (read_time[0] & 0x0f);
    TimeData.minute = ((read_time[1] >> 4)) * 10 + (read_time[1] & 0x0f);
    TimeData.hour   = (read_time[2] >> 4) * 10 + (read_time[2] & 0x0f);
    TimeData.day    = (read_time[3] >> 4) * 10 + (read_time[3] & 0x0f);
    TimeData.month  = (read_time[4] >> 4) * 10 + (read_time[4] & 0x0f);
    TimeData.week   = read_time[5];
    TimeData.year   = (read_time[6] >> 4) * 10 + (read_time[6] & 0x0f) + 2000;
}

void DS1302_wirteRAM(void)
{
    DS1302_wirte_rig(0x8e, 0x00);                // 关闭写保护
    DS1302_wirte_rig(0xC0, TimeRAM.hour_kai);    // 开时
    DS1302_wirte_rig(0xC2, TimeRAM.minute_kai);  // 开分
    DS1302_wirte_rig(0xC4, TimeRAM.hour_guan);   // 关时
    DS1302_wirte_rig(0xC6, TimeRAM.minute_guan); // 关分
    DS1302_wirte_rig(0xC8, TimeRAM.kai);         // 关分
    DS1302_wirte_rig(0xCA, TimeRAM.guan);        // 关分
    DS1302_wirte_rig(0x8e, 0x80);                // 关闭写保护
}
void DS1302_readRAM(void)
{
    TimeRAM.hour_kai    = DS1302_read_rig(0xC1); // 读秒
    TimeRAM.minute_kai  = DS1302_read_rig(0xC3); // 读分
    TimeRAM.hour_guan   = DS1302_read_rig(0xC5); // 读时
    TimeRAM.minute_guan = DS1302_read_rig(0xC7); // 读日
    TimeRAM.kai         = DS1302_read_rig(0xC9); // 读日
    TimeRAM.guan        = DS1302_read_rig(0xCB); // 读日
}

// 将十进制数转换为BCD码
static uint8_t DecToBcd(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

// 设置DS1302时间
void DS1302_SetTime(uint16_t year, uint8_t month, uint8_t day,
                    uint8_t hour, uint8_t minute, uint8_t second, uint8_t week)
{
    DS1302_wirte_rig(0x8e, 0x00);           // 关闭写保护
    DS1302_wirte_rig(0x80, DecToBcd(second)); // 设置秒
    DS1302_wirte_rig(0x82, DecToBcd(minute)); // 设置分
    DS1302_wirte_rig(0x84, DecToBcd(hour));   // 设置时
    DS1302_wirte_rig(0x86, DecToBcd(day));    // 设置日
    DS1302_wirte_rig(0x88, DecToBcd(month));  // 设置月
    DS1302_wirte_rig(0x8a, DecToBcd(week));   // 设置星期
    DS1302_wirte_rig(0x8c, DecToBcd(year % 100)); // 设置年(只存储后两位)
    DS1302_wirte_rig(0x8e, 0x80);           // 开启写保护
}
