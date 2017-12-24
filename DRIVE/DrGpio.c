#include "main.h" 



void DrGpioInit( void )
{
    
    //GPIO_InitTypeDef GPIO_InitStructure;
    
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
  	
    
}


void DrLed(void)
{
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin     = LEDR_PIN;
  	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
  	GPIO_Init(LEDR_PORT, &GPIO_InitStructure);
    LED_R(ON);
    
    GPIO_InitStructure.GPIO_Pin     = LEDG_PIN;
  	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
  	GPIO_Init(LEDG_PORT, &GPIO_InitStructure);
    LED_G(ON);
    
}



