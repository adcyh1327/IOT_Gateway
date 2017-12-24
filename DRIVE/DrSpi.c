#include "main.h"

/******w5500寄存器配置********/
static void W5500_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    RCC_SPI1_RELATE_IO_ENABLE;
	//PA4->CS,PA5->SCK,PA6->MISO,PA7->MOSI		 					 
	GPIO_InitStructure.GPIO_Pin = SPI1_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SPI1_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(SPI1_GPIO_PORT,SPI1_GPIO_PIN);
	//初始化片选输出引脚
	GPIO_InitStructure.GPIO_Pin = W5500_CS_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(W5500_CS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(W5500_CS_PORT, W5500_CS_PIN);
    //初始化硬件复位输出引脚
	GPIO_InitStructure.GPIO_Pin = W5500_RST_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(W5500_RST_PORT, &GPIO_InitStructure);
	GPIO_SetBits(W5500_RST_PORT, W5500_RST_PIN);
}


/* SPI1配置 */
void DrSpi1_Config(void)
{
	SPI_InitTypeDef SPI_InitStruct;
	RCC_SPI1_PERIPH_ENABLE;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStruct);
	
	SPI_Cmd(SPI1, ENABLE);
}



/* SPI2配置 */
void DrSpi2_Config(void)
{
    

}


void W5500_SPI_Config(void)
{
    W5500_GPIO_Configuration();
    DrSpi1_Config();
    //硬件复位
    W5500_RST_ENABLE;
    Delay1ms(30);
    W5500_RST_DISABLE;
    Delay1ms(50);
}
