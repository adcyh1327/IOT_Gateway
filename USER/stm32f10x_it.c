/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "includes.h"
#include "main.h"
  

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned long ulSrandCount = 0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void OS_CPU_PendSVHandler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void OS_CPU_SysTickHandler(void)
{
    OSIntEnter();		//进入中断
    OSTimeTick();       //调用ucos的时钟服务程序 
    ulSrandCount++;
    OSIntExit();        //触发任务切换软中断
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 



void TIM2_IRQHandler(void)
{
    OSIntEnter();
    
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
        /* 100us */
        TIM2_IRQ();
    }
    
    OSIntExit();
}

void TIM3_IRQHandler(void)
{
    OSIntEnter();
    
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);
        /* 1ms */
        TIM3_IRQ();
    }
    
    OSIntExit();
}


void TIM4_IRQHandler(void)
{
    OSIntEnter();
    
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);
        /* 100ms */
        TIM4_IRQ();
    }
    
    OSIntExit();
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    #ifdef STM32F10X_HD
    CanRxMsg  RxMessage;
	CAN_ClearITPendingBit(CAN1,CAN_IT_FMP0);
	CAN_Receive(CAN1, CAN_FIFO0, &(RxMessage));
    CAN1_IRQ(&RxMessage);
    #endif
}

void USART1_IRQHandler(void)  
{
    u8 data;
    if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)
    {  
        
        data = USART1->DR;//
        USART1_IRQ(data);
    }
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) 
    {
        USART_ClearITPendingBit(USART1, USART_IT_TXE);           // Clear the USART transmit 
    }  
    USART1->DR;
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    USART_ClearITPendingBit(USART1, USART_IT_TXE);           // Clear the USART transmit 
}

void USART2_IRQHandler(void)  
{
    u8 data;
    if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET)
    {  
        data = USART2->DR;//
        USART2_IRQ(data);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
    if (USART_GetITStatus(USART2, USART_IT_TC)) 
    {
        USART_ITConfig(USART2, USART_IT_TC, DISABLE);
        USART_ClearITPendingBit(USART2, USART_IT_TXE);           // Clear the USART transmit 
    }  
    USART2->DR;
}

void USART3_IRQHandler(void)  
{
    u8 data;
    if(USART_GetITStatus(USART3,USART_IT_RXNE) != RESET)
    {  
        
        data = USART3->DR;//
        USART3_IRQ(data);
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
    if (USART_GetITStatus(USART3, USART_IT_TC)) 
    {
        USART_ITConfig(USART3, USART_IT_TC, DISABLE);
        USART_ClearITPendingBit(USART3, USART_IT_TXE);           // Clear the USART transmit 
    }  
    USART3->DR;
}

void UART4_IRQHandler(void)  
{
    u8 data;
    if(USART_GetITStatus(UART4,USART_IT_RXNE) != RESET)
    {  
        
        data = UART4->DR;//
        UART4_IRQ(data);
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
    }
    if (USART_GetITStatus(UART4, USART_IT_TC)) 
    {
        USART_ITConfig(UART4, USART_IT_TC, DISABLE);
        #ifdef UART_DMA_ENABLE
            UART4_485_RX_ENABLE;
        #endif
        USART_ClearITPendingBit(UART4, USART_IT_TXE);           // Clear the USART transmit 
    }  
    UART4->DR;
}

void UART5_IRQHandler(void)  
{
    u8 data;
    if(USART_GetITStatus(UART5,USART_IT_RXNE) != RESET)
    {  
        
        data = UART5->DR;//
        UART5_IRQ(data);
    }
    if (USART_GetITStatus(UART5, USART_IT_TXE) != RESET) 
    {
        USART_ClearITPendingBit(UART5, USART_IT_TXE);           // Clear the USART transmit 
    }  
    UART5->DR;
    USART_ClearITPendingBit(UART5, USART_IT_RXNE);
    USART_ClearITPendingBit(UART5, USART_IT_TXE);           // Clear the USART transmit 
}



/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
