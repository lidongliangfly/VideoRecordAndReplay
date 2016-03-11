/*
 * test.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: baixingqiang
 *
 * g++ -o main main.cpp -lX11 -lz
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<zlib.h>



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
	sprintf(filename, "%s-%s%s", record_start_time, record_end_time, ".hust");
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

int ReadBlockFromDisk(void *buffer, size_t compressed_block_size, FILE* stream,
		unsigned int * Frame_number, unsigned int* block_x,
		unsigned int* block_y, unsigned int* block_width,
		unsigned int* block_height)
{

	if ((fread(Frame_number, sizeof(unsigned int), 1, stream)) != 1)
	{
		printf("read error or reach the file's end ！ ");
		return 0;
	}
	fread(block_x, sizeof(unsigned int), 1, stream);
	fread(block_y, sizeof(unsigned int), 1, stream);
	fread(block_width, sizeof(unsigned int), 1, stream);
	fread(block_height, sizeof(unsigned int), 1, stream);
	fread(buffer, compressed_block_size, 1, stream);
	return 1;

}

int GetDatablockFromXImage(XImage* newimg, char* block_data,
		unsigned int block_x, unsigned int block_y, unsigned int block_width,
		unsigned int block_height)
{
	//block_data = (unsigned char*) malloc(block_height * block_width * 4);
	int width = (int) newimg->width;
	//unsigned int height = newimg->height;
	unsigned int rightCornor_x = block_x + block_width;
	unsigned int rightCornor_y = block_y + block_height;
	char* p = newimg->data;
	unsigned int t = 0;
	for (unsigned int i = block_y; i < rightCornor_y; i++)
	{
		for (unsigned int j = block_x; j < rightCornor_x; j++)
		{	//XGetPixel(newimg, j, i);
			*(block_data + t) = *(p + (i * width + j) * 4);	//错误在于赋值
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

int PutDatablockToXImage(XImage* baseimg, char* block_data,
		unsigned int block_x, unsigned int block_y, unsigned int block_width,
		unsigned int block_height)
{
	unsigned int width = baseimg->width;
	//unsigned int height = baseimg->height;
	unsigned int rightCornor_x = block_x + block_width;
	unsigned int rightCornor_y = block_y + block_height;
	unsigned int t = 0;
	for (unsigned int i = block_y; i < rightCornor_y; i++)
	{
		for (unsigned int j = block_x; j < rightCornor_x; j++)
		{
			*(baseimg->data + (i * width + j) * 4) = *(block_data + t);
			t++;
			*(baseimg->data + (i * width + j) * 4 + 1) = *(block_data + t);
			t++;
			*(baseimg->data + (i * width + j) * 4 + 2) = *(block_data + t);
			t++;
			*(baseimg->data + (i * width + j) * 4 + 3) = *(block_data + t);
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

int MyStrcmp(char *s, char *d, int length)//campare the source string and the dest string
{
	int i = 0, r = -1;
	if (((s + i) == NULL) && ((s + i) == NULL))
		return r;
	for (; i < length; i++)
		if (*(s + i) != *(d + i))
		{
			r = 1;
			break;
		}
	if (i == length)
		r = 0;
	return r;
}

int CaptureAndCompare(Display* display, Window desktop, XImage* baseimg,
		XImage* newimg, unsigned int* block_n)
{
	int block_num = 0;	//the length of block_n
	int screen_width = (int) baseimg->width;
	int screen_height = (int) baseimg->height;

	int tile_width = 64, tile_height = 64, i = 0, j = 0;
	int n_x = (baseimg->width - 1) / tile_width + 1, tail_x = baseimg->width
			- tile_width * (n_x - 1);
	int n_y = (baseimg->height - 1) / tile_height + 1, tail_y = baseimg->height
			- tile_height * (n_y - 1);
	int tile_diff[n_x * n_y];	//save the next frame diff
	//compare 2 frames ,TileDiff
	{
		int k = 0, xx = 0, j, i;
		srand((int) time(NULL));
		for (i = 0; i < n_y - 1; i++)
		{
			j = (int) (((float) tile_height) * rand() / RAND_MAX)
					+ tile_height * i;    //the scan line
			for (xx = 0; xx < n_x - 1; xx++)
				tile_diff[k++] = MyStrcmp(
						baseimg->data + j * screen_width * 4
								+ xx * tile_width * 4,
						newimg->data + j * screen_width * 4
								+ xx * tile_width * 4, tile_width * 4);
			tile_diff[k++] = MyStrcmp(
					baseimg->data + j * screen_width * 4 + xx * tile_width * 4,
					newimg->data + j * screen_width * 4 + xx * tile_width * 4,
					tail_x * 4);
		}
		j = (int) (((float) tail_y) * rand() / RAND_MAX) + tile_height * i; //the scan line
		for (xx = 0; xx < n_x - 1; xx++)
			tile_diff[k++] = MyStrcmp(
					baseimg->data + j * screen_width * 4 + xx * tile_width * 4,
					newimg->data + j * screen_width * 4 + xx * tile_width * 4,
					tile_width * 4);
		tile_diff[k++] = MyStrcmp(
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
	if (block_num >= (n_x * n_y * 0.7))
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
				*(block_n++) = j * tile_width;    //x
				*(block_n++) = i * tile_height;    //y
				*(block_n++) = tile_width;    //w
				*(block_n++) = tile_height;    //h
			}
		//the last column
		if (tile_diff[i * n_x + j])
		{
			*(block_n++) = j * tile_width;    //x
			*(block_n++) = i * tile_height;    //y
			*(block_n++) = tail_x;    //;w
			*(block_n++) = tile_height;    //h
		}
	}
	//the last line
	for (j = 0; j < n_x - 1; j++)
		if (tile_diff[i * n_x + j])
		{
			*(block_n++) = j * tile_width;    //x
			*(block_n++) = i * tile_height;    //y
			*(block_n++) = tile_width;    //w
			*(block_n++) = tail_y;    //;h
		}
	//the last block
	if (tile_diff[i * n_x + j])
	{
		*(block_n++) = j * tile_width;    //x
		*(block_n++) = i * tile_height;    //y
		*(block_n++) = tail_x;    //;w
		*(block_n++) = tail_y;    //;h
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
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		printf("could not open  file!!");
		return 0;
	}
	unsigned int screen_width = DisplayWidth(display, 0);
	unsigned int screen_height = DisplayHeight(display, 0);
	base_img = XGetImage(display, desktop, 0, 0, screen_width, screen_height,
			~0,
			ZPixmap);

	/*	char* new_img_data = (char*) malloc(screen_height * screen_width * 4);
	 new_img = XCreateImage(display, DefaultVisual(display, 0), 24, ZPixmap, 0,
	 NULL, screen_width, screen_height, 32, 0);*/
	time_t start_time, end_time;
	time(&start_time);
	struct timeval start_tv;
	gettimeofday(&start_tv, NULL);
	//printf("%s", ctime(&start_time));
	while (1)
	{ //控制帧率的块
	  //sleep(2);
		new_img = XGetImage(display, desktop, 0, 0, screen_width, screen_height,
				~0,
				ZPixmap);
		unsigned int* block_parameter;
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
			fwrite(&no_update, sizeof(size_t), 1, fp);
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
		for (int i = 0; i < block_num; i++)
		{
			if (Frame_number != 0)
			{ //赋值给block变量，需删改
			  //Retrive the width and the height of the screen
				block_x = *(block_parameter + i * 4);
				block_y = *(block_parameter + i * 4 + 1);
				block_width = *(block_parameter + i * 4 + 2);
				block_height = *(block_parameter + i * 4 + 3);
				printf("%d %d %d %d 帧号:%d\n", block_x, block_y, block_width,
						block_height, Frame_number);
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

			WriteBlockToDisk(buf, blen, fp, Frame_number, block_x, block_y,
					block_width, block_height);
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
		{//控制帧率
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
		//控制帧率的块
	}

	/* 释放内存 */
	fflush(fp);
	fclose(fp);
	XDestroyImage(base_img);
	XCloseDisplay(display);
	fp = NULL;
	printf("CompressAndWrite perform successfully!");
	return 1;

}

int UncompressAndDisplay(const char* filename, int frame_rate)
{

	size_t compressed_block_size;
	size_t uncompressed_block_size;
	unsigned int Frame_number;
	int display_flag = -1;
	unsigned int block_x;
	unsigned int block_y;
	unsigned int block_width;
	unsigned int block_height;

	unsigned char* buf = NULL;
	char* img_data = NULL;
	XImage* img;
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		printf("could not open  file!!");
		return 0;
	}
	Display* display = XOpenDisplay(NULL); //connect to a local display
	if (NULL == display)
	{
		printf("CaptureDesktop cannot get root window");
		return 0;
	}
	Window desktop = RootWindow(display, 0); //refer to root window
	if (desktop == 0)
	{
		printf("CaptureDesktop cannot get root window");
		return 0;
	}

	//Retrive the width and the height of the screen
	unsigned int screen_width = DisplayWidth(display, 0);
	unsigned int screen_height = DisplayHeight(display, 0);

	Window window = XCreateSimpleWindow(display, desktop, 0, 0, screen_width,
			screen_height, 1, 0, 0);
	img = XGetImage(display, desktop, 0, 0, screen_width, screen_height, ~0,
	ZPixmap);

	//start reading da from disk
	while (1)
	{
		if (fread(&compressed_block_size, sizeof(size_t), 1, fp) == 0)
		{
			printf("reach the end  or read error !\n");
			return -1;
		}
		if (compressed_block_size == 0)
			continue;
		if ((buf = (unsigned char*) malloc(compressed_block_size)) == NULL)
		{
			printf("no enough memory!\n");
			return -1;
		}
		//读数据
		ReadBlockFromDisk(buf, compressed_block_size, fp, &Frame_number,
				&block_x, &block_y, &block_width, &block_height);

		if ((img_data = (char*) malloc((block_width) * (block_height) * 4))
				== NULL)
		{
			printf("no enough memory!\n");
			return -1;
		}
		//解压缩
		uncompressed_block_size = block_width * block_height * 4;
		if (uncompress((unsigned char*) img_data, &uncompressed_block_size, buf,
				compressed_block_size) != Z_OK)
		{
			printf("uncompress failed!\n");
			return -1;
		}

		if (Frame_number - display_flag == 1)
		{ //用于显示图片 需修改
		  //XSelectInput(display, window, ButtonPressMask | ExposureMask);
			XMapWindow(display, window);
			/*XEvent ev;
			 bool displayflag = true;
			 while (displayflag)
			 {

			 XNextEvent(display, &ev);
			 switch (ev.type)
			 {
			 case Expose:
			 XPutImage(display, window, DefaultGC(display, 0), img, 0, 0,
			 block_x, block_y, block_width, block_height);
			 break;
			 case ButtonPress:
			 displayflag = false;
			 //XUnmapWindow(display, window);
			 }

			 }*/
			XPutImage(display, window, DefaultGC(display, 0), img, 0, 0, 0, 0,
					screen_width, screen_height);
			display_flag++;

			usleep(1000000.0 / frame_rate);
		}

		PutDatablockToXImage(img, img_data, block_x, block_y, block_width,
				block_height);
		//
	}
	/* 释放内存 */
	XUnmapWindow(display, window);
	fclose(fp);
	XDestroyWindow(display, window);
	XDestroyImage(img);
	XCloseDisplay(display);
	if (buf != NULL || img_data != NULL)
	{
		free(buf);
		free(img_data);
		buf = NULL;
		img_data = NULL;
		fp = NULL;
	}

	return 1;
}


int main()
{
	char filename[40];
	int frame_rate = 25;   //the frame rate
	int video_time_length = 0.5 * 60; //the totle time of each video, units are seconds

	int n = 1;
	for (int i = 0; i < n; i++)
	{
		StartAndEndRecordTime(filename, video_time_length);
		CompressAndWrite(filename, frame_rate, video_time_length);
		//UncompressAndDisplay("./20160311140358-20160311140428.hust", frame_rate);
	}

	printf(" Done.\n");

	return 1;
}
