#ifndef __MAIN_H
#define __MAIN_H



#ifdef __cplusplus
 extern "C" {
#endif

     

/* C��׼�� */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t_list.h"
/* ��ǰ���� */
#include "platform_cfg.h"
#include "variable.h"

/* uCOS-II����ϵͳ */
#include "includes.h"

/* �����ͷ�ļ� */
#include "stm32f10x.h"

/* FWLIB */
/* ������������⺯����ͷ�ļ� */
#include "stm32f10x_conf.h"

/* ������ͷ�ļ� */
#include "DrTimer.h"
#include "DrGpio.h"
#include "DrSpi.h"
#include "DrFlash.h"
#include "DrUart.h"
#include "DrCan.h"
#include "DrEthernet.h"

/* ���� */
#include "Task_Main.h"




void Task_Main(void *p_arg);
void Config_DrCommon(void);

	

#ifdef __cplusplus
}
#endif



#endif
