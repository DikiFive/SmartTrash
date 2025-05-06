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
    Sys_Init();              // ��ʼ�������GPIO
    InitTrashSystem();       // ��ʼ������Ͱϵͳ״̬
    uint8_t distance    = 0; // ���������ֵ
    uint8_t smoke_value = 0; // ��������ֵ

    while (1) {

        distance = sonar_mm(); // ��ȡ���������ֵ
        if (distance < 50) {
            Servo_SetAngle(75.0f); // ������Ͱ��
        } else {
            Servo_SetAngle(0.0f); // �ر�����Ͱ��
        }

        OLED_ShowNum(0, 0, distance, 3, OLED_8X16); // ��ʾ���ֵ
        OLED_ShowString(0, 16, "mm", OLED_8X16);    // ��ʾ��λ
        OLED_Update();                              // ����OLED��ʾ
        Delay_ms(100);                              // ��ʱ100����
    }
}
