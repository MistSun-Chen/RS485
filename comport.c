#include "comport.h"
//#define

comport *initComport()
{
    comport *comport_info = NULL;

    if((comport_info = (comport *)malloc(sizeof(comport))) == NULL)
    {
        return NULL;
    }
    memset(comport_info,0,sizeof(comport));
    //无校验
    comport_info->paritybit='N';
    comport_info->com_fd = -1;
    //8个数据位
    comport_info->databit = 8;
    //波特率
    comport_info->baudrate =9600;
    //1个停止位
    comport_info->stopbit = 1;
    comport_info->isopen = 0;
    return comport_info;
}

void comport_term(comport *pcomport)
{
    if(NULL == pcomport)
    {
        return;
    }

    if(!pcomport->isopen)
    {
        close(pcomport->com_fd);
    }

    free(pcomport);
    pcomport = NULL;
}

int _set_baudrate(struct termios *term,speed_t baudrate)
{
	if(cfsetispeed(term,baudrate) < 0)
	{
		printf("cfsetispeed failure:%s\n",strerror(errno));
		return -1;
	}

	if(cfsetospeed(term,baudrate) < 0)
	{
		printf("cfsetospeed failure:%s\n",strerror(errno));
		return -2;
	}
    return 0;
}

void set_baudrate(int baudrate,struct termios *term)
{
	/*0 50 75 110 134 150 200 300 600 1200 1800 2400 4800 9600 19200 38400 57600 115200 230400*/
	switch(baudrate)
	{
		case 0:
			_set_baudrate(term, B0);
            break;
		case 50:
			_set_baudrate(term, B50);
            break;
		case 75:
			_set_baudrate(term, B75);
            break;
		case 110:
			_set_baudrate(term, B110);
            break;
		case 134:
			_set_baudrate(term, B134);
            break;
		case 150:
			_set_baudrate(term, B150);
            break;
		case 200:
			_set_baudrate(term, B200);
            break;
		case 300:
			_set_baudrate(term, B300);
            break;
		case 600:
			_set_baudrate(term, B600);
            break;
		case 1200:
			_set_baudrate(term, B1200);
            break;
		case 1800:
			_set_baudrate(term, B1800);
            break;
		case 2400:
			_set_baudrate(term, B2400);
            break;
		case 4800:
			_set_baudrate(term, B4800);
            break;
		case 9600:
			_set_baudrate(term, B9600);
            break;
		case 19200:
			_set_baudrate(term, B19200);
            break;
		case 38400:
			_set_baudrate(term, B38400);
            break;
		case 57600:
			_set_baudrate(term, B57600);
            break;
		case 115200:
			_set_baudrate(term, B115200);
            break;
		case 230400:
			_set_baudrate(term, B230400);
            break;
		default:
			_set_baudrate(term, B115200);
            break;
	}
}

void set_data_bit(int databit,struct termios *term)
{
	term->c_cflag &= ~CSIZE;
	switch (databit)
	{
		case 8:
			term->c_cflag |= CS8;
			break;
		case 7:
			term->c_cflag |= CS7;
			break;
		case 6:
			term->c_cflag |= CS6;
			break;
		case 5:
			term->c_cflag |= CS5;
			break;
		default:
			term->c_cflag |= CS8;
			break;
	}

}

void set_parity(char paritybit,struct termios *term)
{
	switch (paritybit)
	{
		case 'N':                  /* no parity check */
			term->c_cflag &= ~PARENB;
			break;
		case 'E':                  /* 偶检验 */
			term->c_cflag |= PARENB;
			term->c_cflag &= ~PARODD;
			break;
		case 'O':                  /* 奇校验 */
			term->c_cflag |= PARENB;
			term->c_cflag |= PARODD;
			break;
		default:                   /* no parity check */
			term->c_cflag &= ~PARENB;
			break;
	}

}

void set_stopbit(int stopbit,struct termios *term)
{
	if (2 == stopbit)
	{
		term->c_cflag |= CSTOPB;  /* 2 stop bits */
	}

	else
	{
		term->c_cflag &= ~CSTOPB; /* 1 stop bit */
	}

}

int setComport(comport *p_comport,struct termios *term)
{
    term->c_oflag &= ~OPOST;
    term->c_cflag |= CLOCAL | CREAD;//用于本地连接和接收使用
    term->c_cflag &=~CSIZE;// 先使用CSIZE做位屏蔽
    term->c_oflag = 0; //输出模式
    term->c_lflag = 0; //不激活终端模式
	set_stopbit(p_comport->stopbit,term);
	set_parity(p_comport->paritybit,term);
	set_data_bit(p_comport->databit,term);
	set_baudrate(p_comport->baudrate,term);
    term->c_cc[VMIN]   =   4;
    term->c_cc[VTIME]  =   2;

	tcflush(p_comport->com_fd,TCIOFLUSH);

	if((tcsetattr(p_comport->com_fd,TCSANOW,term)) != 0)
	{
		printf("tcsetattr failure:%s\n",strerror(errno));
		return -1;
	}

    printf("set comport ok!\n");
    //printf("The details are printed below\n");
    //system("sudo sudo stty -F /dev/ttyUSB0 -a");
    return 0;

}

int openComport(comport *p_comport,struct termios *old_term)
{
    struct termios new_term;
	int serial_fd = -1;

    if((NULL == p_comport) && (NULL == old_term))
    {
        return -1;
    }

    if(p_comport->isopen)
    {
        closeComport(p_comport,old_term);
    }

	if((serial_fd = open(p_comport->path,O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
		printf("open a serialport failure:%s\n",strerror(errno));
        return -1;
	}


//	if(isatty(serial_fd) == 0)
//	{
//		printf("open fd is not a terminal device\n");
//        close(serial_fd);
//        return -1;
//	}

	p_comport->com_fd = serial_fd;
    p_comport->isopen = 1;

    if(tcgetattr(serial_fd,old_term) < 0)
	{
		printf("tcgetattr failure:%s\n",strerror(errno));
		return -3;
	}

    memset(&new_term,0,sizeof(new_term));

    if(tcgetattr(serial_fd,&new_term) < 0)
	{
		printf("tcgetattr failure:%s\n",strerror(errno));
		return -3;
    }

    setComport(p_comport,&new_term);
    return 0;

}

int readComport(int fd,char *buff,int buffsize)
{
    fd_set  rset;
//
//    //将reset清零
    FD_ZERO(&rset) ;
//    //将fd加入
    FD_SET(fd, &rset) ;

	int rv = -1;

	if(buff == NULL)
	{
		printf("Cannot pass in null pointer\n");
		return -1;
    }
    struct timeval val;
    val.tv_sec = 0;
    val.tv_usec = 200000;
    rv = select(fd+1, &rset, NULL, NULL, &val) ;
//    rv = select(fd+1, &rset, NULL, NULL, NULL) ;


    if(rv < 0)
	{
        printf("select() failed: %s\n", strerror(errno)) ;
		return -1;
	}
    if(rv == 0)
    {
        printf("select() time out!\n") ;
        return -1;
    }

    printf("Starting read serial !\n");
    rv = read(fd, buff, buffsize) ;
    printf("Reading %s\n",buff);
    printf("Reading finish !\n");

     if(rv < 0)
    {
        printf("Read() error:%s\n",strerror(errno)) ;
        return -1;
    }

	return rv;
}

int writeComport(int fd,char *buff,int buffsize)
{
	int rv = -1;

	if(buff == NULL)
	{
		printf("Cannot pass in null pointer\n");
		return -1;
	}

	if((rv = write(fd,buff,buffsize)) < 0)
	{
		printf("write failure:%s\n",strerror(errno));
        return -1;
	}

//    usleep(10*1000);
//    sleep(1);
	return rv;
}

int closeComport(comport *pcomport,struct termios *old_term)
{
    if(pcomport == NULL && old_term == NULL)
    {
        return -1;
    }

	tcflush(pcomport->com_fd,TCIOFLUSH);

	if((tcsetattr(pcomport->com_fd,TCSANOW,old_term)) != 0)
	{
		printf("Set to the original property failure:%s\n",strerror(errno));
        return -1;
	}

    pcomport->isopen = 0;
	close(pcomport->com_fd);
    free(pcomport);
    pcomport = NULL;
    return 0;
}

char *createJson(cJSON *carrList,cJSON **res,char *result,int size){

    char * str = NULL;

    for(int i = 0; i<size/2;i++){


        //如果result[i][0]大于100，出错类型由0x65到0x68
        //0x65厢号不符合要求
        //0x66容量不符合要求
        //0x67异或结果出错
        //0x68读取帧没有读取到信息
        if(result[i*2] > 0x64)continue;
        else{
            int IntlugNo = (int)result[i*2];
            int Intcap;
            float capacity;

            Intcap = (int)result[i*2+1];
            int ten,singal;
            ten = Intcap / 16;
            singal = Intcap % 16;
            Intcap = ten * 10 + singal ;
            capacity = Intcap / 100.0;
            cJSON *change = cJSON_GetObjectItem(res[IntlugNo-1],"capacity");
            change->valuedouble = capacity;
            cJSON_AddItemToArray(carrList,res[IntlugNo-1]);
        }

    }
    str = cJSON_Print(carrList);
    printf("%s\n",str);
    return str;
}



//char *createJson(char *result,int size){
//
//    char * str = NULL;
//    cJSON *carrList = NULL;
//    carrList = cJSON_CreateArray();
//
//
//    for(int i = 0; i<size/2;i++){
//
//
//        //如果result[i][0]大于100，出错类型由0x65到0x68
//        //0x65厢号不符合要求
//        //0x66容量不符合要求
//        //0x67异或结果出错
//        //0x68读取帧没有读取到信息
//        if(result[i*2] > 0x64)continue;
//
//
//        cJSON *carriage = cJSON_CreateObject();
//        int Intcap,IntlugNo;
//        float capacity;
//
//        IntlugNo = (int)result[i*2];
//
//        Intcap = (int)result[i*2+1];
//        int ten,singal;
//        ten = Intcap / 16;
//        singal = Intcap % 16;
//        Intcap = ten * 10 + singal ;
//        capacity = Intcap / 100.0;
//        char strlugNo[1]= {IntlugNo + '0'};
//
//        cJSON_AddNumberToObject(carriage,"carriageId",CARRIAGE_ID);
//
//        cJSON_AddStringToObject(carriage,"luggageNo",strlugNo);
//
//        cJSON_AddNumberToObject(carriage,"capacity",capacity);
//
//        cJSON_AddItemToArray(carrList,carriage);
//    }
//
//    str = cJSON_Print(carrList);
//    printf("%s\n",str);
//    return str;
//}
//char * createJson(char *meg){
//    char * str = NULL;
//    cJSON* carriage = NULL,*carrList = NULL;
//    carrList = cJSON_CreateArray();
//    carriage = cJSON_CreateObject();
//
//
//    printf("create Json\n");
//    int Intcap,IntlugNo;
//    float capacity;
//
//    IntlugNo = (int)meg[0];
//
//    Intcap = (int)meg[1];
//    int ten,singal;
//    ten = Intcap / 16;
//    singal = Intcap % 16;
//    Intcap = ten * 10 + singal ;
//
////    flcap = float(Intcap);
//
//    capacity = Intcap / 100.0;
//
//
//    char strlugNo[1]= {IntlugNo + '0'};
//
//    cJSON_AddNumberToObject(carriage,"carriageId",CARRIAGE_ID);
//
//    cJSON_AddStringToObject(carriage,"luggageNo",strlugNo);
//
//    cJSON_AddNumberToObject(carriage,"capacity",capacity);
//
//    cJSON_AddItemToArray(carrList,carriage);
//
//    str = cJSON_Print(carrList);
//
//    printf("%s\n",str);
//    return str;
//}

int write_str(char * add,char * str){
    FILE *fp = NULL;
    char str_1[100];

    if((fp = fopen(add,"at+")) == NULL){
        puts("Open file failed\n");
        return -1;
    }else{
        puts("Open file success\n");
    }

    fputs(str,fp);
    fputs("\n",fp);

	fclose(fp);
    return 0;
}


//void readFrame(char *r_buf,int buffsize){
//    for(int i =0 ;i<buffsize;){
//        if(r_buf[i] == 0xFF && i+3 < buffsize){
//            printf("\n");
//            printf("读取到初始帧%02X\n",r_buf[i]);
//            if(r_buf[i+1]>0x08 || r_buf[i+1] < 0x00){
//                printf("厢号不在确定的数据范围以内，厢号由0到8\n");
//                i = i + 4;
//                continue;
//            }
//            printf("读取到厢号00~08:%02X\n",r_buf[i+1]);
//            if(r_buf[i+2]>0x64 || r_buf[i+2] < 0x00){
//                printf("读取到容量%02X，容量不在确定的数据范围以内，容量由0到100\n",r_buf[i+2]);
//                i = i + 4;
//                continue;
//            }
//            printf("读取到容量显示00~100:%02X\n",r_buf[i+2]);
//            char t = r_buf[i+2] ^ r_buf[i+1];
//            if(r_buf[i+3] != t){
//                printf("异或结果出错，异或结果为:%02X\n",r_buf[i+2] ^ r_buf[i+1]);
//            }
//            else{
//                printf("异或结果无异常，可以发送至服务器\n");
//            }
//            printf("开始HTTP服务\n");
//            char * add = "http://192.168.1.132:2002/luggage_capacity_info";
//            char meg[2];
//            meg[0] = r_buf[i+1];
//            meg[1] = r_buf[i+2];
//            char * json = createJson(meg);
//            sendHttp(add,json);
//            write_str("./result.txt",json);
//            printf("send http to server！\n");
//            i = i + 4;
//        }else{
//            printf("%02X",r_buf[i]);
//            ++i;
//        }
//
//    }
//    printf("\n");
//    printf("Read from tty: %s\n",r_buf);
//}

int readFrame(char *r_buf,int buffsize,char *meg){
    for(int i =0 ;i<buffsize;){
        if(r_buf[i] == 0xFF && i+3 < buffsize){
            printf("\n");
            printf("读取到初始帧%02X\n",r_buf[i]);

            if(r_buf[i+1]>0x08 || r_buf[i+1] < 0x00){
                printf("厢号不在确定的数据范围以内，厢号由0到8\n");
                meg[0] = 0x65;
                break;
            }
            printf("读取到厢号00~08:%02X\n",r_buf[i+1]);
            if(r_buf[i+2]>0x64 || r_buf[i+2] < 0x00){
                printf("读取到容量%02X，容量不在确定的数据范围以内，容量由0到100\n",r_buf[i+2]);
                meg[0] = 0x66;
                break;
            }
            printf("读取到容量显示00~100:%02X\n",r_buf[i+2]);
            char t = r_buf[i+2] ^ r_buf[i+1];
            if(r_buf[i+3] != t){
                printf("异或结果出错，异或结果为:%02X\n",r_buf[i+2] ^ r_buf[i+1]);
                meg[0] = 0x67;
                break;
            }
            else{
                printf("异或结果无异常，可以发送至服务器\n");
                meg[0] = r_buf[i+1];
                meg[1] = r_buf[i+2];
                return 1;
            }

//            printf("开始HTTP服务\n");
//            char * add = "http://192.168.1.132:2002/luggage_capacity_info";
//            char meg[2];
//            meg[0] = r_buf[i+1];
//            meg[1] = r_buf[i+2];
//            char * json = createJson(meg);
//            sendHttp(add,json);
//            write_str("./result.txt",json);
//            printf("send http to server！\n");
//            i = i + 4;
        }else{
            printf("%02X",r_buf[i]);
            ++i;
        }
        if(i == buffsize){
            printf("\n");
            meg[0] = 0x68;
        }

    }
    meg[1] = 0x00;
    return -1;
}


int getNoAnsOp(int i){
    printf("Haven't get Number %d data\n",i);
    return 1;
}


void sendHttp(char* add,char *str){

    printf("%s\n",str);

    char *p = http_post(add,str);
    printf("the response of the request is \n%s\n",p);

}
