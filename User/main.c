#include "DK_C8T6.h" // Device header

void main(void)
{
    Sys_Init();                                       // ϵͳ��ʼ������
    OLED_Init();                                      // OLED��ʼ������
    OLED_ShowString(0, 0, "����", OLED_8X16); // ��ʾ�ַ���
    OLED_Update();                                    // ������ʾ
    while (1) {
        // ��ѭ������
    }
}