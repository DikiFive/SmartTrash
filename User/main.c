/**
 * @file main.c
 * @brief ��������Ͱ������
 * @details ʵ�����¹���:
 *          1. ͨ�����⴫�����������Ͱ����״̬
 *          2. ͨ��MQ2�������������
 *          3. ͨ��LED�ͷ���������״̬/������ʾ
 *          4. ͨ��OLED��ʾ����Ͱ״̬��δ����ʱ��
 *          5. ��ʾ����1���յ�������
 */

#include "DK_C8T6.h"
#include "usart1.h"
#include "OLED.h"
#include <stdint.h>

#define WINDOW_SIZE       5 // ����ƽ�����ڴ�С
#define TRIGGER_THRESHOLD 3 // ������������ֵ

uint8_t distance_readings[WINDOW_SIZE]; // �洢����Ĳ���ֵ
uint8_t reading_index = 0;              // ��ǰ�洢λ������
uint8_t trigger_count = 0;              // ��������������

// ���������㻬��ƽ������
uint8_t calculate_average_distance(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sum += distance_readings[i];
    }
    return sum / WINDOW_SIZE;
}

int main(void)
{
    /* ϵͳ��ʼ�� */
    Sys_Init();        // ��ʼ�������GPIO
    InitTrashSystem(); // ��ʼ������Ͱϵͳ״̬
    usart1_Init(9600); // ��ʼ������1��������9600
    OLED_Init();       // ��ʼ��OLED��ʾ

    // ��ʼ�������ȡ����
    for (int i = 0; i < WINDOW_SIZE; i++) {
        distance_readings[i] = 0;
    }

    uint8_t raw_distance = 0; // ������ԭʼ���ֵ
    uint8_t avg_distance = 0; // ������ƽ�����ֵ
    uint8_t smoke_value  = 0; // ��������ֵ

    // ��������ʾ��ʼ��ʾ��Ϣ
    OLED_Clear();+
    OLED_ShowString(0, 0, "UART1:", OLED_8X16);
    OLED_Update();

    while (1) {
        // ��鴮��1�Ƿ���յ�������
        if (USART1_RX_STA == 1)
        {
            // ��OLED����ʾ���յ�������
            OLED_ShowChar(48, 0, USART1_RX_CMD, OLED_8X16); // ��"UART1:"������ʾ���յ����ַ�
            OLED_Update(); // ����OLED��ʾ
            USART1_RX_STA = 0; // ������ձ�־��׼��������һ������
        }

        // raw_distance = sonar_mm(); // ��ȡ������ԭʼ���ֵ

        // // ���µ�ԭʼ���ݴ��뻬������
        // distance_readings[reading_index] = raw_distance;
        // reading_index                    = (reading_index + 1) % WINDOW_SIZE; // ����������ʵ��ѭ���洢

        // // ���㻬��ƽ������
        // avg_distance = calculate_average_distance();

        // // ʹ��ƽ����������߼��жϣ����������������
        // if (avg_distance < 150) {
        //     if (trigger_count < TRIGGER_THRESHOLD) {
        //         trigger_count++;
        //     }
        //     // ֻ���������������ﵽ��ֵ�Ŵ򿪸���
        //     if (trigger_count >= TRIGGER_THRESHOLD) {
        //         Servo_SetAngle(75.0f); // ������Ͱ��
        //     }
        // } else {
        //     trigger_count = 0;    // ���ô���������
        //     Servo_SetAngle(0.0f); // �ر�����Ͱ��
        // }

        // ProcessSensorData();                             // ������������
        // CheckSmoke();                                    // �������
        // CheckCleanupTimeout();                           // �������ʱ
        // UpdateStatusIndicators();                        // ����LED�ͷ�����״̬
        // UpdateOLEDDisplay();                             // ����OLED��ʾ
        // smoke_value = MQ2_GetData_PPM();                 // ��ȡ��������ֵ
        // OLED_ShowNum(0, 48, smoke_value, 4, OLED_8X16);  // ��ʾ��������ֵ-
        // OLED_ShowNum(0, 32, avg_distance, 3, OLED_8X16); // ��ʾ���ֵ
        // OLED_Update();                                   // ����OLED��ʾ
        // Delay_ms(50);
    }
}
