#include "main.h"

#define CP1L_RECE_LENGTH				220
#define CP1L_SEND_LENGTH				220

u8 cp_length=0;
u8 cp_readresult[CP1L_RECE_LENGTH];
u8 cp_sendbuffer[CP1L_SEND_LENGTH];
u8 cp_success=0;
u8 cp_right;
u8 cp_recvOrignBuf[CP1L_RECE_LENGTH];
u8 cp_hand[11][1]={{0x40},{0x30},{0x30},{0x53},{0x43},{0x30},{0x32},{0x35},{0x32},{0x2A},{0x0D}};
//u8 cp_hand[11]={0x40,0x30,0x30,0x53,0x43,0x30,0x32,0x35,0x32,0x2A,0x0D};
u8 CP1LCom=0xff;;								//�ڵ��
u8 Cp1l_Enable=OFF;

/******************************************************************************************************/
//�ڵ��ʼ��
/******************************************************************************************************/
void Hand_connect(void)
{
		u8 x;
	
		for(x=0;x<11;x++)
		{
				DAQ_UartSend(cp_hand[x],1,CHN_UART_CFG);
				TimeDelay(500);
		}
// 			Start_CPCom_Transmit(cp_hand,11);
// 			TimeDelay(5000);
}
/******************************************************************************************************/
//�ڵ��ʼ��
/******************************************************************************************************/
void CP1LCom_Init(u8 nodeID)
{
	Hand_connect();
		CP1LCom=nodeID;
		Cp1l_Enable=ON;
}
	
/******************************************************************************************************/
//���ܣ����������������ݣ�ת�����ض�����ʽ�洢������������
//������ʽ����״̬��Ϣ��һ���ֽڣ��������ݣ�4���ֽڡ�
//Ŀǰ����֡��һ������Ϊ��
//��״̬�Ĵ���:����ģʽ�����ݳ���<�ֽ�>��B,1 /�����ݼĴ���������ģʽ�����ݳ���<��>��w,1
//�������������ݳ��ȣ���λΪ���ֽ�
/******************************************************************************************************/
void cpcom_RecDataType(u8 regtype,u8 *sourcebuf,u8 *targetbuf,u8 datalen)
{
		u8 i;
		u16 cp_val;
		u8 cp_t[4];
		if(RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].regtype=='R')
		{
			cp_val=ASCII2BYTE(sourcebuf[0],sourcebuf[1])*256+ASCII2BYTE(sourcebuf[2],sourcebuf[3]);
			targetbuf[0]=0x00;
			targetbuf[1]=cp_val>>RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].regaddr[7];
		}
		else if((RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].regtype=='D')||	  //����Ϊ��������
				(RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].regtype=='T'))
		{
			for(i=0;i<(datalen/4);i++)
			{
					targetbuf[2*i]=ASCII2BYTE(sourcebuf[4*i+0],sourcebuf[4*i+1]);
					targetbuf[2*i+1]=ASCII2BYTE(sourcebuf[4*i+2],sourcebuf[4*i+3]);
			}
			
		}
		else
		{

		}
		g_u16_RecvTransLen[CP1LCom][g_u8_threadIdx[CP1LCom]]=datalen	/ 2;					  //���ݳ��ȣ���λ���ֽ�
}

/******************************************************************************************************/
//���ܣ������жϴ������������ݽ���������ŵ��ض������У���modbus tcp��ѯ
/******************************************************************************************************/
void UART_CPCom_Recv(u8 data)
{
	static u8 i;
	u8 j;			  

	//data = data & 0x7f;
	if(data==0x40)									   //��ȷ��Ӧ��֡��д/����Ӧ��֡��
	{
		cp_right=1;
	}
	if(cp_right==1)			   //ȷ��Ϊ��ȷ��Ӧ��֡�󣬼�¼֮�������֡
	{												   //ֻ���������ʱ���ж���Ӧ��д����ʱ���ж���Ӧ��������
		cp_readresult[i]=data;
		i++;
		if(data==0x0D)								   //֡������־
		{
			if(GW_WriteStatus(CP1LCom)==WRITE_WAITRESPOND)
			{
					if(0x30==cp_readresult[5] && 0x30==cp_readresult[6])
					{
							if('D'==cp_readresult[4] || 'C'==cp_readresult[4] || 'R'==cp_readresult[4] || 0x41==cp_readresult[4])
							{
									GW_WriteStatus(CP1LCom)=WRITE_RECVSUCCESS;
									memcpy(&g_u8_EthRespData[CP1LCom][(g_u16_StartAddr[CP1LCom][g_u8_RespondID]+g_u8_WriteAddrOffset)*2],
                  &g_u8_Writedata,g_u16_WriteLen*2);
							}
					}
			}
//			i=0;
			cp_right=0;
			cp_success=1;							   //����֡��¼����
		}
	}
	if(cp_success==1)								   //����֡��¼�����󣬽�ȡ�м�����ݲ���,���ķ�������
	{
		cp_length=RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].datalenth*4;	
		if((cp_length+11)==i && 0x52==cp_readresult[3] && 0x30==cp_readresult[5] && 0x30==cp_readresult[6])
			{
					if(('D'==cp_readresult[4]) || ('C'==cp_readresult[4]) || ('R'==cp_readresult[4]))
					{
							if(GW_ReadStatus(CP1LCom)==READ_WAITRESPOND)
							{
									for(j=0;j<cp_length;j++)										//������������������Ϣ��4λascii�����
									{
										cp_recvOrignBuf[j]=cp_readresult[7+j];	
									}
									
								  //�������õĲ���ģʽ��������Ӧ��ת�����洢ת���������
									cpcom_RecDataType(RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].regtype,cp_recvOrignBuf,
										&g_u8_EthRespData[CP1LCom][g_u16_StartAddr[CP1LCom][g_u8_threadIdx[CP1LCom]]*2],cp_length);
									GW_ReadStatus(CP1LCom)=READ_RECVSUCCESS;				//���ɹ������Ϸ����ݣ�������ݳ���Ϊ����NegativeResponse
							}
					}
			}
			cp_success=0;	
			i=0;
	}
}
			
/******************************************************************************************************/
//���ܣ���ʱ����д�����������֡�����Գ�ʱ���д���
//д��־�����Ϳ���/���Ϳ�ʼ/�������/������Ӧ
//����־��������/���ȴ�/���ɹ�
//tips:�����ж�ֻ��Ҫ���ķ������/������Ӧ/���ɹ�
/******************************************************************************************************/
void TIM_1ms_CP1LCom(void)							  				//Timer5��1ms��ʱ�жϴ�����
{	 						 		
  if(Cp1l_Enable==ON)
  {
  	g_u16_SwitchTimer[CP1LCom]++;
  }
	if((ON==MB_NeedWrite(CP1LCom))&&(READ_IDLE==GW_ReadStatus(CP1LCom)))			//�ӵ�дָ���д����
	{     	
    if(GW_WriteStatus(CP1LCom)<WRITE_RECVSUCCESS)						//δд��дδ���
	  {
	     if(g_u16_SwitchTimer[CP1LCom]>1000)		//��ʱ
	     {
	     		g_u16_SwitchTimer[CP1LCom]=0;
					GW_WriteStatus(CP1LCom)=WRITE_RECVSUCCESS;
					NegativeResponse(Err_MBcmd);
	     } 
	     else if((GW_WriteStatus(CP1LCom)==WRITE_IDLE)&&							//��ʱд
	            (g_u16_SwitchTimer[CP1LCom]>WriteTime))
	     {	
		   		GW_WriteStatus(CP1LCom)=WRITE_PRESEND;
		   		g_u16_SwitchTimer[CP1LCom]=0;
	     }
	     else
	     {

	     }
	  }
	  else if(GW_WriteStatus(CP1LCom)==WRITE_RECVSUCCESS)								//д������Ӧ��д������
	  {
      GW_WriteStatus(CP1LCom)=WRITE_DELAY;
      g_u16_SwitchTimer[CP1LCom]=0;
	  }
	  else if(g_u16_SwitchTimer[CP1LCom]>=WriteTime)				//дʱ����
	  {
      PositiveResponse();
      g_u16_SwitchTimer[CP1LCom]=0;
      GW_WriteStatus(CP1LCom)=WRITE_IDLE;
	    MB_NeedWrite(CP1LCom)=OFF;
	    g_u8_threadIdx[CP1LCom]++;
		  ThreadNew(CP1LCom)=ON;
	  }
	  else{

	  }
	}
	else																									//��
	{
		if((g_u16_SwitchTimer[CP1LCom]>UpdateCycle[CP1LCom])&&			//��ʱ��������
		    (g_u8_ProtocalNum[CP1LCom][READ]!=0))
		{	  
		  g_u16_SwitchTimer[CP1LCom]=0;
		  
		  if(READ_RECVSUCCESS==GW_ReadStatus(CP1LCom))												//�ɹ����ܵ���Ӧ
		  {
		    GW_ReadStatus(CP1LCom)=READ_IDLE;
		    g_u16_TimeoutCnt[CP1LCom][g_u8_threadIdx[CP1LCom]]=0;
		    if(OFF==MB_NeedWrite(CP1LCom))																	//��дʱ
		    {
          g_u8_threadIdx[CP1LCom]++;																		//��һ֡
          while(RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].cmdstatus.Bits.fbrd==ON)
          {
            g_u8_threadIdx[CP1LCom]++;
            if(g_u8_ProtocalNum[CP1LCom][READ]<=g_u8_threadIdx[CP1LCom])
      			{			  
      	      g_u8_threadIdx[CP1LCom]=0;
      			}
          }
		      ThreadNew(CP1LCom)=ON;
		    }
		    else
		    {
          GW_ReadStatus(CP1LCom)=READ_IDLE;															//�����д��������
		    }
		  }
		  else																										//��δ��Ӧ
		  {
        g_u16_TimeoutCnt[CP1LCom][g_u8_threadIdx[CP1LCom]]++;
        if(g_u16_TimeoutCnt[CP1LCom][g_u8_threadIdx[CP1LCom]]>
          (THRES_TIMOUTCNT/UpdateCycle[CP1LCom]))	//��ʱ����
        {
          g_u16_RecvTransLen[CP1LCom][g_u8_threadIdx[CP1LCom]]=0;
          g_u16_TimeoutCnt[CP1LCom][g_u8_threadIdx[CP1LCom]]=0;
          if(OFF==MB_NeedWrite(CP1LCom))
  		    {
            g_u8_threadIdx[CP1LCom]++;
            while(RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].cmdstatus.Bits.fbrd==ON)
            {
              g_u8_threadIdx[CP1LCom]++;
              if(g_u8_ProtocalNum[CP1LCom][READ]<=g_u8_threadIdx[CP1LCom])
        			{			  
        	      g_u8_threadIdx[CP1LCom]=0;
        			}
            }
  		      ThreadNew(CP1LCom)=ON;
  		    }
  		    else
  		    {
            GW_ReadStatus(CP1LCom)=READ_IDLE;
  		    }
        }
		  }
		}
	}
	if(g_u8_ProtocalNum[CP1LCom][READ]<=g_u8_threadIdx[CP1LCom])
	{			  
		g_u8_threadIdx[CP1LCom]=0;
	}
}

/******************************************************************************************************/
//���ܣ����ɲ�ͬ�Ĵ�������������֡��ͨ�����ڷ��ͳ�ȥ����̿ڣ�
/******************************************************************************************************/
void cpcom_read(u8 registertype,u16 addr,u16 lenth)
{
		u8 i;
    u8 sum;

  	cp_sendbuffer[0]=0x40;	 													//ǰ��֡
  	cp_sendbuffer[1]=DEC2ASCII(										  	//�豸վ��ַ
			RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].plcaddrstation / 16);
  	cp_sendbuffer[2]=DEC2ASCII(
			RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].plcaddrstation % 16);
  	cp_sendbuffer[3]='R'; 			
		cp_sendbuffer[5]=DEC2ASCII(addr / 1000);										//��ַ/ͨ����				  
		cp_sendbuffer[6]=DEC2ASCII((addr % 1000) / 100);
		cp_sendbuffer[7]=DEC2ASCII((addr % 100) / 10);
		cp_sendbuffer[8]=DEC2ASCII((addr % 10));
		cp_sendbuffer[9]=DEC2ASCII(lenth / 1000);
		cp_sendbuffer[10]=DEC2ASCII((lenth % 1000) / 100);
		cp_sendbuffer[11]=DEC2ASCII((lenth % 100) / 10);
		cp_sendbuffer[12]=DEC2ASCII((lenth % 10));
		cp_sendbuffer[15]=0x2A;
		cp_sendbuffer[16]=0x0D;
  	switch(registertype)
		{
			case 'D':																	  		//�Ĵ�������
					cp_sendbuffer[4]='D';			
					break;

			case 'T':
			case 'C':
					cp_sendbuffer[4]='C';
					break;
				
			case 'R':
					cp_sendbuffer[4]='R';
					cp_sendbuffer[5]=DEC2ASCII((addr>>8)/1000);										//��ַ/ͨ����				  
					cp_sendbuffer[6]=DEC2ASCII(((addr>>8) % 1000)/100);
					cp_sendbuffer[7]=DEC2ASCII(((addr>>8) % 100)/10);
					cp_sendbuffer[8]=DEC2ASCII(((addr>>8) % 10));
					break;

			case 'X':
					cp_sendbuffer[4]=0x52;
					break;

			case 'Y':
					cp_sendbuffer[4]=0x52;
					cp_sendbuffer[5]=DEC2ASCII((addr>>8)/1000);										//��ַ/ͨ����				  
					cp_sendbuffer[6]=DEC2ASCII(((addr>>8) % 1000)/100);
					cp_sendbuffer[7]=DEC2ASCII(((addr>>8) % 100)/10);
					cp_sendbuffer[8]=DEC2ASCII(((addr>>8) % 10));
					break;
		}
		sum=0;
		for(i=0;i<13;i++)
		{
				sum=sum^cp_sendbuffer[i];
		}
		cp_sendbuffer[13]=DEC2ASCII(sum/16);
		cp_sendbuffer[14]=DEC2ASCII(sum%16);
		
		DAQ_UartSend(cp_sendbuffer,17,CHN_UART_CFG);
}

/******************************************************************************************************/
//���ܣ����ɲ�ͬ�Ĵ���д��������֡��ͨ�����ڷ��ͳ�ȥ����̿ڣ�
/******************************************************************************************************/
void cpcom_write(u8 registertype,u16 addr,u16 lenth)
{
		u8 i;
  	u8 sum,index;
		u8 lenTemp;
		u16 cp_val;
		index=0;
  	cp_sendbuffer[index++]=0x40;	 
  	cp_sendbuffer[index++]=DEC2ASCII(										  	//�豸վ��ַ
  	  RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].plcaddrstation / 16);
  	cp_sendbuffer[index++]=DEC2ASCII(
      RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].plcaddrstation % 16);		  
  	switch(registertype)
		{
				case 'D':
						cp_sendbuffer[index++]=0x57;
						cp_sendbuffer[index++]=0x44;
						cp_sendbuffer[index++]=DEC2ASCII((addr / 1000));
						cp_sendbuffer[index++]=DEC2ASCII((addr % 1000) / 100); 
						cp_sendbuffer[index++]=DEC2ASCII((addr % 100) / 10);
						cp_sendbuffer[index++]=DEC2ASCII((addr % 10));
						lenTemp=lenth*2;
						for(i=0;i<lenTemp;i++)
						{
								cp_sendbuffer[index+2*i]=DEC2ASCII(g_u8_Writedata[i]  / 16);
								cp_sendbuffer[index+2*i+1]=DEC2ASCII(g_u8_Writedata[i] % 16);
						}
						index=index+lenth*4;
						sum=0;
						for(i=0;i<index;i++)
						{
								sum=sum^cp_sendbuffer[i];
						}
						cp_sendbuffer[index++]=DEC2ASCII(sum / 16);
						cp_sendbuffer[index++]=DEC2ASCII(sum % 16);
						cp_sendbuffer[index++]=0x2A;
						cp_sendbuffer[index++]=0x0D;
						DAQ_UartSend(cp_sendbuffer,index,CHN_UART_CFG);
						break;

				case 'T':
						cp_sendbuffer[index++]=0x57;
						cp_sendbuffer[index++]=0x43;
						cp_sendbuffer[index++]=DEC2ASCII((addr / 1000));
						cp_sendbuffer[index++]=DEC2ASCII((addr % 1000) / 100); 
						cp_sendbuffer[index++]=DEC2ASCII((addr % 100) / 10);
						cp_sendbuffer[index++]=DEC2ASCII((addr % 10));
						lenTemp=lenth*2;
						for(i=0;i<lenTemp;i++)
						{
								cp_sendbuffer[index+2*i]=DEC2ASCII(g_u8_Writedata[i]  / 16);
								cp_sendbuffer[index+2*i+1]=DEC2ASCII(g_u8_Writedata[i] % 16);
						}
						index=index+lenth*4;
						sum=0;
						for(i=0;i<index;i++)
						{
								sum=sum^cp_sendbuffer[i];
						}
						cp_sendbuffer[index++]=DEC2ASCII(sum / 16);
						cp_sendbuffer[index++]=DEC2ASCII(sum % 16);
						cp_sendbuffer[index++]=0x2A;
						cp_sendbuffer[index++]=0x0D;
						DAQ_UartSend(cp_sendbuffer,index,CHN_UART_CFG);
						break;

				case 'R':
						cp_sendbuffer[3]=0x46;
						cp_sendbuffer[4]=0x41;
						cp_sendbuffer[5]=0x30;
						cp_sendbuffer[6]=0x38;
						cp_sendbuffer[7]=0x30;
						cp_sendbuffer[8]=0x30;
						cp_sendbuffer[9]=0x30;
						cp_sendbuffer[10]=0x30;
						cp_sendbuffer[11]=0x32;
						for(i=12;i<22;i++)
						{
								cp_sendbuffer[i]=0x30;
						}
						cp_sendbuffer[22]=0x46;
						cp_sendbuffer[23]=0x43;
						cp_sendbuffer[24]=0x30;
						cp_sendbuffer[25]=0x30;
						cp_sendbuffer[26]=0x30;
						cp_sendbuffer[27]=0x31;
						cp_sendbuffer[28]=0x30;
						cp_sendbuffer[29]=0x32;
						cp_sendbuffer[30]=0x33;
						cp_sendbuffer[31]=0x30;
						
						cp_sendbuffer[32]=0x30;
						cp_sendbuffer[33]=0x30;
						cp_sendbuffer[34]=DEC2ASCII(((addr >> 8) / 16));
						cp_sendbuffer[35]=DEC2ASCII(((addr >> 8) % 16));
						
						cp_sendbuffer[36]=0x30;
						cp_sendbuffer[37]=DEC2ASCII((addr & 0x0f));
						
						cp_sendbuffer[38]=0x30;
						cp_sendbuffer[39]=0x30;
						cp_sendbuffer[40]=0x30;
						cp_sendbuffer[41]=0x31;
						cp_sendbuffer[42]=0x30;
						cp_sendbuffer[43]=DEC2ASCII(g_u8_Writedata[1] & 0x01);
						sum=0;
						for(i=0;i<44;i++)
						{
								sum=sum^cp_sendbuffer[i];
						}
						cp_sendbuffer[44]=DEC2ASCII(sum / 16);
						cp_sendbuffer[45]=DEC2ASCII(sum % 16);
						cp_sendbuffer[46]=0x2A;
						cp_sendbuffer[47]=0x0D;
						DAQ_UartSend(cp_sendbuffer,48,CHN_UART_CFG);
						break;
		}
}

/******************************************************************************************************/
//��taskһֱ��ѭ��
//���ܣ����Ͷ�/д����������֡
//ͨ���жϱ�־λ��ȷ���Ƿ���Ҫ���͡���־λ�ڶ�ʱ�ж�����λ��
/******************************************************************************************************/
void f_CPcom_task(void)
{				
//	u8 datatyppe;
  	u16 dataaddr;
  	if(0!=ThreadNew(CP1LCom))									   //�忨��
  	{    	
			ThreadNew(CP1LCom)=0;															//��������־															
    	GW_ReadStatus(CP1LCom)=READ_WAITRESPOND;
			dataaddr=(RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].regaddr[6]<<8)	//��ȡ���ݵ�ַ,g_u8_threadIdx[NODE_FPPROGRAM],��ֵ��ʾ��ͬ�ļĴ������ڶ�ʱ�ж��л��ۼ�
      		+ RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].regaddr[7];
    	cpcom_read(RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].regtype,dataaddr,			 //�Ĵ�������/���ݵ�ַ
      			RegisterCfgBuff[CP1LCom][g_u8_threadIdx[CP1LCom]].datalenth);				 //���ݳ���
  	}
  	else if(WRITE_PRESEND==GW_WriteStatus(CP1LCom))					   //�忨д
  	{		   
			GW_WriteStatus(CP1LCom)=WRITE_WAITRESPOND;
			dataaddr = (g_u8_WriteAddr[6]<<8) + g_u8_WriteAddr[7];
    	cpcom_write(RegisterCfgBuff[CP1LCom][g_u8_RespondID].regtype,dataaddr,g_u16_WriteLen);
  	}
  	else
  	{

  	}
}