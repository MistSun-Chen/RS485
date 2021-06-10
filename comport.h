#ifndef _COMPORT_H
#define _COMPORT_H

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include "http.h"
#include "cJSON.h"


#define ARRAY_MAX 64
#define CARRIAGE_ID 1
#define LUGGAGE_NUM 8
#define HTTP_POST "http://192.168.1.132:2002/luggage_capacity_info"


typedef struct _comport{
	char paritybit;//奇偶检验位
	int databit;//数据位
	int com_fd;//fd
	int baudrate;//波特率
	int stopbit;//停止位
    int isopen;
	char path[ARRAY_MAX];//串口路径
}comport;

comport *initComport();

void comport_term(comport *pcomport);

int openComport(comport *p_comport,struct termios *old_term);

int closeComport(comport *p_comport,struct termios *old_term);

int readComport(int fd,char *buff,int buffsize);

int writeComport(int fd,char *buff,int buffsize);

int setComport(comport *p_comport,struct termios *term);

int readFrame(char *r_buf,int buffsize,char *meg);

int getNoAnsOp(int i);

void sendHttp(char* add,char *meg);

char *createJson(cJSON *carrList,cJSON **res,char *result,int size);

int write_str(char * add,char * str);

#endif

