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

// void SolveAllCommitFromClient(SOCKET &remoteSocket, map<string, int> &version){
// 	//将服务器的版本信息发送给客户端供判断
// 	SendFile(remoteSocket, "ServerVersion.txt");

// 	//对应CompareClientWithServer函数的响应
// 	char fileName[32];
// 	while(recv(remoteSocket, fileName, 32, 0) != 1){
// 		RecvFile(remoteSocket);
// 		//更新版本信息
// 		//不是新文件
// 		if(version.find(fileName) != version.end()){
// 			version[fileName] ++;
// 			SendVersionItem(remoteSocket, fileName, version[fileName]);
// 		}else{      //新文件
// 			version[fileName] = 1;
// 		}
// 	}

// 	//占位，阻断发送端的进一步发送
// 	SendRequset(remoteSocket, REQUEST_NOTHING);

// 	//对应DeleteNotExistOnServer函数的响应
// 	while(RecvRequest(remoteSocket) == REQUEST_DELETE){
// 		recv(remoteSocket, fileName, 32, 0);
// 		remove(fileName);
// 		version.erase(version.find(fileName));
// 	}

// 	//更新版本信息文件（递增的和被删的）
// 	UpdateVersionFile(version, "ServerVersion.txt");

// 	cout << "SolveAllCommitFromClient" << endl;
// 	return;
// }

int main(){
	if(!WSAInitialize()){
		return 0;
	}else{
		cout << "Server started" << endl;
	}

	//define two sockets, one for listen, another for accept
	SOCKET serverSocket;
	SOCKET clientSocket;

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

	//the version map
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