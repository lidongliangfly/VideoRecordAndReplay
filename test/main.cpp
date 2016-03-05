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

int ReadDataFromImage(unsigned char *data, XImage* img)
{

	unsigned long red_mask = img->red_mask; //这里的掩码也是4bytes，因为在XImage中ZPixmap中一个像素点也是4bytes，这里的掩码是用来萃取中对应的颜色
	unsigned long green_mask = img->green_mask;
	unsigned long blue_mask = img->blue_mask;
	unsigned long pix;
	unsigned red, green, blue;

	int nbits_mask = img->depth / 3;
	int t = 0;

	for (int i = 0; i < img->height; i++)
	{
		for (int j = 0; j < img->width; j++)
		{
			pix = XGetPixel(img, j, i); //取出每一个像素点，从左到右，从上到下。
			red = (pix & red_mask) >> (nbits_mask + nbits_mask); // 这里的红色的掩码值是00ff0000，做与操作后，向右走16位，取出八位，也就是1byte的红色值
			*(data + t) = red;
			t++;
			green = (pix & green_mask) >> nbits_mask; // G绿色掩码值是0000ff00
			*(data + t) = green;
			t++;
			blue = pix & blue_mask; //蓝色是000000ff
			*(data + t) = blue;
			t++;
		}

	}

	return 1;
}

size_t WriteBlockToDisk(const void* buffer, size_t compressed_block_size,
		FILE* stream, unsigned char* block_flag, unsigned short block_x,
		unsigned short block_y, unsigned short block_width,
		unsigned short block_height, size_t uncompressed_block_size)
{ //count means the number of compressed data
	size_t nmemb = 0;
	nmemb += fwrite(&compressed_block_size, sizeof(size_t), 1, stream);
	nmemb += fputc((int) block_flag, stream);
	nmemb += fwrite(&block_x, sizeof(unsigned short), 1, stream);
	nmemb += fwrite(&block_y, sizeof(unsigned short), 1, stream);
	nmemb += fwrite(&block_width, sizeof(unsigned short), 1, stream);
	nmemb += fwrite(&block_height, sizeof(unsigned short), 1, stream);
	nmemb += fwrite(buffer, compressed_block_size, 1, stream);
	nmemb += fwrite(&uncompressed_block_size, sizeof(size_t), 1, stream);
	if (nmemb
			== 2 * sizeof(size_t) + 1 + 4 * sizeof(unsigned short)
					+ compressed_block_size)
		return nmemb;
	else
	{
		printf("something wrong in WriteBlockToDisk!!!");
		return 0;
	}
}
size_t ReadBlockFromDisk(void *buffer, size_t compressed_block_size,
		FILE* stream, unsigned char* block_flag, unsigned short* block_x,
		unsigned short* block_y, unsigned short* block_width,
		unsigned short* block_height)
{

	size_t uncompressed_block_size;

	if (fread(&compressed_block_size, sizeof(size_t), 1, stream) == 0)
	{
		printf("read error or reach the file's end ！ ");
		return 0;
	}
	*(block_flag) = fgetc(stream);
	fread(block_x, sizeof(unsigned short), 1, stream);
	fread(block_y, sizeof(unsigned short), 1, stream);
	fread(block_width, sizeof(unsigned short), 1, stream);
	fread(block_height, sizeof(unsigned short), 1, stream);
	fread(buffer, compressed_block_size, 1, stream);
	fread(&uncompressed_block_size, sizeof(size_t), 1, stream);
	return uncompressed_block_size;

}

int DesktopCapture_Compress(const char* filename)
{
	Window desktop;
	Display* display;
	XImage* img;
	unsigned short screen_width;
	unsigned short screen_height;

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
	//Retrive the width and the height of the screen
	screen_width = DisplayWidth(display, 0);
	screen_height = DisplayHeight(display, 0);
	//get the image of the root window
	int n = 100;
	gzFile compressfile = gzopen(filename, "wb"); //ax
	FILE *fp = fopen("screentxt", "wb");
	if (compressfile == NULL)
	{
		printf("could not open gz file!!");
		return 0;
	}
	unsigned long tlen = screen_width * screen_height * 3;
	unsigned long blen;
	unsigned char* data = (unsigned char*) malloc(sizeof(unsigned char) * tlen); //图片的长乘以宽乘以3就是图片数据的总大小rgb data
	unsigned char* buf = NULL;
	int imagedepth = DefaultDepth(display, DefaultScreen(display));
	Window window = XCreateSimpleWindow(display, desktop, 0, 0, screen_width,
			screen_height, 1, 0, 0);
	for (int i = 0; i < n; i++)
	{
		/*
		 img = XGetImage(display, desktop, 0, 0, screen_width, screen_height, ~0,
		 ZPixmap);
		 ReadDataFromImage(data, img); //read rgb data from XImage

		 { //压缩部分 计算缓冲区大小，并为其分配内存
		 blen = compressBound(tlen);  //压缩后的长度是不会超过blen的
		 if ((buf = (unsigned char*) malloc(sizeof(unsigned char) * blen))
		 == NULL)
		 {
		 printf("no enough memory!\n");
		 return -1;
		 }

		 //压缩
		 if (compress(buf, &blen, data, tlen) != Z_OK)
		 {
		 printf("compress failed!\n");
		 return -1;
		 }
		 printf("原始文件： %dKB\n", (int) (tlen / 1024));
		 printf("压缩后小于：%dKB\n", (int) (blen / 1024));
		 gzwrite(compressfile, buf, blen);

		 fwrite(buf, 1, blen, fp);

		 }*/
		/* 解压缩 */

		/*		 if (gzread(compressfile, data, tlen) != Z_OK) {
		 printf("uncompress failed!\n");
		 return -1;
		 }*/

		XCreateImage(display, DefaultVisual(display, 0), imagedepth, ZPixmap, 0,
		NULL, screen_width, screen_height, 32, 0);
		/*		if (uncompress(compare_data, &tlen, buf, blen) != Z_OK)
		 {
		 printf("uncompress failed!\n");
		 return -1;
		 }*/

		{ //用于显示图片

			//Visual *visual = DefaultVisual(display, 0);
			/*		if (visual->c_class != TrueColor)
			 {
			 fprintf(stderr, "Cannot handle non true color visual ...\n");
			 return 0;
			 }*/
			XSelectInput(display, window, ButtonPressMask | ExposureMask);
			XMapWindow(display, window);
			XEvent ev;
			bool flag = true;
			while (flag)
			{
				/*				        const char *tir="This is red";
				 const char *tig="This is green";
				 const char *tib="This is blue";*/
				XNextEvent(display, &ev);
				switch (ev.type)
				{
				case Expose:
					XPutImage(display, window, DefaultGC(display, 0), img, 0, 0,
							0, 0, screen_width, screen_height);
					XSetForeground(display, DefaultGC(display, 0), 0x00ff0000); // red
					/*     XDrawString(display, window, DefaultGC(display, 0), 32,     32,     tir, strlen(tir));
					 XDrawString(display, window, DefaultGC(display, 0), 32+256, 32,     tir, strlen(tir));
					 XDrawString(display, window, DefaultGC(display, 0), 32+256, 32+256, tir, strlen(tir));
					 XDrawString(display, window, DefaultGC(display, 0), 32,     32+256, tir, strlen(tir));*/
					XSetForeground(display, DefaultGC(display, 0), 0x0000ff00); // green
					/*  XDrawString(display, window, DefaultGC(display, 0), 32,     52,     tig, strlen(tig));
					 XDrawString(display, window, DefaultGC(display, 0), 32+256, 52,     tig, strlen(tig));
					 XDrawString(display, window, DefaultGC(display, 0), 32+256, 52+256, tig, strlen(tig));
					 XDrawString(display, window, DefaultGC(display, 0), 32,     52+256, tig, strlen(tig));*/
					XSetForeground(display, DefaultGC(display, 0), 0x000000ff); // blue
					/*    XDrawString(display, window, DefaultGC(display, 0), 32,     72,     tib, strlen(tib));
					 XDrawString(display, window, DefaultGC(display, 0), 32+256, 72,     tib, strlen(tib));
					 XDrawString(display, window, DefaultGC(display, 0), 32+256, 72+256, tib, strlen(tib));
					 XDrawString(display, window, DefaultGC(display, 0), 32,     72+256, tib, strlen(tib));*/
					break;

				case ButtonPress:
					flag = false;
					XUnmapWindow(display, window);

				}
			}
		}
	}
	/* 释放内存 */
	if (buf != NULL || data != NULL)
	{
		free(buf);
		free(data);
		buf = NULL;
		data = NULL;
	}
	fclose(fp);
	gzclose(compressfile);
	XDestroyWindow(display, window);
	XDestroyImage(img);
	XCloseDisplay(display);
	return 1;

}

int main()
{

	DesktopCapture_Compress("./screen");

	printf(" Done.\n");
	return 1;
}

