/*
 * test.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: baixingqiang
 *
 * g++ -o main main.cpp -lX11 -lz
 */

#include "VideoRecordAndReplayFunctions.h"



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
/*				UncompressAndDisplay("./20160314170247-20160314170317.hust",
		 frame_rate);*/
	}
	printf("Done.\n"); //测试语句可删除

	return 1;
}
