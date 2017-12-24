#ifndef AB_MICRO232_H
#define AB_MICRO232_H



#define MIT_FX_ACK              0x06        //PLC��ȷ��Ӧ
#define MIT_FX_NCK              0x15        //PLC������Ӧ
#define MIT_FX_STX              0x02        //���Ŀ�ʼ
#define MIT_FX_ETX              0x03        //���Ľ���
#define MIT_FX_ENQ              0x05        //����
#define MIT_FX_0D               0x0D        //0x0D
#define MIT_FX_0A               0x0A        //0x0A
#define TRUE                    1
#define FALSE                   0
#define NONE                    0
#define ERR                     -1

void AB_Micro232_Init(u8 nodeID);
void TIM_1ms_AB_Micro232(void);
void UART_AB_Micro232_Recv(u8 data);
void f_AB_Micro232_task(void);


#endif
