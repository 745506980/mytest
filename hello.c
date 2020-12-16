/*************************************************************************
    > File Name: hello.c
    > 作者:YJK 
    > Mail: 745506980@qq.com 
    > Created Time: 2020年11月27日 星期五 14时01分17秒
 ************************************************************************/

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <stdio.h>  
#include <sys/ioctl.h>  
#include <stdlib.h>  
#include <linux/types.h>  
#include <linux/videodev2.h>  
#include <malloc.h>  
#include <math.h>  
#include <string.h>  
#include <sys/mman.h>  
#include <errno.h>  
#include <assert.h>  
  

#include"./include/x264.h"
char* audioConfig = NULL;
long audioConfigLen = 0;
unsigned int nSendCount = 0;
unsigned char *pps = 0;
unsigned char *sps = 0;
int i_nal = 0;
int pps_len;
int sps_len;

x264_t * h = NULL;
x264_nal_t* nal_t = NULL;
x264_picture_t m_picInput;		
x264_picture_t m_picOutput;		
x264_param_t param;				
//RtmpH264* pRtmpH264 = NULL;

int main(int argc,char *argv[])
{
	int m_width = 640;//宽，根据实际情况改
	int m_height = 480;//高
	int m_frameRate = 30;
	int m_bitRate = 400;
	//m_srcPicFmt=src_pix_fmt;
	int m_baseFrameSize=m_width*m_height;

	x264_param_default(&param);//设置默认参数具体见common/common.c


	//* 使用默认参数，在这里因为是实时网络传输，所以使用了zerolatency的选项，使用这个选项之后就不会有delayed_frames，如果使用的不是这样的话，还需要在编码完成之后得到缓存的编码帧
	x264_param_default_preset(&param, "veryfast", "zerolatency");

	//* cpuFlags
	param.i_threads = X264_SYNC_LOOKAHEAD_AUTO; /* 取空缓冲区继续使用不死锁的保证 */
	param.i_sync_lookahead = X264_SYNC_LOOKAHEAD_AUTO;  /*自动选择最优超前线程缓冲区大小*/

	//* 视频选项
//	param.i_width = m_width;
//
//	param.i_height = m_height;
	param.i_width = 640;
	param.i_height = 480;
	param.i_frame_total = 0; 			//* 编码总帧数.不知道用0.
	param.i_keyint_min = 0;				//关键帧最小间隔   
	param.i_keyint_max = 60;			//关键帧最大间隔
	param.b_annexb = 1;					//1前面为0x00000001,0为nal长度
	param.b_repeat_headers = 1;			//关键帧前面是否放sps跟pps帧，0 否 1，放  实时视频需要让每个I帧前面都有SPS和PPS帧
	//param.i_csp = X264_CSP_I420;		//YUV420
	param.i_csp = X264_CSP_I422;		//YUYV



	//* B帧参数
	param.i_bframe = 0;					//B帧  实时监控 B帧数量为0
	param.b_open_gop = 0;				//是否开启open_gop功能 0 关闭
	param.i_bframe_pyramid = 0;			//不允许B帧作为参考帧
	param.i_bframe_adaptive = X264_B_ADAPT_FAST;

	//* 速率控制参数
	param.rc.i_lookahead = 0;
	param.rc.i_bitrate = 400; //* 码率(比特率,单位Kbps)

	if(1){
		param.rc.i_rc_method = X264_RC_ABR;			   //参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
		param.rc.i_vbv_max_bitrate = (int)m_bitRate*1; // 平均码率模式下，最大瞬时码率，默认0(与-B设置相同)
	}
	else{
		param.rc.b_filler = 1;
		param.rc.i_rc_method = X264_RC_ABR;		 //参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
		param.rc.i_vbv_max_bitrate = m_bitRate;  // 平均码率模式下，最大瞬时码率，默认0(与-B设置相同)
		param.rc.i_vbv_buffer_size  = m_bitRate; //vbv-bufsize
	}

	//* muxing parameters					//I帧间隔
	param.i_fps_den = 1; 			// 帧率分母
	param.i_fps_num = 30;			// 帧率分子			    1秒一个I帧
	param.i_timebase_num = 1;
	param.i_timebase_den = 1000;

	h = x264_encoder_open(&param);	//根据参数初始化X264级别
	x264_picture_init(&m_picOutput);//初始化图片信息
	
/*	switch(pixelformat){
		case V4L2_PIX_FMT_YUYV:{
			x264_picture_alloc(&m_picInput, X264_CSP_I422, m_width, m_height);//图片按I422格式分配空间，最后要x264_picture_clean
			break;
		}
		
		case V4L2_PIX_FMT_YUV420:{
			x264_picture_alloc(&m_picInput, X264_CSP_I420, m_width, m_height);//图片按I420格式分配空间，最后要x264_picture_clean
			break;
		}
		
		default :{
			x264_picture_alloc(&m_picInput, X264_CSP_I420, m_width, m_height);
			break;
		}
	}
	*/
	x264_picture_alloc(&m_picInput, X264_CSP_I422, m_width, m_height);
	m_picInput.i_pts = 0;

	i_nal = 0;
	x264_encoder_headers(h, &nal_t, &i_nal);  //返回NAL单元 用于输出SPS/PPS/SEI这些H264码流的头信息
	//i_nal 是nal_t的单元数
	//返回NAL单元 分别是SPS PPS SEI 0 1 2  每一个p_payload中都是00 00 00 01 然后是相应的数据 所以获取SPS信息需要将前面四个字节的00 00 00 01去掉便是SPS数据信息 i_payload 是p_payload的有效长度 包含标志00 00 00 01
	printf("%d\n",i_nal);
	if (i_nal > 0){
		for (int i = 0; i < i_nal; i++){
			//获取SPS数据，PPS数据
			if (nal_t[i].i_type == NAL_SPS){
				sps = malloc(sizeof(char)*(nal_t[i].i_payload - 4));
				sps_len = nal_t[i].i_payload - 4;
				memcpy(sps, nal_t[i].p_payload + 2, nal_t[i].i_payload - 4);
				printf("i_nal:%d sps_len:%d p_payload:0x%x \n",i , sps_len, sps[0]);
			}
			else if (nal_t[i].i_type == NAL_PPS){
				pps = malloc(sizeof(char) * (nal_t[i].i_payload - 4));
				pps_len = nal_t[i].i_payload - 4;
				memcpy(pps, nal_t[i].p_payload + 4, nal_t[i].i_payload - 4);
				printf("%d\n",i);
			}
			if (nal_t[i].i_type == NAL_SEI)
			{
				printf("--%d\n",i);
			}
		}
	
	}

	return 0;
}
