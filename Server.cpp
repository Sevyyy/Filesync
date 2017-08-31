#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "Communication.h"
#include "HandleRequest.h"

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

//初始化服务器
bool ServerInitialize(SOCKET &serverSocket){
	//启动服务器
	if(!WSAInitialize()){
		return false;
	}else{
		cout << "Server started" << endl;
	}

	//如果没有版本信息文件则新建一个
	fstream fs;
	fs.open(SERVER_VERISON_FILE, ios::in);
	if(!fs){
		ofstream fout;
		fout.open(SERVER_VERISON_FILE);
		fout.close();
	}else{
		fs.close();
	}

	//一些初始化
	//创建套接字
	if(!CreateSocket(serverSocket)){
		return false;
	}else{
		cout << "Server listen socket created" << endl;
	}

	//绑定监听套接字
	if(!BindSocket(serverSocket)){
		return false;
	}else{
		cout << "Server listen socket bind" << endl;
	}

	//开始监听
	if(!Listen(serverSocket)){
		return false;
	}else{
		cout << "Server is listening" << endl;
	}

	return true;
}

int main(){
	//监听套接字和与客户端套接字
	SOCKET serverSocket;
	SOCKET clientSocket;

	//初始化服务器端
	if(!ServerInitialize(serverSocket)){
		cout << "ServerInitialize failed" << endl;
		return 0;
	}

	//客户端在运行时，维护一个文件版本信息的map
	map<string, int> serverVersionMap;
	string serverVersionMapFile = "ServerVersion.txt";
	LoadVersionMap(serverVersionMap, serverVersionMapFile);

	//进入循环->接收连接->接受请求->处理请求->断开连接->进行下一次迭代
	while(1){
		//接收客户端的连接请求
		if(!AcceptClientConnection(serverSocket, clientSocket)){
			cout << "Accept failed, press any key to continue" << endl;
			cin.get();
			cin.get();
			continue;
		}else{
			cout << "Accepted" << endl;
		}

		//接收客户端的请求类型
		int requestType;
		requestType = RecvRequest(clientSocket);

		//处理请求
		switch(requestType){
			//同步服务器全局
			case REQUEST_SYNC:{
				ServerAutoSync(clientSocket);
				break;
			}

			//上传一个文件
			case REQUSET_UPLOAD:{
				SolveFileCommitFromClient(clientSocket, serverVersionMap);
				break;
			}

			//上传全局文件到服务器
			case REQUEST_COMMIT:{
				SolveAllCommitFromClient(clientSocket, serverVersionMap);
				break;
			}
			default:{
				break;
			}
		}

		//关闭连接客户端的套接字
		ShutdownSocket(clientSocket);

	}//while循环结束

	WSAEnd();

	return 0;
}