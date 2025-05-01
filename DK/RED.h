/**
 * @file     RED.h
 * @brief    红外传感器驱动程序头文件
 * @details  声明红外传感器相关的：
 *          - 检测状态标志
 *          - 控制函数接口
 * @author   DikiFive
 * @date     2025-04-30
 * @version  v1.0
 */

#ifndef __RED_H
#define __RED_H

#include <stdint.h>
#include "dk_C8T6.h" // 项目主头文件

/**
 * @brief 红外检测状态标志
 * @note  在中断中更新，在主程序中读取
 */
extern uint8_t RED_Flag;

/**
 * @brief  红外传感器初始化
 * @details 配置GPIO和外部中断：
 *         - PA7配置为上拉输入
 *         - 外部中断配置为下降沿触发
 * @param  无
 * @return 无
 */
void RED_Init(void);

/**
 * @brief  获取红外检测状态
 * @details 直接返回检测标志值
 * @return uint8_t 检测结果：
 *         - 1：检测到障碍物
 *         - 0：未检测到障碍物
 */
uint8_t RED_Get(void);

void CountSensor_Init();
uint8_t Sensor_GetValue();
uint8_t Sensor_GetValue2();

#endif /* __RED_H */
