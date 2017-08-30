#include <iostream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdio>
#include "Communication.h"
#include "HandleRequest.h"

using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

	

int main(){
	if(!WSAInitialize()){
		return 0;
	}

	SOCKET serverSocket;
	map<string, int> clientVersionMap;
	string clientVersionMapFile = "ClientVersion.txt";
	LoadVersionMap(clientVersionMap, clientVersionMapFile);

	if(!CreateSocket(serverSocket)){
		return 0;
	}
	if(!ConnectSocket(serverSocket)){
		return 0;
	}else{
		cout << "Connected" << endl;
	}

	ClientAutoSync(serverSocket, clientVersionMap);

	// int temp;
	// while(cin >> temp){
	// 	if(!Create(serverSocket)){
	// 		return 0;
	// 	}
	// 	if(!Connect(serverSocket)){
	// 		break;
	// 	}else{
	// 		cout << "Connected" << endl;
	// 	}

	// 	SendRequset(serverSocket, temp);
	// 	shutdown(serverSocket, SD_BOTH);
	// }

	// char temp[20];
	// recv(serverSocket, temp, 20, 0);
	// printf("%s\n", temp);

	
	// RecvFile(serverSocket);
	// RecvFile(serverSocket);
	// RecvFile(serverSocket);

	//CloseSocket(serverSocket);
	WSAEnd();

	return 0;
}