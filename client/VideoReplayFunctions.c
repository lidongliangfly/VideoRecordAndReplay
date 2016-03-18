#include "VideoRecordAndReplayFunctions.h"

int ReadBlockFromDisk(void *buffer, size_t compressed_block_size, FILE* stream,
		unsigned int * block_timestamp, unsigned int* block_x,
		unsigned int* block_y, unsigned int* block_width,
		unsigned int* block_height) {

	if ((fread(block_timestamp, sizeof(unsigned int), 1, stream)) != 1) {
		printf("read error or reach the file's end ！\n");
		return 0;
	}
	fread(block_x, sizeof(unsigned int), 1, stream);
	fread(block_y, sizeof(unsigned int), 1, stream);
	fread(block_width, sizeof(unsigned int), 1, stream);
	fread(block_height, sizeof(unsigned int), 1, stream);
	fread(buffer, compressed_block_size, 1, stream);
	return 1;

}

int gzReadBlockFromDisk(void *buffer, unsigned long compressed_block_size,
		gzFile stream, unsigned int * block_timestamp, unsigned int* block_x,
		unsigned int* block_y, unsigned int* block_width,
		unsigned int* block_height) {

	if ((gzread(stream, block_timestamp, sizeof(unsigned int))) != 4) {
		printf("read error or reach the file's end ！\n");
		return 0;
	}
	gzread(stream, block_x, sizeof(unsigned int));
	gzread(stream, block_y, sizeof(unsigned int));
	gzread(stream, block_width, sizeof(unsigned int));
	gzread(stream, block_height, sizeof(unsigned int));
	gzread(stream, buffer, compressed_block_size);
	return 1;

}

int PutDatablockToXImage(XImage* baseimg, char* block_data,
		unsigned int block_x, unsigned int block_y, unsigned int block_width,
		unsigned int block_height) {
	unsigned int width = baseimg->width;
	//unsigned int height = baseimg->height;
	unsigned int rightCornor_x = block_x + block_width;
	unsigned int rightCornor_y = block_y + block_height;
	unsigned int t = 0; //指针数据变量
	unsigned int i = 0; //高
	unsigned int j = 0; //宽
	for (i = block_y; i < rightCornor_y; i++) {
		for (j = block_x; j < rightCornor_x; j++) {
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
	if (t != block_width * block_height * 4) {
		printf("GetDatablockFromXImage inside write error !\n");
		return -1;
	}
	return 1;
}

int UncompressAndDisplay(const char* filename, int frame_rate) {

	unsigned long compressed_block_size;
	unsigned long uncompressed_block_size;
	unsigned int block_timestamp;
	unsigned int pre_block_timestamp = -1;
	unsigned int block_x;
	unsigned int block_y;
	unsigned int block_width;
	unsigned int block_height;
	unsigned int record_replay_time_diff;
	unsigned char* buf = NULL;
	char* img_data = NULL;
	XImage* img;

	struct timeval start_replay_tv;
	struct timeval current_replay_tv;
	//FILE *fp = fopen(filename, "rb");
	gzFile gfp = gzopen(filename, "rb"); //用于测试文件格式
	if (gfp == 0) {
		printf("could not open file: %s !!\n", filename);
		return 0;
	}
	Display* display = XOpenDisplay(NULL); //connect to a local display
	if (NULL == display) {
		printf("CaptureDesktop cannot get root window\n");
		return 0;
	}
	Window desktop = RootWindow(display, 0); //refer to root window
	if (desktop == 0) {
		printf("CaptureDesktop cannot get root window\n");
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
	gettimeofday(&start_replay_tv, NULL);
	while (1) {
		/*		if (fread(&compressed_block_size, sizeof(size_t), 1, fp) == 0)
		 {
		 printf("reach the end  or read error !\n");
		 return -1;
		 }*/
		if (gzread(gfp, &compressed_block_size, sizeof(unsigned long)) == 0) {
			printf("reach the end of file!\n(or read error).\n");
			return -1;
		} //用于测试文件格式
		if (compressed_block_size == 0)
			continue;
		if ((buf = (unsigned char*) malloc(compressed_block_size)) == NULL) {
			printf("no enough memory!\n");
			return -1;
		}
		//读数据
		/*		ReadBlockFromDisk(buf, compressed_block_size, fp, &block_timestamp,
		 &block_x, &block_y, &block_width, &block_height);*/
		gzReadBlockFromDisk(buf, compressed_block_size, gfp, &block_timestamp,
				&block_x, &block_y, &block_width, &block_height); //用于测试文件格式
		if ((img_data = (char*) malloc((block_width) * (block_height) * 4))
				== NULL) {
			printf("no enough memory!\n");
			return -1;
		}
		//解压缩
		uncompressed_block_size = block_width * block_height * 4;
		if (uncompress((unsigned char*) img_data, &uncompressed_block_size, buf,
				compressed_block_size) != Z_OK) {
			printf("uncompress failed!\n");
			return -1;
		}

		if (block_timestamp != pre_block_timestamp) { //用于显示图片 需修改
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

			if (pre_block_timestamp != -1) {
				gettimeofday(&current_replay_tv, NULL);
				record_replay_time_diff = block_timestamp
						- ((current_replay_tv.tv_sec - start_replay_tv.tv_sec)
								* 1000
								+ (int) ((current_replay_tv.tv_usec
										- start_replay_tv.tv_usec) / 1000));
				if (record_replay_time_diff > 0) {
					usleep(record_replay_time_diff * 1000);
				}
			}
			// usleep(1000000.0 / frame_rate);
		}

		PutDatablockToXImage(img, img_data, block_x, block_y, block_width,
				block_height);
		pre_block_timestamp = block_timestamp;
	}
	/* 释放内存 */
	XUnmapWindow(display, window);
	//fclose(fp);
	gzclose(gfp);
	XDestroyWindow(display, window);
	XDestroyImage(img);
	XCloseDisplay(display);
	if (buf != NULL || img_data != NULL) {
		free(buf);
		free(img_data);
		buf = NULL;
		img_data = NULL;
		//fp = NULL;
	}

	return 1;
}
