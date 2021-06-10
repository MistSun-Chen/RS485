#include "comport.h"

int main(int argc , int **argv){
    int rv = -1;
    char *com_path = "/dev/ttyS1";
	comport *comport_info;
    comport_info = initComport();
	strncpy(comport_info->path,com_path,sizeof(comport_info->path));
    struct termios new_term,old_term;

	openComport(comport_info,&old_term);

	char buf[4];
	char buff[4];
    memset(buff,0,sizeof(buff));


    //result
    //当读取到正确数据时，result里存储厢号和剩余容量
    //当读取到错误数据时，result存储错误编号
    char result[LUGGAGE_NUM*2];
    memset(result,0,sizeof(result));


    int i = 1;



    cJSON *res[LUGGAGE_NUM];
    memset(res,0,sizeof(res));

    for ( int j = 0 ;j < LUGGAGE_NUM;j++){
        cJSON *carriage = cJSON_CreateObject();
        cJSON_AddNumberToObject(carriage,"carriageId",CARRIAGE_ID);
        char luggageNo[2]= {j + '1'};
        cJSON_AddStringToObject(carriage,"luggageNo",luggageNo);
        cJSON_AddNumberToObject(carriage,"capacity",1.0);
        res[j] = carriage;
    }
    for (int j = 0 ;j < LUGGAGE_NUM;j++){
        char *str = cJSON_Print(res[j]);
        printf("%s\n",str);
    }



    while(1){
        buf[0] = 0xFF;
        buf[1] = 0x00;
        buf[2] = 0x00;
        cJSON *carrList = NULL;
        carrList = cJSON_CreateArray();


        while(i<=LUGGAGE_NUM){
            buf[1] = buf[1] + 0x01;
            buf[3] = buf[1] ^ buf[2];

            rv = writeComport(comport_info->com_fd,buf,sizeof(buf));

            rv = readComport(comport_info->com_fd,buff,sizeof(buff));

            if(rv == -1){
                getNoAnsOp(i);
            }

            char meg[2];
            memset(meg,0,sizeof(meg));
            readFrame(buff,sizeof(buff),meg);


            result[(i-1)*2] = meg[0];
            result[(i-1)*2 +1] = meg[1];

            memset(buff,0,sizeof(buff));

            if(i == LUGGAGE_NUM){
                char *s = createJson(carrList,res,result,sizeof(result));
                write_str("./result.txt",s);
                sendHttp(HTTP_POST,s);

                i = 1;
                buf[1] = 0x00;
                sleep(15);
                tcflush(comport_info->com_fd,TCIOFLUSH);
            }else{
                i++;
            }
        }


    }

//    while(1){
//
//        rv = readComport(comport_info->com_fd,buff,sizeof(buff));
//        sleep(2);
//        if(rv == -1){
//            printf("Haven't get Number %d data\n",i);
//        }
//        readFrame(buff,sizeof(buff));
//    }

	closeComport(comport_info,&old_term);
	return 0;

}