#include "main.h"

const unsigned long SystemFrequency = 72000000; /* 主频72Mhz */

#define STKSIZE_MAIN                     STK_SIZE_64  //main函数的堆栈大小，经验值，修改需慎重
OS_STK STK_Buffer_main[STKSIZE_MAIN];


/*
** main主函数
*/
int main(void)
{   
    Config_DrCommon();  //所有外设的初始化，包含串口、定时器、W5500、IO等
    
    SysTick_Config(SystemFrequency/OS_TICKS_PER_SEC);  //TICK初始化
	
    OSInit();  //OS初始化，
    
	OSTaskCreate(Task_Main,(void *)0,&STK_Buffer_main[STKSIZE_MAIN-1], TASK_PRIO_MAIN);//创建主task
    
	OSStart();//启动OS，系统将由OS接管，正常情况下不会执行到下一行
    return 0;
}


/*
** UCOSII任务启动之前的初始化
*/
void Config_DrCommon(void)
{
    //RCC_DeInit();
    SystemInit();
    //NVIC_DeInit();
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);  //设置中断向量表的偏移地址，用于bootloader模式下
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3); //中断级别为1，一个位表示抢占级别，3个位表示中断优先级，数字越小级别越高
    ReadFlashCfg();//读FLASH数据
    DrLed();
    DrTimer_Init();
    DrUsart_Init();
    DrCAN_Init();
    #ifdef W5500_ENABLE
        W5500_SPI_Config();
        Ethernet_Init();
    #endif
    
    /* 独立看门狗，时间=64/40k*312=500ms */
    
}




