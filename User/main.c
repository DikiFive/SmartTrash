#include "DK_C8T6.h" // Device header

int main(void)
{
    Sys_Init();                               // 系统初始化函数
    OLED_ShowString(0, 0, "烟雾", OLED_8X16); // 显示字符串
    OLED_Update();                            // 更新显示
    while (1) {
        Servo_SetAngle(75); // 打开垃圾桶
        Delay_ms(1000);      // 延时1秒
        Servo_SetAngle(0);  // 关闭垃圾桶
        Delay_ms(1000);
    }
}