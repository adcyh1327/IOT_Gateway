#include "main.h"
#include "httputil.h"
#include "variable.h"
#include "webpage.h"
#include "wizchip_conf.h"

extern char tx_buf[MAX_URI_SIZE];
extern char rx_buf[MAX_URI_SIZE];
//extern void network_init(void); 

u8 webdata[6];
enum AddrIndex{
    idx_sid=1,idx_mbaddr,idx_mblen,idx_polltm,idx_lcport,idx_remport,idx_lcid,idx_devid,idx_devnum,idx_totim
};

static void make_basic_config_setting_json_callback(char* buf)
{
  sprintf(buf,"settingsCallback({\
  				\"ver\":\"%s\",\
                \"fuc\":\"%s\",\
                \"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\
				\"protocol\":\"%d\",\
				\"sid\":\"%d\",\
				\"modaddr\":\"%d\",\
                \"modlen\":\"%d\",\
                \"modtim\":\"%d\",\
                \"ip\":\"%d.%d.%d.%d\",\
				\"locport\":\"%d\",\
                \"ipgoal\":\"%d.%d.%d.%d\",\
                \"portgoal\":\"%d\",\
                \"sub\":\"%d.%d.%d.%d\",\
                \"gw\":\"%d.%d.%d.%d\",\
                \"use4\":\"%d\",\
                \"baud4\":\"%d\",\
                \"CANlocID4\":\"%x\",\
                \"CANresID4\":\"%x\",\
                \"CANresIDnum4\":\"%d\",\
                \"datatype4\":\"%d\",\
                \"use1\":\"%d\",\
                \"baud1\":\"%d\",\
                \"databit1\":\"%d\",\
                \"parity1\":\"%d\",\
                \"stopbit1\":\"%d\",\
                \"flow1\":\"%d\",\
                \"datatype1\":\"%d\",\
                \"use2\":\"%d\",\
                \"baud2\":\"%d\",\
                \"databit2\":\"%d\",\
                \"parity2\":\"%d\",\
                \"stopbit2\":\"%d\",\
                \"flow2\":\"%d\",\
                \"datatype2\":\"%d\",\
                \"use3\":\"%d\",\
                \"baud3\":\"%d\",\
                \"databit3\":\"%d\",\
                \"parity3\":\"%d\",\
                \"stopbit3\":\"%d\",\
                \"datatype3\":\"%d\",\
                \"tout\":\"%d\",\
                });",platform_version,funcTion,
                gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5],
				gWIZNETINFO.session_mode,gWIZNETINFO.stationID,
				gWIZNETINFO.mbtcp_addr,USARTCAN.datalen,gWIZNETINFO.polltime,
                gWIZNETINFO.iplocal[0],gWIZNETINFO.iplocal[1],gWIZNETINFO.iplocal[2],gWIZNETINFO.iplocal[3],gWIZNETINFO.portlocal,
                gWIZNETINFO.ipgoal[0],gWIZNETINFO.ipgoal[1],gWIZNETINFO.ipgoal[2],gWIZNETINFO.ipgoal[3],gWIZNETINFO.portgoal,
                gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3],gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3],
                USARTCAN.can[EnCAN],USARTCAN.can[canBaudrate],USARTCAN.can[LocalID],USARTCAN.can[DeviceID],USARTCAN.can[IDNum],USARTCAN.can[canDatatype],
                USARTCAN.Usart[RS232_1][EnUart],USARTCAN.Usart[RS232_1][uartBaudrate],USARTCAN.Usart[RS232_1][Databits],USARTCAN.Usart[RS232_1][Chkbits],USARTCAN.Usart[RS232_1][Stopbits],USARTCAN.Usart[RS232_1][Flowctrl],USARTCAN.Usart[RS232_1][uartDatatype],
                USARTCAN.Usart[RS232_2][EnUart],USARTCAN.Usart[RS232_2][uartBaudrate],USARTCAN.Usart[RS232_2][Databits],USARTCAN.Usart[RS232_2][Chkbits],USARTCAN.Usart[RS232_2][Stopbits],USARTCAN.Usart[RS232_2][Flowctrl],USARTCAN.Usart[RS232_2][uartDatatype],
                USARTCAN.Usart[RS485][EnUart],USARTCAN.Usart[RS485][uartBaudrate],USARTCAN.Usart[RS485][Databits],USARTCAN.Usart[RS485][Chkbits],USARTCAN.Usart[RS485][Stopbits],USARTCAN.Usart[RS485][uartDatatype],
                USARTCAN.tout
                );
}


/*
 ********************************************************************
 * 函数名：do_http
 * 描述  ：网络状态监测，以及处理HTTP协议函数
 * 输入  : 无 		   
 * 输出  ：无
 ********************************************************************
*/

unsigned char do_http(void)
{
    unsigned int len;
    unsigned char ret;
    st_http_request *http_request;
    
    ret=OFF;    
    memset(rx_buf,0x00,MAX_URI_SIZE);         // 清空接收缓存
    http_request = (st_http_request*)rx_buf;  // http 的请求
  
    /* 循环查询网页的请求 */
    switch(getSn_SR(SOCK_HTTP))                      // 获取 socket 状态
    {
        /*socket初始化完成*/
        case SOCK_INIT:
            listen(SOCK_HTTP);
            break;
        
        case SOCK_LISTEN:

            break;
        /*socket连接建立*/
        case SOCK_ESTABLISHED:
            if(getSn_IR(SOCK_HTTP) & Sn_IR_CON)
            {
                setSn_IR(SOCK_HTTP, Sn_IR_CON);
							
            }
            if ((len = getSn_RX_RSR(SOCK_HTTP)) > 0)		
            {
                len = recv(SOCK_HTTP, (unsigned char*)http_request, len);  // 接收Web端的数据
                *(((unsigned char*)http_request)+len) = 0;
                ret=proc_http(SOCK_HTTP, (unsigned char*)http_request);        // 处理请求
                disconnect(SOCK_HTTP);                                     // 断开socket连接
            }
            break;
        /*socket等待关闭状态*/   
        case SOCK_CLOSE_WAIT:   
            if ((len = getSn_RX_RSR(SOCK_HTTP)) > 0)
            {
                len = recv(SOCK_HTTP, (unsigned char*)http_request, len);  // 接收Web端的数据       
                *(((unsigned char*)http_request)+len) = 0;

                ret=proc_http(SOCK_HTTP, (unsigned char*)http_request);        // 处理请求
            }
            
            disconnect(SOCK_HTTP);                                         // 断开socket连接
            break;
            
        case SOCK_CLOSED:                   
            socket(SOCK_HTTP, Sn_MR_TCP, 80, 0x00);                        // 初始化 socket 
            break;
        
        default:
            break;
  }// end of switch
  return ret;
}

/*
 ********************************************************************
 * 函数名：proc_http
 * 描述  ：处理来自Web服务器的请求
 * 输入  : SOCKET ID  和 接收的数据存放的地址 	   
 * 输出  ：无
 *******************************************************************
*/

unsigned char proc_http(SOCKET s, unsigned char* buf)
{

    char* name;                   // get  方法请求文件名称
    char req_name[32]={0x00,};    // post 方法请求文件名称
    unsigned long file_len=0;
    unsigned int send_len=0;
    unsigned char * http_response, * webpage;
    u8 ret;
    
    ret=0;
    st_http_request *http_request;

    memset(tx_buf,0x00,MAX_URI_SIZE);
    http_response = (unsigned char*)rx_buf;
    http_request  = (st_http_request*)tx_buf;

    parse_http_request(http_request, buf);    // 分析请求后, 转化成 http_request
    /*分析*/
    switch (http_request->METHOD)		
    {
        case METHOD_ERR :
            memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
            send(s, (unsigned char *)http_response, strlen((char const*)http_response));
        break;
        
        case METHOD_HEAD:
           
        case METHOD_GET: //网页请求             
            name = http_request->URI;    /*从uri中获取文件名*/
            if(strcmp(name,"/index.htm")==0 || strcmp(name,"/")==0 || (strcmp(name,"/index.html")==0) ||  (strcmp(name,"*")==0))
            {
                file_len = strlen(INDEX_HTML);
                make_http_response_head((unsigned char*)http_response, PTYPE_HTML,file_len);
                send(s,http_response,strlen((char const*)http_response));
                send_len=0;
                while(file_len)
                {
                    if(file_len>1024)
                    {
                        if(getSn_SR(s)!=SOCK_ESTABLISHED)
                        {
                            return 0;
                        }
                        send(s, (unsigned char *)INDEX_HTML+send_len, 1024);  //wwwwwwww
                        send_len+=1024;
                        file_len-=1024;
                        OSTimeDlyHMSM(0, 0, 0, 10);
                        //delay_us(100);
                    }
                    else
                    {
                        send(s, ((unsigned char *)INDEX_HTML)+send_len, file_len);
                        //send(s, req_name, file_len);
                        send_len+=file_len;
                        file_len-=file_len;
                    } 
                }
            }
            if(strcmp(name,"/w5500.js")==0)
            {
                memset(tx_buf,0,MAX_URI_SIZE);
                make_basic_config_setting_json_callback(tx_buf);
                sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
                send(s, (unsigned char *)http_response, strlen((char const*)http_response));
            }
        break;

        /*POST method*/
        case METHOD_POST:
            mid(http_request->URI, "/", " ", req_name);/*从uri中获取文件名*/
        
            if(strcmp(req_name,"config.cgi")==0)
            {
                cgi_ipconfig(http_request);     //获取更改的IP
                make_cgi_response(3,gWIZNETINFO.iplocal,tx_buf);                  
                sprintf((char *)http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:%d\r\n\r\n%s",strlen(tx_buf),tx_buf);
                
                send(s, (unsigned char *)http_response, strlen((char *)http_response));	
                
                disconnect(s);
                Ethernet_Init();       //初始化网络
				ret = ON;		//接收到网络配置完成标志
//								commonconfig_init();		//网页配置完成后，重新配置通信接口
               // connect(SOCK_HTTP,gWIZNETINFO.ipgoal, gWIZNETINFO.modbusport);
                
            }
            break;
            
        default :
        break;
    }
    return ret;
}
//=============================10n次方======================
unsigned int nn2(u8 i)
{
    u32 ret = 1;
    while(i--)
    {
       ret = ret *10;
    }
    return ret;
}
void app_addr_(unsigned char* addr,unsigned int name)
{
	u8 i;
    u8 u;
    
	char taddr[30]={0};
	char * nexttok;
	unsigned int num = 0;
	strcpy(taddr,(char *)addr);
	u = strlen(addr);
	nexttok = taddr;

    for(i=u;i>0;i--)
    {
        num += AscToHex(nexttok[u-i])*nn2(i-1);
    }
    switch(name)
    {
        case idx_sid:
            gWIZNETINFO.stationID = num;
        break;
        case idx_mbaddr:
            gWIZNETINFO.mbtcp_addr = num;
        break;
        case idx_mblen:
            USARTCAN.datalen = num;
        break;
        case idx_polltm:
            gWIZNETINFO.polltime = num;
        break;
        case idx_lcport:
            gWIZNETINFO.portlocal = num;
        break;
        case idx_remport:
            gWIZNETINFO.portgoal = num;
        break;
        case idx_lcid:
            USARTCAN.can[LocalID] = ((num/100)<<8)+(num/10%10)*16+(num%10);
        break;
        case idx_devid:
            USARTCAN.can[DeviceID] = ((num/100)<<8)+(num/10%10)*16+(num%10);
        break;
        case idx_devnum:
            USARTCAN.can[IDNum] = num;
        break;
        case idx_totim:
            USARTCAN.tout = num;
        break;
        default:break;
        
    }
    nexttok = NULL;
}
void cgi_ipconfig(st_http_request *http_request)
{
    unsigned char * param;

    if((param = get_http_param_value(http_request->URI,"protocol")))
    {
        gWIZNETINFO.session_mode = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"sid")))
    {
       app_addr_((unsigned char*)param,idx_sid);		
    }
    if((param = get_http_param_value(http_request->URI,"modaddr")))
    {
       app_addr_((unsigned char*)param,idx_mbaddr);		
    }
    if((param = get_http_param_value(http_request->URI,"modlen")))
    {
       app_addr_((unsigned char*)param,idx_mblen);		
    }
    if((param = get_http_param_value(http_request->URI,"modtim")))
    {
       app_addr_((unsigned char*)param,idx_polltm);		
    }
    
    //Device setting
    if((param = get_http_param_value(http_request->URI,"ip")))
    {
        inet_addr_((unsigned char*)param, gWIZNETINFO.iplocal);	
    }
    if((param = get_http_param_value(http_request->URI,"locport")))
    {   
        app_addr_((unsigned char*)param,idx_lcport);
    }    
    if((param = get_http_param_value(http_request->URI,"ipgoal")))
    {
        inet_addr_((unsigned char*)param, gWIZNETINFO.ipgoal);	
    }
    if((param = get_http_param_value(http_request->URI,"portgoal")))
    {   
        app_addr_((unsigned char*)param,idx_remport);
    }
    if((param = get_http_param_value(http_request->URI,"sub")))
    {
        inet_addr_((unsigned char*)param, gWIZNETINFO.sn);		
    }    
    if((param = get_http_param_value(http_request->URI,"gw")))
    {
        inet_addr_((unsigned char*)param, gWIZNETINFO.gw);	
    }
    //==============================can=================================
    if((param = get_http_param_value(http_request->URI,"use4")))
    {
        USARTCAN.can[EnCAN] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"baud4")))
    {
        USARTCAN.can[canBaudrate] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"CANlocID4")))
    {
       app_addr_((unsigned char*)param,idx_lcid);		
    }
    if((param = get_http_param_value(http_request->URI,"CANresID4")))
    {
       app_addr_((unsigned char*)param,idx_devid);		
    }
    if((param = get_http_param_value(http_request->URI,"CANresIDnum4")))
    {
       app_addr_((unsigned char*)param,idx_devnum);		
    }   
    if((param = get_http_param_value(http_request->URI,"datatype4")))
    {
        USARTCAN.can[canDatatype] = AscToHex(*param);		
    }
    //======================================USART1
    if((param = get_http_param_value(http_request->URI,"use1")))
    {
        USARTCAN.Usart[RS232_1][EnUart] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"baud1")))
    {
        USARTCAN.Usart[RS232_1][uartBaudrate] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"databit1")))
    {
        USARTCAN.Usart[RS232_1][Databits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"parity1")))
    {
        USARTCAN.Usart[RS232_1][Chkbits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"stopbit1")))
    {
        USARTCAN.Usart[RS232_1][Stopbits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"flow1")))
    {
        USARTCAN.Usart[RS232_1][Flowctrl] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"datatype1")))
    {
        USARTCAN.Usart[RS232_1][uartDatatype] = AscToHex(*param);		
    }
    
    //======================================USART2
    if((param = get_http_param_value(http_request->URI,"use2")))
    {
        USARTCAN.Usart[RS232_2][EnUart] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"baud2")))
    {
        USARTCAN.Usart[RS232_2][uartBaudrate] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"databit2")))
    {
        USARTCAN.Usart[RS232_2][Databits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"parity2")))
    {
        USARTCAN.Usart[RS232_2][Chkbits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"stopbit2")))
    {
        USARTCAN.Usart[RS232_2][Stopbits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"flow2")))
    {
        USARTCAN.Usart[RS232_2][Flowctrl] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"datatype2")))
    {
        USARTCAN.Usart[RS232_2][uartDatatype] = AscToHex(*param);		
    }
    
    //====================================.Usart[RS485]
    if((param = get_http_param_value(http_request->URI,"use3")))
    {
        USARTCAN.Usart[RS485][EnUart] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"baud3")))
    {
        USARTCAN.Usart[RS485][uartBaudrate] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"databit3")))
    {
        USARTCAN.Usart[RS485][Databits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"parity3")))
    {
        USARTCAN.Usart[RS485][Chkbits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"stopbit3")))
    {
        USARTCAN.Usart[RS485][Stopbits] = AscToHex(*param);		
    }
    if((param = get_http_param_value(http_request->URI,"datatype3")))
    {
        USARTCAN.Usart[RS485][uartDatatype] = AscToHex(*param);		
    }
    
    //=================================================配置            
    
    if((param = get_http_param_value(http_request->URI,"tout")))
    {
        app_addr_((unsigned char*)param,idx_totim);	    
    }    
    /* Program the network parameters received into eeprom */
}
void make_cgi_response(unsigned int delay, char* url,char* cgi_response_buf)
{
  sprintf(cgi_response_buf,"<html><head><title>iWeb - Configuration</title><script language=javascript>j=%d;function func(){document.getElementById('delay').innerText=' '+j + ' ';j--;setTimeout('func()',1000);if(j==0)location.href='http://%d.%d.%d.%d/';}</script></head><body onload='func()'>please wait for a while, the module will boot in<span style='color:red;' id='delay'></span> seconds.</body></html>",delay,url[0],url[1],url[2],url[3]);
}
