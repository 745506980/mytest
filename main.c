/*************************************************************************
    > File Name: main.c
    > 作者:YJK 
    > Mail: 745506980@qq.com 
    > Created Time: 2020年11月21日 星期六 18时46分19秒
 ************************************************************************/

#include <linux/videodev2.h>
#include<stdio.h>
#include <sys/select.h>
#include"include/camer.h"
#include"include/rtmp_send.h"
#include"include/x264_encoder.h"


#define DEVICE_NAME "/dev/video0"

#define FILE_NAME "./out.yuv"

RTMP *rtmp = NULL;
RTMPPacket *packet_sp = NULL;

#define URL "rtmp://192.168.12.144:1935/live/yuan"
//#define URL "rtmp://127.0.0.1/live/yuan"
int main(int argc,char *argv[])
{
	/*	1、图像格式　　如YUYV  YUV420 
	 *	2、宽高　　640x480
	 *	3、帧率	
	 *	4、
	 *	
	 *
	 * */
	
	
	int fps = 30;
	int bitrate = 1200;


//	uint32_t pixformat = V4L2_PIX_FMT_YUYV;
	uint32_t pixformat = V4L2_PIX_FMT_YUV420;
	int ret = 0;
	Encode en ;
	sps_pps sp;
	unsigned int timer_ = 1000/fps;  //ms
	//初始化和连接RMTP 
	ret = Rtmp_Begin(URL);
	
	if (ret == -1){
		fprintf(stderr,"RTMP_Begin is error!\n");
		exit(EXIT_FAILURE);
	}	
	//编码器初始化
	ret = Encode_init(&en, &sp, pixformat, WIDTH, HEIGHT, fps, bitrate, 0);
	if (ret < 0){
		fprintf(stderr,"Encode_init is error\n");
		exit(EXIT_FAILURE);
	}
	//获取sps pps 封包
	packet_sp = Create_sps_packet(&sp);
		
	ret = open_device(DEVICE_NAME);
	if (ret == -1) 
		exit(EXIT_FAILURE);
	open_file(FILE_NAME);
	init_device(pixformat);
	init_mmap();
	start_stream();
	uint32_t timer = 0;
	int i = 90;
	struct timeval start,end;
	gettimeofday(&start, NULL);
	flv_header();
	while(1)
	{
		timer = timer + timer_;
//		printf("timer:%d\n",timer);
		ret = process_frame(&en, &sp, pixformat, timer);	
		if (ret == -1) break;
//		printf("frame---------------\n");
//		usleep(500);
//		printf("frame:%d\n",i);
//		i--;
	}
	gettimeofday(&end, NULL);
	printf("time:%ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
	end_stream();
	close_mmap();
	close_device();
	Encode_end(en.handle, &en.picture, sp.sps, sp.pps);
	RTMP_END(rtmp);
	return 0;
}
