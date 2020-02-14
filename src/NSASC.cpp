#include <stdio.h>						//STANDARD INPUT OUTPUT
#include "winsock2.h"					//WINDOWS SOCKETS
#include <chrono>						//CONN TIMEOUT NEEDS TIME LIB
#include <windows.h>
#include "resource.h"					//INCLUDE RESOURCE FILE FOR LINK TO RESOURCE
#include <ws2tcpip.h>					//NEED THIS FOR IP CONVERSION 
#pragma comment(lib, "ws2_32.lib")		//INCLUDE LIBRARY ON COMPILE
#pragma comment(lib, "winmm.lib")
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

sockaddr_in SERV_ADDR;
unsigned short PORT = 65534;  			//DEFAULT PORT
char RCV_BUFF[50];
int SOCK_LEN;
WSADATA wsaData;						//WSADATA OBJECT
SOCKET SOCK;							//CREATE SOCKET
int CONNECTED = 0;
LPCSTR WAVE_DATA;
int delay = 18950;
//#########CHANGE THIS##########
const char SERV_IP[] = "10.20.48.145";
//##############################

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

int recvfromTimeOutUDP(long sec, long usec)
{
    // Setup timeval variable
    struct timeval timeout;
    struct fd_set fds;
    timeout.tv_sec = sec;
    timeout.tv_usec = usec;
    // Setup fd_set structure
    FD_ZERO(&fds);
    FD_SET(SOCK, &fds);
    // -1 error occurred
    // 0 timed out
    // > 0 data ready to be read
    return select(0, &fds, 0, 0, &timeout);
}


//ping server until responce received
int beacon()
{
	while(!CONNECTED)
	{	memset(RCV_BUFF, 0, sizeof(RCV_BUFF));
		printf("BEACON MASTER\n");
		sendto(SOCK, "{-}7", strlen("{-}7"), 0,(sockaddr *)&SERV_ADDR, sizeof(SERV_ADDR));
		Sleep(1000);
		if(recvfromTimeOutUDP(5,0) > 0)
		{
			recvfrom(SOCK, RCV_BUFF, sizeof(RCV_BUFF), 0,(sockaddr *)&SERV_ADDR, &SOCK_LEN);
			RCV_BUFF[strcspn(RCV_BUFF, "\n")] = 0;
			if(strcmp(RCV_BUFF, "{-}7")==0)
			{
				printf("MASTER: %s\n", RCV_BUFF);
				CONNECTED = 1;
			}
			
		}
	}
	return 0;
}

int processCommands()
{
	while(!(strcmp(RCV_BUFF, "EXIT")==0))
	{
		memset(&RCV_BUFF, '\0', sizeof(RCV_BUFF));
		recvfrom(SOCK, RCV_BUFF, sizeof(RCV_BUFF), 0,(sockaddr *)&SERV_ADDR, &SOCK_LEN);
		RCV_BUFF[strcspn(RCV_BUFF, "\n")] = 0;												//REMOVE TRAILING NEW LINE
		printf("MASTER: %s\n", RCV_BUFF);
		if(strcmp(RCV_BUFF, "EXECUTE")==0)
		{
			//DELAY FOR ADVENTURE2 is 18900
			//NEVER DELAY THE RICK ROLL IT IS INEVITABLE
			Sleep(delay);
			PlaySound(WAVE_DATA, GetModuleHandle(NULL), SND_MEMORY | SND_ASYNC);
		}
	}
	printf("EXIT RECIEVED\n");
	PlaySound(NULL, 0, 0);
	CONNECTED = 0;
	return 0;
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
	SERV_ADDR.sin_family = AF_INET;										//USING IP PROTO		
	SERV_ADDR.sin_addr.s_addr = inet_addr(SERV_IP);						//CONVERT IP AND SET AS DEST
	SERV_ADDR.sin_port = htons(PORT);
	SOCK_LEN = sizeof(SERV_ADDR);
	if((SOCK = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)		//TRY CREATING SOCKET
	{
		printf("Could not create socket: %d" , WSAGetLastError());
		WSACleanup();
		exit(1);
	}
	return 0;
}

int main()
{
	WAVE_DATA = loadFromResource();
	PlaySound(WAVE_DATA, GetModuleHandle(NULL), SND_MEMORY | SND_ASYNC);			//PLAY SOUND TO LOAD LIBRARIES FOR MAXIMUM SPEED
	Sleep(100);
	PlaySound(NULL, 0, 0);															//END SOUND
	setupWSA();
	setupSocket();
	Sleep(500);
	while(1){
		beacon();
		processCommands();
	}
	return 0;
}