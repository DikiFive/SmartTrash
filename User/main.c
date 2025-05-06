/**
 * @file main.c
 * @brief ��������Ͱ������
 * @details ʵ�����¹���:
 *          1. ���⴫�����������Ͱ����״̬����ͨ��LED�ͷ�����ָʾ
 *          2. ������������忿�����Զ����ظ�
 *          3. 3����δ��������ʾ
 *          4. �������ƿ��ظ�
 *          5. �����ⱨ��
 *          6. OLED��ʾϵͳ״̬
 */

#include "DK_C8T6.h"

int main(void)
{
    /* ϵͳ��ʼ�� */
    Sys_Init();        // ��ʼ�������GPIO
    InitTrashSystem(); // ��ʼ������Ͱϵͳ״̬��״̬����
    // DS1302_SetTime(2025, 5, 6, 20, 25, 0, 2); // ��,��,��,ʱ,��,��,����
    // У׼ʱ���ʹ�ã�У׼��ע�͵��������±�������
    while (1) {
        ProcessSerialCommands();  // ������������������ƣ�
        HandleUltrasonicSensor(); // �����������������Զ����ظ�
        ProcessSensorData();      // ������⴫��������
        CheckSmoke();             // �������
        CheckCleanupTimeout();    // �������ʱ
        UpdateStatusIndicators(); // ����LED�ͷ�����״̬
        UpdateOLEDDisplay();      // ����OLED��ʾ����������ֵ�;��룩
    }
}
