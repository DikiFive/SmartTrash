#include "DK_C8T6.h" // Device header

int main(void)
{
    Sys_Init();                               // ϵͳ��ʼ������
    OLED_ShowString(0, 0, "����", OLED_8X16); // ��ʾ�ַ���
    OLED_Update();                            // ������ʾ
    while (1) {
        Servo_SetAngle(75); // ������Ͱ
        Delay_ms(1000);      // ��ʱ1��
        Servo_SetAngle(0);  // �ر�����Ͱ
        Delay_ms(1000);
    }
}