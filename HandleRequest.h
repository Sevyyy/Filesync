#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUEST_H

//用于处理本项目所需要的各种请求操作，包括文件传输和文件同步比较
//相关的辅助函数也定义在这里了，进一步重构再改进
//代码里依然后部分硬编码，时间允许再加入配置文件一块

//包含头文件
#include <sys/stat.h>
#include <Windows.h>  
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <ws2tcpip.h>

#include <map>
#include <algorithm>
#include <string>
#include <fstream>

#include "Communication.h"

using namespace std;

//宏定义
#define SERVER_VERSION_FILE "ServerVersion.txt"       //服务器版本信息文件
#define CLIENT_VERSION_FILE "ClientVersion.txt"       //客户端版本信息文件
#define CLIENT_FIX_TIME_FILE "ClientFixTime.txt"      //客户端修改时间信息文件
#define DEFAULT_FILE_KEY "I AM A FILE"    //文件块标识
#define DEFAULT_RQST_KEY "I AM A RQST"    //请求块标识

//请求码
#define REQUEST_SYNC 11                //同步请求
#define REQUSET_UPLOAD 12			   //单上传请求
#define REQUEST_COMMIT 13              //全局提交请求
#define REQUEST_BAD 10                 //无效请求

#define REQUEST_NOTHING 20             //空请求
#define REQUEST_FILE 21			       //请求文件
#define REQUEST_DELETE 22              //请求删除服务器文件

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

struct VersionItem{
	char name[28];
	int version;
	VersionItem(){}
	VersionItem(const char *_name, int _version):version(_version){
		copy(_name, _name+28, name);
	}
};

//发送请求，一般用于客户端向服务器发送请求
void SendRequset(SOCKET &remoteSocket, int requsetType);

//接收请求，一般用于服务器接收客户端的请求
int RecvRequest(SOCKET &remoteSocket);

//加载版本信息的文件，到一个map里
void LoadVersionMap(map<string, int> &versionMap, string fileName);

//同步请求中的服务器端提供的为客户端进行同步的操作
void ServerAutoSync(SOCKET &remoteSocket);

//同步请求中的客户端所进行的操作
void ClientAutoSync(SOCKET &remoteSocket);

//处理客户端上传单个文件的请求，版本文件硬编码了
void SolveFileCommitFromClient(SOCKET &remoteSocket, map<string, int> &version);

//客户端同步一个文件到服务器，存在硬编码
void CommitFileToServer(SOCKET &remoteSocket, string fileName);

//服务器端用于处理客户端上传全局文件的请求
void SolveAllCommitFromClient(SOCKET &remoteSocket, map<string, int> &version);

//用于客户端请求同步全局文件到服务器端
void CommitAllToServer(SOCKET &remoteSocket);


#endif