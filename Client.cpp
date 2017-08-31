#include <iostream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <io.h>
#include "Communication.h"
#include "HandleRequest.h"

using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

void PrintMenu(){
	system("cls");
	cout << "**********************" << endl;
	cout << "*       A Name       *" << endl;
	cout << "*                    *" << endl;
	cout << "*  1. AutoSync       *" << endl;
	cout << "*  2. Commit file    *" << endl;
	cout << "*  3. Commit all     *" << endl;
	cout << "*  0. quit           *" << endl;
	cout << "*                    *" << endl;
	cout << "**********************" << endl;
	cout << "      Enter : ";
	return;
}

void PressWait(){
	cout << "Press any key to continue" << endl;
	cin.get();
	cin.get();
	return;
}


// //辅助函数，用于上传客户端全局时对比客户端和服务器文件状态
// void CompareClientWithServer(SOCKET &remoteSocket, map<string, int> &serverVersion, map<string, int> &clientVersion){
// 	//一些数据结构和变量的初始化，为遍历目录准备
// 	long hFile = 0;
// 	struct _finddata_t fileInfo;
// 	string path = ".\\";
// 	string pathName;
// 	string exdName;

// 	//加载本地修改时间信息
// 	map<string, int> clientFixTime;
// 	LoadFixTimeMap(clientFixTime);

// 	//若目录下为空
// 	if ((hFile = _findfirst(pathName.assign(path).append("\\*").c_str(), &fileInfo)) == -1) {
//         return;
//     }

//     do {  //循环client的遍历每一个文件，文件名在fileInfo.name中

//     	//获取文件修改时间
//     	struct _stat state;
//     	_stat(fileInfo.name, &state);

//     	//若服务器上有
//         if(serverVersion.find(fileInfo.name) != serverVersion.end()){
//         	//版本和服务器相当且未改过，不上传
//         	if(clientVersion[fileInfo.name] == serverVersion[fileInfo.name]
//         	&& clientFixTime[fileInfo.name] == state.st_mtime){
//         		//cout << "Need not to upload" << endl;
//         	}else{    //否则上传
//         		char fileName[32];
//         		strcpy(fileName, fileInfo.name);
//         		send(remoteSocket, fileName, 32, 0);
//         		SendFile(remoteSocket, fileInfo.name);
//         		//接收返回的版本号
//         		struct VersionItem versionItem;
//         		RecvVersionItem(remoteSocket, versionItem);
//         		clientVersion[fileInfo.name] = versionItem.version;
//         	}
//         }else if(IsFile(fileInfo.name)){   //服务器上没有的，为新文件
//         	char fileName[32];
//     		strcpy(fileName, fileInfo.name);
//     		send(remoteSocket, fileName, 32, 0);
//         	SendFile(remoteSocket, fileInfo.name);
//         	clientVersion[fileInfo.name] = 1;
//         }
//     } while (_findnext(hFile, &fileInfo) == 0);

//     //一个用来停止发送的trick
//     char end[1];
//     send(remoteSocket, end, 1, 0);

//     //占位，先不继续发，收到再发，确保上面的trick在接收端能正常接收
//     RecvRequest(remoteSocket);

//     //关闭目录查找
//     _findclose(hFile);

//     return;
// }

// //辅助函数，用于删除客户端删除了但服务器有的文件
// void DeleteNotExistOnServer(SOCKET &remoteSocket, map<string, int> serverVersion, map<string, int> &clientVersion){
// 	map<string, int>::iterator it;
// 	struct _stat state;
// 	for(it = serverVersion.begin(); it != serverVersion.end(); it++){
// 		//判断文件在客户端是否存在，不存在则发送删除请求和文件名
// 		if(_stat(it->first.c_str(), &state) != 0){
// 			SendRequset(remoteSocket, REQUEST_DELETE);
// 			char fileName[32];
// 			strcpy(fileName, it->first.c_str());
// 			send(remoteSocket, fileName, 32, 0);
// 			clientVersion.erase(clientVersion.find(fileName));
// 		}
// 	}
// 	//删除请求结束，用于判断
// 	SendRequset(remoteSocket, REQUEST_NOTHING);

// 	return;
// }

// void CommitAllToServer(SOCKET &remoteSocket){
// 	SendRequset(remoteSocket, REQUEST_COMMIT);

// 	map<string, int> serverVersion;
// 	map<string, int> clientVersion;
// 	RecvFile(remoteSocket);
// 	LoadVersionMap(serverVersion, "ServerVersion.txt");
// 	LoadVersionMap(clientVersion, "ClientVersion.txt");

// 	//对比客户端和服务器文件状态
// 	CompareClientWithServer(remoteSocket, serverVersion, clientVersion);

// 	//删除客户端删除了但服务器有的文件
// 	DeleteNotExistOnServer(remoteSocket, serverVersion, clientVersion);

// 	//更新本地版本信息和修改时间信息
//     UpdateVersionFile(clientVersion, "ClientVersion.txt");
//     UpdateFixTime(clientVersion);

//     remove("ServerVersion.txt");
// 	return;
// }






int main(){
	if(!WSAInitialize()){
		return 0;
	}

	//socket for server
	SOCKET serverSocket;

	if(!CreateSocket(serverSocket)){
		cout << "Creat socket failed, press any key to continue" << endl;
		cin.get();
		cin.get();
		return 0;
	}else{
		cout << "Created socket" << endl;
	}

	//connect
	if(!ConnectSocket(serverSocket)){
		cout << "Connect failed, press any key to continue" << endl;
		cin.get();
		cin.get();
		return 0;
	}else{
		cout << "Connected" << endl;
	}

	//auto sync when client start up
	ClientAutoSync(serverSocket);
	cout << "Client auto synchronized, press any key to continue" << endl;
	cin.get();

	//CUI
	while(1){
		PrintMenu();

		int choice;
		cin >> choice;
		if(choice < 0 || choice > 3){    //invalid input
			continue;
		}else if(choice == 0){
			cout << "Byebye" << endl;
			break;
		}

		cout << endl;

		//create
		if(!CreateSocket(serverSocket)){
			cout << "Creat socket failed, ";
			PressWait();
			continue;
		}else{
			cout << "Created socket" << endl;
		}

		//connect
		if(!ConnectSocket(serverSocket)){
			cout << "Connect failed, ";
			PressWait();
			continue;
		}else{
			cout << "Connected" << endl;
		}

		switch(choice){
			case 1:{
				ClientAutoSync(serverSocket);
				PressWait();
				break;
			}

			case 2:{
				string fileName;
				cout << "Enter file name : ";
				cin >> fileName;
				CommitFileToServer(serverSocket, fileName);
				PressWait();
				break;
			}

			case 3:{
				CommitAllToServer(serverSocket);
				PressWait();
				break;
			}
		}
		
		//close socket gracefully
		ShutdownSocket(serverSocket);

	}//end while

	WSAEnd();

	return 0;
}