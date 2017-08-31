//包含头文件
#include "Communication.h"
#include "HandleRequest.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <algorithm>
#include <io.h>

using namespace std;

//向remote套接字发送一个文件，封装了错误处理，有FileBlock做校验，可用于服务器端和客户端
bool SendFile(SOCKET &remoteSocket, string fileName){
	//打开文件
	FILE * p_file = fopen(fileName.c_str(), "rb");
	if(p_file == NULL){
		cout << "Open file failed" << endl;
		return false;
	}

	//获取文件长度（大小）
	fseek(p_file, 0, SEEK_END);
	int fileLength = ftell(p_file);
	fseek(p_file, 0, SEEK_SET);

	//创建FileBlock并发送，用于告诉对方这是一个文件，文件大小和文件名
	struct FileBlock fileBlock(fileName.c_str(), fileLength);
	char fileBlockBuffer[sizeof(fileBlock)];
	memcpy(fileBlockBuffer, &fileBlock, sizeof(FileBlock));
	send(remoteSocket, fileBlockBuffer, sizeof(FileBlock), 0);

	//开始分块读文件并分块发送
	int sendCount;
	char sendBuffer[DEFAULT_BUFFER_SIZE];
	while(!feof(p_file)){
		sendCount = fread(sendBuffer, 1, DEFAULT_BUFFER_SIZE, p_file);
		cout << "sendCount : " << sendCount << endl;
		send(remoteSocket, sendBuffer, sendCount, 0);
	}
	cout << "File send successfully" << endl;

	//关闭文件
	fclose(p_file);

	return true;
}

//从remote套接字读取文件，封装了错误处理，可用于服务器端和客户端
bool RecvFile(SOCKET &remoteSocket){
	//接收FileBlock
	struct FileBlock fileBlock;
	char fileBlockBuffer[sizeof(FileBlock)];
	recv(remoteSocket, fileBlockBuffer, sizeof(FileBlock), 0);
	memcpy(&fileBlock, fileBlockBuffer, sizeof(FileBlock));

	//判断是否为文件并获取文件名和文件大小
	int fileLength;
	char fileName[48];
	if(strcmp(fileBlock.key, DEFAULT_FILE_KEY) == 0){
		fileLength = fileBlock.size;
		strcpy(fileName, fileBlock.name);
	}else{
		cout << "Not a file" << endl;
		return false;
	}

	//打开要写的文件
	FILE *p_file = fopen(fileName, "wb");
	if(p_file == NULL){
		cout << "create file failed" << endl;
		return false;
	}

	//分块接收文件数据并写入文件
	int recvCount;
	char recvBuffer[DEFAULT_BUFFER_SIZE];
	int T = fileLength/DEFAULT_BUFFER_SIZE + 1;
	while(T--){
		if(T != 0){    //没到尾
			recvCount = recv(remoteSocket, recvBuffer, DEFAULT_BUFFER_SIZE, 0);
		}else{         //最后一块，不满DEFAULT_BUFFER_SIZE
			recvCount = recv(remoteSocket, recvBuffer, fileLength % DEFAULT_BUFFER_SIZE, 0);
		}
		if(recvCount == -1){
			cout << "Error : " << WSAGetLastError() << endl;
		}
		fwrite(recvBuffer, 1, recvCount, p_file);
	}

	cout << "Receive done!" << endl;

	//关闭文件
	fclose(p_file);

	return true;
}

//发送请求，一般用于客户端向服务器发送请求
void SendRequset(SOCKET &remoteSocket, int requsetType){
	//构建请求头
	struct RequestBlock requestBlock(requsetType);
	char requestBlockBuffer[sizeof(RequestBlock)];
	memcpy(requestBlockBuffer, &requestBlock, sizeof(RequestBlock));
	send(remoteSocket, requestBlockBuffer, sizeof(RequestBlock), 0);
}

int RecvRequest(SOCKET &remoteSocket){
	struct RequestBlock requestBlock;
	char requestBlockBuffer[sizeof(RequestBlock)];
	recv(remoteSocket, requestBlockBuffer, sizeof(RequestBlock), 0);
	memcpy(&requestBlock, requestBlockBuffer, sizeof(RequestBlock));

	if(strcmp(requestBlock.key, DEFAULT_RQST_KEY) == 0){
		return requestBlock.type;
	}else{
		return REQUEST_BAD;
	}
}

//加载版本信息的文件，到一个map里
void LoadVersionMap(map<string, int> &versionMap, string fileName){
	ifstream fin;
	fin.open(fileName.c_str());
	string name;
	int version;
	while(fin >> name){
		fin >> version;
		versionMap[name] = version;
	}
	fin.close();
	return;
}

//同步请求中的服务器端提供的为客户端进行同步的操作
void ServerAutoSync(SOCKET &remoteSocket){
	//将服务器端的版本信息发送到客户端以检验
	string versionFile = "ServerVersion.txt";
	SendFile(remoteSocket, versionFile);

	//循环接受所请求的文件名并发送文件，当请求的大小为1时停止
	//此处是用一个trick做循环停止的
	char fileName[32];
	while(recv(remoteSocket, fileName, 32, 0) != 1){
		printf("%s\n", fileName);
		SendFile(remoteSocket, fileName);
	}

	return;
}

//辅助函数，判断一个文件是否可删，此处硬编码严重，保留代码文件和可执行文件，版本信息等
bool IsFile(string fileName){
	if(fileName == ".")
		return false;
	if(fileName == "..")
		return false;
	if(fileName == "client.exe")
		return false;
	if(fileName == "server.exe")
		return false;
	if(fileName == "ServerVersion.txt")
		return false;
	if(fileName == "ClientVersion.txt")
		return false;
	return true;
}


//辅助函数，用于非上传前的同步操作中，删除服务器上没有但客户端有的文件
void DeleteNotExist(map<string, int> &serverVersionMap){
	//一些数据结构和变量的初始化，为遍历目录准备
	long hFile = 0;
	struct _finddata_t fileInfo;
	string path = ".\\";
	string pathName;
	string exdName;

	//若目录下为空
	if ((hFile = _findfirst(pathName.assign(path).append("\\*").c_str(), &fileInfo)) == -1) {
        return;
    }
    do {  //循环遍历每一个文件，如果服务器上没有的就删除文件
        if(serverVersionMap.find(fileInfo.name) == serverVersionMap.end() && IsFile(fileInfo.name)){
        	remove(fileInfo.name);
        }
    } while (_findnext(hFile, &fileInfo) == 0);

    //关闭目录查找
    _findclose(hFile);

    return;
}


//辅助函数，判断服务器端和客户端文件的版本状态和是否存在，
//并从服务器端请求文件，用于非上传前的同步
void CompareServerWithClient(SOCKET &remoteSocket, map<string, int>& serverVersionMap, map<string, int>& clientVersionMap){
	map<string, int>::iterator it_server;
	//对服务器的每一个文件
	for(it_server = serverVersionMap.begin(); it_server != serverVersionMap.end(); it_server++){
		//如果在客户端存在
		if(clientVersionMap.find(it_server->first) != clientVersionMap.end()){
			//如果客户端的该文件版本号比服务器的低（或不同），请求文件
			if(it_server->second != clientVersionMap[it_server->first]){
				char fileName[32];
				strcpy(fileName, it_server->first.c_str());
				printf("%s\n", fileName);
				send(remoteSocket, fileName, 32 ,0);
				RecvFile(remoteSocket);
			}
		}else{  //如果客户端不存在，像服务器请求
			char fileName[32];
			strcpy(fileName, it_server->first.c_str());
			printf("%s\n", fileName);
			send(remoteSocket, fileName, 32 ,0);
			RecvFile(remoteSocket);
		}
	}

	//发送一字节表示完成，是一个trick
	char end[1];
	send(remoteSocket, end, 1 ,0);

	return;
}


////同步请求中的客户端所进行的操作
void ClientAutoSync(SOCKET &remoteSocket, map<string, int>& clientVersionMap){
	//获取服务器文件的版本信息，放入一个map中
	RecvFile(remoteSocket);
	map<string, int> serverVersionMap;
	string serverVersionMapFile = "ServerVersion.txt";
	LoadVersionMap(serverVersionMap, serverVersionMapFile);
	
	//判断本地哪些文件需要删除
	DeleteNotExist(serverVersionMap);
	
	//比较版本信息，从服务器请求所需文件
	CompareServerWithClient(remoteSocket, serverVersionMap, clientVersionMap);

	//保存最新的版本信息
	remove("ClientVersion.txt");
	rename("ServerVersion.txt" ,"ClientVersion.txt");
	clientVersionMap = serverVersionMap;

	return;
}	

//处理客户端上传单个文件的请求，版本文件硬编码了
void SolveCommitFromClient(SOCKET &remoteSocket, map<string, int> &version){
	//接收versionItem
	struct VersionItem versionItem;
	char versionItemBuffer[sizeof(VersionItem)];
	recv(remoteSocket, versionItemBuffer, sizeof(VersionItem), 0);
	memcpy(&versionItem, versionItemBuffer, sizeof(VersionItem));

	//判断
	if(versionItem.version > 0){           //如果版本大于0，即不为-1（未修改），则可能需要请求上传
		if(versionItem.version == 1){      //新文件version为1
			RecvFile(remoteSocket);
			version[versionItem.name] = 1;
			UpdateVersionFile(version, "ServerVersion.txt");
		}else{                             //非新文件，判断版本是否是在最新版本上的改动
			if(versionItem.version == version[versionItem.name]){     //是的话请求上传
				SendRequset(remoteSocket, REQUEST_FILE);
				RecvFile(remoteSocket);
				version[versionItem.name]++;
				UpdateVersionFile(version, "ServerVersion.txt");
			}else{                       //不需要上传但并且提示客户端进行同步
				SendRequset(remoteSocket, REQUEST_NOTHING);
			}
		}
	}
}

//更新修改时间的文件，用于客户端
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

//加载修改时间的文件，用于客户端
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

//发送版本信息（文件名加版本号）
void SendVersionItem(SOCKET &remoteSocket, string fileName, int version){
	struct VersionItem versionItem(fileName.c_str(), version);
	char versionItemBuffer[sizeof(VersionItem)];
	memcpy(versionItemBuffer, &versionItem, sizeof(VersionItem));
	send(remoteSocket, versionItemBuffer, sizeof(VersionItem), 0);
}

//更新版本文件
void UpdateVersionFile(map<string, int> &version, string fileName){
	ofstream fout;
	fout.open(fileName.c_str());
	map<string, int>::iterator it;
	for(it = version.begin(); it != version.end(); it++){
		fout << it->first << " " << it->second << endl;
	}
	fout.close();
	return;
}

//客户端同步一个文件到服务器，存在硬编码
void CommitFileToServer(SOCKET &remoteSocket, string fileName){
	//加载修改时间和版本信息
	map<string, int> fixTime;
	map<string, int> version;
	LoadVersionMap(version, "ClientVersion.txt");
	LoadFixTimeMap(fixTime);

	//获取文件修改时间
	struct _stat state;
	_stat(fileName.c_str(), &state);

	//判断
	if(fixTime.find(fileName) != fixTime.end()){          //如果不是新文件
		if(fixTime[fileName] == state.st_mtime){         //未修改，不需要上传
			SendVersionItem(remoteSocket, fileName, -1);
			cout << "Never fix, do not need upload" << endl;
		}else{    //修改过，需要上传
			SendVersionItem(remoteSocket, fileName, version[fileName]);
			if(RecvRequest(remoteSocket) == REQUEST_FILE){    //是基于最新版本修改的
				SendFile(remoteSocket, fileName);
				version[fileName]++;
				UpdateVersionFile(version, "ClientVersion.txt");
				SaveFixTime(version);
			}else{
				cout << "Need Sync first" << endl;
			}
		}
	}else{ //新文件，直接上传
		SendVersionItem(remoteSocket, fileName, 1);
		SendFile(remoteSocket, fileName);
		version[fileName] = 1;
		UpdateVersionFile(version, "ClientVersion.txt");
		SaveFixTime(version);
	}
	
	return;
}