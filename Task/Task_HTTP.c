
#include "Task_HTTP.h"
// #include "socket.h"	// Just include one header for WIZCHIP
// #include "dhcp.h"
#include "main.h"
#include "httputil.h"

#define STRUCT_OFFSET(stru_name, element) (unsigned long)&((struct stru_name*)0)->element


void Task_HTTP(void *p_arg)
{
    
    (void)p_arg;
    
    while (1)
    {
        if(do_http())
        {
            HTTP_DataHandler();    //HTTP网页更新数据请求
        }
        OSTimeDlyHMSM(0, 0, 0, 1);
    }
    
}




