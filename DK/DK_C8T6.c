#include "DK_C8T6.h"

/* �������� */
#define SMOKE_THRESHOLD_ADC 3722 // ����������ֵ(3V��Ӧ��ADCֵ = 3.0/3.3 * 4095)
#define CLEANUP_TIMEOUT_S   180  // ����ʱʱ��(3����)

/* ȫ�ֱ��� */
static uint32_t last_cleanup_time   = 0; // �ϴ�����ʱ��(ϵͳ��������)
static uint8_t trash_status         = 0; // ����Ͱ״̬: 0-��, 1-������, 2-����
static uint8_t display_needs_update = 1; // OLED��ʾ���±�־
static uint8_t time_overflow        = 0; // ʱ�������־

void Sys_Init(void)
{
    ADCx_Init(ADC1); // Initialize ADC1
    OLED_Init();     // Initialize OLED display
    LED_All_Init();  // Initialize LED
    Servo_Init();    // Initialize servo motor
    // Buzzer_Init();     // Initialize buzzer
    usart1_Init(9600);  // Initialize USART1 with baud rate 9600
    CountSensor_Init(); // Initialize red infrared sensors

    MQ2_Init();     // Initialize MQ2 smoke sensor
    Timer_Init();   // Initialize timer for tracking
    HC_SR04_Init(); // Initialize ultrasonic sensor
}

void InitTrashSystem(void)
{
    last_cleanup_time    = system_runtime_s;
    trash_status         = 0;
    display_needs_update = 1;
    time_overflow        = 0;

    OLED_Clear();
    OLED_Update();
}

void ProcessSensorData(void)
{
    uint8_t bottom_sensor = Sensor_GetValue();  // �ײ�������
    uint8_t top_sensor    = Sensor_GetValue2(); // ����������
    uint8_t old_status    = trash_status;

    if (bottom_sensor && top_sensor) {
        trash_status      = 0;                // ����Ͱ��
        last_cleanup_time = system_runtime_s; // ���������ʱ��
    } else if (!bottom_sensor && top_sensor) {
        trash_status = 1; // ������
    } else if (!bottom_sensor && !top_sensor) {
        trash_status = 2; // ����Ͱ����
    }

    if (old_status != trash_status) {
        display_needs_update = 1;
    }
}

void CheckSmoke(void)
{
    uint16_t smoke_value = MQ2_GetData();
    if (smoke_value >= SMOKE_THRESHOLD_ADC) {
        LED_Sys_ON(); // �����-��ʾ��⵽����
        Buzzer_ON();  // ����������
    }
}

void CheckCleanupTimeout(void)
{
    uint32_t current_time = system_runtime_s;
    uint32_t time_since_cleanup;

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
}

void UpdateStatusIndicators(void)
{
    switch (trash_status) {
        case 0:            // ��
            LED1_ON();     // �̵���
            LED2_OFF();    // �Ƶ���
            LED_Sys_OFF(); // �����
            Buzzer_OFF();  // �������ر�
            break;
        case 1:            // ������
            LED1_OFF();    // �̵���
            LED2_ON();     // �Ƶ���
            LED_Sys_OFF(); // �����
            Buzzer_OFF();  // �������ر�
            break;
        case 2:           // ����
            LED1_OFF();   // �̵���
            LED2_OFF();   // �Ƶ���
            LED_Sys_ON(); // �����
            Buzzer_ON();  // ����������
            break;
    }
}

void UpdateOLEDDisplay(void)
{
    static uint32_t last_display_time = 0;
    if (system_runtime_s != last_display_time) {
        display_needs_update = 1;
        last_display_time    = system_runtime_s;
    }

    if (display_needs_update) {
        uint32_t current_time = system_runtime_s;
        uint32_t time_since_cleanup;

        if (time_overflow) {
            time_since_cleanup = (UINT32_MAX - last_cleanup_time) + current_time;
        } else {
            time_since_cleanup = current_time - last_cleanup_time;
        }

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
        display_needs_update = 0;
    }
}
