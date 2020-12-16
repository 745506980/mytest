#########################################################################
# File Name: Makefile
# 作者:YJK 
# mail: 745506980@qq.comc
# Created Time: 2020年11月27日 星期五 15时32分31秒
#########################################################################
#    $^ 当前规则中的所有依赖
#    $< 当前规则中的第一个依赖
#    $@ 当前规则中触发命令生成的目标
#    @ 不把执行的信息打印到显示屏上

CROSS_COMPILE = arm-linux-
CC=$(CROSS_COMPILE)gcc

TARGET = Send_h264

OBJS += rtmp_send.o
OBJS += x264_encoder.o
OBJS += camer.o
OBJS += main.o


INCLUDES = -I./include/

LINK_OPTS = -lpthread -lrt -I./include -L./lib -lx264 -lrtmp -lssl -lz -lcrypto
CFLAGS = -Wall -O2 -g 



$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LINK_OPTS)

%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS)


clean:
	rm $(OBJS) $(TARGET)

