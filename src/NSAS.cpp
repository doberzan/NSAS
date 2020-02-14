#include <stdio.h>						//STANDARD INPUT OUTPUT
#include "winsock2.h"					//WINDOWS SOCKETS
#include <chrono>						//CONN TIMEOUT NEEDS TIME LIB
#include <process.h>					//REQUIRED FOR THREADDING
#include <windows.h>
#include <iostream>
#include "resource.h"			//INCLUDE RESOURCE FILE FOR LINK TO RESOURCE
#pragma comment(lib, "ws2_32.lib")		//INCLUDE LIBRARY ON COMPILE
#pragma comment(lib, "winmm.lib")

using namespace std;
unsigned short PORT = 65534;  			//DEFAULT PORT
int NUMBER_OF_CLIENTS = 0;
const int MAX_CLIENTS = 30;
char RCV_BUFF[500];
int SOCK_LEN;
DWORD server_thread;
WSADATA wsaData;						//WSADATA OBJECT
SOCKET SOCK;							//CREATE SOCKET
sockaddr_in SERV_ADDR;
sockaddr_in CURR_CLIENT;
sockaddr_in CLIENT_SOCKS[MAX_CLIENTS];	//MAX OF 30 CLIENTS
LPCSTR WAVE_DATA;

//####################
//# NSAS SERVER V1.0 #
//####################



//GOOD LORD
char* loadFromResource()
{
	HMODULE hModule = GetModuleHandle(NULL); 										//GET HANDLE TO CURRENT EXECUTABLE
	HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(IDR_WAVE1), "WAVE");		//FIND THE RESOURCE AND RETURN HANDLE
	HGLOBAL resource_mem = LoadResource(NULL,hResource);							//LOAD RESOURCE VIA HANDLE (DOSENT LOAD TO MEMORY)
	LPCSTR lpWavInMemory = (LPCSTR)LockResource(resource_mem);						//CAST TO STRING IDK
	DWORD dwSize = SizeofResource(hModule, hResource);								//GET SIZE OF RESOURCE TO CALC MEMORY SPACE NEEDED TO LOAD
	printf("PAYLOAD SIZE: %d\n", dwSize);											//DEBUG
	char *AUDIO_FILE = new char[dwSize];											//ALLOCATE A HUGE CHAR BUFFER
	memcpy(AUDIO_FILE, lpWavInMemory, dwSize);										//WRITE DATA AT 'lpWavInMemory' address to AUDIO_FILE char buffer
	return AUDIO_FILE;
}





int setupWSA()
{
	int WSA_STATUS = WSAStartup(MAKEWORD(2,2), &wsaData);				//INIT WSA
	if (WSA_STATUS == 10040) {											//DOESNT WORK RN
		printf("WSA Error: Client has sent too much data");
	}else if(WSA_STATUS != 0){
		printf("WSA Critical Error: %d\n", WSA_STATUS);
		WSACleanup();
		exit(1);
	}
	return 0;
}

int setupSocket()
{
	SOCK_LEN = sizeof(SERV_ADDR);
	if((SOCK = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)		//TRY CREATING SOCKET
	{
		printf("Could not create socket: %d" , WSAGetLastError());
		WSACleanup();
		exit(1);
	}
	return 0;
}

int bindServer()
{
	memset(&SERV_ADDR, '\0', sizeof(SERV_ADDR));						//CLEAR MEMORY AT &SERV_ADDR
	SERV_ADDR.sin_family = AF_INET;										//USING IP PROTO
	SERV_ADDR.sin_addr.s_addr = INADDR_ANY;								//BIND TO 0.0.0.0 FOR ALL INTERFACES
	SERV_ADDR.sin_port = htons(PORT);
    if (bind(SOCK, ( sockaddr *)&SERV_ADDR, sizeof(SERV_ADDR)) == SOCKET_ERROR) 		
	{
        printf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(SOCK);
        WSACleanup();
        return 1;
    }
	return 0;
}

int sendData(char buff[])
{
	for(int i = 0; i < NUMBER_OF_CLIENTS; i++)
	{
		sendto(SOCK, buff, 50, 0, (sockaddr *)&CLIENT_SOCKS[i], sizeof(CLIENT_SOCKS[i]));
		printf("COMMAND: %s\n", buff);
	}
	return 0;
}

unsigned int __stdcall listenForClients(void* data)
{
	int DUPE = 0;
	while(1){
		memset(&RCV_BUFF, '\0', sizeof(RCV_BUFF));																		//CLEAR RCV_BUFF
		if (recvfrom(SOCK, RCV_BUFF, sizeof(RCV_BUFF), 0,(sockaddr *)&CURR_CLIENT, &SOCK_LEN) == SOCKET_ERROR) 			//RECEIVE DATA FROM CLIENT
		{
			printf( "Server Listen Critical Error: %ld\n", WSAGetLastError());
			closesocket(SOCK);
			WSACleanup();
			exit(1);
		}
		//CHECK FOR DUPLICATES
		if(NUMBER_OF_CLIENTS != 0){
			for(int i =0; i < NUMBER_OF_CLIENTS; i ++)
			{
				//printf("%s\n",inet_ntoa(CLIENT_SOCKS[i].sin_addr));
				char TEMP1[17];
				char TEMP2[17];
				strcpy(TEMP1, inet_ntoa(CLIENT_SOCKS[i].sin_addr));
				strcpy(TEMP2, inet_ntoa(CURR_CLIENT.sin_addr));
				if(strcmp(TEMP1, TEMP2) == 0) 
				{
					//printf("Client1: %s\nClient2: %s\n", inet_ntoa(CURR_CLIENT.sin_addr), inet_ntoa(CLIENT_SOCKS[i].sin_addr));
					//printf("Dupe Client: %s\n", RCV_BUFF);
					DUPE = 1;
					continue;
				}
			}
		if(DUPE == 1){
			DUPE = 0;
			continue;
		}
		}
		sendto(SOCK, RCV_BUFF, sizeof(RCV_BUFF), 0, (struct sockaddr*) &CURR_CLIENT, sizeof(CURR_CLIENT));					//ACK THE CLIENT WITH A MIRROR RESPONCE
		printf("Number of Clients: %d\n", NUMBER_OF_CLIENTS+1);
		CLIENT_SOCKS[NUMBER_OF_CLIENTS] = CURR_CLIENT;
		NUMBER_OF_CLIENTS ++;
	}
	return 0;
}



int main()
{
	WAVE_DATA = loadFromResource();
	setupWSA();
	setupSocket();
	bindServer();
	HANDLE handle = (HANDLE)_beginthreadex(0,0,&listenForClients, 0, 0,0);				//STARTS SERVER THREAD
	char input[50];
	while(!(strcmp(input, "EXIT")==0))
	{
		memset(&input, '\0', sizeof(input));
		cin.getline(input, sizeof(input));
		if(strcmp(input, "EXECUTE")==0)
		{
			Sleep(10);
			PlaySound(WAVE_DATA, GetModuleHandle(NULL), SND_MEMORY | SND_ASYNC);
			sendData(input);
			
		}else{
			sendData(input);
		}
	} 
	CloseHandle(handle);
	return 0;
}