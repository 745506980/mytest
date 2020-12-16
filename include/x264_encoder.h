/*************************************************************************
    > File Name: x264_encoder.h
    > 作者:YJK 
    > Mail: 745506980@qq.com 
    > Created Time: 2020年11月26日 星期四 16时31分02秒
 ************************************************************************/

#ifndef __X264_ENCODER_H
#define __X264_ENCODER_H
#include<stdint.h>
#include"x264.h"
#include<linux/videodev2.h>
#include<stdlib.h>
#include<string.h>
typedef struct encode{
	x264_param_t param; //相关配置信息
	x264_nal_t *nal;
	x264_picture_t picture;	
	x264_t *handle;
}Encode;  



/*SPS PPS*/
typedef struct sps_pps{
	uint8_t * sps;
	uint8_t * pps;
	uint32_t sps_len;
	uint32_t pps_len;
}sps_pps;
/*x264编码*/

/*编码一帧数据*/
int Encode_frame(Encode *en, uint32_t pixformat, sps_pps * sp, uint8_t *frame, uint32_t width, uint32_t height, uint32_t timer);

int Encode_init(Encode *en, sps_pps *sp, uint32_t pixformat, uint32_t width, uint32_t height, uint32_t fps, uint32_t bitrate, int ConstantBitRate);

void Encode_end(x264_t *handle, x264_picture_t *picture, uint8_t * sps, uint8_t *pps);
 
int sps_pps_packet(sps_pps * sp);



#endif
/*防止头文件重复定义*/
