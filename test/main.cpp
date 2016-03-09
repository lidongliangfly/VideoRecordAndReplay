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

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<zlib.h>

size_t WriteBlockToDisk(const void* buffer, size_t compressed_block_size,
		FILE* stream, unsigned char block_flag, unsigned int block_x,
		unsigned int block_y, unsigned int block_width,
		unsigned int block_height)
{ //compressed_block_size means the number of compressed data bytes
	size_t nmemb = 0;
	nmemb += fwrite(&compressed_block_size, sizeof(size_t), 1, stream);
	nmemb += fputc(block_flag, stream);
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

/*int CaptureAndCompare(Display* display,Drawable desktop,unsigned int screen_width,unsigned int screen_height, XImage* baseimg,XImage* newimg,unsigned int* block_n)
{
	if(newimg!=NULL)
		newimg=NULL;
	newimg== XGetImage(display, desktop, 0, 0, screen_width,
			screen_height, ~0,
			ZPixmap);



	这里是函数体，比较不同并保存(按block_x,block_y,block_width,block_height)数据在block_n


	if 更新基础帧
	XDestroyImage(baseimg);
	baseimg=newimg;

	//返回block_n的长度(按block_x,block_y,block_width,block_height)
}*/

int CompressAndWrite(const char* filename)
{
	Window desktop;
	Display* display;
	XImage* img;
	XImage* base_img;
	XImage* compare_img;
	unsigned char block_flag;
	unsigned int block_x;
	unsigned int block_y;
	unsigned int block_width;
	unsigned int block_height;
	unsigned long uncompress_block_size;
	unsigned long blen;
	unsigned char* buf = NULL;
	unsigned int* block_num=(unsigned int*)malloc(550);
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
	{ //控制帧率的块

		//CaptureAndCompare

		{ //赋值给block变量，需删改
		  //Retrive the width and the height of the screen
			block_flag = 1;
			block_x = 0;
			block_y = 0;
			block_width = DisplayWidth(display, 0);
			block_height = DisplayHeight(display, 0);

		}
		int n = 10;
		uncompress_block_size = block_width * block_height * 4;
		for (int i = 0; i < n; i++)
		{

			img = XGetImage(display, desktop, block_x, block_y, block_width,
					block_height, ~0,
					ZPixmap);

			//压缩部分 计算缓冲区大小，并为其分配内存
			blen = compressBound(uncompress_block_size);  //压缩后的长度是不会超过blen的
			if ((buf = (unsigned char*) malloc(sizeof(unsigned char) * blen))
					== NULL)
			{
				printf("no enough memory!\n");
				return -1;
			}

			//压缩
			if (compress(buf, &blen, (unsigned char*) img->data,
					uncompress_block_size) != Z_OK)
			{
				printf("compress failed!\n");
				return -1;
			}

			WriteBlockToDisk(buf, blen, fp, block_flag, block_x, block_y,
					block_width, block_height);
			if (buf != NULL)
			{
				free(buf);
				fp = NULL;

			}
		}
		//控制帧率的块
	}

	/* 释放内存 */
	fclose(fp);
	XDestroyImage(img);
	XCloseDisplay(display);
	fp = NULL;

	return 1;

}

int ReadBlockFromDisk(void *buffer, size_t compressed_block_size, FILE* stream,
		int * block_flag, unsigned int* block_x, unsigned int* block_y,
		unsigned int* block_width, unsigned int* block_height)
{
	if ((*block_flag = fgetc(stream)) == EOF)
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

int UncompressAndDisplay(const char* filename)
{

	size_t compressed_block_size;
	size_t uncompressed_block_size;
	int block_flag;
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
	Visual* visual = DefaultVisual(display, 0);
	//Retrive the width and the height of the screen
	unsigned int screen_width = DisplayWidth(display, 0);
	unsigned int screen_height = DisplayHeight(display, 0);

	Window window = XCreateSimpleWindow(display, desktop, 0, 0, screen_width,
			screen_height, 1, 0, 0);

	//start reading da from disk
	while (1)
	{
		if (fread(&compressed_block_size, sizeof(size_t), 1, fp) == 0)
		{
			printf("reach the end of file or read error !\n");
			return -1;
		}
		if ((buf = (unsigned char*) malloc(compressed_block_size)) == NULL)
		{
			printf("no enough memory!\n");
			return -1;
		}
		//读数据
		ReadBlockFromDisk(buf, compressed_block_size, fp, &block_flag, &block_x,
				&block_y, &block_width, &block_height);

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

		img = XCreateImage(display, visual, 24, ZPixmap, 0, img_data,
				block_width, block_height, 32, 0);

		{ //用于显示图片 需修改
			XSelectInput(display, window, ButtonPressMask | ExposureMask);
			XMapWindow(display, window);
			XEvent ev;
			bool display_flag = true;
			while (display_flag)
			{

				XNextEvent(display, &ev);
				switch (ev.type)
				{
				case Expose:
					XPutImage(display, window, DefaultGC(display, 0), img, 0, 0,
							block_x, block_y, block_width, block_height);
					break;
				case ButtonPress:
					display_flag = false;
					XUnmapWindow(display, window);

				}
			}
		}
	}
	/* 释放内存 */
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

	CompressAndWrite("./screen");
	UncompressAndDisplay("./screen");
	printf(" Done.\n");

	return 1;
}

