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

int DataCompressFromImage(const char *filename, XImage* img, int screenWidth,
		int screenHeight) {
/*	FILE* fp;
	fp = fopen(filename, "wb");

	if (fp == NULL) {
		printf("cannot write file !!");
		return 0;
	}*/
	unsigned long red_mask = img->red_mask; //这里的掩码也是4bytes，因为在XImage中ZPixmap中一个像素点也是4bytes，这里的掩码是用来萃取中对应的颜色
	unsigned long green_mask = img->green_mask;
	unsigned long blue_mask = img->blue_mask;
	unsigned long pix;
	unsigned red, green, blue;

	int nbits_mask = img->depth / 3;
	int t = 0;
	unsigned char* data = (unsigned char*) malloc(
			sizeof(unsigned char) * screenWidth * screenHeight * 3); //图片的长乘以宽乘以3就是图片数据的总大小
	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
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
			//printf("%d %d %d\t",red,blue,green);
			//fprintf(fp, "%d %d %d\t", red, blue, green);
		}
		//fprintf(fp, "\n");
	}
/*	fwrite(data, sizeof(unsigned char), screenWidth * screenHeight * 3, fp);
	fclose(fp);*/

	/*压缩部分*/
	unsigned long tlen = screenWidth * screenHeight * 3;
	unsigned char* buf = NULL;
	unsigned long blen;

	/* 计算缓冲区大小，并为其分配内存 */
	blen = compressBound(tlen); /* 压缩后的长度是不会超过blen的 */
	if ((buf = (unsigned char*) malloc(sizeof(unsigned char) * blen)) == NULL) {
		printf("no enough memory!\n");
		return -1;
	}

	/* 压缩 */
	if (compress(buf, &blen, data, tlen) != Z_OK) {
		printf("compress failed!\n");
		return -1;
	}
	printf("原始文件： %dKB\n", (int) (tlen / 1024));
	printf("压缩后小于：%dKB\n", (int) (blen / 1024));
	/*fwrite */
/*	FILE* compress_fp = fopen("compress_fp", "wb");
	fwrite(buf, blen, 1, compress_fp);
	fclose(compress_fp);*/
	/*gzwrite */
	gzFile compress = gzopen("compress", "wb");
	gzwrite(compress, buf, blen);
	gzclose(compress);

	/* 解压缩 */
	unsigned char* compare_data = (unsigned char*) malloc(tlen);/*错误对比*/
	if (uncompress(compare_data, &tlen, buf, blen) != Z_OK) {
		printf("uncompress failed!\n");
		return -1;
	}
/*	错误对比
	for (unsigned int k = 0; k < tlen; k++)
		if (*(compare_data + k) != *(data + k))
			printf("error!\n");
	fwrite
	FILE* uncompress_fp = fopen("uncompress_fp", "wb");
	fwrite(data, tlen, 1, uncompress_fp);
	fclose(uncompress_fp);
	gzwrite
	gzFile uncompress = gzopen("uncompress", "wb");
	gzwrite(uncompress, data, tlen);
	gzclose(uncompress);*/

	/* 释放内存 */
	if (buf != NULL || data != NULL) {
		free(buf);
		free(data);
		buf = NULL;
		data = NULL;
	}

	return 1;
}

/*显示*/
void processEvent(Display *display, Window window, XImage *ximage, int width,
		int height) {
	/*        const char *tir="This is red";
	 const char *tig="This is green";
	 const char *tib="This is blue";*/
	XEvent ev;
	XNextEvent(display, &ev);
	switch (ev.type) {
	case Expose:
		XPutImage(display, window, DefaultGC(display, 0), ximage, 0, 0, 0, 0,
				width, height);
		XSetForeground(display, DefaultGC(display, 0), 0x00ff0000); // red
				/*        XDrawString(display, window, DefaultGC(display, 0), 32,     32,     tir, strlen(tir));
				 XDrawString(display, window, DefaultGC(display, 0), 32+256, 32,     tir, strlen(tir));
				 XDrawString(display, window, DefaultGC(display, 0), 32+256, 32+256, tir, strlen(tir));
				 XDrawString(display, window, DefaultGC(display, 0), 32,     32+256, tir, strlen(tir));*/
		XSetForeground(display, DefaultGC(display, 0), 0x0000ff00); // green
				/*        XDrawString(display, window, DefaultGC(display, 0), 32,     52,     tig, strlen(tig));
				 XDrawString(display, window, DefaultGC(display, 0), 32+256, 52,     tig, strlen(tig));
				 XDrawString(display, window, DefaultGC(display, 0), 32+256, 52+256, tig, strlen(tig));
				 XDrawString(display, window, DefaultGC(display, 0), 32,     52+256, tig, strlen(tig));*/
		XSetForeground(display, DefaultGC(display, 0), 0x000000ff); // blue
				/*        XDrawString(display, window, DefaultGC(display, 0), 32,     72,     tib, strlen(tib));
				 XDrawString(display, window, DefaultGC(display, 0), 32+256, 72,     tib, strlen(tib));
				 XDrawString(display, window, DefaultGC(display, 0), 32+256, 72+256, tib, strlen(tib));
				 XDrawString(display, window, DefaultGC(display, 0), 32,     72+256, tib, strlen(tib));*/
		break;

	case ButtonPress:
		exit(0);
	}
}

int CaptureDesktop(const char* filename) {
	Window desktop;
	Display* display;
	XImage* img;

	int screen_width;
	int screen_height;

	display = XOpenDisplay(NULL); //connect to a local display
	if (NULL == display) {
		printf("CaptureDesktop cannot get root window");
		return 0;
	}
	desktop = RootWindow(display, 0); //refer to root window
	if (desktop == 0) {
		printf("CaptureDesktop cannot get root window");
		return 0;
	}
	//Retrive the width and the height of the screen
	screen_width = DisplayWidth(display, 0);
	screen_height = DisplayHeight(display, 0);
	//get the image of the root window
	img = XGetImage(display, desktop, 0, 0, screen_width, screen_height, ~0,
	ZPixmap);

	DataCompressFromImage(filename, img, screen_width, screen_height);

	{/*用于显示图片*/
		Visual *visual = DefaultVisual(display, 0);
		Window window = XCreateSimpleWindow(display, RootWindow(display, 0), 0,
				0, screen_width, screen_height, 1, 0, 0);
		if (visual->c_class != TrueColor) {
			fprintf(stderr, "Cannot handle non true color visual ...\n");
			exit(1);
		}

		XSelectInput(display, window, ButtonPressMask | ExposureMask);
		XMapWindow(display, window);
		while (1) {
			processEvent(display, window, img, screen_width, screen_height);
		}
	}

	XDestroyImage(img);
	XCloseDisplay(display);
	return 1;

}

int main() {
	CaptureDesktop("./screen");
	printf("Done.\n");
	return 1;
}

