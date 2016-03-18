#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<zlib.h>

////////////////////////////记录函数

int StartAndEndRecordTime(char *filename, int video_time_length);

int WriteBlockToDisk(const void* buffer, size_t compressed_block_size,
		FILE* stream, unsigned int Frame_number, unsigned int block_x,
		unsigned int block_y, unsigned int block_width,
		unsigned int block_height);

int gzWriteBlockToDisk(const void* buffer, unsigned long compressed_block_size,
		gzFile stream, unsigned int Frame_number, unsigned int block_x,
		unsigned int block_y, unsigned int block_width,
		unsigned int block_height);

int GetDatablockFromXImage(XImage* newimg, char* block_data,
		unsigned int block_x, unsigned int block_y, unsigned int block_width,
		unsigned int block_height);

int XImageDataCmp(char *s, char *d, int length);

int CaptureAndCompare(Display* display, Window desktop, XImage* baseimg,
		XImage* newimg, unsigned int* block_n);

int CompressAndWrite(const char* filename, int frame_rate,
		int video_time_length, const char* SeverIpAddress );

///////////////////////////////回播函数

int ReadBlockFromDisk(void *buffer, size_t compressed_block_size, FILE* stream,
		unsigned int * Frame_number, unsigned int* block_x,
		unsigned int* block_y, unsigned int* block_width,
		unsigned int* block_height);

int gzReadBlockFromDisk(void *buffer, unsigned long compressed_block_size,
		gzFile stream, unsigned int * Frame_number, unsigned int* block_x,
		unsigned int* block_y, unsigned int* block_width,
		unsigned int* block_height);

int PutDatablockToXImage(XImage* baseimg, char* block_data,
		unsigned int block_x, unsigned int block_y, unsigned int block_width,
		unsigned int block_height);

int UncompressAndDisplay(const char* filename, int frame_rate);

/////////////////////合并函数

struct DiffBlock
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	struct DiffBlock * next;
};

void AscendingSortList(struct DiffBlock *head); //对链表进行选择排序(排序稳定)，结果为非递减顺序（排序成列优先）
int ConcatenateDiffBlocks(struct DiffBlock *head, unsigned int* block_n,
		int block_num);
void CreateList(struct DiffBlock **head, unsigned int A[], int length); //创建不含头结点的单向链表
int ListConvertToArray(struct DiffBlock *head, unsigned int* block_n);
struct DiffBlock * ConcatenateNodes_x(struct DiffBlock **headp); //合并横向右相邻的块
struct DiffBlock * ConcatenateNodes_y(struct DiffBlock **headp); //合并纵向相邻的块
void SwapDiffBlockNode(struct DiffBlock *p1, struct DiffBlock *p2); // swap 2 nodes's data
