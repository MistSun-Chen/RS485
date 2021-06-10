#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "cJSON.h"
#include "http.h"

#define BUFFER_SIZE 1024
#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
    "Content-Type:application/json\r\nContent-Length: %d\r\n\r\n%s"
//#define HTTP_POST "POST /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n"\
//    "Content-Type:application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s"
#define HTTP_GET "GET /%s HTTP/1.1\r\nHOST: %s:%d\r\nAccept: */*\r\n\r\n"


/*


    struct sockaddr_in
    {
        sa_family_t sin_family; //地址族(Address Family)
        uint16_t sin_port; //16位TCP/UDP端口号(0-65535)
        struct in_addr sin_addr;    //32位IP地址(0.0.0.0-255.255.255.255)
        char sin_zero[8];   //不使用
    }
	struct hostent
	{
		char *h_name;         //正式主机名
		char **h_aliases;     //主机别名
		int h_addrtype;       //主机IP地址类型：IPV4-AF_INET
		int h_length;		  //主机IP地址字节长度，对于IPv4是四字节，即32位
		char **h_addr_list;	  //主机的IP地址列表
	};

*/

static int http_tcpclient_create(const char *host, int port){
    struct hostent *he;
    struct sockaddr_in server_addr;
    int socket_fd;

    if((he = gethostbyname(host))==NULL){
        return -1;
    }

   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(port);
   server_addr.sin_addr = *((struct in_addr *)he->h_addr);

   //创建套接字
    if((socket_fd = socket(AF_INET,SOCK_STREAM,0))==-1){
        return -1;
    }

    if(connect(socket_fd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
        return -1;
    }

    return socket_fd;
}

static void http_tcpclient_close(int socket){
    close(socket);
}

static int http_parse_url(const char *url,char *host,char *file,int *port)
{
    printf("start http_parse_url\n");
    printf("%s\n%s\n%s\n%d\n",url,host,file,port);
    char *ptr1,*ptr2;
    int len = 0;
    if(!url || !host || !file || !port){
        return -1;
    }

    ptr1 = (char *)url;
    printf("url %s\n",ptr1);
    if(!strncmp(ptr1,"http://",strlen("http://"))){
        ptr1 += strlen("http://");
    }else{
        return -1;
    }
    printf("ptr1 %s\n",ptr1);

    ptr2 = strchr(ptr1,'/');

    printf("ptr2 %s\n",ptr2);

    if(ptr2){
        len = strlen(ptr1) - strlen(ptr2);
        memcpy(host,ptr1,len);
        host[len] = '\0';
        if(*(ptr2 + 1)){
            memcpy(file,ptr2 + 1,strlen(ptr2) - 1 );
            file[strlen(ptr2) - 1] = '\0';
        }
        printf("host is %s\n",host);
        printf("file is %s\n",file);
    }else{
        memcpy(host,ptr1,strlen(ptr1));
        host[strlen(ptr1)] = '\0';
        printf("host is %s\n",host);
    }
    //get host and ip
    ptr1 = strchr(host,':');
    printf("ptr1 is %s\n",ptr1);
    if(ptr1){
        *ptr1++ = '\0';
        *port = atoi(ptr1);
    }else{
        *port = MY_HTTP_DEFAULT_PORT;
    }
    printf("port is %d \n",*port);

    return 0;
}


static int http_tcpclient_recv(int socket,char *lpbuff){
    int recvnum = 0;

    recvnum = recv(socket, lpbuff,BUFFER_SIZE*4,0);

    return recvnum;
}

static int http_tcpclient_send(int socket,char *buff,int size){
    int sent=0,tmpres=0;

    while(sent < size){
        tmpres = send(socket,buff+sent,size-sent,0);
        if(tmpres == -1){
            return -1;
        }
        sent += tmpres;
    }
    return sent;
}

static char *http_parse_result(const char*lpbuf)
{
    char *ptmp = NULL,*ptmp_1 = NULL,*ptmp_2 = NULL;
    char *response = NULL;
    ptmp_1 = (char*)strstr(lpbuf,"HTTP/1.0");
    ptmp_2 = (char*)strstr(lpbuf,"HTTP/1.1");
    if(ptmp_1 ==NULL && ptmp_2 == NULL){
        printf("http/1.1 and http/1.2 not faind\n");
    }else{
        if(ptmp_1)ptmp = ptmp_1;
        if(ptmp_2)ptmp = ptmp_2;
    }
//    ptmp = (char*)strstr(lpbuf,"HTTP/1.1");
    if(!ptmp){
        printf("http/1.1 not faind\n");
    }
    if(atoi(ptmp + 9)!=200){
        printf("result:\n%s\n",lpbuf);
        return NULL;
    }

    ptmp = (char*)strstr(lpbuf,"\r\n\r\n");
    if(!ptmp){
        printf("ptmp is NULL\n");
        return NULL;
    }
    response = (char *)malloc(strlen(ptmp)+1);
    if(!response){
        printf("malloc failed \n");
        return NULL;
    }
    strcpy(response,ptmp+4);
    return response;
}

char * http_post(const char *url,const char *post_str){

    char post[BUFFER_SIZE] = {'\0'};
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE*4] = {'\0'};
    char *ptmp;
    char host_addr[BUFFER_SIZE] = {'\0'};
    char file[BUFFER_SIZE] = {'\0'};
    int port = 0;
    int len=0;
	char *response = NULL;

    if(!url || !post_str){
        printf("      failed!\n");
        return NULL;
    }

    if(http_parse_url(url,host_addr,file,&port)){
        printf("http_parse_url failed!\n");
        return NULL;
    }
    //printf("host_addr : %s\tfile:%s\t,%d\n",host_addr,file,port);

    socket_fd = http_tcpclient_create(host_addr,port);
    if(socket_fd < 0){
        printf("http_tcpclient_create failed\n");
        return NULL;
    }

    sprintf(lpbuf,HTTP_POST,file,host_addr,port,strlen(post_str),post_str);

    if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf)) < 0){
        printf("http_tcpclient_send failed..\n");
        return NULL;
    }
	printf("发送请求:\n%s\n",lpbuf);

    /*it's time to recv from server*/
    if(http_tcpclient_recv(socket_fd,lpbuf) <= 0){
        printf("http_tcpclient_recv failed\n");
        return NULL;
    }

    printf("接收消息:\n%s\n",lpbuf);
    http_tcpclient_close(socket_fd);

    return http_parse_result(lpbuf);
//    return lpbuf;
}

char * http_get(const char *url)
{

    char post[BUFFER_SIZE] = {'\0'};
    int socket_fd = -1;
    char lpbuf[BUFFER_SIZE*4] = {'\0'};
    char *ptmp;
    char host_addr[BUFFER_SIZE] = {'\0'};
    char file[BUFFER_SIZE] = {'\0'};
    int port = 0;
    int len=0;

    if(!url){
        printf("      failed!\n");
        return NULL;
    }

    if(http_parse_url(url,host_addr,file,&port)){
        printf("http_parse_url failed!\n");
        return NULL;
    }
    //printf("host_addr : %s\tfile:%s\t,%d\n",host_addr,file,port);

    socket_fd =  http_tcpclient_create(host_addr,port);
    if(socket_fd < 0){
        printf("http_tcpclient_create failed\n");
        return NULL;
    }

    sprintf(lpbuf,HTTP_GET,file,host_addr,port);
//    printf("%s\n",lpbuf);

    if(http_tcpclient_send(socket_fd,lpbuf,strlen(lpbuf)) < 0){
        printf("http_tcpclient_send failed..\n");
        return NULL;
    }
	printf("发送请求:\n%s\n",lpbuf);

    if(http_tcpclient_recv(socket_fd,lpbuf) <= 0){
        printf("http_tcpclient_recv failed\n");
        return NULL;
    }
    http_tcpclient_close(socket_fd);
    printf("接收请求:\n%s\n",lpbuf);
    return http_parse_result(lpbuf);
//    return lpbuf;
}



//int main(int argc , int **argv){
//
//
////Post Request
//    char * str = NULL;
////    cJSON* cjson_test = NULL;
////    cJSON* cjson_address = NULL;
////    cJSON* cjson_skill = NULL;
////
////    cjson_test = cJSON_CreateObject();
////    cJSON_AddStringToObject(cjson_test, "name", "testjson");
////    cJSON_AddNumberToObject(cjson_test, "numbertest", 22);
////    cJSON_AddNumberToObject(cjson_test, "floattest", 55.5);
////
////
////    cjson_address = cJSON_CreateObject();
////    cJSON_AddStringToObject(cjson_address, "country", "China");
////    cJSON_AddNumberToObject(cjson_address, "zip-code", 111111);
////    cJSON_AddItemToObject(cjson_test, "recurtest", cjson_address);
////
////    cjson_skill = cJSON_CreateArray();
////    cJSON_AddItemToArray(cjson_skill, cJSON_CreateString( "C" ));
////    cJSON_AddItemToArray(cjson_skill, cJSON_CreateString( "Java" ));
////    cJSON_AddItemToArray(cjson_skill, cJSON_CreateString( "Python" ));
////    cJSON_AddItemToObject(cjson_test, "skill", cjson_skill);
////
////    cJSON_AddFalseToObject(cjson_test, "student");
////
////    /* 打印JSON对象(整条链表)的所有数据 */
////    str = cJSON_Print(cjson_test);
//////    printf("%s\n", str);
//
//    cJSON* carriage = NULL;
//    carriage = cJSON_CreateObject();
//    cJSON_AddNumberToObject(carriage,"carriageId",1);
//    cJSON_AddStringToObject(carriage,"luggageNo","2");
//    cJSON_AddNumberToObject(carriage,"capacity",0.7);
//
//    str = cJSON_Print(carriage);
//    printf("%s\n",str);
//
//    char *p = http_post("http://192.168.22.129:5000/predict",str);
//    printf("the response of the request is \n%s\n",p);
//
//    //Get Request
////    char *p = http_get("http://192.168.22.128:5000/");
//    return 0;
//}