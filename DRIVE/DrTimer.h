#ifndef __DR_TIMER_H
#define __DR_TIMER_H	  



/* �жϷ������ */
void TIM2_IRQ(void);
void TIM3_IRQ(void);
void TIM4_IRQ(void);
    

/* ����ʱ����ʼ��ʹ�� */
void Delay1ms(unsigned int  l_u8Count);


/***************************��ʼ��*******************************/
void DrTimer_Init(void);


#endif


