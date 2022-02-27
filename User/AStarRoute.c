#include "AStarRoute.h"



int srcX, srcY, dstX, dstY;

MapNode graph[Height][Width];	  //��ͼ�ڵ�
Close close[Height][Width] = {0}; //��ͼ��Ա

const Point dir[4] = //�ϱ����������Ҷ�
	{
		{-1, 0}, // North 				//���ƶ���x-1
		{0, -1}, // West
		{1, 0},	 // South
		{0, 1},	 // East
};
	
/* ����һ�ŵ�ͼ�����õ�ͼ���ڵ����ԣ�������ƶ�����*/
void initGraph(int map[Height][Width], int sx, int sy, int dx, int dy)
{
	int i, j;
	srcX = sx; //���X����
	srcY = sy; //���Y����
	dstX = dx; //�յ�X����
	dstY = dy; //�յ�Y����
	for (i = 0; i < Height; i++)
	{
		for (j = 0; j < Width; j++)
		{
			graph[i][j].x = i; //��ͼ����X
			graph[i][j].y = j; //��ͼ����Y
			graph[i][j].value = map[i][j];
			graph[i][j].reachable = (graph[i][j].value == Reachable); // �ڵ��Ƿ�ɵ���
			graph[i][j].sur = 0;									  //�ɵ����ڽӽڵ�λ��
			if (!graph[i][j].reachable)
			{
				continue;
			}
			//���ó��߿��⣬�ɵ����ڽӽڵ�λ��
			if (j > 0)
			{
				if (graph[i][j - 1].reachable) // left�ڵ�ɵ���
				{
					graph[i][j].sur |= West;
					graph[i][j - 1].sur |= East;
				}
				// if (i > 0)
				// {
				// 	if (graph[i - 1][j - 1].reachable && graph[i - 1][j].reachable && graph[i][j - 1].reachable) // up-left�ڵ�ɵ���
				// 	{
				// 		graph[i][j].sur |= North_West;
				// 		graph[i - 1][j - 1].sur |= South_East;
				// 	}
				// }
			}
			if (i > 0)
			{
				if (graph[i - 1][j].reachable) // up�ڵ�ɵ���
				{
					graph[i][j].sur |= North;
					graph[i - 1][j].sur |= South;
				}
				// if (j < Width - 1)
				// {
				// 	if (graph[i - 1][j + 1].reachable && graph[i - 1][j].reachable && map[i][j + 1] == Reachable) // up-right�ڵ�ɵ���
				// 	{
				// 		graph[i][j].sur |= North_East;
				// 		graph[i - 1][j + 1].sur |= South_West;
				// 	}
				// }
			}
		}
	}
}

//��ͼ�ڵ��Ա��ʼ������
//������㼰�յ�
void initClose(Close cls[Height][Width], int sx, int sy, int dx, int dy)
{
	// ��ͼClose���ʼ������
	int i, j;
	for (i = 0; i < Height; i++)
	{
		for (j = 0; j < Width; j++)
		{
			cls[i][j].cur = &graph[i][j];			// Close����ָ�ڵ�
			cls[i][j].vis = !graph[i][j].reachable; // �ܷ񱻷���
			cls[i][j].from = NULL;					// ���ڵ�
			cls[i][j].G = cls[i][j].F = 0;
			cls[i][j].H = 10 * abs(dx - i) + 10 * abs(dy - j); // ����ʮ�������⸡������
		}
	}
	cls[sx][sy].F = cls[sx][sy].H; //��ʼ�����۳�ʼֵ
	cls[sy][sy].G = 0;			   //�Ʋ����Ѵ���ֵ
								   // cls[dx][dy].G = Infinity;
}

// Open���ʼ��
void initOpen(Open *q) //���ȶ��г�ʼ��
{
	q->length = 0; // ���ڳ�Ա����ʼΪ0
}

/*�����ȶ��У�Open������ӳ�Ա
   ������*/
void push(Open *q, Close cls[Height][Width], int x, int y, int g)
{
	Close *t;
	int i, mintag;

	cls[x][y].G = g; //����ӽڵ������
	cls[x][y].F = cls[x][y].G + cls[x][y].H;

	q->Array[q->length++] = &(cls[x][y]);
	mintag = q->length - 1;
	for (i = 0; i < q->length - 1; i++) //ȷ����Fֵ��С�Ľڵ�
	{
		if (q->Array[i]->F < q->Array[mintag]->F)
		{
			mintag = i;
		}
	}
	t = q->Array[q->length - 1];
	q->Array[q->length - 1] = q->Array[mintag];
	q->Array[mintag] = t; //��Fֵ��С�ڵ����ڱ�ͷ
}

//ȡ��Open list���е�Fֵ��С�ĳ�Ա
Close *shift(Open *q)
{
	return q->Array[--q->length];
}

Open q;	  // Open��
Close *p; // list���Ա
Close *astar()
{								   // A*�㷨����
	int i, curX, curY, surX, surY; //��ǰ�ڵ����꣬Ŀ��ڵ�����
	int surG;

	initOpen(&q);
	initClose(close, srcX, srcY, dstX, dstY); //������� �� �յ㣬�����������Ϊ��Ա
	close[srcX][srcY].vis = 1;
	push(&q, close, srcX, srcY, 0); //������Close list������Ϊ���ɷ���

	while (q.length)
	{
		p = shift(&q);
		curX = p->cur->x;
		curY = p->cur->y;
		if (!p->H)
		{
			return p;
		}
		for (i = 0; i < 4; i++)
		{
			if (!(p->cur->sur & (1 << i * 2)))
			{
				continue;
			}
			surX = curX + dir[i].x;
			surY = curY + dir[i].y;

			// surG = p->G + sqrt((curX - surX) * (curX - surX) + (curY - surY) * (curY - surY));
			// surG = p->G + abs(curX - surX) + abs(curY - surY);
			//����������ʮ�������⸡������
			if (abs(dir[i].x) > 0 && abs(dir[i].y) > 0)
				surG = p->G + 14; // 1.414
			else
				surG = p->G + 10;
			if (!close[surX][surY].vis) //��ǰ�ڵ��Ա����Openlist��
			{
				close[surX][surY].vis = 1; //����
				close[surX][surY].from = p;
				close[surX][surY].dir = (i + 2) % 4 + 1;
				push(&q, close, surX, surY, surG);
			}
			else
			{
				if (surG <= close[surX][surY].G) // Openlist���Ѿ�����ʱ�����G��Сʱ�����µ�ǰ�ڵ��Ա
				{
					close[surX][surY].vis = 1;
					close[surX][surY].from = p;
					close[surX][surY].dir = (i + 2) % 4 + 1;
					push(&q, close, surX, surY, surG);
				}
			}
		}
	}
	return 0; //�޽��
}

void FindPath(int *way, int sx, int sy, int dx, int dy)
{
	int map[10][10] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
		{0, 0, 0, 1, 1, 1, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	};

	initGraph(map, sx, sy, dx, dy);
	Close *t = astar();
	Close *ret = t;
	int len = 0;

	while (ret != NULL)
	{
		//printf("from:%d,%d  dir:%d\n", ret->cur->x, ret->cur->y, ret->dir);
		ret = ret->from;
		len++;
	}
	// int route[len][3];
	way[len]=-1;
	for (int i = len - 1; i >= 0; i--)
	{
		// route[i][0] = t->cur->x;
		// route[i][1] = t->cur->y;
		way[i] = t->dir;
		t = t->from;
	}
}
