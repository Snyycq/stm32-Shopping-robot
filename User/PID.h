#include "stm32f4xx.h"

typedef struct PID
{
	float kp;
	float ki;
	float kd;

	float error_acc;
	float lastError;
	float prevError;

	float target;
	
	float filter;
	float lastFilter;
} PID;

void s_PIDInit(PID *s_PID);

int32_t Straight_PID(PID *s_PID, uint8_t position);					//���������pid�ṹ����ǰλ�� ���أ�ֱ��pid����ֵ
void Turn_I(int nSpeed,int d_speed, int turn_angle); //�����������׼ǰ���ٶȣ�ת�ٲ��ת�Ƕ�  ���أ���תpid����ֵ
