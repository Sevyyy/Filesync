#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUEST_H

//用于处理本项目所需要的各种请求操作，包括文件传输和文件同步比较
//相关的辅助函数也定义在这里了，进一步重构再改进
//代码里依然后部分硬编码，时间允许再加入配置文件一块

//包含头文件
#include "Communication.h"
#include <Windows.h>  
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <ws2tcpip.h>
#include <map>
#include <algorithm>
#include <string>

using namespace std;

//宏定义
#define DEFAULT_FILE_KEY "I AM A FILE"    //文件块标识
#define DEFAULT_RQST_KEY "I AM A RQST"    //请求块标识

#define REQUEST_SYNC 11                //同步请求
#define REQUSET_UPLOAD 12			   //上传请求
#define REQUEST_BAD 10                 //无效请求

//文件块，用于文件传输文件之前的确认和属性传输
struct FileBlock{
	char key[12];
	char name[48];
	int size;
	FileBlock(){}
	FileBlock(const char *_name, int _size):key(DEFAULT_FILE_KEY), size(_size){
		copy(_name, _name+48, name);
	}
};

//请求块，用于发送请求的确认及请求类型的传输
struct RequestBlock{
	char key[12];
	int type;
	RequestBlock(){}
	RequestBlock(int _type):key(DEFAULT_RQST_KEY), type(_type){}
};


//向remote套接字发送一个文件，封装了错误处理，有FileBlock做校验，可用于服务器端和客户端
bool SendFile(SOCKET &remoteSocket, string fileName);

//从remote套接字读取文件，封装了错误处理，可用于服务器端和客户端
bool RecvFile(SOCKET &remoteSocket);

//发送请求，一般用于客户端向服务器发送请求
void SendRequset(SOCKET &remoteSocket, int requsetType);

//接收请求，一般用于服务器接收客户端的请求
int RecvRequest(SOCKET &remoteSocket);

//加载版本信息的文件，到一个map里
void LoadVersionMap(map<string, int> &versionMap, string fileName);

//同步请求中的服务器端提供的为客户端进行同步的操作
void ServerAutoSync(SOCKET &remoteSocket);

//辅助函数，判断一个文件是否可删，此处硬编码严重，保留代码文件和可执行文件，版本信息等
bool IsFile(string fileName);

//辅助函数，用于非上传前的同步操作中，删除服务器上没有但客户端有的文件
void DeleteNotExist(map<string, int> &serverVersionMap);


//辅助函数，判断服务器端和客户端文件的版本状态和是否存在，
//并从服务器端请求文件，用于非上传前的同步
void CompareServerWithClient(SOCKET &remoteSocket, map<string, int>& serverVersionMap, map<string, int>& clientVersionMap);

//同步请求中的客户端所进行的操作
void ClientAutoSync(SOCKET &remoteSocket, map<string, int>& clientVersionMap);

#endif