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
//#include "AStarRoute.h"
#include "DFS_Map.h"
#include "NanoCommunication.h"
#include "ArmSolution.h"

void NVIC_Configuration(void); //�ж�����
void GPIO_Config(void);		   //ͨ����������˿�����

//ȫ�ֱ���
uint16_t n10msCount;
uint8_t b100msFlag;
uint16_t n100msCount;






//����Ԥ���ٶ�
int32_t nSpeed;

//�������ͼ��Ϊ�йصı���
extern int car_direction;

extern uint8_t begin, jump, count[6];//ѭ��ģ�����ز���
extern uint8_t line_position;//ֱ�ߵ�ǰ���ڵ�λ��

int main(void)
{
	n100msCount = 0;
	GPIO_Config();		  //�˿ڳ�ʼ��
	NVIC_Configuration(); //�жϳ�ʼ��
	Led_Init(1);		  //ֻʹ��LED2��LED3�����ſ�����������;
	Key_Init(2);		  //ֻʹ��Key1��Key2�����ſ�����������;
	init_AMT1450_UART();  // ��ʼ��amt1450����ͨ�ţ�����ʹ��UART5
	USART2_Init();
	USART1_Init();
	TCRT5000_config();
	Delay_ms(100);

	
	printf("hello\n");
	
	while (Key_Released(2) == 0)
	{
	} //���Key1û�а��£���һֱ�ȴ�

	Delay_ms(10);
	AMT1450_UART_Cmd(ENABLE);

	//�����س�ʼ����
	
	MotorDriver_Init(1);
	MotorDriver_Init(2);
	MotorDriver_Start(1, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(2, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(3, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(4, PWM_DUTY_LIMIT / 2);
	Encoder_Init(2);

	MotorController_Init(13500, 75, 2); //��ʼ��������������1������תһȦ������������������2������ֱ������λmm������3�����������Ҫ����
	MotorController_Enable(ENABLE);
	MotorController_SetAcceleration(10000); //���ü��ٶ�ֵ����λ��mm/��*��
	Delay_ms(100);
	
	
	//����һ��ѭ��pid����������ʼ����
	PID s_PID;
	s_PIDInit(&s_PID);

	nSpeed = 700;

	ArmDriver_Init();


	printf("hello\n");
	int t=1;
	int height =100;int flag=1;
	//Arm_Grab();
	SetServoAngle(5, 75);
	ArmSolution(-120,20);
	
	
	while(1){

		extern int grab_flag;
		if(grab_flag==1){
			Arm_Grab();
		}

		if (b10msFlag == 1)
		{
			b10msFlag = 0; //�� 10ms ��־λ����
			n10msCount++;
			// 10ms��־ÿ10����������100ms
			if (n10msCount % 10 == 0)
				b100msFlag = 1;
			
			t++;
			if(t%5==0){
					height=height+1*flag;
					if(height>=180){
						flag=-1;
					}
					if(height<0){
						flag=1;
					}
				//ArmSolution(-120,100);
				//SetServoAngle(4, height);
			}

			//Slow_Pwm(4);
		}



	}
	

	//Straight_back_mm(200,250);
	while (1)
	{

		//
		//����ѭ��ִ�еĴ����
		//

		//Crossing_Detection();

		//Map_Action(); //��ͼ��Ϊ

		//int32_t sp_out = Straight_PID(map_count, map[map_index][1]); //ֱ�ߣ�Ŀ��������pid
		//int32_t fpid_out = Follow_PID(&s_PID, line_position);			 //ѭ��pid
		//MotorController_SetSpeed(1, fpid_out+250);				 //�������
		//MotorController_SetSpeed(2, fpid_out-250);
			// MotorController_SetSpeed(1, 0); //�������
			// MotorController_SetSpeed(2, 0);
			// Delay_ms(10000);


		//������ϵͳ�ĵδ��ʱ��
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
