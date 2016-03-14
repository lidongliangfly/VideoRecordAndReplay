#include "VideoRecordAndReplayFunctions.h"

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
		printf("could not open file: %s !!\n", filename);
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

	//start reading data from disk
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
			/*	XEvent ev;
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
