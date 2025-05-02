/**
 * @file main.c
 * @brief ��������Ͱ������
 * @details ʵ�����¹���:
 *          1. ͨ�����⴫�����������Ͱ����״̬
 *          2. ͨ��MQ2�������������
 *          3. ͨ��LED�ͷ���������״̬/������ʾ
 *          4. ͨ��OLED��ʾ����Ͱ״̬��δ����ʱ��
 */

#include "DK_C8T6.h"

int main(void)
{
    /* ϵͳ��ʼ�� */
    Sys_Init();           // ��ʼ�������GPIO
    InitTrashSystem();    // ��ʼ������Ͱϵͳ״̬
    uint8_t distance = 0; // ���������ֵ

    while (1) {

        Servo_SetAngle(0);  // ���ö���Ƕ�Ϊ0��
        Delay_ms(1000);     // ��ʱ1�룬�ȴ��������Ŀ��λ��
        Servo_SetAngle(75);  // ���ö���Ƕ�Ϊ75��
        Delay_ms(1000);     // ��ʱ1�룬�ȴ��������Ŀ��λ��
        ProcessSensorData();                         // ������������
        CheckSmoke();                                // �������
        CheckCleanupTimeout();                       // �������ʱ
        UpdateStatusIndicators();                    // ����LED�ͷ�����״̬
        UpdateOLEDDisplay();                         // ����OLED��ʾ
        distance = sonar_mm();                       // ��ȡ���ֵ
        OLED_ShowNum(0, 32, distance, 3, OLED_8X16); // ��ʾ���ֵ
        OLED_Update();                               // ����OLED��ʾ
        Delay_ms(50);                                // ������ʱ,����CPUռ��
    }
}
