#include "DK_C8T6.h" // Device header

void main(void)
{
    Sys_Init();                                       // 系统初始化函数
    OLED_Init();                                      // OLED初始化函数
    OLED_ShowString(0, 0, "烟雾", OLED_8X16); // 显示字符串
    OLED_Update();                                    // 更新显示
    while (1) {
        // 主循环代码
    }
}