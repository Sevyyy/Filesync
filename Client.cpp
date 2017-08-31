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