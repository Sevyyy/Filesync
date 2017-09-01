#include <iostream>
#include <string>
#include <fstream>

#include "Communication.h"
#include "HandleRequest.h"

using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

//客户端打印菜单
void PrintMenu(){
	system("cls");
	cout << "**********************" << endl;
	cout << "*       A Name       *" << endl;
	cout << "*                    *" << endl;
	cout << "*  1. AutoSync       *" << endl;
	cout << "*  2. Commit file    *" << endl;
	cout << "*  3. Commit all     *" << endl;
	cout << "*  0. Quit           *" << endl;
	cout << "*                    *" << endl;
	cout << "**********************" << endl;
	cout << "      Enter : ";
	return;
}

//辅助函数，进行press any key to continue 的逻辑
void PressWait(){
	cout << "Press any key to continue" << endl;
	cin.get();
	cin.get();
	return;
}

//初始化客户端
bool ClientInitialize(SOCKET &serverSocket){
	//设置服务器IP
	string IP;
	cout << "Enetr server ip : ";
	cin >> IP;

	//启动客户端
	if(!WSAInitialize()){
		return false;
	}

	//如果客户端版本信息文件不存在，则创建一个
	fstream fs;
	fs.open(CLIENT_VERSION_FILE, ios::in);
	if(!fs){
		ofstream fout;
		fout.open(CLIENT_VERSION_FILE);
		fout.close();
	}else{
		fs.close();
	}

	//创建服务器套接字
	if(!CreateSocket(serverSocket)){
		cout << "Creat socket failed, ";
		PressWait();
		return false;
	}else{
		cout << "Created socket" << endl;
	}

	//连接服务器
	if(!ConnectSocket(serverSocket, IP)){
		cout << "Connect failed, ";
		PressWait();
		return false;
	}else{
		cout << "Connected" << endl;
	}

	//连接后的自动同步
	ClientAutoSync(serverSocket);
	cout << "Client auto synchronized, ";
	PressWait();

	return true;
}

int main(){
	//连接服务器的套接字
	SOCKET serverSocket;

	if(!ClientInitialize(serverSocket)){
		cout << "Client initialize failed, ";
		PressWait();
		return 0;
	}

	//进入客户端的CUI
	while(1){
		//打印菜单
		PrintMenu();

		//输入选择
		int choice;	
		cin >> choice;
		if(choice < 0 || choice > 3){    //非法输入
			continue;
		}else if(choice == 0){            //退出
			cout << "Byebye" << endl;
			break;
		}

		//创建套接字
		if(!CreateSocket(serverSocket)){
			cout << "Creat socket failed, ";
			PressWait();
			continue;
		}else{
			cout << "Created socket" << endl;
		}

		//连接客户端
		if(!ConnectSocket(serverSocket)){
			cout << "Connect failed, ";
			PressWait();
			continue;
		}else{
			cout << "Connected" << endl;
		}

		switch(choice){
			//同步服务器全局
			case 1:{
				ClientAutoSync(serverSocket);
				PressWait();
				break;
			}

			//上传一个文件
			case 2:{
				string fileName;
				cout << "Enter file name : ";
				cin >> fileName;
				CommitFileToServer(serverSocket, fileName);
				PressWait();
				break;
			}

			//上传全局文件到服务器
			case 3:{
				CommitAllToServer(serverSocket);
				PressWait();
				break;
			}
		}
		
		//优雅地关闭客户端套接字
		ShutdownSocket(serverSocket);

	}//while 循环结束

	WSAEnd();

	return 0;
}