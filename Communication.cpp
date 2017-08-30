//包含头文件
#include "Communication.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <algorithm>

using namespace std;

//WSA的启动函数，封装了错误处理，可用于服务器端和客户端
bool WSAInitialize(){
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)){
		cout << "Server start up failed, ";
		cout << "with error code : " << WSAGetLastError() << endl;
		return false;
	}
	return true;
}

//WSA的关闭函数，可用于服务器端和客户端
void WSAEnd(){
	WSACleanup();
}

//创建一个套接字，封装了错误处理，可用于服务器端和客户端
bool CreateSocket(SOCKET & connectSocket){
	connectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(connectSocket == INVALID_SOCKET){
		cout << "Create socket failed, ";
		cout << "with error code : " << WSAGetLastError() << endl;
		WSACleanup();
		return false;
	}
	return true;
}

//绑定socket与地址端口，封装了错误处理，一般用于服务器端
bool BindSocket(SOCKET & listenSocket){
	//socket地址信息
	SOCKADDR_IN tempSockaddrIn;
	tempSockaddrIn.sin_family = AF_INET;
	tempSockaddrIn.sin_addr.S_un.S_addr = htonl(INADDR_ANY);   //任意地址
  	tempSockaddrIn.sin_port = htons(DEFAULT_PORT);

  	//绑定
  	int bindResult;
	bindResult = bind(listenSocket, (SOCKADDR*)&tempSockaddrIn, sizeof(SOCKADDR));
	if(bindResult == SOCKET_ERROR){
    	cout << "Server bind error, ";
    	cout << "with error code : " << WSAGetLastError() << endl;
    	closesocket(listenSocket);
    	WSACleanup();
    	return false;
    }
    return true;
}

//让socket开始监听，封装了错误处理，一般用于服务器端
bool Listen(SOCKET & listenSocket){
	if(listen(listenSocket, SOMAXCONN)){
    	cout << "Server socket listen failed, ";
    	cout << "with error code : " << WSAGetLastError() << endl;
    	closesocket(listenSocket);
    	WSACleanup();
    	return false;
    }
    return true;
}

//接受客户端的连接请求，封装了错误处理，一般用于服务器端
bool AcceptClientConnection(SOCKET &listenSocket, SOCKET &connectSocket){
	connectSocket = INVALID_SOCKET;
	connectSocket = accept(listenSocket, NULL, NULL);

	if(connectSocket == INVALID_SOCKET){
		cout << "Server accept connection failed : ";
		cout << "with error code : " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	return true;
}

//连接到远程socket，封装了错误处理，一般用于客户端连接服务器
bool ConnectSocket(SOCKET &connectSocket){
	//socket地址信息
	SOCKADDR_IN tempSockaddrIn;
	tempSockaddrIn.sin_family = AF_INET;
	tempSockaddrIn.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");   //本机测试用回调地址
  	tempSockaddrIn.sin_port = htons(DEFAULT_PORT);

	int connectResult = connect(connectSocket, (SOCKADDR*)&tempSockaddrIn, sizeof(SOCKADDR));
	if(connectResult == SOCKET_ERROR){
		cout << "Client connect socket failed, ";
		cout << "with error code : " << WSAGetLastError() << endl;
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
		return false;
	}
	return true;
}

//关闭一个socket
void CloseSocket(SOCKET &socket){
	closesocket(socket);
	socket = INVALID_SOCKET;
}
