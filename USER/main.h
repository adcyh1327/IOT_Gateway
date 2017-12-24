#ifndef __MAIN_H
#define __MAIN_H



#ifdef __cplusplus
 extern "C" {
#endif

     

/* C标准库 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_list.h"
/* 提前声明 */
#include "platform_cfg.h"
#include "variable.h"

/* uCOS-II操作系统 */
#include "includes.h"

/* 外设的头文件 */
#include "stm32f10x.h"

/* FWLIB */
/* 包含所有外设库函数的头文件 */
#include "stm32f10x_conf.h"

/* 驱动层头文件 */
#include "DrTimer.h"
#include "DrGpio.h"
#include "DrSpi.h"
#include "DrFlash.h"
#include "DrUart.h"
#include "DrCan.h"
#include "DrEthernet.h"

/* 任务 */
#include "Task_Main.h"




void Task_Main(void *p_arg);
void Config_DrCommon(void);

	

#ifdef __cplusplus
}
#endif



#endif
