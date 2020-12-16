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
	uint32_t datasize = 16 + sp->sps_len + sp->sps_len;
	
	int i = 0;

#if 0
	/*flv封装*/
	packet->m_body[i++] = 0x09;   //video 9
	/*data size*/
	packet->m_body[i++] = (datasize >> 16) & 0xFF;
	packet->m_body[i++] = (datasize >> 8) & 0xFF;
	packet->m_body[i++] = datasize & 0xFF;
	/*timestamp*/
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	/*stream id*/	
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	
#endif	
	/*填充视频包数据*/
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

int flv_header()
{
    
	RTMPPacket *packet = malloc(RTMP_HEADER_SIZE + 214);
	packet->m_body = (char *)packet + RTMP_HEADER_SIZE; 
	/*flv header*/
    /*signature 3Byte*/
	int i = 0;
#if 0
    packet->m_body[i++] = 0x46;
    packet->m_body[i++] = 0x4c;                                                                                                                                              
    packet->m_body[i++] = 0x56;
    /*version*/ 
    packet->m_body[i++] = 0x01;
    /*flags*/
    packet->m_body[i++] = 0x01;  //0x01 video  0x04 audio   av 0x01 | 0x04  = 0x05
    /*dataoffset*/
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x09;

	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
    	
	

#endif 

	/*第一个amf*/
	packet->m_body[i++] = 0x02;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x0d;
	packet->m_body[i++] = 0x40;
	packet->m_body[i++] = 0x73;
	packet->m_body[i++] = 0x65;
	packet->m_body[i++] = 0x74;
	packet->m_body[i++] = 0x44;
	packet->m_body[i++] = 0x61;
	packet->m_body[i++] = 0x74;
	packet->m_body[i++] = 0x61;
	packet->m_body[i++] = 0x46;
	packet->m_body[i++] = 0x72;
	packet->m_body[i++] = 0x61;
	packet->m_body[i++] = 0x6d;
	packet->m_body[i++] = 0x65;
	/*第二个amf*/

	packet->m_body[i++] = 0x02;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x0a;
	packet->m_body[i++] = 0x6f;
	packet->m_body[i++] = 0x6e;
	packet->m_body[i++] = 0x4d;
	packet->m_body[i++] = 0x65;
	packet->m_body[i++] = 0x74;
	packet->m_body[i++] = 0x61;
	packet->m_body[i++] = 0x44;
	packet->m_body[i++] = 0x61;
	packet->m_body[i++] = 0x74;
	packet->m_body[i++] = 0x61;
	//ECMA array
	packet->m_body[i++] = 0x08;
	//长度
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x08;
	//duration
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x08;
	packet->m_body[i++] = 'd';
	packet->m_body[i++] = 'u';
	packet->m_body[i++] = 'r';
	packet->m_body[i++] = 'a';
	packet->m_body[i++] = 't';
	packet->m_body[i++] = 'i';
	packet->m_body[i++] = 'o';
	packet->m_body[i++] = 'n';
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	//width
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x05;
	packet->m_body[i++] = 'w';
	packet->m_body[i++] = 'i';
	packet->m_body[i++] = 'd';
	packet->m_body[i++] = 't';
	packet->m_body[i++] = 'h';
	packet->m_body[i++] = 0x00;

	packet->m_body[i++] = 0x40;
	packet->m_body[i++] = 0x84;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;

	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x06;
	packet->m_body[i++] = 'h';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 'i';
	packet->m_body[i++] = 'g';
	packet->m_body[i++] = 'h';
	packet->m_body[i++] = 't';
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x40;
	packet->m_body[i++] = 0x76;
	packet->m_body[i++] = 0x80;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	

	//videodatarate
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x0d;
	packet->m_body[i++] = 'v';
	packet->m_body[i++] = 'i';
	packet->m_body[i++] = 'd';
	packet->m_body[i++] = 'e';	
	packet->m_body[i++] = 'o';
	packet->m_body[i++] = 'd';
	packet->m_body[i++] = 'a';
	packet->m_body[i++] = 't';
	packet->m_body[i++] = 'a';
	packet->m_body[i++] = 'r';
	packet->m_body[i++] = 'a';
	packet->m_body[i++] = 't';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	//framerate
	packet->m_body[i++] = 0x00;	
	packet->m_body[i++] = 0x09;
	packet->m_body[i++] = 'f';
	packet->m_body[i++] = 'r';
	packet->m_body[i++] = 'a';
	packet->m_body[i++] = 'm';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 'r';
	packet->m_body[i++] = 'a';
	packet->m_body[i++] = 't';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x40;
	packet->m_body[i++] = 0x39;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	//videocodecid
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x0c;
	packet->m_body[i++] = 'v';
	packet->m_body[i++] = 'i';
	packet->m_body[i++] = 'd';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 'o';
	packet->m_body[i++] = 'c';
	packet->m_body[i++] = 'o';
	packet->m_body[i++] = 'd';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 'c';
	packet->m_body[i++] = 'i';
	packet->m_body[i++] = 'd';
	packet->m_body[i++] = 0x00;
	
	packet->m_body[i++] = 0x40;
	packet->m_body[i++] = 0x1c;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;	
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	//encoder
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x07;
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 'n';
	packet->m_body[i++] = 'c';
	packet->m_body[i++] = 'o';
	packet->m_body[i++] = 'd';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 'r';
	packet->m_body[i++] = 0x02;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x0d;
	packet->m_body[i++] = 'L';
	packet->m_body[i++] = 'a';
	packet->m_body[i++] = 'v';
	packet->m_body[i++] = 'f';
	packet->m_body[i++] = 0x35;
	packet->m_body[i++] = 0x38;
	packet->m_body[i++] = 0x2e;
	packet->m_body[i++] = 0x31;
	packet->m_body[i++] = 0x32;
	packet->m_body[i++] = 0x2e;
	packet->m_body[i++] = 0x31;
	packet->m_body[i++] = 0x30;
	packet->m_body[i++] = 0x30;
	//filesize
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x08;
	packet->m_body[i++] = 'f';
	packet->m_body[i++] = 'i';
	packet->m_body[i++] = 'l';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 's';
	packet->m_body[i++] = 'i';
	packet->m_body[i++] = 'z';
	packet->m_body[i++] = 'e';
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	//end
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x00;
	packet->m_body[i++] = 0x09;
	
	packet->m_packetType = 0x12; //配置信息 

	printf("%d\n",i);
    //数据长度
    packet->m_nBodySize = i;

    //块流ID
    packet->m_nChannel = 0x06; //Audio 和 video的通道   
    packet->m_nTimeStamp = 0;   //绝对时间戳
    packet->m_hasAbsTimestamp = 0; //相对时间戳
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE; //ChunkMsgHeader的类型(4种)
    packet->m_nInfoField2 = rtmp->m_stream_id;  //消息流ID     	
	
	RTMP_SendPacket(rtmp, packet, FALSE);
	

	free(packet);

}
int Send_h264_packet(uint8_t * H264_Stream, sps_pps * sp, int length, int type, uint32_t timer)
{
	RTMPPacket *packet = (RTMPPacket *)malloc(RTMP_HEADER_SIZE + length + 20);
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
//		flv_header();	
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


