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

void NVIC_Configuration(void); //�ж�����
void GPIO_Config(void);		   //ͨ����������˿�����

//ȫ�ֱ���
uint16_t n10msCount;
uint8_t b100msFlag;
uint16_t n100msCount;

//����Ԥ���ٶ�
int32_t nSpeed;

//�������ͼ��Ϊ�йصı���
int map_index = 0;
int map_count = 0;
extern int car_direction;
//��ͼ��ǰһ��Ϊ��Ϊ��1��2��3��4�ֱ�Ϊ��ת����ת�����ת�������ת����һ��Ϊ�ڼ���·�ڡ�
int map[][2] = {{1, 8}, {1, 13}, {2, 19}, {2, 20}, {2, 26}, {1, 28}, {3, 29}, {1, 30}, {1, 31}, {2, 33}, {2, 36}, {1, 42}, {1, 43}, {1, 44}, {1, 45}};

//��ͼ��Ϊ������Ŀǰ���ڵڼ���·�ڣ�ִ�����ת�����
void Map_Action(int *count)
{

	if (*count == map[map_index][1])
	{
		switch (map[map_index][0])
		{
		case 1:
			Straight_go_mm(300, 221 / 2); //�߹������һ�볤
			TurnBY_PID(90);				  // Turn_I(0,300,90);
			Car_Direction_change(1);	  //С������ת�䣬1Ϊ����
			break;
		case 2:
			Straight_go_mm(300, 221 / 2); //�߹������һ�볤
			TurnBY_PID(-90);
			Car_Direction_change(-1);
			break;
		case 3:
			Straight_go_mm(300, 221 / 2); //�߹������һ�볤
			TurnBY_PID(180);
			Car_Direction_change(2);
			break;
		case 4:
			Turn_I(850, 200, 90); //����ת��·�ڶ���+1����Ϊ����һ��·��
			Car_Position_add(1);
			Car_Direction_change(1);
			*count += 2;
			Car_Position_add(1);
			break;
		default:
			break;
		}
		map_index++;
	}
}

int main(void)
{
	n100msCount = 0;
	GPIO_Config();		  //�˿ڳ�ʼ��
	NVIC_Configuration(); //�жϳ�ʼ��
	Led_Init(1);		  //ֻʹ��LED2��LED3�����ſ�����������;
	Key_Init(2);		  //ֻʹ��Key1��Key2�����ſ�����������;
	init_AMT1450_UART();  // ��ʼ��amt1450����ͨ�ţ�����ʹ��UART5
	USART2_Init();
	// TCRT5000_config();
	Delay_ms(100);

	while (Key_Released(2) == 0)
	{
	} //���Key1û�а��£���һֱ�ȴ�

	Delay_ms(10);
	AMT1450_UART_Cmd(ENABLE);
	//	amt1450_Test_UART();        //����AMT1450����while��

	//�����س�ʼ����
	MotorDriver_Init(2);
	MotorDriver_Start(1, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(2, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(3, PWM_DUTY_LIMIT / 2);
	MotorDriver_Start(4, PWM_DUTY_LIMIT / 2);
	Encoder_Init(2);

	MotorController_Init(330, 64, 2); //��ʼ��������������1������תһȦ������������������2������ֱ������λmm������3�����������Ҫ����
	MotorController_Enable(ENABLE);
	MotorController_SetAcceleration(8000); //���ü��ٶ�ֵ����λ��mm/��*��
	Delay_ms(100);

	//����һ��ѭ��pid����������ʼ����
	PID s_PID;
	s_PIDInit(&s_PID);

	nSpeed = 700;

	//�����־���������ж��ǲ��ǻ���·��
	uint8_t crossing_flag = 0;

	while (1)
	{

		//
		//����ѭ��ִ�еĴ����
		//

		// AMT1450ѭ��ģ���ʹ�ã���https://www.luheqiu.com/deane/begin-smart_tracking_car/
		uint8_t begin, jump, count[6]; // ���6�����䣬��3����
		uint8_t position;
		get_AMT1450Data_UART(&begin, &jump, count); //�����ݴ洢������������
		if (jump == 2)
			position = 0.5f * (count[0] + count[1]); // position=����������м�λ�ã����ߵ�λ��

		//�����ɫû�����䣬�������Ϊ��ɫ����û����·�ڣ���
		if (jump == 0 && begin == 0 && crossing_flag == 1)
		{
			map_count++;		 //��ͼ������1
			Car_Position_add(1); //С��λ�ü�1
			crossing_flag = 0;	 //��־����·��
		}
		//������߳��֣���ô����ʻ����·�ڣ���־=1
		if (jump == 2)
		{
			crossing_flag = 1;
		}

		Map_Action(&map_count); //��ͼ��Ϊ

		int32_t sp_out = Straight_PID(map_count, map[map_index][1]); //ֱ�ߣ�Ŀ��������pid
		int32_t fpid_out = Follow_PID(&s_PID, position);			 //ѭ��pid
		MotorController_SetSpeed(1, fpid_out - sp_out);				 //�������
		MotorController_SetSpeed(2, fpid_out + sp_out);				 //�������

		//������Щ��֪����ʲô�ã�Ҳ����ɾ���´�����ѧ����
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
