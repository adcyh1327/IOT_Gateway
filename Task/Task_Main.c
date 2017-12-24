#include "main.h"
#include "Task_TCPIP.h"
#include "Task_ModbusTCp.h"
#include "Task_MQTT.h"
#include "Task_HTTP.h"
#include "Task_IO.h"
#include "Task_LED.h"
#include "transport.h"

#define STKSIZE_LEDR                     STK_SIZE_32
#define STKSIZE_LEDG                     STK_SIZE_32
#define STKSIZE_TCPIP                    STK_SIZE_512
#define STKSIZE_MBTCP                    STK_SIZE_256
#define STKSIZE_MQTT                     STK_SIZE_1024                   
#define STKSIZE_HTTP                     STK_SIZE_512
#define STKSIZE_BACKGRD                  STK_SIZE_1024

typedef  void (*FunVoidType)(void);
#define ApplicationMsp          0x8008000 
#define ApplicationVect         (ApplicationMsp+4)

OS_STK STK_ledR[STKSIZE_LEDR];
OS_STK STK_ledG[STKSIZE_LEDG];
#ifdef TCPIP_ENABLE
OS_STK STK_TCPIP[STKSIZE_TCPIP];
#endif
#ifdef MBTCP_ENABLE
OS_STK STK_MBTCP[STKSIZE_MBTCP];
#endif
#ifdef MQTT_ENABLE
OS_STK STK_MQTT[STKSIZE_MQTT];
#endif
#ifdef HTTP_ENABLE
OS_STK STK_HTTP[STKSIZE_HTTP];
#endif

OS_STK STK_BACKGRD[STKSIZE_BACKGRD];


void f_GenSoftwareReset(void);
void Task_BackGround(void *p_arg);

// #pragma arm section code=".ARM.__at_0x08000000"
// void f_JumpAppl(void)
// {
//     u32   m_JumpAddress;	
//     FunVoidType JumpToApplication;          /* call this function to jump to appl */
//     if (((*(vu32*)ApplicationMsp) & 0x2FFE0000 ) == 0x20000000)/*MSP check*/
//     {
//         /* Jump to user application */
//         m_JumpAddress = *(vu32*) (ApplicationVect);
//         JumpToApplication = (FunVoidType) m_JumpAddress;
//         /* Initialize user application's Stack Pointer */
//         __MSR_MSP(*(vu32*) ApplicationMsp);
//         JumpToApplication();	 
//         while(1);	  
//     }
// }
// #pragma arm section

void Task_Main(void *p_arg)
{
    
    (void)p_arg;

	OSTaskCreate(Task_LedR, (void *)&gWIZNETINFO, (OS_STK*)&STK_ledR[STKSIZE_LEDR-1], TASK_PRIO_LEDR);
    OSTaskCreate(Task_LedG, (void *)0, (OS_STK*)&STK_ledG[STKSIZE_LEDG-1], TASK_PRIO_LEDG);//LED���񴴽�
    
    #ifdef TCPIP_ENABLE
    if((gWIZNETINFO.session_mode==S_tcpip_client)||(gWIZNETINFO.session_mode==S_tcpip_server))
    {
        //OSTaskCreate(Task_TCPIP, (void *)&gWIZNETINFO, (OS_STK*)&STK_TCPIP[STKSIZE_TCPIP-1], TASK_PRIO_TCPIP);//����TCPIP������
    }
    #endif
    #ifdef MBTCP_ENABLE
    if((gWIZNETINFO.session_mode==S_mb_client)||(gWIZNETINFO.session_mode==S_mb_server))
    {
        //OSTaskCreate(Task_ModbusTCP, (void *)&gWIZNETINFO, (OS_STK*)&STK_MBTCP[STKSIZE_MBTCP-1], TASK_PRIO_MBTCP);//����MODBUS TCP����
    }
    #endif
    
    #ifdef MQTT_ENABLE
    if(gWIZNETINFO.session_mode==S_mqtt)
    {
        OSTaskCreate(MQTT_task, (void *)&gWIZNETINFO, &STK_MQTT[STKSIZE_MQTT-1], TASK_PRIO_MQTT); //����MQTT
    }
    #endif
    
    #ifdef HTTP_ENABLE
    //OSTaskCreate(Task_HTTP, (void *)&gWIZNETINFO, (OS_STK*)&STK_HTTP[STKSIZE_HTTP-1], TASK_PRIO_HTTP);//������ҳͨѶ����
 	#endif
    
    OSTaskCreate(Task_BackGround, (void *)&gWIZNETINFO, (OS_STK*)&STK_BACKGRD[STKSIZE_BACKGRD-1], TASK_PRIO_BACKGRD);//������̨task��������ѯ��״̬�ģ����жϴ��ڻ�CAN�Ƿ��յ���֡
    
    OSTaskSuspend(TASK_PRIO_MAIN);
    
}


void Task_BackGround(void *p_arg)
{
    #ifdef WATCHDOG_ENABLE
        IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
        IWDG_SetPrescaler(IWDG_Prescaler_256);//ʱ�ӷ�Ƶ40k/256=156hz��6.4ms��
        IWDG_SetReload(468);//���Ź���ʱʱ��Ϊ3s ���ܴ���0xfff��4095�� 781Ϊ5s
        IWDG_ReloadCounter();
        IWDG_Enable();
    #endif
    struct wiz_NetInfo_t * Ethparm;
    Ethparm = (struct wiz_NetInfo_t *)p_arg;
    u8 i;
    while(1)
    {
        IWDG_ReloadCounter();
        W5500_StatusDetect();
        for(i=0;i<NUM_UARTCAN;i++)//�����˿�����ͨ��
        {
            if(USARTCAN_Recv[i].newupd==ON)//�Ƿ��յ���֡
            {
                LED_BLINK_ONCE(S_NORMAL);//�յ���֡����SYS����˸һ��
                if(Ethparm->conn_status)//ֻ�ж˿ڽ��������˲Ż�����ת��
                {
                    USARTCAN_Recv[i].newupd=OFF;
                    if(Ethparm->session_mode==S_mqtt)
                    {
                        MQTT_DataHandler(i,&USARTCAN_Recv[i]);
                    }
                    else if((Ethparm->session_mode==S_tcpip_client)||(Ethparm->session_mode==S_tcpip_server))
                    {
                        //TCPIP_DataHandler(i,&USARTCAN_Recv[i],p_arg);
                    }
                    else if((Ethparm->session_mode==S_mb_client)||(Ethparm->session_mode==S_mb_server))
                    {
                        //MBTCP_DataHandler(i,&USARTCAN_Recv[i],p_arg);
                    }
                    else
                    {
                        LED_BLINK_ONCE(S_FAULT);
                        //errorcode
                    }
                }
            }
        }
        OSTimeDlyHMSM(0, 0, 0, 1);//�ز����٣��������ȼ��͵�����ò�������
    }
}

//��̫������ת���ڻ�CAN
void ETH2Usartcan_send(u8 uartcan_chn,u8 *databuf,u16 lenth)
{
    u16 canid;
    OS_ENTER_CRITICAL();
    switch(uartcan_chn)
    {
        case RS232_1:
            if(USARTCAN.Usart[RS232_1][EnUart]==ON)//�ö˿ڱ������ó�ʹ�ܲŻ����ת�������򱨴�
            {    
                USART2_Send_Data(databuf,lenth);
            }
            else
            {
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            break;
        case RS232_2:
            if(USARTCAN.Usart[RS232_2][EnUart]==ON)//�ö˿ڱ������ó�ʹ�ܲŻ����ת�������򱨴�
            {
                USART3_Send_Data(databuf,lenth);
            }
            else
            {
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            break;
        case RS485:
            if(USARTCAN.Usart[RS485][EnUart]==ON)//�ö˿ڱ������ó�ʹ�ܲŻ����ת�������򱨴�
            {
                UART4_Send_Data(databuf,lenth);
            }
            else
            {
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            break;
        case CAN_CHN:
            if(USARTCAN.can[EnCAN]==ON)//�ö˿ڱ������ó�ʹ�ܲŻ����ת�������򱨴�
            {
                if(lenth<=8)
                {
                    canid = (databuf[0]<<8) + (databuf[1]);
                    if(CAN1_SendData(canid,&databuf[2],lenth)!=ON)
                    {
                        LED_BLINK_ONCE(S_FAULT);
                        //errorcode
                    }
                }
                else
                {
                    LED_BLINK_ONCE(S_FAULT);
                    //errorcode
                }
            }
            else
            {
                LED_BLINK_ONCE(S_FAULT);
                //errorcode
            }
            break;
        default:
            LED_BLINK_ONCE(S_FAULT);
            //errorcode
            break;
    }
    OS_EXIT_CRITICAL();
    LED_BLINK_ONCE(S_NORMAL);
}

//��flash�����ݣ�˳�����͵�ַ���ñ��һ�£��������ִ���
void ReadFlashCfg(void)
{
    struct EthernetCfg_t *readdata;
    u8 flashdata[sizeof(struct EthernetCfg_t)];
    FlashReadData(CONFIG_BASEADDR,flashdata,sizeof(struct EthernetCfg_t));
    readdata = (struct EthernetCfg_t *)&flashdata;
    
    if((readdata->cfgflag[0]==0xAA)&&(readdata->cfgflag[1]==0x55)&&(readdata->cfgflag[2]==0xAA)&&(readdata->cfgflag[3]==0x55))
    {
   
    }
    else
    {
        memcpy(&gWIZNETINFO.mac[3],&gWIZNETINFO.iplocal[1],3);//mac����ip�仯
        return;
    }
    
    if(readdata->session_mode > S_boundary)
    {
        memcpy(&gWIZNETINFO.mac[3],&gWIZNETINFO.iplocal[1],3);//mac����ip�仯
        return;
    }
    gWIZNETINFO.session_mode = readdata->session_mode;
    gWIZNETINFO.stationID = readdata->stationID;
    //Ԥ��1
    gWIZNETINFO.mbtcp_addr = readdata->mbtcp_addr;
    USARTCAN.datalen = readdata->mbtcp_datalen;
    gWIZNETINFO.polltime = readdata->polltime;
    //Ԥ��2
    
    memcpy(gWIZNETINFO.iplocal,readdata->localIP,4);
    gWIZNETINFO.portlocal = readdata->localport;
    memcpy(&gWIZNETINFO.mac[3],&gWIZNETINFO.iplocal[1],3);//mac����ip�仯
    //Ԥ��3
    memcpy(gWIZNETINFO.ipgoal,readdata->remoteIP,4);
    gWIZNETINFO.portgoal = readdata->remoteport;
    //Ԥ��4
    memcpy(gWIZNETINFO.sn,readdata->submask,4);
    memcpy(gWIZNETINFO.gw,readdata->gatewayaddr,4);
    //Ԥ��5
    
    USARTCAN.can[EnCAN] = readdata->can_en;
    USARTCAN.can[canBaudrate] = readdata->canbaudrate;
    USARTCAN.can[LocalID] = readdata->can_localID;
    USARTCAN.can[DeviceID] = readdata->can_deviceID;
    USARTCAN.can[IDNum] = readdata->can_device_num;
    USARTCAN.can[canDatatype] = readdata->can_datatype;
    //Ԥ��6
    
    USARTCAN.Usart[RS232_1][EnUart] = readdata->rs232_1_en;
    USARTCAN.Usart[RS232_1][uartBaudrate] = readdata->rs232_1_baudrate;
    USARTCAN.Usart[RS232_1][Databits] = readdata->rs232_1_databit;
    USARTCAN.Usart[RS232_1][Chkbits] = readdata->rs232_1_chkbit;
    USARTCAN.Usart[RS232_1][Stopbits] = readdata->rs232_1_stopbit;
    USARTCAN.Usart[RS232_1][Flowctrl] = readdata->rs232_1_flowctrl;
    USARTCAN.Usart[RS232_1][uartDatatype] = readdata->rs232_1_datatype;
    //Ԥ��7
    
    USARTCAN.Usart[RS232_2][EnUart] = readdata->rs232_2_en;
    USARTCAN.Usart[RS232_2][uartBaudrate] = readdata->rs232_2_baudrate;
    USARTCAN.Usart[RS232_2][Databits] = readdata->rs232_2_databit;
    USARTCAN.Usart[RS232_2][Chkbits] = readdata->rs232_2_chkbit;
    USARTCAN.Usart[RS232_2][Stopbits] = readdata->rs232_2_stopbit;
    USARTCAN.Usart[RS232_2][Flowctrl] = readdata->rs232_2_flowctrl;
    USARTCAN.Usart[RS232_2][uartDatatype] = readdata->rs232_2_datatype;
    //Ԥ��8
    
    USARTCAN.Usart[RS485][EnUart] = readdata->rs485_en;
    USARTCAN.Usart[RS485][uartBaudrate] = readdata->rs485_baudrate;
    USARTCAN.Usart[RS485][Databits] = readdata->rs485_databit;
    USARTCAN.Usart[RS485][Chkbits] = readdata->rs485_chkbit;
    USARTCAN.Usart[RS485][Stopbits] = readdata->rs485_stopbit;
    USARTCAN.Usart[RS485][uartDatatype] = readdata->rs485_datatype;
    //Ԥ��9
    
    USARTCAN.tout = readdata->to_thres;
    //Ԥ��10
}

/*****************************************/
void dylms(unsigned int u)
{
		unsigned int x=0;
		unsigned int y=0;
		for(y=0;y<u;y++)
			for(x=0;x<10000;x++);
}

/**********************************/
void HTTP_DataHandler(void)
{
    struct EthernetCfg_t flashdata;
    flashdata.cfgflag[0] = 0xAA;
    flashdata.cfgflag[1] = 0x55;
    flashdata.cfgflag[2] = 0xAA;
    flashdata.cfgflag[3] = 0x55;
    flashdata.session_mode=gWIZNETINFO.session_mode;
    flashdata.stationID=gWIZNETINFO.stationID;
    //Ԥ��1
    flashdata.mbtcp_addr=gWIZNETINFO.mbtcp_addr;
    flashdata.mbtcp_datalen=USARTCAN.datalen;
    flashdata.polltime=gWIZNETINFO.polltime;
    //Ԥ��2
    
    memcpy(flashdata.localIP,gWIZNETINFO.iplocal,4);
    flashdata.localport=gWIZNETINFO.portlocal;
    //Ԥ��3
    memcpy(flashdata.remoteIP,gWIZNETINFO.ipgoal,4);
    flashdata.remoteport=gWIZNETINFO.portgoal;
    //Ԥ��4
    memcpy(flashdata.submask,gWIZNETINFO.sn,4);
    memcpy(flashdata.gatewayaddr,gWIZNETINFO.gw,4);
    //Ԥ��5
    
    flashdata.can_en = USARTCAN.can[EnCAN];
    flashdata.canbaudrate = USARTCAN.can[canBaudrate];
    flashdata.can_localID = USARTCAN.can[LocalID];
    flashdata.can_deviceID = USARTCAN.can[DeviceID];
    flashdata.can_device_num = USARTCAN.can[IDNum];
    flashdata.can_datatype = USARTCAN.can[canDatatype];
    //Ԥ��6
    
    flashdata.rs232_1_en = USARTCAN.Usart[RS232_1][EnUart];
    flashdata.rs232_1_baudrate = USARTCAN.Usart[RS232_1][uartBaudrate];
    flashdata.rs232_1_databit = USARTCAN.Usart[RS232_1][Databits];
    flashdata.rs232_1_chkbit = USARTCAN.Usart[RS232_1][Chkbits];
    flashdata.rs232_1_stopbit = USARTCAN.Usart[RS232_1][Stopbits];
    flashdata.rs232_1_flowctrl = USARTCAN.Usart[RS232_1][Flowctrl];
    flashdata.rs232_1_datatype = USARTCAN.Usart[RS232_1][uartDatatype];
    //Ԥ��7
    
    flashdata.rs232_2_en = USARTCAN.Usart[RS232_2][EnUart];
    flashdata.rs232_2_baudrate = USARTCAN.Usart[RS232_2][uartBaudrate];
    flashdata.rs232_2_databit = USARTCAN.Usart[RS232_2][Databits];
    flashdata.rs232_2_chkbit = USARTCAN.Usart[RS232_2][Chkbits];
    flashdata.rs232_2_stopbit = USARTCAN.Usart[RS232_2][Stopbits];
    flashdata.rs232_2_flowctrl = USARTCAN.Usart[RS232_2][Flowctrl];
    flashdata.rs232_2_datatype = USARTCAN.Usart[RS232_2][uartDatatype];
    //Ԥ��8
    
    flashdata.rs485_en = USARTCAN.Usart[RS485][EnUart];
    flashdata.rs485_baudrate = USARTCAN.Usart[RS485][uartBaudrate];
    flashdata.rs485_databit = USARTCAN.Usart[RS485][Databits];
    flashdata.rs485_chkbit = USARTCAN.Usart[RS485][Chkbits];
    flashdata.rs485_stopbit = USARTCAN.Usart[RS485][Stopbits];
    flashdata.rs485_datatype = USARTCAN.Usart[RS485][uartDatatype];
    //Ԥ��9
    
    flashdata.to_thres = USARTCAN.tout;
    //Ԥ��10
    
    OS_ENTER_CRITICAL();
    FlashErase(CONFIG_BASEADDR,1);
    FlashWriteData(CONFIG_BASEADDR,(u8 *)&flashdata,sizeof(flashdata));
    OS_EXIT_CRITICAL();
    dylms(500);
    f_GenSoftwareReset();//flash���º�������λ�������µ���������
}

void f_GenSoftwareReset(void)
{
    __set_FAULTMASK(1);
	SCB->AIRCR = AIRCR_VECTKEY_MASK | (u32)0x04;
}

