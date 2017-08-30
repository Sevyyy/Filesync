#include <iostream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include "Communication.h"
#include "HandleRequest.h"

using namespace std;

#define REQUEST_NOTHING 20
#define REQUEST_FILE 21

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

struct VersionItem{
	char name[28];
	int version;
	VersionItem(){}
	VersionItem(const char *_name, int _version):version(_version){
		copy(_name, _name+28, name);
	}
};

void SaveFixTime(map<string, int> &versionMap){
	ofstream fout;
	fout.open("ClientFixTime.txt");
	map<string, int>::iterator it;
	struct _stat state;
	for(it = versionMap.begin(); it != versionMap.end(); it++){
		_stat(it->first.c_str(), &state);
		fout << it->first << " " << state.st_mtime << endl;
	}
	fout.close();
	return;
}

void LoadFixTimeMap(map<string, int> &fixTime){
	ifstream fin;
	fin.open("ClientFixTime.txt");
	string name;
	int time;
	while(fin >> name){
		fin >> time;
		fixTime[name] = time;
	}
	fin.close();
	return;
}

void CommitFileToServer(SOCKET &remoteSocket, string fileName){
	map<string, int> fixTime;
	map<string, int> version;
	LoadVersionMap(version, "ClientVersion.txt");
	LoadFixTimeMap(fixTime);
	struct _stat state;
	_stat(fileName.c_str(), &state);
	if(fixTime.find(fileName) != fixTime.end()){
		if(fixTime[fileName] == state.st_mtime){
			struct VersionItem versionItem(fileName.c_str(), -1);
			char versionItemBuffer[sizeof(VersionItem)];
			memcpy(versionItemBuffer, &versionItem, sizeof(VersionItem));
			send(remoteSocket, versionItemBuffer, sizeof(VersionItem), 0);
			cout << "Never fix, do not need upload" << endl;
		}else{   //fixed file
			struct VersionItem versionItem(fileName.c_str(), version[fileName]);
			char versionItemBuffer[sizeof(VersionItem)];
			memcpy(versionItemBuffer, &versionItem, sizeof(VersionItem));
			send(remoteSocket, versionItemBuffer, sizeof(VersionItem), 0);
			if(RecvRequest(remoteSocket) == REQUEST_FILE){
				SendFile(remoteSocket, fileName);
			}
		}
	}else{ //new file
		struct VersionItem versionItem(fileName.c_str(), 1);
		char versionItemBuffer[sizeof(VersionItem)];
		memcpy(versionItemBuffer, &versionItem, sizeof(VersionItem));
		send(remoteSocket, versionItemBuffer, sizeof(VersionItem), 0);
		SendFile(remoteSocket, fileName);
	}
	return;
}


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
	SaveFixTime(clientVersionMap);


	int temp;
	cin >> temp;

	CommitFileToServer(serverSocket, "newfile.txt");

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