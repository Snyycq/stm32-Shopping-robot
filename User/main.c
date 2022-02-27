#include "stm32f4xx.h"
#include "misc.h"
#include "delay.h"
#include "KeyLed.h"
#include "MotorDriver.h"
#include "MotorController.h"
#include "amt1450_uart.h"
#include "tcrt5000.h"
#include "PID.h"
#include "BTModule.h"
#include "AStarRoute.h"

void NVIC_Configuration(void); //�ж�����
void GPIO_Config(void);		   //ͨ����������˿�����

//ȫ�ֱ���
uint16_t n10msCount;
uint8_t b100msFlag;
uint16_t n100msCount;

int32_t nSpeed;

int map_index=0;
int map_count=0;
extern int car_direction;

void Map_Action(int count){
	int map[]={1,1,2,2,2,1,3,1,1,2,2,1,1,1,1};//1��ת 2��ת 3 ���ת
	int index[]={8,13,19,20,26,28,29,30,31,33,36,42,43,44,45};
	if (count==index[map_index]){
			Delay_ms(60);
		switch(map[map_index]){
			case 1:Turn_I(0,500,90);  //Turn_I(0,300,90);
				break;
			case 2:Turn_I(0,500,-90);
				break;
			case 3:Turn_I(0,500,180);
				break;
			case 4:Turn_I(800,200,90);//����ת��·��+1/2
				break;
			default:
				break;
		}
		map_index++;
	}
}
void Map_Action2(int *count){
	//int map[]={4,1,2,2,2,1,3,1,1,2,2,1,1,1,1};//1��ת 2��ת 3 ���ת
	//int index[]={7,13,19,20,26,28,29,30,31,33,36,42,43,44,45};
	int map[][2]={{4,7},{1,13},{2,19},{2,20},{2,26},{1,28},{3,29}
	,{1,30},{1,31},{2,33},{2,36},{1,42},{1,43},{1,44},{1,45}};
	if (*count==map[map_index][1]){
			Delay_ms(60);
		switch(map[map_index][0]){
			case 1:Turn_I(0,500,90);  //Turn_I(0,300,90);
				Car_Direction_change(1);
				break;
			case 2:Turn_I(0,500,-90);
			Car_Direction_change(-1);
				break;
			case 3:Turn_I(0,500,180);
			Car_Direction_change(2);
				break;
			case 4:Turn_I(850,200,90);//����ת��·��+1/2
			Car_Position_add();
			Car_Direction_change(1);
			*count+=2;
			Car_Position_add();
			
				break;
			default:
				break;
		}
		map_index++;
	}
}

//Turn_I(400,300,90);//����������ת��һ�ֻ���������

int main(void)
{
	n100msCount = 0;
	GPIO_Config();		  //�˿ڳ�ʼ��
	NVIC_Configuration(); //�жϳ�ʼ��
	Led_Init(1);		  //ֻʹ��LED2��LED3�����ſ�����������;
	Key_Init(2);		  //ֻʹ��Key1��Key2�����ſ�����������;
	init_AMT1450_UART();  // ��ʼ��amt1450����ͨ�ţ�����ʹ��UART5
	USART2_Init();
	//TCRT5000_config();
	Delay_ms(100);

	while (Key_Released(2) == 0)
	{
	} //���Key1û�а��£���һֱ�ȴ�

	Delay_ms(10);
	AMT1450_UART_Cmd(ENABLE);
	//	amt1450_Test_UART();        //����AMT1450����while��

	MotorDriver_Init(2);
	MotorDriver_Start(1, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(2, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(3, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(4, PWM_DUTY_LIMIT / 2);
	Encoder_Init(2);

	MotorController_Init(330, 82, 2); //��ʼ��������������1������תһȦ������������������2������ֱ������λmm������3�����������Ҫ����
	MotorController_Enable(ENABLE);
	MotorController_SetAcceleration(8000); //���ü��ٶ�ֵ����λ��mm/��*��
	Delay_ms(100);

	PID s_PID;
	s_PIDInit(&s_PID);

	nSpeed = 700;
	
	
uint8_t crossing_flag=0;

	printf("hello");
	int way[50]={0};
	FindPath(way,car_position[0],car_position[1],2,2);
	for(int i=1;i<50&&way[i]!=-1;i++){
		printf("%d\n",way[i]);
	}
	printf("break\n\n\n");
	//int way[50]={0};
	Drive_Route(700,2,2);
	
	Drive_Route(700,4,1);
	Drive_Route(700,4,7);
	while (0)
	{

		//
		//����ѭ��ִ�еĴ����
		//

		// if(Key_Released(2)==1)  ����ʹ��

		uint8_t begin, jump, count[6]; // ���6�����䣬��3����
		uint8_t position;
		get_AMT1450Data_UART(&begin, &jump, count); //�����ݴ洢������������
		if (jump == 2)
			position = 0.5f * (count[0] + count[1]); // position=����������м�λ��

		
		if(jump==0&&begin==0&&crossing_flag==1){
			map_count++;
			Car_Position_add();
			
			//printf("dir:%d  ; pos:%d,%d\r\n",car_direction,car_position[0],car_position[1]);
			crossing_flag=0;
		}
		if(jump==2){
			crossing_flag=1;
		}
		
		
		//Delay_ms(10);
		
		Map_Action2(&map_count);
		
		
		int32_t spid_out = Straight_PID(&s_PID, position);
		MotorController_SetSpeed(1, -nSpeed + spid_out);
		MotorController_SetSpeed(2, nSpeed + spid_out);

		
		
		
		
		
		if (b10msFlag == 1)
		{
			b10msFlag = 0; //�� 10ms ��־λ����
			n10msCount++;

			// 10ms��־ÿ10����������100ms
			if (n10msCount % 10 == 0)
				b100msFlag = 1;

			if (n10msCount == 50) // 500msʱ����LED
			{
				LED2_ON();
			}
			else if (n10msCount >= 100) // 1000msʱ�ر�LED��ͬʱ�������㣬����һ���ٶ�ֵ
			{
				LED2_OFF();
				n10msCount = 0;
			}
		}
	}
}

/*�ж����ú���*/
void NVIC_Configuration(void)
{
	/* Configure one bit for preemption priority */
	/* ���ȼ��� ˵������ռ���ȼ����õ�λ�����������ȼ����õ�λ����������2��2*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	//�������ʱ���ж�
	if (SysTick_Config(SystemCoreClock / 100000))
	{
		/* Capture error */
		while (1)
			;
	}
}

/*�˿����ú���*/
void GPIO_Config(void)
{
	//ʹ��GPIOA/GPIOC�����ߣ�����˿ڲ��ܹ����������������˿ڣ����Բ���ʹ�ܡ�
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	  // PB2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //�������
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
