#include "PID.h"
#include "delay.h"
#include "MotorController.h"
#include "amt1450_uart.h"
#include "MotorDriver.h"
#include "AStarRoute.h"
#include "stdlib.h"

//С��Ĭ�Ϸ����λ�á�
int car_direction = 3;
int car_position[2] = {9, 8};
#define car_D 320 //С��ֱ������

//ѭ��pid�Ĳ�����ʼ��
void s_PIDInit(PID *s_PID)
{
	s_PID->error_acc = 0;
	s_PID->lastError = 0;
	s_PID->prevError = 0;

	s_PID->kp = 8;//old:3.8 0 1.4
	s_PID->ki = 0;
	s_PID->kd = 1.4;
	s_PID->target = 76;

	s_PID->filter = 0.5;
	s_PID->lastFilter = 0;
}

//ת��pid�ĳ�ʼ������
void turn_PIDInit(PID *s_PID)
{
	s_PID->error_acc = 0;
	s_PID->lastError = 0;
	s_PID->prevError = 0;

	s_PID->kp = 4.5;
	s_PID->ki = 0;
	s_PID->kd = 3;

	s_PID->filter = 0.3;
	s_PID->lastFilter = 0;
}
//ѭ����pid
int32_t Follow_PID(PID *s_PID, uint8_t position)
{
	int iError, output = 0;
	iError = s_PID->target - position; //���ֵ����
	s_PID->error_acc += iError;		   //����

	output = s_PID->kp * iError + s_PID->ki * s_PID->error_acc * 0.5f + s_PID->kd * iError - s_PID->lastError;
	output = s_PID->filter * output + (1 - s_PID->filter) * s_PID->lastFilter; //�˲�����

	s_PID->lastFilter = output; //�˲�ֵ�洢
	s_PID->lastError = iError;	// errorֵ�洢

	return (output);
}

//ֱ�е�αpid������Ŀ������Լ��ĸ��������ж�Ӧ�ó������ٶȣ�����Զ�Ϳ�һ�㣬���������һ�㡣
//�������ڻ��Ǽ��ѭ��pid��800-900���ٶȻ������Ǽ�����
int32_t Straight_PID(int nowPos, int targetPos)
{
	extern float Motor_speed1;
	extern float Motor_speed2;

	float dspeed = fabs(Motor_speed1 - Motor_speed2) / 2; //���˫��ת�ٵ�ʵ��ֵ��ƽ��ֵ

	int kp = 700;
	int out = (targetPos - nowPos) * kp;
	if (out >= 1000)
		out = 1000;
	return (out + dspeed) / 2; //Ŀ���ٶ��������ٶȼӺ�ƽ����Ϊ���ٶȹ��ȸ�ƽ��
}

//ת���pid
int32_t Turn_PID(PID *t_PID, int now_angle, int target_angle)
{
	int iError, output = 0;
	iError = target_angle - now_angle; //���ֵ����

	output = t_PID->kp * iError + t_PID->kd * iError - t_PID->lastError;

	t_PID->lastFilter = output; //�˲�ֵ�洢
	t_PID->lastError = iError;	// errorֵ�洢
	return (output);
}

//ͨ���ٶ�pid��ʵ��ת������Ŀ�귽�򼴿ɣ�ʵ��ת���ȿ������
void TurnBY_PID(int turn_angle)
{
	PID t_PID;
	turn_PIDInit(&t_PID);

	extern float Motor_speed1;
	extern float Motor_speed2;

	double now_angle = 0; //��ǰ�Ƕ�

	int flag_left = 1;	//�Ƿ�Ϊ��ת��־
	if (turn_angle < 0) //����Ŀ��Ƕ������һ��������޸ģ��Ա����
	{
		flag_left = 0;
		turn_angle = -turn_angle;
	}

	while (now_angle < turn_angle)
	{
		//����dt�����ֻ�����ǰ�ĽǶȣ�ԭ����̫���ס�
		Delay_10us(10);
		float dspeed = fabs(Motor_speed1 + Motor_speed2);
		now_angle = now_angle + (dspeed * 0.0001f) / (2 * 3.14159f * car_D) * 360;

		//��ʼǿ������,��ת�Ƕ���Ԥ�ڵ�2/3����ʱ�������⵽��������λ�ã�ֱ���Ƴ���ת��
		//��׼�Ļ�ȡǰ��̽ͷ������
		uint8_t begin, jump, count[6];
		uint8_t position;
		get_AMT1450Data_UART(&begin, &jump, count);

		if (jump == 2)
			position = 0.5f * (count[0] + count[1]); // position=����������м�λ�ã������ߵ�λ��

		if (position > 60 && position < 85 && now_angle > turn_angle / 3.0 * 2) //�����������λ�ò�����ת�Ƕ���Ԥ�ڵ�2/3����ʱ��ֱ���˳�
		{
			break;
		}

		//�������û��ת�����ϣ���һֱת
		if (now_angle > turn_angle - 3 && jump <= 1)
		{
			turn_angle += 5;
		}

		int out = (Turn_PID(&t_PID, now_angle, turn_angle) + dspeed / 2) / 2; //����ƽ��ʹ��ת��pid�õ���ֵ
		//�����Ƿ�Ϊ��ת�仯һ�£�ͬʱ��Ҫ�̶�30�ٶȣ���Ȼ����ת��̫����
		if (flag_left == 0)
			out = -out - 70;
		else
			out = out + 70;
		//������ٶ�ֵ
		MotorController_SetSpeed(1, out);
		MotorController_SetSpeed(2, out);
	}
	//Сͦ��һ��
	//MotorController_SetSpeed(1, 0);
	//MotorController_SetSpeed(2, 0);
	//Delay_ms(50);
}

//ת�������������׼ǰ���ٶȣ�ת�ٲ��ת�Ƕ�  �Զ�ת��Ԥ��Ƕȣ�ԭ�������pidת���ͬС��
void Turn_I(int nSpeed, int d_speed, int turn_angle)
{

	if (turn_angle < 0)
	{
		d_speed = -d_speed;
		turn_angle = -turn_angle;
	}

	MotorController_SetSpeed(1, nSpeed + d_speed);
	MotorController_SetSpeed(2, -nSpeed + d_speed);
	extern float Motor_speed1;
	extern float Motor_speed2;

	double now_angle = 0;
	while (now_angle < turn_angle)
	{
		Delay_10us(10);
		float dspeed = Motor_speed1 + Motor_speed2;
		if (dspeed < 0)
			dspeed = -dspeed;
		now_angle = now_angle + (dspeed * 0.0001f) / (2 * 3.14159f * car_D) * 360;

		//��ʼǿ������,��ת�Ƕ���Ԥ�ڵ�2/3����ʱ�������⵽��������λ�ã�ֱ���Ƴ���ת��
		uint8_t begin, jump, count[6]; // ���6�����䣬��3����
		uint8_t position;
		get_AMT1450Data_UART(&begin, &jump, count);

		if (jump == 2)
			position = 0.5f * (count[0] + count[1]); // position=����������м�λ��

		if (position > 50 && position < 100 && now_angle > turn_angle / 3.0 * 2)
		{
			break;
		}

		//�������û��ת�����ϣ���һֱת
		if (now_angle > turn_angle - 3 && jump <= 1)
		{
			turn_angle += 5;
		}
	}

	MotorController_SetSpeed(1, 0);
	MotorController_SetSpeed(2, 0);
	Delay_ms(50);
}

//С����ǰ����任+1��-1��С��������ʱ���Դ�Ϊ1234ѭ��
void Car_Direction_change(int add)
{
	while (add != 0)
	{
		if (add > 0)
		{
			car_direction += 1;
			add--;
		}
		else if (add < 0)
		{
			car_direction -= 1;
			add++;
		}
		if (car_direction > 4)
			car_direction = 1;
		if (car_direction < 1)
			car_direction = 4;
	}
};

//С��λ������+������������·�ھ��ǼӼ����������С��Ŀǰ�ķ����Զ��Ӻ�С�����ڵ����ꡣ
void Car_Position_add(int add)
{
	while (add--)
	{

		switch (car_direction)
		{
		case 1:
			car_position[0] += 1;
			break;
		case 2:
			car_position[1] += 1;
			break;
		case 3:
			car_position[0] -= 1;
			break;
		case 4:
			car_position[1] -= 1;
			break;
		default:
			break;
		}
	}
};

//��������һ�����Ӻ��˳�
void Straight_go(int nSpeed) //ֱ��һ����˳�
{
	PID s_PID;
	s_PIDInit(&s_PID);
	int crossing_flag = 0;

	while (1)
	{
		uint8_t begin, jump, count[6]; // ���6�����䣬��3����
		uint8_t position;
		get_AMT1450Data_UART(&begin, &jump, count); //�����ݴ洢������������
		if (jump == 2)
		{
			crossing_flag = 1;
			position = 0.5f * (count[0] + count[1]); // position=����������м�λ��
		}
		if (jump == 0 && begin == 0 && crossing_flag == 1)
		{
			Car_Position_add(1);
			crossing_flag = 0;
			printf("dir:%d  ; pos:%d,%d\r\n", car_direction, car_position[0], car_position[1]);
			break;
		}

		int32_t spid_out = Follow_PID(&s_PID, position);
		MotorController_SetSpeed(1, -nSpeed + spid_out);
		MotorController_SetSpeed(2, nSpeed + spid_out);
	}
}

void Straight_go_mm(int nSpeed, int distance) //ֱ�߶��ٺ��ף������ٶȺ�Ŀ����룬�Զ�����߶��ٺ��ס�
{

	MotorController_SetSpeed(1, nSpeed);
	MotorController_SetSpeed(2, -nSpeed);
	extern float Motor_speed1;
	extern float Motor_speed2;

	double dis = 0;

	PID s_PID;
	s_PIDInit(&s_PID);
	int crossing_flag = 0;

	while (dis < distance)
	{
		Delay_10us(10);
		dis = dis + (fabs(Motor_speed1 - Motor_speed2) / 2.0f) * 0.0001f; //�����ٶȻ��֣�����·�̳��ȡ�
		
		
		uint8_t begin, jump, count[6]; // ���6�����䣬��3����
		uint8_t position;
		get_AMT1450Data_UART(&begin, &jump, count); //�����ݴ洢������������
		if (jump == 2)
		{
			crossing_flag = 1;
			position = 0.5f * (count[0] + count[1]); // position=����������м�λ��
		}
		if (jump == 0 && begin == 0 && crossing_flag == 1)
		{
			Car_Position_add(1);
			crossing_flag = 0;
			printf("dir:%d  ; pos:%d,%d\r\n", car_direction, car_position[0], car_position[1]);
			break;
		}

		int32_t spid_out = Follow_PID(&s_PID, position);
		MotorController_SetSpeed(1, nSpeed + spid_out);
		MotorController_SetSpeed(2, -nSpeed + spid_out);
		
	}
}

//��С������ķ��������أ�ת���������������ÿ�����Ҫ�ĵġ�
void Turn_to_dir(int to_dir) //ת��Ŀ��Ƕ�
{
	if (abs(car_direction - to_dir) == 2)
	{
		Turn_I(0, 500, 180);
		Car_Direction_change(2);
	}
	else
	{
		if (car_direction == 1 || car_direction == 4)
		{
			if (car_direction == 1)
			{
				if (car_direction + 3 == to_dir)
				{
					Turn_I(0, 500, -90);
					Car_Direction_change(-1);
				}
				else
				{
					Turn_I(0, 500, 90);
					Car_Direction_change(1);
				}
			}
			else
			{
				if (car_direction - 3 == to_dir)
				{
					Turn_I(0, 500, 90);
					Car_Direction_change(1);
				}
				else
				{
					Turn_I(0, 500, -90);
					Car_Direction_change(-1);
				}
			}
		}
		else
		{
			if (car_direction > to_dir)
			{
				Turn_I(0, 500, -90);
				Car_Direction_change(-1);
			}
			else
			{
				Turn_I(0, 500, 90);
				Car_Direction_change(1);
			}
		}
	}
}

// Drive_Route,�Զ�����·������������ߣ����ÿ�����Ҫ�ĵġ�
void Drive_Route(int nSpeed, int tox, int toy)
{
	int way[50] = {0};

	//FindPath(way, car_position[0], car_position[1], tox, toy);

	for (int i = 1; i < 50 && way[i] != -1; i++)
	{
		printf("%d\n", way[i]);
		if (car_direction == way[i])
		{
			Straight_go(nSpeed);
		}
		else
		{
			Turn_to_dir(way[i]);
			i = i - 1;
		}
	}

	MotorController_SetSpeed(1, 0);
	MotorController_SetSpeed(2, 0);
	Delay_ms(300);
};
