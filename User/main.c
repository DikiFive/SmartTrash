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

/* �������� */
#define SMOKE_THRESHOLD_ADC 3722 // ����������ֵ(3V��Ӧ��ADCֵ = 3.0/3.3 * 4095)
#define CLEANUP_TIMEOUT_S   180  // ����ʱʱ��(3����)

/* ȫ�ֱ��� */
static uint32_t last_cleanup_time   = 0; // �ϴ�����ʱ��(ϵͳ��������)
static uint8_t trash_status         = 0; // ����Ͱ״̬: 0-��, 1-������, 2-����
static uint8_t display_needs_update = 1; // OLED��ʾ���±�־
static uint8_t time_overflow        = 0; // ʱ�������־

int main(void)
{
    /* ϵͳ��ʼ�� */
    Sys_Init();                           // ��ʼ��ϵͳʱ�ӡ�GPIO��
    last_cleanup_time = system_runtime_s; // ��ʼ������ʱ��Ϊ��ǰϵͳʱ��

    /* OLED��ʾ��ʼ�� */
    OLED_Clear(); // ����
    OLED_Update();
    while (1) {
        /* ��ȡ���⴫����״̬ */
        uint8_t bottom_sensor = Sensor_GetValue();  // �ײ�������
        uint8_t top_sensor    = Sensor_GetValue2(); // ����������

        /* �ж�����Ͱ״̬������LED/������ */
        uint8_t old_status = trash_status;
        if (bottom_sensor && top_sensor) {
            trash_status = 0;                     // ����Ͱ��
            LED1_ON();                            // �̵���-��ʾ��
            LED2_OFF();                           // �Ƶ���
            LED_Sys_OFF();                        // �����
            Buzzer_OFF();                         // �������ر�
            last_cleanup_time = system_runtime_s; // ���������ʱ��
        } else if (!bottom_sensor && top_sensor) {
            trash_status = 1; // ������
            LED1_OFF();       // �̵���
            LED2_ON();        // �Ƶ���-��ʾ������
            LED_Sys_OFF();    // �����
            Buzzer_OFF();     // �������ر�
        } else if (!bottom_sensor && !top_sensor) {
            trash_status = 2; // ����Ͱ����
            LED1_OFF();       // �̵���
            LED2_OFF();       // �Ƶ���
            LED_Sys_ON();     // �����-��ʾ��
            Buzzer_ON();      // ����������
        }

        // ״̬�ı�ʱ��Ҫ������ʾ
        if (old_status != trash_status) {
            display_needs_update = 1;
        }

        /* ������� */
        uint16_t smoke_value = MQ2_GetData();
        if (smoke_value >= SMOKE_THRESHOLD_ADC) {
            LED_Sys_ON(); // �����-��ʾ��⵽����
            Buzzer_ON();  // ����������
        }

        /* �������ʱ */
        uint32_t current_time = system_runtime_s;
        uint32_t time_since_cleanup;

        // ����ʱ��������
        if (current_time < last_cleanup_time) {
            time_overflow = 1;
        }

        if (time_overflow) {
            time_since_cleanup = (UINT32_MAX - last_cleanup_time) + current_time;
        } else {
            time_since_cleanup = current_time - last_cleanup_time;
        }

        if (time_since_cleanup >= CLEANUP_TIMEOUT_S && trash_status != 0) {
            LED_Sys_ON();             // �����-��ʾ��Ҫ����
            Buzzer_ON();              // ����������
            display_needs_update = 1; // ��Ҫ������ʾʱ��
        }

        /* ����OLED��ʾ */
        static uint32_t last_display_time = 0;
        // ÿ�����һ��ʱ����ʾ
        if (system_runtime_s != last_display_time) {
            display_needs_update = 1;
            last_display_time    = system_runtime_s;
        }

        // ������Ҫʱ������ʾ
        if (display_needs_update) {
            OLED_Clear();

            /* ��ʾ����Ͱ״̬ */
            OLED_ShowString(0, 0, "����:", OLED_8X16);
            if (trash_status == 0) {
                OLED_ShowString(40, 0, "��", OLED_8X16);
            } else if (trash_status == 1) {
                OLED_ShowString(40, 0, "��", OLED_8X16);
            } else {
                OLED_ShowString(40, 0, "��", OLED_8X16);
            }

            /* ��ʾδ����ʱ�� */
            OLED_ShowString(0, 16, "Time:", OLED_8X16);
            uint16_t minutes = time_since_cleanup / 60;
            uint16_t seconds = time_since_cleanup % 60;

            // ��ʾ���ӣ�ȷ��������ʾ��λ��
            if (minutes < 10) {
                OLED_ShowString(40, 16, "0", OLED_8X16);
                OLED_ShowNum(48, 16, minutes, 1, OLED_8X16);
            } else {
                OLED_ShowNum(40, 16, minutes, 2, OLED_8X16);
            }
            OLED_ShowString(56, 16, ":", OLED_8X16);

            // ��ʾ������ȷ��������ʾ��λ��
            if (seconds < 10) {
                OLED_ShowString(64, 16, "0", OLED_8X16);
                OLED_ShowNum(72, 16, seconds, 1, OLED_8X16);
            } else {
                OLED_ShowNum(64, 16, seconds, 2, OLED_8X16);
            }

            OLED_Update();
            display_needs_update = 0; // ���ø��±�־
        }

        Delay_ms(50); // ������ʱ,����CPUռ��
    }
}
