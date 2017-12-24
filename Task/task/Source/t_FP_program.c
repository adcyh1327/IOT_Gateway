#include "main.h"

#define FPPROGRAM_SEND_LENGTH             220          //���ͻ�������С
#define FPPROGRAM_RECE_LENGTH             220         //���ܻ�������С

u8 fp_length=0;
u8 fp_readresult[FPPROGRAM_RECE_LENGTH];
u8 FP_SendBuffer[FPPROGRAM_SEND_LENGTH];
u8 fp_success=0;
u8 fp_read_rcv_flag;
u8 fp_right;
u8 fp_recvOrignBuf[FPPROGRAM_RECE_LENGTH];
u8 fp_w_f;
u8 FP_Program=0xff;;
u8 FP_Program_Enable=OFF;

u8 xxx;
u8 sw[20]={'%',0x30,0x31,'#','R','D','D',0x30,0x30,0x30,0x31,0x30,0x30,0x30,0x30,0x31,0x30,0x35,0x35,0x0D};
u8 test[10];

void FPprogram_Init(u8 nodeID)
{
  FP_Program = nodeID;
  FP_Program_Enable=ON;
}

	
/******************************************************************************************************/
//���ܣ����������������ݣ�ת�����ض�����ʽ�洢������������
//������ʽ����״̬��Ϣ��һ���ֽڣ��������ݣ�4���ֽڡ�
//Ŀǰ����֡��һ������Ϊ��
//��״̬�Ĵ���:����ģʽ�����ݳ���<�ֽ�>��B,1 /�����ݼĴ���������ģʽ�����ݳ���<��>��w,1
//�������������ݳ��ȣ���λΪ���ֽ�
/******************************************************************************************************/
void fpprogram_RecDataType(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
  u8 i;
  if((regtype=='R')||(regtype=='X')||(regtype=='Y'))			  //����Ϊ״̬��Ϣ
  {
    for(i=0;i<datalen;i++)
    {
      targetbuf[2*i]=ASCII2BYTE('0','0');
      targetbuf[2*i+1]=ASCII2BYTE('0',sourcebuf[i]);
    }
    g_u16_RecvTransLen[FP_Program][g_u8_threadIdx[FP_Program]]= 						  //���ݳ��ȣ���λ���ֽ�
       RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].datalenth*2;
  }
  else if((regtype=='D')||(regtype=='T'))	  //����Ϊ��������
  {
				for(i=0;i<(datalen/4);i++)
				{
					targetbuf[2*i]=ASCII2BYTE(sourcebuf[4*i+2],sourcebuf[4*i+3]);
					targetbuf[2*i+1]=ASCII2BYTE(sourcebuf[4*i+0],sourcebuf[4*i+1]);
				}
				g_u16_RecvTransLen[FP_Program][g_u8_threadIdx[FP_Program]]=						  //���ݳ��ȣ���λ���ֽ�
        RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].datalenth * 2;
  }
  else
  {

  }
}

/******************************************************************************************************/
//���ܣ������жϴ������������ݽ���������ŵ��ض������У���modbus tcp��ѯ
/******************************************************************************************************/
void UART_FPProgram_Recv(u8 data)
{
	static u8 i;
	u8 j;			  

	//data = data & 0x7f;
	if(data=='$')									   //��ȷ��Ӧ��֡��д/����Ӧ��֡��
	{
		fp_right=1;
		memset(fp_readresult,0,FPPROGRAM_RECE_LENGTH);
		if(GW_WriteStatus(FP_Program)==WRITE_WAITRESPOND)			//���д�ɹ������Ϸ�PositiveResponse
			{
				GW_WriteStatus(FP_Program)=WRITE_RECVSUCCESS;
				memcpy(&g_u8_EthRespData[FP_Program][(g_u16_StartAddr[FP_Program][g_u8_RespondID]+
				        g_u8_WriteAddrOffset)*2],&g_u8_Writedata,g_u16_WriteLen*2);
				//PositiveResponse();
			}
	}
	else if(data=='!')								   //�����Ӧ��֡
	{
//		if(GW_SendStatus(FP_Program)=STARTSEND)	GW_SendStatus(FP_Program)=SENDIDLE;		//д������д����
		NegativeResponse(Err_MBcmd);
	}
	else
	{
		
	}
	if(fp_right==1)			   //ȷ��Ϊ��ȷ��Ӧ��֡�󣬼�¼֮�������֡
	{												   //ֻ���������ʱ���ж���Ӧ��д����ʱ���ж���Ӧ��������
		fp_readresult[i]=data;
		i++;
		if(data==0x0D)								   //֡������־
		{
			i=0;
			fp_right=0;
			fp_success=1;							   //����֡��¼����
		}
	}
	if(fp_success==1)								   //����֡��¼�����󣬽�ȡ�м�����ݲ���
	{
		if(fp_readresult[2]=='C')							   //��������Ϊ״̬��Ϣ��1���ֽڣ�ascii��ʾ
		{
			fp_recvOrignBuf[0]=fp_readresult[3];
			fp_length=1;
		}
		if(fp_readresult[2]=='D'||fp_readresult[2]=='K')	  //��������Ϊ�������ݣ�4���ֽڣ�ascii��ʾ
		{
      fp_length=RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].datalenth*4;
			for(j=0;j<fp_length;j++)
			{
				fp_recvOrignBuf[j]=fp_readresult[3+j];	
			}
		}													  //�������õĲ���ģʽ��������Ӧ��ת�����洢ת���������
		fpprogram_RecDataType(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].regtype,fp_recvOrignBuf,
			&g_u8_EthRespData[FP_Program][g_u16_StartAddr[FP_Program][g_u8_threadIdx[FP_Program]]*2],fp_length);
		GW_ReadStatus(FP_Program)=READ_RECVSUCCESS;				//���ɹ������Ϸ����ݣ�������ݳ���Ϊ����NegativeResponse
		fp_success=0;	
//		readwaite=0;
	}
}
			
/******************************************************************************************************/
//���ܣ���ʱ����д�����������֡�����Գ�ʱ���д���
//д��־�����Ϳ���/���Ϳ�ʼ/�������/������Ӧ
//����־��������/���ȴ�/���ɹ�
//tips:�����ж�ֻ��Ҫ���ķ������/������Ӧ/���ɹ�
/******************************************************************************************************/
void TIM_1ms_FPProgram(void)							  				//Timer5��1ms��ʱ�жϴ�����
{	 						 		
  if(FP_Program_Enable==ON)
  {
  	g_u16_SwitchTimer[FP_Program]++;
  }
  if((ON==MB_NeedWrite(FP_Program))&&(READ_IDLE==GW_ReadStatus(FP_Program)))
	{ 	
    if(GW_WriteStatus(FP_Program)<WRITE_RECVSUCCESS)
	  {
	     if(g_u16_SwitchTimer[FP_Program]>1000)
	     {
	     		g_u16_SwitchTimer[FP_Program]=0;
					GW_WriteStatus(FP_Program)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(FP_Program)==WRITE_IDLE)&&
	            (g_u16_SwitchTimer[FP_Program]>WriteTime))
	     {	
		   		GW_WriteStatus(FP_Program)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[FP_Program]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(FP_Program)==WRITE_RECVSUCCESS)
	  {
      GW_WriteStatus(FP_Program)=WRITE_DELAY;
      g_u16_SwitchTimer[FP_Program]=0;
	  }
	  else if(g_u16_SwitchTimer[FP_Program]>=WriteTime)
	  {
      PositiveResponse();
      g_u16_SwitchTimer[FP_Program]=0;
      GW_WriteStatus(FP_Program)=WRITE_IDLE;
	    MB_NeedWrite(FP_Program)=OFF;
	    g_u8_threadIdx[FP_Program]++;
		  ThreadNew(FP_Program)=ON;
	  }
	  else{

	  }
	}
	else
	{
		if((g_u16_SwitchTimer[FP_Program]>UpdateCycle[FP_Program])&&
		    (g_u8_ProtocalNum[FP_Program][READ]!=0))
		{	  
		  g_u16_SwitchTimer[FP_Program]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(FP_Program))
		  {
		    GW_ReadStatus(FP_Program)=READ_IDLE;
		    g_u16_TimeoutCnt[FP_Program][g_u8_threadIdx[FP_Program]]=0;
		    if(OFF==MB_NeedWrite(FP_Program))
		    {
          g_u8_threadIdx[FP_Program]++;
          while(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[FP_Program]++;
            if(g_u8_ProtocalNum[FP_Program][READ]<=g_u8_threadIdx[FP_Program])
      			{			  
      	      g_u8_threadIdx[FP_Program]=0;
      			}
          }
		      ThreadNew(FP_Program)=ON;
		    }
		    else
		    {
          
		    }
		  }
		  else
		  {
        g_u16_TimeoutCnt[FP_Program][g_u8_threadIdx[FP_Program]]++;
        if(g_u16_TimeoutCnt[FP_Program][g_u8_threadIdx[FP_Program]]>
          (THRES_TIMOUTCNT/UpdateCycle[FP_Program]))
        {
          g_u16_RecvTransLen[FP_Program][g_u8_threadIdx[FP_Program]]=0;
          g_u16_TimeoutCnt[FP_Program][g_u8_threadIdx[FP_Program]]=0;
          if(OFF==MB_NeedWrite(FP_Program))
  		    {
            g_u8_threadIdx[FP_Program]++;
            while(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[FP_Program]++;
              if(g_u8_ProtocalNum[FP_Program][READ]<=g_u8_threadIdx[FP_Program])
        			{			  
        	      g_u8_threadIdx[FP_Program]=0;
        			}
            }
  		      ThreadNew(FP_Program)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(FP_Program)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[FP_Program][READ]<=g_u8_threadIdx[FP_Program])
	{			  
		g_u8_threadIdx[FP_Program]=0;
	}
}

/******************************************************************************************************/
//���ܣ����ɲ�ͬ�Ĵ�������������֡��ͨ�����ڷ��ͳ�ȥ����̿ڣ�
/******************************************************************************************************/
void fpprogram_read(u8 registertype,u16 addr,u16 lenth)
{
		u8 i,index=0;
		u8 sum;
    memset(FP_SendBuffer,0,FPPROGRAM_SEND_LENGTH);
  	FP_SendBuffer[index++]='%';	 
  	FP_SendBuffer[index++]=DEC2ASCII(										  	//�豸վ��ַ
  	  RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].plcaddrstation/16);
  	FP_SendBuffer[index++]=DEC2ASCII(
      RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].plcaddrstation%16);
  	FP_SendBuffer[index++]='#';
  	FP_SendBuffer[index++]='R';												 	//��
  	switch(registertype)
		{
			case 'D':																	  		//�Ĵ�������
					FP_SendBuffer[index++]='D';									  
					FP_SendBuffer[index++]='D'; 								  		//���ݴ���
					FP_SendBuffer[index++]=DEC2ASCII(addr/10000); 
					FP_SendBuffer[index++]=DEC2ASCII((addr % 10000)/1000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
					addr = addr + lenth-1;
					FP_SendBuffer[index++]=DEC2ASCII(addr/10000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 10000)/1000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
					sum=0;
					for(i=0;i<index;i++)
					{
						sum=sum^FP_SendBuffer[i];
					}
					FP_SendBuffer[index++]=DEC2ASCII(sum/16);
					FP_SendBuffer[index++]=DEC2ASCII(sum%16);
					FP_SendBuffer[index++]=0x0D;
					break;

			case 'T':
					FP_SendBuffer[index++]='K';
					FP_SendBuffer[index++]=DEC2ASCII(addr/1000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
					addr = addr + lenth-1;
					FP_SendBuffer[index++]=DEC2ASCII(addr/1000);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
					sum=0;
					for(i=0;i<index;i++)
					{
							sum=sum^FP_SendBuffer[i];
					}
					FP_SendBuffer[index++]=DEC2ASCII(sum/16);
					FP_SendBuffer[index++]=DEC2ASCII(sum%16);
					FP_SendBuffer[index++]=0x0D;
					break;
				
				case 'R':
				case 'X':
				case 'Y':
          FP_SendBuffer[index++]='C';
				  FP_SendBuffer[index++]='S';	
					FP_SendBuffer[index++]=registertype;
					FP_SendBuffer[index++]=DEC2ASCII((addr/16)/100);
  				FP_SendBuffer[index++]=DEC2ASCII(((addr/16)%100)/10);
  				FP_SendBuffer[index++]=DEC2ASCII(((addr/16)%100)%10);
  				FP_SendBuffer[index++]=DEC2ASCII(addr%16);			
  				sum=0;
  				for(i=0;i<index;i++)
  				{
  					sum=sum^FP_SendBuffer[i];
  				}
  				FP_SendBuffer[index++]=DEC2ASCII(sum/16);
  				FP_SendBuffer[index++]=DEC2ASCII(sum%16);
  				FP_SendBuffer[index++]=0x0D;
					break;
				default:break;
		}
		DAQ_UartSend(FP_SendBuffer,index,CHN_UART_CFG);
}

/******************************************************************************************************/
//���ܣ����ɲ�ͬ�Ĵ���д��������֡��ͨ�����ڷ��ͳ�ȥ����̿ڣ�
/******************************************************************************************************/
void fpprogram_write(u8 registertype,u16 addr,u16 lenth)
{
		u8 i,index=0;
  	u8 sum;
    memset(FP_SendBuffer,0,FPPROGRAM_SEND_LENGTH);
  	FP_SendBuffer[index++]='%';	 
  	FP_SendBuffer[index++]=DEC2ASCII(RegisterCfgBuff[FP_Program][g_u8_RespondID].plcaddrstation/16);
  	FP_SendBuffer[index++]=DEC2ASCII(RegisterCfgBuff[FP_Program][g_u8_RespondID].plcaddrstation%16);
  	FP_SendBuffer[index++]='#';
  	FP_SendBuffer[index++]='W';			  
  	switch(registertype)
		{
				case 'D':
						FP_SendBuffer[index++]='D';
						FP_SendBuffer[index++]='D';
						FP_SendBuffer[index++]=DEC2ASCII(addr/10000); 
						FP_SendBuffer[index++]=DEC2ASCII((addr % 10000)/1000);
						FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
						FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
						FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
						addr = addr + lenth-1;
  					FP_SendBuffer[index++]=DEC2ASCII(addr/10000);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 10000)/1000);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
  					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
						for(i=0;i<lenth;i++)
						{
								FP_SendBuffer[index+4*i]=DEC2ASCII(g_u8_Writedata[2*i+1]  /16);
								FP_SendBuffer[index+4*i+1]=DEC2ASCII(g_u8_Writedata[2*i+1] %16);
								FP_SendBuffer[index+4*i+2]=DEC2ASCII(g_u8_Writedata[2*i]  /16);
								FP_SendBuffer[index+4*i+3]=DEC2ASCII(g_u8_Writedata[2*i] %16);
								
						}
						index += lenth*4;
						sum=0;
						for(i=0;i<index;i++)
						{
								sum=sum^FP_SendBuffer[i];
						}
						FP_SendBuffer[index++]=DEC2ASCII(sum/16);
						FP_SendBuffer[index++]=DEC2ASCII(sum%16);
						FP_SendBuffer[index++]=0x0D;
						break;

				case 'T':
						FP_SendBuffer[index++]='K';
						FP_SendBuffer[index++]=DEC2ASCII(addr/1000);
						FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
						FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
						FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
						addr = addr + lenth-1;
  					FP_SendBuffer[index++]=DEC2ASCII(addr/1000);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 1000)/100);
  					FP_SendBuffer[index++]=DEC2ASCII((addr % 100)/10);
  					FP_SendBuffer[index++]=DEC2ASCII(addr % 10);
						for(i=0;i<lenth;i++)
						{
								FP_SendBuffer[index+4*i+1]=DEC2ASCII(g_u8_Writedata[i]  /16);
								FP_SendBuffer[index+4*i]=DEC2ASCII(g_u8_Writedata[i] %16);
								FP_SendBuffer[index+4*i+3]=DEC2ASCII(g_u8_Writedata[i+1]  /16);
								FP_SendBuffer[index+4*i+2]=DEC2ASCII(g_u8_Writedata[i+1] %16);
						}
						index += lenth*4;
						sum=0;
						for(i=0;i<18;i++)
						{
								sum=sum^FP_SendBuffer[i];
						}
						FP_SendBuffer[index++]=DEC2ASCII(sum/16);
						FP_SendBuffer[index++]=DEC2ASCII(sum%16);
						FP_SendBuffer[index++]=0x0D;
						break;

				case 'R':
				    FP_SendBuffer[index++]='C';
				    FP_SendBuffer[index++]='S';	
						FP_SendBuffer[index++]=registertype;
						FP_SendBuffer[index++]=DEC2ASCII((addr/16)/100);
    				FP_SendBuffer[index++]=DEC2ASCII(((addr/16)%100)/10);
    				FP_SendBuffer[index++]=DEC2ASCII(((addr/16)%100)%10);
    				FP_SendBuffer[index++]=DEC2ASCII(addr%16);			
    				FP_SendBuffer[index++]=DEC2ASCII(g_u8_Writedata[1]&0x0f);	//modbus���ٷ���һ����
    				sum=0;
    				for(i=0;i<13;i++)
    				{
    					sum=sum^FP_SendBuffer[i];
    				}
    				FP_SendBuffer[index++]=DEC2ASCII(sum/16);
    				FP_SendBuffer[index++]=DEC2ASCII(sum%16);
    				FP_SendBuffer[index++]=0x0D;
						break;
				default:break;
		}
		DAQ_UartSend(FP_SendBuffer,index,CHN_UART_CFG);
}

/******************************************************************************************************/
//��taskһֱ��ѭ��
//���ܣ����Ͷ�/д����������֡
//ͨ���жϱ�־λ��ȷ���Ƿ���Ҫ���͡���־λ�ڶ�ʱ�ж�����λ��
/******************************************************************************************************/
void f_FPprogram_task(void)
{				
//	u8 datatyppe;
  	u16 dataaddr;
  	if(0!=ThreadNew(FP_Program))									   //�忨��
  	{    	
			ThreadNew(FP_Program)=0;															//��������־															
    	GW_ReadStatus(FP_Program)=READ_WAITRESPOND;
			dataaddr=(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].regaddr[6]<<8)	//��ȡ���ݵ�ַ,g_u8_threadIdx[FP_Program],��ֵ��ʾ��ͬ�ļĴ������ڶ�ʱ�ж��л��ۼ�
      		+ RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].regaddr[7];
    	fpprogram_read(RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].regtype,dataaddr,			 //�Ĵ�������/���ݵ�ַ
      			RegisterCfgBuff[FP_Program][g_u8_threadIdx[FP_Program]].datalenth);				 //���ݳ���
  	}
  	else if(WRITE_PRESEND==GW_WriteStatus(FP_Program))					   //�忨д
  	{		   
			GW_WriteStatus(FP_Program)=WRITE_WAITRESPOND;
			dataaddr=(g_u8_WriteAddr[6]<<8)+g_u8_WriteAddr[7];
    	fpprogram_write(RegisterCfgBuff[FP_Program][g_u8_RespondID].regtype,dataaddr,g_u16_WriteLen);
  	}
  	else
  	{

  	}
}