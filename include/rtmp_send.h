/*************************************************************************
    > File Name: rtmp_send.h
    > 作者:YJK 
    > Mail: 745506980@qq.com 
    > Created Time: 2020年12月03日 星期四 13时02分00秒
 ************************************************************************/

#ifndef __RTMP_SEND_H 
#define __RTMP_SEND_H

#include"librtmp/rtmp.h"
#include"librtmp/amf.h"
#include"x264_encoder.h"

extern RTMPPacket * packet_sp;
extern RTMP * rtmp;

typedef struct profile{
	int fps;
	int bitrate;
	uint32_t pixformat;
}profile;



int Rtmp_Begin(char * URL);


/*对H264码流进行封包
 * type = 1 为IDR
 * type = 0 非IDR
 * */
int Send_h264_packet(uint8_t * H264_Stream, sps_pps *sp, int length, int type, uint32_t timer);



/*获取对sps和pps进行封包*/
RTMPPacket * Create_sps_packet(sps_pps *sp);


void RTMP_END();

 
#endif
/*防止头文件重复定义*/
