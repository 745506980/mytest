/*************************************************************************
    > File Name: main.c
    > 作者:YJK 
    > Mail: 745506980@qq.com 
    > Created Time: 2020年11月21日 星期六 18时46分19秒
 ************************************************************************/

#include <linux/videodev2.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include "include/camer.h"
#include "include/rtmp_send.h"
#include "include/x264_encoder.h"
#include <pthread.h>

#define DEVICE_NAME "/dev/video0"

#define FILE_NAME "./out.yuv"

RTMP *rtmp = NULL;
RTMPPacket *packet_sp = NULL;


//帧缓冲区
uint8_t * frame_buf = NULL;

uint8_t *encode_frame_buf = NULL;

Encode en ;
sps_pps sp;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *
thread_fun(void * arg)
{

	profile * pro = (profile *)arg;
	uint32_t timer_ = 1000/pro->fps;
	uint32_t timer = 0;
	int ret = 0;
	/*对帧数据进行编码*/

    int i = 90;
#if 0
	struct timeval start,end;
    gettimeofday(&start, NULL);
#endif
	while (i)	
	{

		//lock
		pthread_mutex_lock(&mutex);
		ret = Encode_frame(&en, pro->pixformat, &sp, frame_buf, WIDTH, HEIGHT, timer);
		if (ret == -1){
			fprintf(stderr, "Encode_frame IS ERROR!\n");
			//unlock 
			pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);

		}
		//unlock
		pthread_mutex_unlock(&mutex);
	
		timer += timer_;		
		i--;
	}
#if 0
	gettimeofday(&end, NULL);
    printf("time:%ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);

#endif 
	

	return (void *)0;
}


//#define URL "rtmp://192.168.12.144:1935/live/yuan"
#define URL "rtmp://127.0.0.1/live/yuan"
int main(int argc,char *argv[])
{
	/*	1、图像格式　　如YUYV  YUV420 
	 *	2、宽高　　640x480
	 *	3、帧率	
	 *	4、
	 *	
	 *
	 * */		
	profile arg;
	arg.fps = 30;
	arg.bitrate = 1280;
	arg.pixformat = V4L2_PIX_FMT_YUV420;
	
	pthread_t tid;

	//	uint32_t pixformat = V4L2_PIX_FMT_YUYV;
	
	int ret = 0;
	//配置 摄像头
	ret = open_device(DEVICE_NAME);
    if (ret == -1) 
        exit(EXIT_FAILURE);
    open_file(FILE_NAME);
        
    init_device(arg.pixformat);
	
	/*分配帧缓冲区*/

	frame_buf = (uint8_t *)malloc(frame_size);
	if (frame_buf == NULL){
		fprintf(stderr,"frame_buf mallo is error !\n");
		exit(EXIT_FAILURE);
	}
    init_mmap();

	//初始化和连接RMTP 
	ret = Rtmp_Begin(URL);
	
	if (ret == -1){
		fprintf(stderr,"RTMP_Begin is error!\n");
		exit(EXIT_FAILURE);
	}	
	//编码器初始化
	ret = Encode_init(&en, &sp, arg.pixformat, WIDTH, HEIGHT, arg.fps, arg.bitrate, 0);
	if (ret < 0){
		fprintf(stderr,"Encode_init is error\n");
		exit(EXIT_FAILURE);
	}
	
	//获取sps pps 封包
	packet_sp = Create_sps_packet(&sp);
	
	//设置分离属性
	pthread_attr_t attr;
	ret = pthread_attr_init(&attr);
	if (ret == -1){
		fprintf(stderr, "pthread_attr_init!\n");
		exit(EXIT_FAILURE);
	}
	
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (ret == -1){
		fprintf(stderr, "pthread_attr_setdetachstate!\n");
		exit(EXIT_FAILURE);
	}
	//创建分离属性的编码线程　
	ret = pthread_create(&tid, NULL, thread_fun, (void *)&arg);
	if (ret == -1){
		fprintf(stderr, "pthread_create is error!");
		exit(EXIT_FAILURE);			
	}		
	pthread_attr_destroy(&attr);
	
	start_stream();

	int i = 90;
#if 0
	struct timeval start,end;
	gettimeofday(&start, NULL);
	
#endif		
	while (i)
	{

		//	lock
		pthread_mutex_lock(&mutex);
	
		ret = read_frame(frame_buf); //获取一帧数据		
		if (ret == -1){
			fprintf(stderr,"read_frame is error!\n");
			pthread_mutex_unlock(&mutex);
			exit(EXIT_FAILURE);
		}
		//	unlock
		pthread_mutex_unlock(&mutex);
//		i--;
	}	
	
#if 0
	gettimeofday(&end, NULL);
	printf("time:%ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);
	pthread_join(tid, NULL);	
#endif 
	
	end_stream();
	
	close_mmap();
	
	close_device();
	
	Encode_end(en.handle, &en.picture, sp.sps, sp.pps);
	
	RTMP_END(rtmp);
	
	
	return 0;
}

