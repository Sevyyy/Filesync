#include <iostream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>
#include <io.h>
#include "Communication.h"
#include "HandleRequest.h"

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

int main(){
	//启动服务器
	if(!WSAInitialize()){
		return 0;
	}else{
		cout << "Server started" << endl;
	}

	//监听套接字和与客户端
	SOCKET serverSocket;
	SOCKET clientSocket;

	//一些初始化
	if(!CreateSocket(serverSocket)){
		return 0;
	}else{
		cout << "Server listen socket created" << endl;
	}

	if(!BindSocket(serverSocket)){
		return 0;
	}else{
		cout << "Server listen socket bind" << endl;
	}

	if(!Listen(serverSocket)){
		return 0;
	}else{
		cout << "Server is listening" << endl;
	}

	//客户端在运行时
	map<string, int> serverVersionMap;
	string serverVersionMapFile = "ServerVersion.txt";
	LoadVersionMap(serverVersionMap, serverVersionMapFile);

	//recursively accept request and handle
	while(1){
		//accept
		if(!AcceptClientConnection(serverSocket, clientSocket)){
			cout << "Accept failed, press any key to continue" << endl;
			cin.get();
			cin.get();
			continue;
		}else{
			cout << "Accepted" << endl;
		}

		int requestType;
		requestType = RecvRequest(clientSocket);

		switch(requestType){
			case REQUEST_SYNC:{
				ServerAutoSync(clientSocket);
				break;
			}

			case REQUSET_UPLOAD:{
				SolveFileCommitFromClient(clientSocket, serverVersionMap);
				break;
			}

			case REQUEST_COMMIT:{
				SolveAllCommitFromClient(clientSocket, serverVersionMap);
				break;
			}
			default:{
				break;
			}
		}

		//close socket gracefully
		ShutdownSocket(clientSocket);
	}//end while

	WSAEnd();
	return 0;
}