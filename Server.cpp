#include <iostream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>
#include "Communication.h"
#include "HandleRequest.h"

using namespace std;

#define REQUEST_NOTHING 20
#define REQUEST_FILE 21

#pragma comment (lib, "Ws2_32.lib")

struct VersionItem{
	char name[28];
	int version;
	VersionItem(){}
	VersionItem(const char *_name, int _version):version(_version){
		copy(_name, _name+28, name);
	}
};

void UpdateVersionFile(map<string, int> &version){
	ofstream fout;
	fout.open("ServerVersion.txt");
	map<string, int>::iterator it;
	for(it = version.begin(); it != version.end(); it++){
		fout << it->first << " " << it->second << endl;
	}
	fout.close();
	return;
}

void SolveCommitFromClient(SOCKET &remoteSocket, map<string, int> &version){
	struct VersionItem versionItem;
	char versionItemBuffer[sizeof(VersionItem)];
	recv(remoteSocket, versionItemBuffer, sizeof(VersionItem), 0);
	memcpy(&versionItem, versionItemBuffer, sizeof(VersionItem));

	if(versionItem.version > 0){
		if(versionItem.version == 1){    //new file
			RecvFile(remoteSocket);
			version[versionItem.name] = 1;
			UpdateVersionFile(version);
		}else{
			if(versionItem.version == version[versionItem.name]){
				SendRequset(remoteSocket, REQUEST_FILE);
				RecvFile(remoteSocket);
				version[versionItem.name]++;
				UpdateVersionFile(version);
			}else{         //need not upload but sync
				SendRequset(remoteSocket, REQUEST_NOTHING);
			}
		}
	}
}

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

	if(!AcceptClientConnection(serverSocket, clientSocket)){
		return 0;
	}else{
		cout << "Accepted" << endl;
	}

	ServerAutoSync(clientSocket);

	SolveCommitFromClient(clientSocket, serverVersionMap);


	// while(1){
	// 	if(!AcceptClientConnection(serverSocket, clientSocket)){
	// 		break;
	// 	}else{
	// 		cout << "Accepted" << endl;
	// 	}

	// 	cout << RecvRequest(clientSocket) << endl;
	// 	shutdown(clientSocket, SD_BOTH);
	// 	clientSocket = INVALID_SOCKET;
	// }




	// char temp[20] = "HelloWorld";
	// send(clientSocket, temp, 20, 0);
	// printf("%s\n", temp);

	// string file1 = "file1.txt";
	// string file2 = "file2.txt";
	// string file3 = "file3.txt";
	// SendFile(clientSocket, file1);
	// SendFile(clientSocket, file2);
	// SendFile(clientSocket, file3);

	// CloseSocket(clientSocket);

	WSAEnd();
	return 0;
}