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

int32_t Follow_PID(PID *s_PID, uint8_t position);	  //���������pid�ṹ����ǰλ�� ���أ�ֱ��pid����ֵ
void Turn_I(int nSpeed, int d_speed, int turn_angle); //�����������׼ǰ���ٶȣ�ת�ٲ��ת�Ƕ�  ���أ���תpid����ֵ

extern int car_direction;	//С������
extern int car_position[2]; //С��λ��

extern int crossing_flag;//����ʮ��·�ڼ�����ʱ����
extern int map_count;//��ͼʮ��·�ڼ���
extern int map_index;//���ڵ�ͼ�ڲ���һ������������

extern uint8_t begin, jump, count[6];//ѭ��ģ�����ز���
extern uint8_t line_position;//ֱ�ߵ�ǰ���ڵ�λ��

void Car_Direction_change(int add); //С���������
void Car_Position_add(int add);		//С��λ��+������������·�ھ��ǼӼ���
void Drive_Route(int nSpeed, int tox, int toy);
void Straight_go_mm(int nSpeed, int distance); //ֱ�߶��پ��롣
void TurnBY_PID(int turn_angle);			   //ʹ��pidת��

int32_t Straight_PID(int nowPos, int targetPos);
void Crossing_Detection(void);//����Ƿ񾭹����̵�
void Map_Action(int *map_index);//��ͼ��Ϊ������Ŀǰ���ڵڼ���·�ڣ�ִ�����ת�����
