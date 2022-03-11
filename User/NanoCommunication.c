#include "BTModule.h"

//__IO uint8_t nRx2Counter=0; //�����ֽ���
//__IO uint8_t USART_Rx2Buff[FRAME_BYTE_LENGTH]; //���ջ�����
//__IO uint8_t USART_FrameFlag = 0; //������������֡��־��1������0������

void USART1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;  //NVIC�ж������ṹ��
	/* config USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	 /* Connect USART pins to AF7 */
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
	
	/* USART2 GPIO config */ 
	/* Configure USART2 Tx (PD.05) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd =GPIO_PuPd_UP;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* Configure USART2 Rx (PD.06) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd =GPIO_PuPd_UP;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	

	
  /* Enable the USART OverSampling by 8 */
  USART_OverSampling8Cmd(USART1, ENABLE);  	

	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
//	USART_InitStructure.USART_BaudRate = 19200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	/* Enable USART2 Receive and Transmit interrupts */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE); 

	USART_Cmd(USART1, ENABLE);
	
		//ʹ��USART2�ж�
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	
}

/******************************************************
		��ʽ�������������
        "\r"	�س���	   USART_OUT(USART1, "abcdefg\r")   
		"\n"	���з�	   USART_OUT(USART1, "abcdefg\r\n")
		"%s"	�ַ���	   USART_OUT(USART1, "�ַ����ǣ�%s","abcdefg")
		"%d"	ʮ����	   USART_OUT(USART1, "a=%d",10)
**********************************************************/
void USART1_OUT(USART_TypeDef* USARTx, uint8_t *Data,...){ 
	const char *s;
    int d;
    char buf[16];
    va_list ap;
    va_start(ap, Data);

	while(*Data!=0){				                          //�ж��Ƿ񵽴��ַ���������
		if(*Data==0x5c){									  //'\'
			switch (*++Data){
				case 'r':							          //�س���
					USART_SendData(USARTx, 0x0d);	   
					Data++;
					break;
				case 'n':							          //���з�
					USART_SendData(USARTx, 0x0a);	
					Data++;
					break;
				
				default:
					Data++;
				    break;
			}
	
			 
		}
		else if(*Data=='%'){									  //
			switch (*++Data){				
				case 's':										  //�ַ���
                	s = va_arg(ap, const char *);
                	for ( ; *s; s++) {
                    	USART_SendData(USARTx,*s);
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
            	case 'd':										  //ʮ����
                	d = va_arg(ap, int);
                	itoa(d, buf, 10);
                	for (s = buf; *s; s++) {
                    	USART_SendData(USARTx,*s);
						while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
				default:
					Data++;
				    break;
			}		 
		}
		else USART_SendData(USARTx, *Data++);
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);
	}
}

/*
void USART_GetChar(uint8_t nChar) //���ڽ��յ�һ���ֽ�
{
//	if(USART_FrameFlag == 1) return;   //����ϴε�����֡��û��������򷵻�
//	
//	USART_Rx2Buff[nRx2Counter++]=nChar;  //���浽������
//	if(nChar == CMD_BYTE_END)   //�����֡������־
//	{
//		if(nRx2Counter == CMD_BYTE_LENGTH)  //���֡������־���ˣ����ҽ��յ��ֽ������õ���֡Ҫ���ֽ���������ճɹ����ñ�־Ϊ1
//		{
//			USART_FrameFlag = 1;
//		}
//		nRx2Counter=0;
//	}
//	else if(nRx2Counter>=CMD_BYTE_LENGTH)
//	{
//		nRx2Counter = 0;
//	}
	if(USART_FrameFlag == 1) return;   //����ϴε�����֡��û��������򷵻�
	
	if(nRx2Counter==0 && nChar == FRAME_START)
	{
		USART_Rx2Buff[nRx2Counter++]=nChar;  //���浽������
	}
	else if(nRx2Counter>0) //���յ�֡ͷ�Ժ�ż�������
	{
		USART_Rx2Buff[nRx2Counter++]=nChar;  //���浽������
		if(nRx2Counter>=FRAME_BYTE_LENGTH)  //���յ�һ֡����
		{
			nRx2Counter = 0;
			if(USART_Rx2Buff[FRAME_BYTE_LENGTH-1] == FRAME_END) //������һ���ֽ���֡β��������֡����
			{
				USART_FrameFlag=1;
			}
		}
	}	
}
*/
void USART1_Process(void) //��������֡
{
//	uint8_t i;
	if(USART_FrameFlag == 1)
	{
		//������ԭ�ⲻ�����ͻ�ȥ
		for(int i=0;i<FRAME_BYTE_LENGTH;i++)
		{
			USART_SendData(USART1,USART_Rx2Buff[i]);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
		}
		
		//if(USART_Rx2Buff[1] == 0x11) //��������ֽڵ���0x11����������PID����ָ���ЩЭ������Լ�����
		//������ϣ�����־��0
		USART_FrameFlag = 0; 
	}
}


/**
  * @brief  This function handles USART2 interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)      //����2�жϷ���
{
	uint8_t getchar;
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)	   //�ж϶��Ĵ����Ƿ�Ϊ��
  {	
    /* Read one byte from the receive data register */
    getchar = USART_ReceiveData(USART1);   //�����Ĵ��������ݻ��浽���ջ�������
		
    USART_GetChar(getchar);
  }
  
  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)                   //�����Ϊ�˱���STM32 USART��һ���ֽڷ��Ͳ���ȥ��BUG 
  { 
     USART_ITConfig(USART1, USART_IT_TXE, DISABLE);					     //��ֹ���ͻ��������ж�
  }	
  
}
