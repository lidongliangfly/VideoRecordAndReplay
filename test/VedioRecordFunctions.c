#include "VideoRecordAndReplayFunctions.h"

int StartAndEndRecordTime(char *filename, int video_time_length)
{
	char record_start_time[20]; //starting recording time
	char record_end_time[20]; //the end of the recording time
	time_t time_now;
	struct tm *time_start = NULL;
	struct tm *time_end = NULL;

	time(&time_now);
	time_start = localtime(&time_now);
	//data format: year, month, day, hour, minute, seconds
	sprintf(record_start_time, "%4d%.2d%.2d%.2d%.2d%.2d",
			time_start->tm_year + 1900, time_start->tm_mon + 1,
			time_start->tm_mday, time_start->tm_hour, time_start->tm_min,
			time_start->tm_sec);

	time_now += video_time_length;
	time_end = localtime(&time_now);
	//data format: year, month, day, hour, minute, seconds
	sprintf(record_end_time, "%d%.2d%.2d%.2d%.2d%.2d", time_end->tm_year + 1900,
			time_end->tm_mon + 1, time_end->tm_mday, time_end->tm_hour,
			time_end->tm_min, time_end->tm_sec);

	//filename format: starting recording time - the end of the recording time
	sprintf(filename, "%s-%s%s", record_start_time, record_end_time,
			".hust.gz");
	return 1;
}

int WriteBlockToDisk(const void* buffer, size_t compressed_block_size,
		FILE* stream, unsigned int Frame_number, unsigned int block_x,
		unsigned int block_y, unsigned int block_width,
		unsigned int block_height)
{ //compressed_block_size means the number of compressed data bytes
	int nmemb = 0;
	nmemb += fwrite(&compressed_block_size, sizeof(size_t), 1, stream);
	nmemb += fwrite(&Frame_number, sizeof(unsigned int), 1, stream);
	nmemb += fwrite(&block_x, sizeof(unsigned int), 1, stream);
	nmemb += fwrite(&block_y, sizeof(unsigned int), 1, stream);
	nmemb += fwrite(&block_width, sizeof(unsigned int), 1, stream);
	nmemb += fwrite(&block_height, sizeof(unsigned int), 1, stream);
	nmemb += fwrite(buffer, compressed_block_size, 1, stream);
	if (nmemb == 7)
		return nmemb;
	else
	{
		printf("something wrong in WriteBlockToDisk!!!\n");
		return 0;
	}
}

int gzWriteBlockToDisk(const void* buffer, unsigned long compressed_block_size,
		gzFile stream, unsigned int Frame_number, unsigned int block_x,
		unsigned int block_y, unsigned int block_width,
		unsigned int block_height)
{ //compressed_block_size means the number of compressed data bytes
	int nmemb = 0;
	nmemb += gzwrite(stream, &compressed_block_size, sizeof(unsigned long));
	nmemb += gzwrite(stream, &Frame_number, sizeof(unsigned int));
	nmemb += gzwrite(stream, &block_x, sizeof(unsigned int));
	nmemb += gzwrite(stream, &block_y, sizeof(unsigned int));
	nmemb += gzwrite(stream, &block_width, sizeof(unsigned int));
	nmemb += gzwrite(stream, &block_height, sizeof(unsigned int));
	nmemb += gzwrite(stream, buffer, compressed_block_size);
	if (nmemb == compressed_block_size + 20 + sizeof(unsigned long))
		return nmemb;
	else
	{
		printf("something wrong in WriteBlockToDisk!!!\n");
		return 0;
	}
}

int GetDatablockFromXImage(XImage* newimg, char* block_data,
		unsigned int block_x, unsigned int block_y, unsigned int block_width,
		unsigned int block_height)
{

	int width = (int) newimg->width;
	//unsigned int height = newimg->height;
	unsigned int i;
	unsigned int j;
	unsigned int rightCornor_x = block_x + block_width;
	unsigned int rightCornor_y = block_y + block_height;
	char* p = newimg->data;
	unsigned int t = 0;
	for (i = block_y; i < rightCornor_y; i++)
	{
		for (j = block_x; j < rightCornor_x; j++)
		{
			*(block_data + t) = *(p + (i * width + j) * 4);
			t++;
			*(block_data + t) = *(p + (i * width + j) * 4 + 1);
			t++;
			*(block_data + t) = *(p + (i * width + j) * 4 + 2);
			t++;
			*(block_data + t) = *(p + (i * width + j) * 4 + 3);
			t++;
		}
	}
	if (t != block_width * block_height * 4)
	{
		printf("GetDatablockFromXImage inside write error !\n");
		return -1;
	}
	return 1;
}

int XImageDataCmp(char *s, char *d, int length)
{
	int i = 0, r = -1;
	for (; i < length; i++)
	{
		if (*(s + i) ^ *(d + i))
		{	//两个字符相异
			r = 1;
			break;
		}
	}
	if (i == length)	//字符串比较完，且完全相同
	{
		r = 0;
	}
	return r;
}

int CaptureAndCompare(Display* display, Window desktop, XImage* baseimg,
		XImage* newimg, unsigned int* block_n)
{
	int block_num = 0, block_n_index = 0;//the length of block_n,新添加的指针索引变量block_n_index
	int screen_width = (int) baseimg->width;
	//int screen_height = (int) baseimg->height;

	int tile_width = 64, tile_height = 64, i = 0, j = 0;
	int n_x = (baseimg->width - 1) / tile_width + 1, tail_x = baseimg->width
			- tile_width * (n_x - 1);
	int n_y = (baseimg->height - 1) / tile_height + 1, tail_y = baseimg->height
			- tile_height * (n_y - 1);
	int tile_diff[n_x * n_y];	//save the next frame diff
	//unsigned int tile_diff[264];
	//compare 2 frames ,TileDiff
	{
		int k = 0, xx = 0, j, i;
		srand((int) time(NULL));
		for (i = 0; i < n_y - 1; i++)
		{
			j = (int) (((float) tile_height) * rand() / RAND_MAX)
					+ tile_height * i;    //the scan line
			for (xx = 0; xx < n_x - 1; xx++)
				tile_diff[k++] = XImageDataCmp(
						baseimg->data + j * screen_width * 4
								+ xx * tile_width * 4,
						newimg->data + j * screen_width * 4
								+ xx * tile_width * 4, tile_width * 4);
			tile_diff[k++] = XImageDataCmp(
					baseimg->data + j * screen_width * 4 + xx * tile_width * 4,
					newimg->data + j * screen_width * 4 + xx * tile_width * 4,
					tail_x * 4);
		}
		j = (int) (((float) tail_y) * rand() / RAND_MAX) + tile_height * i; //the scan line
		for (xx = 0; xx < n_x - 1; xx++)
			tile_diff[k++] = XImageDataCmp(
					baseimg->data + j * screen_width * 4 + xx * tile_width * 4,
					newimg->data + j * screen_width * 4 + xx * tile_width * 4,
					tile_width * 4);
		tile_diff[k++] = XImageDataCmp(
				baseimg->data + j * screen_width * 4 + xx * tile_width * 4,
				newimg->data + j * screen_width * 4 + xx * tile_width * 4,
				tail_x * 4);
	}    //compare 2 frames done;save result to tile_diff[]
	for (i = 0; i < n_y; i++)    //calculate block_num
		for (j = 0; j < n_x; j++)
			if (tile_diff[i * n_x + j])
				block_num++;
	if (!block_num)
		return 0;
	//if 更新基础帧
	if (block_num >= (n_x * n_y * 0.5))
	{
		XDestroyImage(baseimg);
		baseimg = newimg;
		return 1;    //更新基础帧
	}
	//这里是函数体，比较不同并保存(按block_x,block_y,block_width,block_height)数据在block_n
	for (i = 0; i < n_y - 1; i++)
	{
		for (j = 0; j < n_x - 1; j++)
			if (tile_diff[i * n_x + j])
			{
				*(block_n + block_n_index++) = j * tile_width;    //x
				*(block_n + block_n_index++) = i * tile_height;    //y
				*(block_n + block_n_index++) = tile_width;    //w
				*(block_n + block_n_index++) = tile_height;    //h
			}
		//the last column
		if (tile_diff[i * n_x + j])
		{
			*(block_n + block_n_index++) = j * tile_width;    //x
			*(block_n + block_n_index++) = i * tile_height;    //y
			*(block_n + block_n_index++) = tail_x;    //;w
			*(block_n + block_n_index++) = tile_height;    //h
		}
	}
	//the last line
	for (j = 0; j < n_x - 1; j++)
		if (tile_diff[i * n_x + j])
		{
			*(block_n + block_n_index++) = j * tile_width;    //x
			*(block_n + block_n_index++) = i * tile_height;    //y
			*(block_n + block_n_index++) = tile_width;    //w
			*(block_n + block_n_index++) = tail_y;    //;h
		}
	//the last block
	if (tile_diff[i * n_x + j])
	{
		*(block_n + block_n_index++) = j * tile_width;    //x
		*(block_n + block_n_index++) = i * tile_height;    //y
		*(block_n + block_n_index++) = tail_x;    //;w
		*(block_n + block_n_index++) = tail_y;    //;h
	}
	{    //进行区域合并
		struct DiffBlock *head = NULL;
		block_num = ConcatenateDiffBlocks(head, block_n, block_num * 4);
	}

	return block_num;  //返回block_n的长度(按block_x,block_y,block_width,block_height)
}

int CompressAndWrite(const char* filename, int frame_rate,
		int video_time_length)
{
	Window desktop;
	Display* display;
	XImage* base_img;
	XImage* new_img;
	unsigned int Frame_number = 0;
	unsigned int block_x;
	unsigned int block_y;
	unsigned int block_width;
	unsigned int block_height;
	unsigned long uncompress_block_size = 0;
	unsigned long blen;
	unsigned char* buf = NULL;
	int block_num = 0;
	char* block_data = NULL;
	display = XOpenDisplay(NULL); //connect to a local display
	if (NULL == display)
	{
		printf("CaptureDesktop cannot get root window");
		return 0;
	}
	desktop = RootWindow(display, 0); //refer to root window
	if (desktop == 0)
	{
		printf("CaptureDesktop cannot get root window");
		return 0;
	}
	//get the image of the root window
	//FILE *fp = fopen(filename, "wb");
	gzFile gfp = gzopen(filename, "wb9"); //用于测试文件格式
	if (gfp == 0)
	{
		printf("could not open  file!!");
		return 0;
	}
	unsigned int screen_width = DisplayWidth(display, 0);
	unsigned int screen_height = DisplayHeight(display, 0);
	base_img = XGetImage(display, desktop, 0, 0, screen_width, screen_height,
			~0,
			ZPixmap);
	/* new_img = XCreateImage(display, DefaultVisual(display, 0), 24, ZPixmap, 0,
	 NULL, screen_width, screen_height, 32, 0);*/
	time_t start_time, end_time;
	time(&start_time);
	struct timeval start_tv;
	gettimeofday(&start_tv, NULL);
	//printf("%s", ctime(&start_time));
	int x = 0; //测试用，可删除
	while (1)
	{ //控制帧率的块
		x += block_num; //测试用，可删除
		new_img = XGetImage(display, desktop, 0, 0, screen_width, screen_height,
				~0,
				ZPixmap);
		unsigned int* block_parameter;
		int i=0;//处理blocks循环变量
		if ((block_parameter = (unsigned int*) malloc(2200)) == NULL)
		{
			printf("no enough memory!\n");
			return -1;
		}
		block_num = CaptureAndCompare(display, desktop, base_img, new_img,
				block_parameter);

		if (Frame_number != 0 && block_num == 0)
		{
			size_t no_update = 0;
			//fwrite(&no_update, sizeof(size_t), 1, fp);
			gzwrite(gfp, &no_update, sizeof(size_t)); //用于测试文件格式
			continue;
		}
		if (Frame_number == 0)
		{
			block_num = 1;
			block_x = 0;
			block_y = 0;
			block_width = screen_width;
			block_height = screen_height;
		}

		for (i = 0; i < block_num; i++)
		{
			if (Frame_number != 0)
			{ //赋值给block变量
				block_x = *(block_parameter + i * 4);
				block_y = *(block_parameter + i * 4 + 1);
				block_width = *(block_parameter + i * 4 + 2);
				block_height = *(block_parameter + i * 4 + 3);
				printf("%d %d %d %d 帧号:%d 块数:%d\n", block_x, block_y,
						block_width, block_height, Frame_number, x);
			}

			uncompress_block_size = block_width * block_height * 4;
			if ((block_data = (char*) malloc(uncompress_block_size)) == NULL)
			{
				printf("no enough memory!\n");
				return -1;
			}
			if (GetDatablockFromXImage(new_img, block_data, block_x, block_y,
					block_width, block_height) != 1)
			{
				printf("the GetDatablockFromXImage function has an error!\n");
				return -1;
			}

			//压缩部分 计算缓冲区大小，并为其分配内存
			blen = compressBound(uncompress_block_size);  //压缩后的长度是不会超过blen的
			if ((buf = (unsigned char*) malloc(sizeof(unsigned char) * blen))
					== NULL)
			{
				printf("no enough memory!\n");
				return -1;
			}

			//压缩
			if (compress(buf, &blen, (unsigned char*) block_data,
					uncompress_block_size) != Z_OK)
			{
				printf("compress failed!\n");
				return -1;
			}

			/*			WriteBlockToDisk(buf, blen, fp, Frame_number, block_x, block_y,
			 block_width, block_height);*/
			gzWriteBlockToDisk(buf, blen, gfp, Frame_number, block_x, block_y,
					block_width, block_height);  //用于测试文件格式
			if (buf != NULL || block_data != NULL)
			{
				free(buf);
				free(block_data);
				block_data = NULL;
				buf = NULL;

			}
		}

		Frame_number++;  //帧号
		free(block_parameter);
		block_parameter = NULL;
		XDestroyImage(new_img);

		while (1)
		{  //控制帧率
			struct timeval end_tv;
			gettimeofday(&end_tv, NULL);
			if (((end_tv.tv_sec - start_tv.tv_sec)
					+ 0.000001 * (end_tv.tv_usec - start_tv.tv_usec))
					>= Frame_number * 1.0 / frame_rate)
			{
				break;
			}
		}
		time(&end_time);
		if (difftime(end_time, start_time) >= video_time_length)
		{
			break;
		}
		//控制帧率的块end
	}

	/* 释放内存 */
	//fflush(fp);
	//fclose(fp);
	//fp = NULL;
	gzclose(gfp);	//用于测试文件格式
	XDestroyImage(base_img);
	XCloseDisplay(display);

	printf("CompressAndWrite perform successfully!\n");
	return 1;

}
