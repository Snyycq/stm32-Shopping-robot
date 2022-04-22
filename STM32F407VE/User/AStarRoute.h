#include <math.h>
#include "stdio.h"
#include "stdlib.h"

#define MaxLength 50
#define Height 10
#define Width 10

//�ڵ�����
#define Reachable 0	  //���Ե���Ľڵ�
#define Bar 1		  //�ϰ���
#define Pass 2		  //
#define Source 3	  //���
#define Destination 4 //�յ�

// 8bits�ֱ��������8���ڵ��Ƿ�ɵ���
#define North (1 << 0)
#define North_West (1 << 1)
#define West (1 << 2)
#define South_West (1 << 3)
#define South (1 << 4)
#define South_East (1 << 5)
#define East (1 << 6)
#define North_East (1 << 7)

typedef struct //����
{
	int x, y;
} Point;



typedef struct //�ڵ�
{
	int x, y;	   //�ڵ�����
	int reachable; //�ڵ�����
	int sur;	   //���ڽڵ�ķ�λ
	int value;
} MapNode;

typedef struct Close // list�����Ա
{
	MapNode *cur;		//�ڵ�
	char vis;			//��Чֵ���Ƿ��ѱ�����close list
	struct Close *from; //���ڵ� �����Ա
	int F, G;
	int H;
	int dir; //��һ����λ
} Close;

typedef struct //���ȶ��У�Open��
{
	int length;				 //��ǰ���еĳ���
	Close *Array[MaxLength]; //���۽���ָ��
} Open;

void FindPath(int *way, int sx, int sy, int dx, int dy);
