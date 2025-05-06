/**********************************************************************************************************************
   此处为C8T6给MP3模块发送数据；调试时注意看好LED就可以初步判断代码是否OK。




                 可读性和美观性！！！！细腻一点，每个语句的根源功用都仔细梳理好。

    2022.05.13   测试用任意串口来赋能语音识别   之   串口    1     13.15 测试成功

                                 PA9-TX
                 PA10-RX

2022.09.26   重点在此处：

**********************************************************************************************************************/
#include "usart1.h"
#include "UART3.h"

// #include "usart2.h"

////////////////////////printf打印函数，三个串口一次只能使用一个，需要时解封即可////////////////////////////////////////////
////加入以下代码,支持printf函数,而不需要选择use MicroLIB
// #if 1
// #pragma import(__use_no_semihosting)
////标准库需要的支持函数
// struct __FILE
//{
//	int handle;

//};

// FILE __stdout;
////定义_sys_exit()以避免使用半主机模式
// void _sys_exit(int x)
//{
//	x = x;
// }
////重定义fputc函数
// int fputc(int ch, FILE *f)
//{
//	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕
//     USART1->DR = (u8) ch;
//	return ch;
// }
// #endif

static int Send_buf[10] = {0};

u8 USART1_RX_STA = 0; // 接收状态标记
u8 USART1_RX_CMD;     // 存储接收到的命令
u8 USART1_NewCmd = 0; // 新命令标志位，主循环检查后需要清零

/******************************************************************************
* 模块描述
* 项目代号或名称 ：  LD3320和STM32F103C8T6及MP3模块搭建语音识别控制系统
* 创建日期       ：  2022/05/13
* 创建人         ：  志城
* 模块功能       ：  usart1初始化，发送数据给MP3模块
* 修改日期       ：
* 参考文档       ：  精通STM32F4库函数版
* 项目运行平台   ：  STM32F103C8T6
* 其它           ：  串口初始化流程：   1、串口时钟使能、GPIO时钟使能。
                                        2、GPIO端口初始化设置。
                                        3、串口参数初始化并开启接收中断。
                                        5、初始化NVIC并开启中断。
                                                                                6、使能串口。
                                                                                7、编写中断服务函数。
                                        8、编写数据发送函数.

*******************************************************************************/

void usart1_Init(u32 bound)
{
    // GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);                      // 使能USART1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); // 使能GPIOA时钟，端口复用时钟

    // USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;       // PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 万勿省略此条，一经删除则发送不了数据
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);            // 初始化GPIOA.9

    // USART1_RX	  GPIOA.10
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;           // PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);                // 初始化GPIOA.10

    // USART 初始化设置
    USART_InitStructure.USART_BaudRate            = bound;                          // 串口波特率
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;            // 字长为8位数据格式
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;               // 一个停止位
    USART_InitStructure.USART_Parity              = USART_Parity_No;                // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;  // 收发模式
    USART_Init(USART1, &USART_InitStructure);                                       // 初始化串口1

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 开启串口接受中断

    // Usart2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;      // 抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;      // 子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE; // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                                // 根据指定的参数初始化VIC寄存器

    USART_Cmd(USART1, ENABLE);
}

/******************************************************************************
* 函数名称       ： USART1_SendByte(uint8_t  Data)
* 创建日期       ：  2022/05/12
* 创建人         ：  志城

* 函数功能       ：  调用USART1_SendByte();将数据逐个发送出来

* 输入参数类型   ： int len = 8；在for语句处逐个将Send_buf[]数据发送出来

* 输出参数类型   ：
* 返回信息       ：
* 内在逻辑       ： 串口1发送数据给PM3模块的内在逻辑应该是：
                    1、在main函数的while()语句中，根据case值来调用：Uart_SendCMD(0x03,0,0x01);
                    2、这串数据通过串口2被发送出来，这样一来逻辑就清晰了，按道理我也可以把数据放在串口1来发送， 将LD和MP3集成在一个串口上。
基础知识储备：

函数名  ：USART_GetFlagStatus
函数原型：FlagStatus USART_GetFlagStatus(USART_TypeDef* USARTx, uint16_t USART_FLAG)
功能描述：检查指定的USART标志位设置与否
输入参数1：USARTx可以是1、2或者3，来选择USART外设
输入参数2：USART_FLAG：待检查的USART标志位
输出参数：无
返回值：USART_FLAG的新状态（SET或者RSET)
先决条件：无
被调用函数：无


#define USART_FLAG_CTS           CTS标志位
#define USART_FLAG_LBD           LIN中断检测标志位
#define USART_FLAG_TXE           发送数据寄存器空标志位
#define USART_FLAG_TC            发送完成标志位
#define USART_FLAG_RXNE          接收数据寄存器非空标志位
#define USART_FLAG_IDLE          空闲总线标志位
#define USART_FLAG_ORE           溢出错误标志位
#define USART_FLAG_NE            噪声错误标志位
#define USART_FLAG_FE            帧错误标志位
#define USART_FLAG_PE            奇偶错误标志位


*******************************************************************************/
void USART1_SendByte(uint8_t Data) // 串口发送一个字节；字节 (byte)    1byte=8bit
{

    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // USART_FLAG_TXE发送寄存器空

    USART_SendData(USART1, Data);                                // 从串口1发送传入的数据
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); // USART_FLAG_TC发送完成标志
}

/******************************************************************************
* 函数名称       ： USART1_IRQHandler(void)
* 创建日期       ：  2022/05/13
* 创建人         ：  志城
* 函数功能       ：  一旦串口1有数据发送过来立刻产生中断并读取数据，将数据传递给USART_RX_CMD然后通过USART_RX_STA=1进入主函数While语句
* 输入参数类型   ： 无
* 输出参数类型   ：
* 返回信息       ： 无
* 内在逻辑       ： 读取串口1接收中断数据，进入While语句进行case

基础知识储备：
函数名  ：USART_GetITStatus
函数原型：ITStatus USART_GetITStatus(USART_TypeDef* USARTx, uint16_t USART_IT)
功能描述：检查指定的USART中断发生与否
输入参数1：USARTx可以是1、2或者3，来选择USART外设
输入参数2：USART_FLAG：待检查的USART中断源
输出参数：无
返回值：USART_IT的新状态
先决条件：无
被调用函数：无

#define USART_IT_PE            奇偶错误中断
#define USART_IT_TXE           发送中断
#define USART_IT_TC            发送完成中断
#define USART_IT_RXNE          接收中断
#define USART_IT_IDLE          总线空闲中断
#define USART_IT_LBD           LIN中断探测中断
#define USART_IT_CTS           CTS中断
#define USART_IT_ERR
#define USART_IT_ORE           溢出错误中断
#define USART_IT_NE            噪声错误中断
#define USART_IT_FE            帧错误中断


*******************************************************************************/
void USART1_IRQHandler(void) // 串口2中断服务程序
{
    u8 Res = 0;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) // 接收中断
    {
        Res           = USART_ReceiveData(USART1); // 读取接收到的数据
        USART1_RX_CMD = Res;                       // 把接收到的数据传递给USART_RX_CMD
        USART1_NewCmd = 1;                         // 设置新命令标志位
    }
}

/******************************************************************************
* 函数名称       ： void SendCmd(int len)
* 创建日期       ：  2022/05/12
* 创建人         ：  志城

* 函数功能       ：  调用USART2_SendByte();将数据逐个发送出来

* 输入参数类型   ： int len = 8；在for语句处逐个将Send_buf[]数据发送出来

* 输出参数类型   ：
* 返回信息       ：
* 内在逻辑       ： 串口2发送数据给PM3模块的内在逻辑应该是：
                    1、在main函数的while()语句中，根据case值来调用：Uart_SendCMD(0x03,0,0x01);
                    2、这串数据通过串口2被发送出来，这样一来逻辑就清晰了，按道理我也可以把数据放在串口1来发送， 将LD和MP3集成在一个串口上。

*******************************************************************************/
void USART1_SendCmd(int len)
{

    int i = 0;
    USART1_SendByte(0x7E); // 起始

    for (i = 0; i < len; i++) // 数据
    {
        USART1_SendByte(Send_buf[i]); // len 为8 ；依次将Send_buf[0]、Send_buf[1]　！！！�sSend_buf[8]  发送出来
    }

    USART1_SendByte(0xEF); // 结束
}

/********************************************************************************************
 - 功能描述：求和校验
 - 隶属模块：
 - 参数说明：
 - 返回说明：
 - 注：      和校验的思路如下
             发送的指令，去掉起始和结束。将中间的6个字节进行累加，最后取反码
             接收端就将接收到的一帧数据，去掉起始和结束。将中间的数据累加，再加上接收到的校验
             字节。刚好为0.这样就代表接收到的数据完全正确。
********************************************************************************************/
void USART1_DoSum(int *Str, int len)
{
    int xorsum = 0;
    int i;

    for (i = 0; i < len; i++) {
        xorsum = xorsum + Str[i];
    }
    xorsum         = 0 - xorsum;
    *(Str + i)     = (int)(xorsum >> 8);
    *(Str + i + 1) = (int)(xorsum & 0x00ff);
}

/********************************************************************************************
 - 功能描述： 串口向外发送命令[包括控制和查询]
 - 隶属模块： 外部
 - 参数说明： CMD:表示控制指令，请查阅指令表，还包括查询的相关指令
              feedback:是否需要应答[0:不需要应答，1:需要应答]
              data:传送的参数
 - 返回说明：
 - 注：
********************************************************************************************/
void Uart1_SendCMD(int CMD, int feedback, int dat)
{
    Send_buf[0] = 0xff;            // 保留字节
    Send_buf[1] = 0x06;            // 长度
    Send_buf[2] = CMD;             // 控制指令
    Send_buf[3] = feedback;        // 是否需要反馈
    Send_buf[4] = (int)(dat >> 8); // datah
    Send_buf[5] = (int)(dat);      // datal
    USART1_DoSum(&Send_buf[0], 6); // 校验     &Send_buf[0],6     取Send_buf[0]数组起始地址，长度6位
    USART1_SendCmd(8);             // 发送此帧数据
}

/******************************************************************************
* 函数名称       ： void Uart1_SendCMD2(int CMD ,int dat1 , int dat2 ,int dat3)
* 创建日期       ：  2022/09/25
* 创建人         ：  志城

* 函数功能       ：  将数据放入buf，等待后面依次发出

* 输入参数类型   ： int CMD ,int dat1 , int dat2 ,int dat3   一共四个参数，可根据需要进行加减

* 输出参数类型   ：
* 返回信息       ：
* 内在逻辑       ： 串口2发送数据给PM3模块的内在逻辑应该是：
                    1、在main函数的while()语句中，根据case值来调用：Uart_SendCMD(0x03,0,0x01);
                    2、这串数据通过  USART1_SendCmd(6);函数里面的for循环，依次执行USART1_SendByte(Send_buf[i]);
                       依次将Send_buf[0]、Send_buf[1]、Send_buf[8]  发送出来	。
                    3、可按照需要加减输入参数即可得到自己想要的输出。
*******************************************************************************/
void Uart1_SendCMD2(int CMD, int dat1, int dat2, int dat3)
{
    Send_buf[0] = 0xff;        // 保留字节
    Send_buf[1] = 0x06;        // 长度
    Send_buf[2] = CMD;         // 控制指令
    Send_buf[3] = (int)(dat1); //
    Send_buf[4] = (int)(dat2); // datah2
    Send_buf[5] = (int)(dat3); // datal3

    USART1_SendCmd(6); // 发送此帧数据
}
