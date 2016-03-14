#include <stdio.h>
#include <malloc.h>

struct DiffBlock
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	struct DiffBlock * next;
};

void SwapDiffBlockNode(DiffBlock *p1, DiffBlock *p2) // swap 2 nodes's data
{
	int tmp;
	tmp = p1->x;
	p1->x = p2->x;
	p2->x = tmp;
	tmp = p1->y;
	p1->y = p2->y;
	p2->y = tmp;
	tmp = p1->width;
	p1->width = p2->width;
	p2->width = tmp;
	tmp = p1->height;
	p1->height = p2->height;
	p2->height = tmp;
}

void CreateList(DiffBlock **head, unsigned int *A, int length) //创建不含头结点的单向链表
{
	DiffBlock *loc_head = NULL, *tail;
	int i = 0; //新添加的指针索引变量
	if (length && (A + i) && (A + i + 1) && (A + i + 2) && (A + i + 3))
	{
		loc_head = (DiffBlock*) malloc(sizeof(DiffBlock));
		loc_head->next = NULL;
		loc_head->x = *(A + i++);
		loc_head->y = *(A + i++);
		loc_head->width = *(A + i++);
		loc_head->height = *(A + i++);
		tail = loc_head;
		while (i < length)
			if ((A + i) && (A + i + 1) && (A + i + 2) && (A + i + 3))
			{
				tail->next = (DiffBlock*) malloc(sizeof(DiffBlock));
				tail = tail->next;
				tail->x = *(A + i++);
				tail->y = *(A + i++);
				tail->width = *(A + i++);
				tail->height = *(A + i++);
			}
		tail->next = NULL;
	}
	*head = loc_head;
}

DiffBlock * ConcatenateNodes_x(DiffBlock **headp) //合并横向右相邻的块
{
	DiffBlock *head = *headp, *current = *headp, *next = NULL;
	while (current)
	{
		next = current->next;
		if ((current->next)
				&& (current->x + current->width == current->next->x))
		{ //下节点存在且右相邻
			current->width += next->width;
			current->next = current->next->next;
			free(next);
		} //合并后（宽度增加）释放相邻节点
		else
			current = current->next; //当前节点无相邻节点，继续处理其下一节点
	}
	return head;
}

/*void AscendingSortList(DiffBlock *head)//对链表进行选择排序(排序不稳定)，结果为非递减顺序（排序成列优先）
 {
 DiffBlock *p1=head,*p2;
 int len=0,i,j;
 while(p1)   {len++;p1=p1->next;}
 for(i=0,p1=head;i<len-1;i++,p1=p1->next)
 for(j=i+1,p2=p1->next;j<len;j++,p2=p2->next)
 if(p1->x > p2->x) SwapDiffBlockNode(p1,p2);
 }*/

void AscendingSortList(DiffBlock *head) //对链表进行冒泡排序(排序稳定)，结果为非递减顺序（排序成列优先）
{
	DiffBlock *p1 = head, *p2;
	int len = 0, i, j;
	while (p1)
	{
		len++;
		p1 = p1->next;
	}
	for (i = 0, p1 = head; i < len - 1; i++, p1 = p1->next)
		for (j = 0, p2 = head; j < len - i - 1; j++, p2 = p2->next)
			if (p2->x > p2->next->x)
				SwapDiffBlockNode(p2, (p2->next)); //当前节点比后继节点数据大 交换
}

DiffBlock * ConcatenateNodes_y(DiffBlock **headp) //合并纵向相邻的块
{
	DiffBlock *head = *headp, *current = *headp, *next = NULL;
	while (current)
	{
		next = current->next;
		if ((current->next) && (current->x == next->x)
				&& (current->width == next->width)
				&& (current->y + current->height == current->next->y))
		{ //下节点存在,x相同且矩形宽相同，即合并后可以构成矩形
		  //合并后（高度增加）释放相邻节点
			current->height += next->height;
			current->next = current->next->next;
			free(next);
		}
		else
			current = current->next;            //当前节点无相邻节点，继续处理其下一节点
	}
	return head;
}

int ListConvertToArray(DiffBlock *head, unsigned int* block_n)
{
	DiffBlock *p;
	int length = 0, i = 0;            //新添加的指针索引变量i
	p = head;
	while (p)
	{
		*(block_n + i++) = p->x;
		*(block_n + i++) = p->y;
		*(block_n + i++) = p->width;
		*(block_n + i++) = p->height;
		length++;
		p = p->next;
	}
	return length;
}

int ConcatenateDiffBlocks(DiffBlock *head, unsigned int* block_n, int block_num)
{
	CreateList(&head, block_n, block_num);
	ConcatenateNodes_x(&head);
	AscendingSortList(head);
	ConcatenateNodes_y(&head);
	return ListConvertToArray(head, block_n);
}
