/**
 * @file     DK_C8T6.h
 * @brief    STM32F103C8T6主控制程序头文件
 * @details  定义了系统的：
 *          - 工作模式
 *          - 数据结构
 *          - 功能函数接口
 * @author   DikiFive
 * @date     2025-04-30
 * @version  v1.0
 */

#ifndef __DK_C8T6_H
#define __DK_C8T6_H

#include <stdint.h>    // 标准整数类型定义
#include "stm32f10x.h" // STM32F10x外设库头文件
#include "sys.h"
#include "math.h"

// 包含所有外设驱动头文件
#include "adcx.h"
#include "Common.h"
#include "Buzzer.h"
#include "Delay.h"
#include "Ds1302.h"
#include "HC_SR04.h"
#include "LED.h"
#include "mq2.h"
#include "OLED.h"
#include "RED.h"
#include "usart1.h"
#include "Servo.h"
#include "Timer.h"

void Sys_Init(void); // 系统初始化函数声明

// 垃圾桶状态监控和控制函数声明
void ProcessSensorData(void);      // 处理传感器数据
void CheckSmoke(void);             // 检查烟雾
void CheckCleanupTimeout(void);    // 检查清理超时
void UpdateStatusIndicators(void); // 更新状态指示器
void UpdateOLEDDisplay(void);      // 更新OLED显示
void InitTrashSystem(void);        // 初始化垃圾桶系统状态

// 获取系统运行时间(秒)
extern uint32_t system_runtime_s;

#endif /* __DK_C8T6_H */
