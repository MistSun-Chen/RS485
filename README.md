# 基于LinuxC的RK3288开发板的RS485串口通信以及http请求发送



## rk3288c相关资料

* [官方文档](https://wiki.t-firefly.com/zh_CN/AIO-3288C/started.html)

* [开发板资源镜像下载](https://www.t-firefly.com/doc/download/51.html)
* [交叉编译环境搭建](https://jingyan.baidu.com/article/fea4511a413d2bb6bb9125f6.html)





## 串口通信参考

* [串口通信基础](https://blog.csdn.net/caijiwyj/article/details/90314312)
* [串口通信详解](https://www.cnblogs.com/jimmy1989/p/3545749.html)
* [串口编程抽象化](https://blog.csdn.net/Shallwen_Deng/article/details/89482502)
* 串口通信实现：comport.c与comport.h



## LinuxC网络编程

* [Linux C网络编程基础](https://blog.csdn.net/qq_37653144/article/details/81605294)
* [Linux C实现HTTP get及post请求](https://blog.csdn.net/sjin_1314/article/details/41776679)
* HTTP请求实现：http.c与http.h，cJSON为开源代码，可以方便的创建和解析json数据





## 运行流程

1. 下载官方Ubuntu固件并按照文档烧写到rk3288开发板(**运行环境**)上
2. 利用虚拟机或者双系统创建纯净ubuntu系统(**开发环境**)搭建交叉编译环境
3. 确定开发板的串口文件名(本例中为"/dev/ttyS1")
4. 在comportc.h中更改HTTP_POST地址
5. 利用交叉编译工具编译源文件，将可执行文件传到开发板上

在开发环境

```bash
make
scp rs485 firefly@ip_of_rk3288:/home/firefly
```

在运行环境

```bash
cd /home/firefly
sudo ./rs485
```





本项目作用在接收串口数据，判断数据结构是否正确，如果正确，创建json格式数据，并将数据发送至远程Web服务器。



## 本项目串口接收与发送数据格式规则

### 数据大小：4byte

0xFF----------------起始帧（读到FF再开始往后读）

0x03----------------功能帧1（本例中规定其大于0小于等于8）

0x20----------------功能帧2（本例中规定其大于0小于等于0x64）

0x23----------------异或帧（错误判断，防止功能帧发送错误起检验作用）



