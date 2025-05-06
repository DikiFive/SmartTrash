#include "DK_C8T6.h"

/* �������� */
#define SMOKE_THRESHOLD_PPM 300  // ����Ũ�ȱ�����ֵ(PPM)
#define CLEANUP_TIMEOUT_S   10   // ����ʱʱ��(3����)
#define WINDOW_SIZE         5    // ����ƽ�����ڴ�С
#define TRIGGER_THRESHOLD   3    // ������������ֵ
#define CLOSE_DISTANCE      50   // ����Ͱ�Ǵ򿪾���(����)
#define CLOSE_DELAY_MS      1000 // ����Ͱ�ǹر��ӳ�ʱ��(����)

/* ȫ�ֱ��� */
static uint32_t last_cleanup_time   = 0; // �ϴ�����ʱ��(ϵͳ��������)
static uint8_t trash_status         = 0; // ����Ͱ״̬: 0-��, 1-������, 2-����
static uint8_t display_needs_update = 1; // OLED��ʾ���±�־
static uint8_t time_overflow        = 0; // ʱ�������־
static uint8_t smoke_alert_active   = 0; // �����������־
static uint8_t cleanup_alert_active = 0; // ����ʱ���������־

/* ��������ر��� */
static uint8_t distance_readings[WINDOW_SIZE]; // �洢����Ĳ���ֵ
static uint8_t reading_index         = 0;      // ��ǰ�洢λ������
static uint8_t trigger_count         = 0;      // ��������������
static uint32_t lid_close_time       = 0;      // ����Ͱ��Ԥ���ر�ʱ��
static uint8_t lid_closing_scheduled = 0;      // ����Ͱ���Ƿ��ڵȴ��ر�

// ���������㻬��ƽ������
static uint8_t calculate_average_distance(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        sum += distance_readings[i];
    }
    return sum / WINDOW_SIZE;
}

void HandleUltrasonicSensor(void)
{
    uint8_t raw_distance = sonar_mm(); // ��ȡ������ԭʼ���ֵ

    // ���µ�ԭʼ���ݴ��뻬������
    distance_readings[reading_index] = raw_distance;
    reading_index                    = (reading_index + 1) % WINDOW_SIZE; // ����������ʵ��ѭ���洢

    // ���㻬��ƽ������
    uint8_t avg_distance = calculate_average_distance();

    // ʹ��ƽ����������߼��жϣ����������������
    if (avg_distance < CLOSE_DISTANCE) { // ������������
        if (trigger_count < TRIGGER_THRESHOLD) {

            trigger_count++;
        }
        // ֻ���������������ﵽ��ֵ�Ŵ򿪸���
        if (trigger_count >= TRIGGER_THRESHOLD) {
            Servo_SetAngle(75.0f);     // ������Ͱ��
            lid_closing_scheduled = 0; // ȡ���Ѽƻ��Ĺظ�
        }
    } else {
        if (trigger_count >= TRIGGER_THRESHOLD) { // ֮ǰ�Ǵ�״̬
            if (!lid_closing_scheduled) {
                // �����ӳٹر�ʱ��
                lid_close_time        = system_runtime_ms + CLOSE_DELAY_MS;
                lid_closing_scheduled = 1;
            }
        }
        trigger_count = 0; // ���ô���������
    }

    // ����Ƿ���Ҫ�ر�����Ͱ��
    if (lid_closing_scheduled && system_runtime_ms >= lid_close_time) {
        Servo_SetAngle(0.0f); // �ر�����Ͱ��
        lid_closing_scheduled = 0;
    }
}

void ProcessSerialCommands(void)
{
    if (USART1_NewCmd) {
        if (USART1_RX_CMD == 0x11) {
            Servo_SetAngle(75.0f); // ������Ͱ��
        } else if (USART1_RX_CMD == 0x22) {
            Servo_SetAngle(0.0f); // �ر�����Ͱ��
        }
        USART1_NewCmd = 0; // ��������־λ
    }
}

void Sys_Init(void)
{
    ADCx_Init(ADC1);    // Initialize ADC1
    OLED_Init();        // Initialize OLED display
    LED_All_Init();     // Initialize LED
    Servo_Init();       // Initialize servo motor
    Buzzer_Init();      // Initialize buzzer
    usart1_Init(9600);  // Initialize USART1 with baud rate 9600
    UART3_Init(9600);   // ��ʼ������3
    CountSensor_Init(); // Initialize red infrared sensors
    MQ2_Init();         // Initialize MQ2 smoke sensor
    Timer_Init();       // Initialize timer for tracking
    HC_SR04_Init();     // Initialize ultrasonic sensor
    DS1302_GPIO_Init(); // Initialize DS1302 (time kept by backup battery)
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

    // ���⴫��������ֵ��0-���ڵ���1-δ�ڵ�
    if (bottom_sensor && top_sensor) {
        trash_status      = 0;                // ������������δ���ڵ�������Ͱ��
        last_cleanup_time = system_runtime_s; // ���������ʱ��
    } else if (!bottom_sensor && top_sensor) {
        trash_status = 1; // �ײ����ڵ���������
    } else if (bottom_sensor && !top_sensor) {
        trash_status = 1; // �������ڵ���������
    } else if (!bottom_sensor && !top_sensor) {
        trash_status = 2; // ���������������ڵ�������Ͱ����
    }

    if (old_status != trash_status) {
        display_needs_update = 1;
    }
}

void CheckSmoke(void)
{
    uint16_t smoke_ppm_value = MQ2_GetData_PPM();                        // ��ȡPPMֵ
    smoke_alert_active       = (smoke_ppm_value >= SMOKE_THRESHOLD_PPM); // ��PPM��ֵ�Ƚ�
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

    // ��������Ͱ�ǿ�ʱ��鳬ʱ
    if (trash_status != 0) {
        cleanup_alert_active = (time_since_cleanup >= CLEANUP_TIMEOUT_S);
    } else {
        cleanup_alert_active = 0;
    }
    display_needs_update = 1; // ��Ҫ������ʾʱ��
}

void UpdateStatusIndicators(void)
{
    // ���ȼ����������� > ����ʱ���� > ����Ͱ������ > ����״ָ̬ʾ
    if (smoke_alert_active) {
        // ������������ȼ�
        LED1_OFF();   // �̵���
        LED2_OFF();   // �Ƶ���
        LED_Sys_ON(); // �����
        Buzzer_ON();  // ����������
    } else if (cleanup_alert_active) {
        // ����ʱ���������ȼ�
        LED1_OFF();   // �̵���
        LED2_OFF();   // �Ƶ���
        LED_Sys_ON(); // �����
        Buzzer_ON();  // ����������
    } else {
        // ����״ָ̬ʾ
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

        /* ��ʾ����Ͱ״̬�;��� */
        OLED_ShowString(0, 0, "����:", OLED_8X16);
        if (trash_status == 0) {
            OLED_ShowString(40, 0, "��", OLED_8X16);
        } else if (trash_status == 1) {
            OLED_ShowString(40, 0, "��", OLED_8X16);
        } else {
            OLED_ShowString(40, 0, "��", OLED_8X16);
        }

        /* ��ʾ���� */
        uint8_t avg_distance = calculate_average_distance();
        OLED_ShowString(80, 0, "D:", OLED_8X16);
        OLED_ShowNum(96, 0, avg_distance / 10, 3, OLED_8X16);

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

        /* ��ʾʵʱ���ں�ʱ�� */
        DS1302_read_realTime();

        // ��ʾ������
        OLED_ShowNum(0, 32, TimeData.year, 4, OLED_8X16);
        OLED_ShowString(32, 32, "/", OLED_8X16);
        OLED_ShowNum(40, 32, TimeData.month, 2, OLED_8X16);
        OLED_ShowString(56, 32, "/", OLED_8X16);
        OLED_ShowNum(64, 32, TimeData.day, 2, OLED_8X16);

        // ��ʾʱ����
        OLED_ShowNum(0, 48, TimeData.hour, 2, OLED_8X16);
        OLED_ShowString(16, 48, ":", OLED_8X16);
        OLED_ShowNum(24, 48, TimeData.minute, 2, OLED_8X16);
        OLED_ShowString(40, 48, ":", OLED_8X16);
        OLED_ShowNum(48, 48, TimeData.second, 2, OLED_8X16);

        /* ��ʾ����ֵ */
        OLED_ShowString(72, 48, "P:", OLED_8X16);
        uint16_t smoke_ppm = MQ2_GetData_PPM();
        OLED_ShowNum(88, 48, smoke_ppm, 4, OLED_8X16);

        OLED_Update();
        display_needs_update = 0;
    }
}
