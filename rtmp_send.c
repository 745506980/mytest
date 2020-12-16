/*************************************************************************
    > File Name: rtmp_send.c
    > 作者:YJK 
    > Mail: 745506980@qq.com 
    > Created Time: 2020年12月03日 星期四 15时02分31秒
 ************************************************************************/

#include<stdio.h>


#include "include/librtmp/amf.h"
#include"include/rtmp_send.h"
#include "include/librtmp/rtmp.h"

#define RTMP_HEADER_SIZE (sizeof(RTMPPacket) + RTMP_MAX_HEADER_SIZE)


int	
Rtmp_Begin(char * URL)
{
	int ret = 0;
	//申请内存-RTMP
	rtmp = RTMP_Alloc(); 
	if (rtmp == NULL){
		perror("RTMP_Alloc");
		return -1;
	}
	/*初始化*/
	RTMP_Init(rtmp);
	
	/*设置地址*/
	ret = RTMP_SetupURL(rtmp, URL);
	if (ret == FALSE){
		perror("RTMP_SetupURL");
		return -1;
	}
	/*开启输出模式*/
	RTMP_EnableWrite(rtmp);
	/*连接服务器*/
	ret = RTMP_Connect(rtmp, NULL);
	if (ret == FALSE){
		perror("RTMP_Connect");
		return -1;
	}
	/*连接流*/
	ret = RTMP_ConnectStream(rtmp, 0);
	if (ret == FALSE){
		perror("RTMP_ConnectStream");
		return -1;
	}
	
	return 0;
}



/*SPS PPS*/
RTMPPacket * Create_sps_packet(sps_pps *sp)
{
	/*分配packet空间*/
	RTMPPacket * packet = (RTMPPacket *)malloc(RTMP_HEADER_SIZE + 512);
	if (packet == NULL) return NULL;

	packet->m_body = (char *)packet + RTMP_HEADER_SIZE;
	
	/*填充视频包数据*/
	int i = 0;

	packet->m_body[i++] = 0x17;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;

	packet->m_body[i++] = 0x01; //版本号
		
	packet->m_body[i++] = sp->sps[1]; //配置信息 baseline 宽高
	packet->m_body[i++] = sp->sps[2]; //兼容性 
	packet->m_body[i++] = sp->sps[3]; //profile_level
	packet->m_body[i++] = 0xFF;   //几个字节表示NALU的长度 0xFF & 3 + 1  4个字节

	packet->m_body[i++] = 0xE1;  //SPS个数 0xE1 & 0x1F = 1
	/*sps 长度 2个字节*/  //网络中采用大端模式
	packet->m_body[i++] = (sp->sps_len >> 8) & 0xFF; //低8位 
	packet->m_body[i++] = sp->sps_len & 0xFF;		//高8位
	//sps数据
	memcpy(&packet->m_body[i], sp->sps, sp->sps_len);
	i += sp->sps_len;
	//pps个数
	packet->m_body[i++] = 0x01;
	//整个pps长度
	packet->m_body[i++] = (sp->pps_len >> 8) & 0xFF;
	packet->m_body[i++] = sp->pps_len & 0xFF;
	//pps数据
	memcpy(&packet->m_body[i], sp->pps, sp->pps_len);
	i += sp->pps_len;
	
	//packet配置
	/*massage type id (1~7)控制协议8，9音视频，10以后ＡＭＦ编码消息*/
	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO; //视频格式 
	
	//数据长度
	packet->m_nBodySize = i; 
	
	//块流ID
	packet->m_nChannel = 0x04; //Audio 和 video的通道   
	packet->m_nTimeStamp = 0;	//绝对时间戳
	packet->m_hasAbsTimestamp = 0; //相对时间戳
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE; //ChunkMsgHeader的类型(4种)
	packet->m_nInfoField2 = rtmp->m_stream_id;  //消息流ID			
	return packet;
}


int Send_h264_packet(uint8_t * H264_Stream, sps_pps * sp, int length, int type, uint32_t timer)
{
	RTMPPacket *packet = (RTMPPacket *)malloc(RTMP_HEADER_SIZE + length + 9);
	if (packet == NULL){
		fprintf(stderr, "packet malloc is error!\n");
		return -1;
	}
	int i = 0;
	packet->m_body = (char *)packet + RTMP_HEADER_SIZE;
	int ret = 0;	
	if (type == 1) //关键帧
	{
		packet->m_body[i++] = 0x17;		
		packet->m_body[i++] = 0x01;
		packet->m_body[i++] = 0x00;
		packet->m_body[i++] = 0x00;
		packet->m_body[i++] = 0x00;	
		//NALUSIZE 数据长度 
		packet->m_body[i++] = (length >> 24) & 0xFF;
		packet->m_body[i++] = (length >> 16) & 0xFF;
		packet->m_body[i++] = (length >> 8) & 0xFF;		
		packet->m_body[i++] = length & 0xFF;
				
		memcpy(&packet->m_body[i], H264_Stream, length);
		i += length;
		
		packet->m_packetType = RTMP_PACKET_TYPE_VIDEO; 
		//数据长度 
		packet->m_nBodySize = i;		
		packet->m_hasAbsTimestamp = 0;
		packet->m_nTimeStamp = timer;
		packet->m_nChannel = 0x04;
		packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
		packet->m_nInfoField2 = rtmp->m_stream_id;
		//SPS PPS 包
//		packet_sp = Create_sps_packet(sp, rtmp);
		ret = RTMP_SendPacket(rtmp, packet_sp, FALSE); //TRUE 放进发送队列
		if (RTMP_IsConnected(rtmp))
		{	
			ret = RTMP_SendPacket(rtmp, packet, FALSE);
		}
//		printf("------------IDR-----------\n");
//		free(packet_sp);
//		packet_sp = NULL;
	}
	if (type == 0)
	{
		packet->m_body[i++] = 0x27;
		packet->m_body[i++] = 0x01;
		packet->m_body[i++] = 0x00;
		packet->m_body[i++] = 0x00;
		packet->m_body[i++] = 0x00;
		
		packet->m_body[i++] = (length >> 24) & 0xFF; 
		packet->m_body[i++] = (length >> 16) & 0xFF;
		packet->m_body[i++] = (length >> 8) & 0xFF;
		packet->m_body[i++] = length & 0xFF;
		
		memcpy(&packet->m_body[i], H264_Stream, length);
		i += length;

		packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;

		packet->m_nBodySize = i;
		packet->m_nTimeStamp = timer;
		packet->m_hasAbsTimestamp = 0;
		packet->m_nChannel = 0x04;
		packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
		packet->m_nInfoField2 = rtmp->m_stream_id;			
		ret = RTMP_SendPacket(rtmp, packet, FALSE);	
	
	}
	free(packet);
	return ret;
}
void RTMP_END()                
{               
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
	free(packet_sp);
}


