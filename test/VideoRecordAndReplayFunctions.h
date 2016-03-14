#ifndef BLOCKCOMBINE_H
#define BLOCKCOMBINE_H

#endif // BLOCKCOMBINE_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<zlib.h>


struct DiffBlock
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	struct DiffBlock * next;
};
////////////////////////////记录函数


int StartAndEndRecordTime(char *filename, int video_time_length);

int WriteBlockToDisk(const void* buffer, size_t compressed_block_size,
		FILE* stream, unsigned int Frame_number, unsigned int block_x,
		unsigned int block_y, unsigned int block_width,
		unsigned int block_height);

int GetDatablockFromXImage(XImage* newimg, char* block_data,
		unsigned int block_x, unsigned int block_y, unsigned int block_width,
		unsigned int block_height);

int XImageDataCmp(char *s, char *d, int length);

int CaptureAndCompare(Display* display, Window desktop, XImage* baseimg,
        XImage* newimg, unsigned int* block_n);


int CompressAndWrite(const char* filename, int frame_rate,
		int video_time_length);

///////////////////////////////回播函数


int ReadBlockFromDisk(void *buffer, size_t compressed_block_size, FILE* stream,
		unsigned int * Frame_number, unsigned int* block_x,
		unsigned int* block_y, unsigned int* block_width,
		unsigned int* block_height);

int PutDatablockToXImage(XImage* baseimg, char* block_data,
		unsigned int block_x, unsigned int block_y, unsigned int block_width,
		unsigned int block_height);


int UncompressAndDisplay(const char* filename, int frame_rate);


/////////////////////
void AscendingSortList(DiffBlock *head); //对链表进行选择排序(排序稳定)，结果为非递减顺序（排序成列优先）
int ConcatenateDiffBlocks(DiffBlock *head, unsigned int* block_n,
		int block_num);
void CreateList(DiffBlock **head, unsigned int A[], int length); //创建不含头结点的单向链表
int ListConvertToArray(DiffBlock *head, unsigned int* block_n);
DiffBlock * ConcatenateNodes_x(DiffBlock **headp); //合并横向右相邻的块
DiffBlock * ConcatenateNodes_y(DiffBlock **headp); //合并纵向相邻的块
void SwapDiffBlockNode(DiffBlock *p1, DiffBlock *p2); // swap 2 nodes's data
