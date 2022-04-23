#ifndef __Nano_MODULE_H
#define __Nano_MODULE_H
#include "stm32f4xx.h"
#include "stdio.h"
#include "stdarg.h"



//ʹ��USART2����ӦPA2/TX��PA3/RX��


//ԭ����Ƶ���9
#define FRAME_BYTE_LENGTH 9 //����ͨѶһ֡���ݵ��ֽ�������֡ͷ��֡β����Ʃ��20���ֽ�Ϊһ������������֡����1���ֽ�֡ͷ����2���ֽڴ����������ͣ���3~6�ֽ��������������7���ֽ�Ϊ֡β
#define FRAME_START 0xA5 //֡ͷ
#define FRAME_END 0x5A  //֡β



void USART1_Init(void);
void USART1_OUT(USART_TypeDef* USARTx, uint8_t *Data,...);
//char *itoa(int value, char *string, int radix);
//int fputc(int ch, FILE *f);
//void USART_GetChar(uint8_t nChar); //���ڽ��յ�һ���ֽ�
//void USART_Process(void);
void USART1_Process(void);


/*car_flag
 - 1��ʻ��
 - 2ͣ�ٵȴ���λ���ź�
 - ץȡ״̬
    - һ�����3
    - �ֿ���4
 - ����״̬5
*/
extern uint8_t car_flag;//С��״̬�����������
#define Car_Driving 1
#define Car_Waiting 2
#define Car_Grab_Normal 3
#define Car_Grab_Store 4
#define Car_Stop 5

#endif
