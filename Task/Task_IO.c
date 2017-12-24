#include "main.h"



void Task_IO(void *p_arg)
{
    
    (void)p_arg;
    
    DrGpioInit();
    
    while (1)
    {
        OSTimeDlyHMSM(0, 0, 0, 20);
    }
    
}





