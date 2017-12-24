#include "main.h"
#include "httputil.h"

#ifdef __VARIABLE_H_

char platform_version[] = {"MJ-Trans-V1.03 20170405"};
char funcTion[] = {"��������-Э��ת����ͨ�ð�"};

const  u32   APPL_CRC __attribute__((at(APP_CRC_ADDR)))={0xA1A2A3A4};

//�����������
wiz_NetInfo gWIZNETINFO = { 
                            .session_mode = S_mqtt,//��̫������ģʽ
                            .conn_status = OFF,//socket����״̬
                            .stationID = 1,//modbusվ��ַ����MQTTʱ��Ӧ�ı��ڵ�ID
                            .mbtcp_addr = 0x0,//mb���õ���ʼ��ַ
                            .polltime = 1000,//��mb clientʱ��server���ڷ��Ͷ�����
                            .tcpsocket = SOCK_MQTT,//socket 
                            .mac = {0x48, 0x53, 0x00, 0x00, 0xab, 0xcd},//mac��ַ�����3λ����ΪIP�ĺ�3���ֽ�
                            .iplocal = {192, 168, 88, 60},//����IP��ַ
                            .portlocal=PORT_LOCAL,//���ض˿ںţ�Ĭ��Ϊ5000
                            .sn = {255, 255, 255, 0},//��������
                            .gw = {192, 168, 88, 1},//���ص�ַ
                            .dns = {0, 0, 0, 0},
                            .dhcp = NETINFO_STATIC,
                            .ipgoal = {192, 168, 88, 76}, //Ŀ��IP��ַ
                            .portgoal=1883,//Ŀ��˿ں�
                           }; //Ŀ��˿�

wiz_NetTimeout gWIZNetTimeout =
                           {
                               .retry_cnt = 2,         //���Դ���
                               .time_100us = 25000     //����2�Σ�ÿ�μ��2.5s
                           };

                           
Interface_Info USARTCAN = { 
                            .Usart[RS232_1] = {1,6, 0, 0, 0, 0,0}, //RS232  // USER 19200 8 ODD 1 CTS 
                            .Usart[RS232_2] = {1,6, 0, 0, 0, 0,0},
                            .Usart[RS485] =   {1,6, 0, 0, 0, 0,0},
                            .UsartProt[RS232_1]=
                            {
                                .FrameStartInfo = 0,//FrameStartEn|byte_1,  //������Ϊ0ʱ��Ϊ��Э�飬�������ݾ�Ϊ��Ч���ݣ����ֽڼ�ʱ�䳬������ʱ�����Ϊ������һ֡
                                .FrameStart = {0x02},
                                .FrameEndInfo = 0,//FrameStartEn|byte_2,//ͬ֡ͷ�������ƣ�ǰ�ߵĺ�ʹ�ܲű�ʾ��֡β�����һ��Ϊ�ֽ�����
                                .FrameEnd = {0x0d,0x0a,},
                                .checksum = CheckSum_None//��ǰ��0,����ʵ����Ҫ����Ҫ����
                            },
                            .UsartProt[RS232_2]=
                            {
                                .FrameStartInfo = 0,//FrameStartEn|byte_1,
                                .FrameStart = {0x02},
                                .FrameEndInfo = 0,//FrameStartEn|byte_2,
                                .FrameEnd = {0x0d,0x0a,},
                                .checksum = CheckSum_None
                            },
                            .UsartProt[RS485]=
                            {
                                .FrameStartInfo = 0,//FrameStartEn|byte_1,
                                .FrameStart = {0x02},
                                .FrameEndInfo = 0,//FrameStartEn|byte_2,
                                .FrameEnd = {0x0d,0x0a,},
                                .checksum = CheckSum_None
                            },
                            .can = {1,3,0x700,0x701,1,0},
                            .datalen=50,//���ڻ�CAN��ÿ��ͨ���������ݵ��ֳ���
                            .tout = 12,  //����֡����������ֽڼ�ʱ������������
                          };

                          
const u32 RS232_baud[12] = {25600,12800,115200,57600,56000,38400,19200,14400,9600,4800,2400,1200};//���ڲ���������
const u16 RS232_lenth[2] = {USART_WordLength_8b,USART_WordLength_9b};//����λ����
const u16 RS232_parity[3] = {USART_Parity_No,USART_Parity_Odd,USART_Parity_Even};//����У��λ����
const u16 RS232_stop[2]  ={USART_StopBits_1,USART_StopBits_2};//ֹͣλ����
const u16 RS232_FlowCntl[4] = {USART_HardwareFlowControl_None,USART_HardwareFlowControl_RTS_CTS,};//��������
const u16 CAN_baud[14] ={0,1,2,3,4,5,6,7,8,9,10,11,12,13};//{1000,800,666,500,400,250,200,125,100,80,50,40,20,10};//CAN����������

struct USARTCAN_Recv_t USARTCAN_Recv[NUM_UARTCAN];//���ڻ�CAN�Ľ��ձ��ݻ�������ʶ����Ч֡���ȡ����ֱ�ӿ�������̫��������


char DataType[t_typemax][20]=//�������ͣ���Ҫ����MQTT��JSON����ʽ������
{
    {"t_HEX"},
    {"t_ASCII"},
    {"t_BOOL"},
};



u16 g_u16_TCPIPsendlen;           //tcpip���ķ��ͳ���

u16 cpu_sr;                        //cpu�ж�״̬

u8 Check_XOR(u8 *data,u8 lenth)//���У��
{
    u8 i,temp;
    temp=0;
    for(i=0;i<lenth;i++)
    {
        temp ^= data[i];
    }
    return temp;
}

unsigned char AscToHex(unsigned char aChar)//ASCIIתhex�����������Χ0x00-0x0F�����뷶ΧΪ0x30-0x39,0x41-0x46��0x61-0x66
{
    unsigned char ret;
    if((aChar >= 0x30)&&(aChar <= 0x39))
    {
        ret = aChar - 0x30;
    }
    else if((aChar >= 0x41)&&(aChar <= 0x46))
    {
        ret = aChar - 0x37;
    }
    else if((aChar >= 0x61)&&(aChar <= 0x66))
    {
       ret = aChar - 0x57;
    }
    else
    {        
       ret = 0xFF;
    }
    return ret;
}

unsigned char HexToAsc(unsigned char aHex)//HEXתASCII�����������Χ0x30-0x39,0x41-0x46,���뷶ΧΪ0x00-0x0F
{
    unsigned char ret;
    if(aHex <=9 )
    {
        ret = aHex + 0x30;
    }
    else if((aHex >= 10)&&(aHex <= 15)) 
    {
        ret =  aHex + 0x37;
    }
    else
    {
        ret = 0xFF;
    }
        return ret;
}





#endif









