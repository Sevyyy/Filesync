//包含头文件
#include "HandleRequest.h"

#include <io.h>

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <map>
#include <algorithm>

#include "Communication.h"
using namespace std;

//向remote套接字发送一个文件，封装了错误处理，有FileBlock做校验，可用于服务器端和客户端
bool SendFile(SOCKET &remoteSocket, string fileName){
	//打开文件
	FILE * p_file = fopen(fileName.c_str(), "rb");
	if(p_file == NULL){
		cout << "Open file failed : " << fileName << endl;
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
		send(remoteSocket, sendBuffer, sendCount, 0);
	}

	//关闭文件
	fclose(p_file);

	return true;
}

//从remote套接字读取文件，封装了错误处理，可用于服务器端和客户端
bool RecvFile(SOCKET &remoteSocket){
	//接收FileBlock
	struct FileBlock fileBlock;
	char fileBlockBuffer[sizeof(FileBlock)];
	int temp = recv(remoteSocket, fileBlockBuffer, sizeof(FileBlock), 0);
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
		cout << "Create file failed : " << fileName << endl;
		return false;
	}

	//分块接收文件数据并写入文件
	int recvCount;
	char recvBuffer[DEFAULT_BUFFER_SIZE];
	int rest = fileLength;
	while(rest > 0){
		recvCount = recv(remoteSocket, recvBuffer, min(rest, DEFAULT_BUFFER_SIZE), 0);
		fwrite(recvBuffer, 1, recvCount, p_file);
		rest -= recvCount;
	}

	//关闭文件
	fclose(p_file);

	return true;
}

//发送版本信息（文件名加版本号）
void SendVersionItem(SOCKET &remoteSocket, string fileName, int version){
	struct VersionItem versionItem(fileName.c_str(), version);
	char versionItemBuffer[sizeof(VersionItem)];
	memcpy(versionItemBuffer, &versionItem, sizeof(VersionItem));
	send(remoteSocket, versionItemBuffer, sizeof(VersionItem), 0);
}

//接收版本信息
void RecvVersionItem(SOCKET &remoteSocket, struct VersionItem &versionItem){
	char versionItemBuffer[sizeof(VersionItem)];
	recv(remoteSocket, versionItemBuffer, sizeof(VersionItem), 0);
	memcpy(&versionItem, versionItemBuffer, sizeof(VersionItem));
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

//更新修改时间的文件，用于客户端
void UpdateFixTime(map<string, int> &versionMap){
	ofstream fout;
	fout.open(CLIENT_FIX_TIME_FILE);
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
	fin.open(CLIENT_FIX_TIME_FILE);
	string name;
	int time;
	while(fin >> name){
		fin >> time;
		fixTime[name] = time;
	}
	fin.close();
	return;
}

//同步请求中的服务器端提供的为客户端进行同步的操作
void ServerAutoSync(SOCKET &remoteSocket){
	//将服务器端的版本信息发送到客户端以检验
	SendFile(remoteSocket, SERVER_VERSION_FILE);

	//循环接受所请求的文件名并发送文件，当请求的大小为1时停止
	//此处是用一个trick做循环停止的
	char fileName[32];
	while(recv(remoteSocket, fileName, 32, 0) != 1){
		SendFile(remoteSocket, fileName);
	}

	//占位，阻断发送端的进一步发送
	SendRequset(remoteSocket, REQUEST_NOTHING);

	cout << "ServerAutoSync" << endl;
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
	if(fileName == SERVER_VERSION_FILE)
		return false;
	if(fileName == CLIENT_VERSION_FILE)
		return false;
	if(fileName == CLIENT_FIX_TIME_FILE)
		return false;
	return true;
}


//辅助函数，用于非上传前的同步操作中，删除服务器上没有但客户端有的文件
void DeleteNotExistOnClient(map<string, int> &serverVersionMap){
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


//辅助函数，判断服务器端和客户端文件的版本状态和是否存在，并从服务器端请求文件，用于非上传前的同步
void CompareServerWithClient(SOCKET &remoteSocket, map<string, int>& serverVersionMap, map<string, int>& clientVersionMap){
	//读取修改时间信息	
	map<string, int> fixTime;
	LoadFixTimeMap(fixTime);

	map<string, int>::iterator it;
	//对服务器的每一个文件
	for(it = serverVersionMap.begin(); it != serverVersionMap.end(); it++){
		//如果在客户端存在
		if(clientVersionMap.find(it->first) != clientVersionMap.end()){
			//如果客户端的该文件版本号比服务器的低（或不同），请求文件
			if(it->second != clientVersionMap[it->first]){
				char fileName[32];
				strcpy(fileName, it->first.c_str());
				send(remoteSocket, fileName, 32 ,0);
				RecvFile(remoteSocket);
			}else{      //版本号一样
				//获取修改时间
				struct _stat state;
				_stat(it->first.c_str(), &state);
				//修改过，请求文件
				if(fixTime[it->first] != state.st_mtime){
					char fileName[32];
					strcpy(fileName, it->first.c_str());
					send(remoteSocket, fileName, 32 ,0);
					RecvFile(remoteSocket);
				}
			}
		}else{  //如果客户端不存在，像服务器请求
			char fileName[32];
			strcpy(fileName, it->first.c_str());
			send(remoteSocket, fileName, 32 ,0);
			RecvFile(remoteSocket);
		}
	}

	//发送一字节表示完成，是一个trick
	char end[1];
	send(remoteSocket, end, 1 ,0);

	//占位，先不继续发，收到再发，确保上面的trick在接收端能正常接收
    RecvRequest(remoteSocket);

	return;
}


////同步请求中的客户端所进行的操作
void ClientAutoSync(SOCKET &remoteSocket){
	//读取客户端版本信息
	map<string, int> clientVersion;
	LoadVersionMap(clientVersion, CLIENT_VERSION_FILE);

	//发送同步请求给服务器
	SendRequset(remoteSocket, REQUEST_SYNC);
	//获取服务器文件的版本信息，放入一个map中
	RecvFile(remoteSocket);
	map<string, int> serverVersion;
	LoadVersionMap(serverVersion, SERVER_VERSION_FILE);
	
	//判断本地哪些文件需要删除
	DeleteNotExistOnClient(serverVersion);
	
	//比较版本信息，从服务器请求所需文件
	CompareServerWithClient(remoteSocket, serverVersion, clientVersion);

	//保存最新的版本和修改时间信息
	remove(CLIENT_VERSION_FILE);
	rename(SERVER_VERSION_FILE ,CLIENT_VERSION_FILE);
	clientVersion = serverVersion;
	UpdateFixTime(clientVersion);

	cout << "Client synchronized!" << endl;
	return;
}	

//处理客户端上传单个文件的请求，版本文件硬编码了
void SolveFileCommitFromClient(SOCKET &remoteSocket, map<string, int> &version){
	//接收versionItem
	struct VersionItem versionItem;
	RecvVersionItem(remoteSocket, versionItem);

	//判断
	if(versionItem.version >= 0){           //如果版本大于0，即不为-1（未修改），则可能需要请求上传
		if(versionItem.version == 0){      //新文件version为0
			RecvFile(remoteSocket);
			version[versionItem.name] = 1;
			UpdateVersionFile(version, SERVER_VERSION_FILE);
		}else{                             //非新文件
			RecvFile(remoteSocket);
			version[versionItem.name]++;
			//返回服务器版本号让客户端更新
			SendVersionItem(remoteSocket, versionItem.name, version[versionItem.name]);
			UpdateVersionFile(version, SERVER_VERSION_FILE);
		}
	}
	cout << "SolveFileCommitFromClient" << endl;
}



//客户端同步一个文件到服务器，存在硬编码
void CommitFileToServer(SOCKET &remoteSocket, string fileName){
	//加载修改时间和版本信息
	map<string, int> fixTime;
	map<string, int> version;
	LoadVersionMap(version, CLIENT_VERSION_FILE);
	LoadFixTimeMap(fixTime);

	//判断文件是否存在并获取文件修改时间
	struct _stat state;
	if(_stat(fileName.c_str(), &state) != 0){
		cout << "File Not Exist" << endl;
		return;
	}

	SendRequset(remoteSocket, REQUSET_UPLOAD);

	//判断
	if(fixTime.find(fileName) != fixTime.end()){          //如果不是新文件
		if(fixTime[fileName] == state.st_mtime){         //未修改，不需要上传
			SendVersionItem(remoteSocket, fileName, -1);
			cout << "File never fixed, do not need uploading" << endl;
		}else{    //修改过，需要上传
			SendVersionItem(remoteSocket, fileName, version[fileName]);
			SendFile(remoteSocket, fileName);
			//接收返回的版本号
			struct VersionItem versionItem;
			RecvVersionItem(remoteSocket, versionItem);
			version[fileName] = versionItem.version;
		}
	}else{ //新文件，直接上传
		SendVersionItem(remoteSocket, fileName, 0);
		SendFile(remoteSocket, fileName);
		version[fileName] = 1;
	}

	//更新版本和修改信息
	UpdateVersionFile(version, CLIENT_VERSION_FILE);
	UpdateFixTime(version);

	cout << "Commit complited!" << endl;
	
	return;
}

//服务器端用于处理客户端上传全局文件的请求
void SolveAllCommitFromClient(SOCKET &remoteSocket, map<string, int> &version){
	//将服务器的版本信息发送给客户端供判断
	SendFile(remoteSocket, SERVER_VERSION_FILE);

	//对应CompareClientWithServer函数的响应
	char fileName[32];
	while(recv(remoteSocket, fileName, 32, 0) != 1){
		RecvFile(remoteSocket);
		//更新版本信息
		//不是新文件
		if(version.find(fileName) != version.end()){
			version[fileName] ++;
			SendVersionItem(remoteSocket, fileName, version[fileName]);
		}else{      //新文件
			version[fileName] = 1;
		}
	}

	//占位，阻断发送端的进一步发送
	SendRequset(remoteSocket, REQUEST_NOTHING);

	//对应DeleteNotExistOnServer函数的响应
	while(RecvRequest(remoteSocket) == REQUEST_DELETE){
		recv(remoteSocket, fileName, 32, 0);
		remove(fileName);
		version.erase(version.find(fileName));
	}

	//更新版本信息文件（递增的和被删的）
	UpdateVersionFile(version, SERVER_VERSION_FILE);

	cout << "SolveAllCommitFromClient" << endl;
	return;
}

//辅助函数，用于上传客户端全局时对比客户端和服务器文件状态
void CompareClientWithServer(SOCKET &remoteSocket, map<string, int> &serverVersion, map<string, int> &clientVersion){
	//一些数据结构和变量的初始化，为遍历目录准备
	long hFile = 0;
	struct _finddata_t fileInfo;
	string path = ".\\";
	string pathName;
	string exdName;

	//加载本地修改时间信息
	map<string, int> clientFixTime;
	LoadFixTimeMap(clientFixTime);

	//若目录下为空
	if ((hFile = _findfirst(pathName.assign(path).append("\\*").c_str(), &fileInfo)) == -1) {
        return;
    }

    do {  //循环client的遍历每一个文件，文件名在fileInfo.name中

    	//获取文件修改时间
    	struct _stat state;
    	_stat(fileInfo.name, &state);

    	//若服务器上有
        if(serverVersion.find(fileInfo.name) != serverVersion.end()){
        	//版本和服务器相当且未改过，不上传
        	if(clientVersion[fileInfo.name] == serverVersion[fileInfo.name]
        	&& clientFixTime[fileInfo.name] == state.st_mtime){
        		//无操作
        	}else{    //否则上传
        		char fileName[32];
        		strcpy(fileName, fileInfo.name);
        		send(remoteSocket, fileName, 32, 0);
        		SendFile(remoteSocket, fileInfo.name);
        		//接收返回的版本号
        		struct VersionItem versionItem;
        		RecvVersionItem(remoteSocket, versionItem);
        		clientVersion[fileInfo.name] = versionItem.version;
        	}
        }else if(IsFile(fileInfo.name)){   //服务器上没有的，为新文件
        	char fileName[32];
    		strcpy(fileName, fileInfo.name);
    		send(remoteSocket, fileName, 32, 0);
        	SendFile(remoteSocket, fileInfo.name);
        	clientVersion[fileInfo.name] = 1;
        }
    } while (_findnext(hFile, &fileInfo) == 0);

    //一个用来停止发送的trick
    char end[1];
    send(remoteSocket, end, 1, 0);

    //占位，先不继续发，收到再发，确保上面的trick在接收端能正常接收
    RecvRequest(remoteSocket);

    //关闭目录查找
    _findclose(hFile);

    return;
}

//辅助函数，用于删除客户端删除了但服务器有的文件
void DeleteNotExistOnServer(SOCKET &remoteSocket, map<string, int> serverVersion, map<string, int> &clientVersion){
	map<string, int>::iterator it;
	struct _stat state;
	for(it = serverVersion.begin(); it != serverVersion.end(); it++){
		//判断文件在客户端是否存在，不存在则发送删除请求和文件名
		if(_stat(it->first.c_str(), &state) != 0){
			SendRequset(remoteSocket, REQUEST_DELETE);
			char fileName[32];
			strcpy(fileName, it->first.c_str());
			send(remoteSocket, fileName, 32, 0);
			clientVersion.erase(clientVersion.find(fileName));
		}
	}
	//删除请求结束，用于判断
	SendRequset(remoteSocket, REQUEST_NOTHING);

	return;
}

//用于客户端请求同步全局文件到服务器端
void CommitAllToServer(SOCKET &remoteSocket){
	SendRequset(remoteSocket, REQUEST_COMMIT);

	map<string, int> serverVersion;
	map<string, int> clientVersion;
	RecvFile(remoteSocket);
	LoadVersionMap(serverVersion, SERVER_VERSION_FILE);
	LoadVersionMap(clientVersion, CLIENT_VERSION_FILE);

	//对比客户端和服务器文件状态
	CompareClientWithServer(remoteSocket, serverVersion, clientVersion);

	//删除客户端删除了但服务器有的文件
	DeleteNotExistOnServer(remoteSocket, serverVersion, clientVersion);

	//更新本地版本信息和修改时间信息
    UpdateVersionFile(clientVersion, CLIENT_VERSION_FILE);
    UpdateFixTime(clientVersion);

    remove(SERVER_VERSION_FILE);
	return;
}