#ifndef COMMUNICATION_H
#define COMMUNICATION_H

//服务器和客户端之间的连接和文件传输相关的函数和一些相关结构体和宏在此声明与实现

//包含头文件
#include <Windows.h>  
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <ws2tcpip.h>
#include <string> 

//宏定义
#define DEFAULT_IP "192.168.205.102"
#define DEFAULT_PORT 2333                  //默认端口
#define DEFAULT_BUFFER_SIZE 512            //默认缓冲块大小

//WSA的启动函数，封装了错误处理，可用于服务器端和客户端
bool WSAInitialize();

//WSA的关闭函数，可用于服务器端和客户端
void WSAEnd();

//创建一个套接字，封装了错误处理，可用于服务器端和客户端
bool CreateSocket(SOCKET & connectSocket);

//绑定socket与地址端口，封装了错误处理，一般用于服务器端
bool BindSocket(SOCKET & listenSocket);

//让socket开始监听，封装了错误处理，一般用于服务器端
bool Listen(SOCKET & listenSocket);

//接受客户端的连接请求，封装了错误处理，一般用于服务器端
bool AcceptClientConnection(SOCKET &listenSocket, SOCKET &connectSocket);

//连接到远程socket，封装了错误处理，一般用于客户端连接服务器
bool ConnectSocket(SOCKET &connectSocket, std::string ip = DEFAULT_IP);

//关闭一个socket
void CloseSocket(SOCKET &socket);

//优雅地关闭一个socket
void ShutdownSocket(SOCKET &socket);

#endif