#########################################################################
# File Name: build.sh
# 作者:YJK 
# mail: 745506980@qq.comc
# Created Time: 2020年11月27日 星期五 15时33分00秒
#########################################################################
#!/bin/bash
arm-linux-gcc -o hello hello.c -L./lib -lx264 -lpthread -ldl -lm -static -std=c99

