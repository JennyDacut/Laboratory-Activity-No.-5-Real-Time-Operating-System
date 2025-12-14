#include "LCD.h"

int main(void)
{
    HAL_Init();
    LCD_Init();
    LCD_Clear();
    LCD_ShowString(10, 10, "Hello World");
    while(1)
    {
        // main loop
    }
}
