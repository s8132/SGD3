
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 1313

const int STRLEN = 256;

bool SendData(SOCKET ClientScoket, char* buffer)
{
	send(ClientScoket, buffer, strlen(buffer), 0);
	return true;
}

bool ReceiveData(SOCKET ClientSocket, char* buffer, int size)
{
	int i = recv(ClientSocket, buffer, size, 0);
	buffer[i]='\0';
	return true;
}

void initWinSock() {
    WSADATA wsaData;
    int iResult;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        exit( 1 );
    }

}

SOCKET bindOnAny(unsigned int port) {
    int iResult;
    SOCKET ListenSocket = INVALID_SOCKET;
	sockaddr_in service;
    service.sin_family = AF_INET;
	service.sin_addr.s_addr = 0;//inet_addr("127.0.0.1");
    service.sin_port = htons(port);

    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //----------------------
    // Bind the socket.
    iResult = bind(ListenSocket, (SOCKADDR *) &service, sizeof (service));
    if (iResult == SOCKET_ERROR) {
        wprintf(L"bind failed with error %u\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else
        wprintf(L"bind returned success\n");
	return ListenSocket;
}

int __cdecl main(void) 
{
    int iResult;
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
	SOCKET ClientSocket2 = INVALID_SOCKET;

    int iSendResult;
    char recvbufOne[STRLEN];
	char recvbufTwo[STRLEN];
	char sendbuf[STRLEN];
	char sendbufTwo[STRLEN];
    int recvbuflen = STRLEN;

	initWinSock();
	ListenSocket = bindOnAny(DEFAULT_PORT);    
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
	
	
    // Accept a client socket
		//Client 1
    cout << "I' m waiting for first player" << endl;

	ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

	cout << "I' m waiting for second player" << endl;

	ClientSocket2 = accept(ListenSocket, NULL, NULL);
    if (ClientSocket2 == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }


    // No longer need server socket
    closesocket(ListenSocket);

	SendData(ClientSocket, "w");
	SendData(ClientSocket2, "b");

	bool done = false;
	int tmp=0;
	while(!done){
		
		cout << "\nZaczyna graczI" << endl;
		ReceiveData(ClientSocket, recvbufOne, recvbuflen);
		cout << "GraczI: " << recvbufOne[0] << endl;
		SendData(ClientSocket2, recvbufOne);
		cout << "Wysylam do graczaII, pole graczaI: " << recvbufOne << endl;
		cout << "int otrzymanej wiadomosci: " << atoi(recvbufOne) << endl;

		cout <<"\nKolej graczaII" << endl;
		ReceiveData(ClientSocket2, recvbufTwo, recvbuflen);
		cout <<"GraczII: " << recvbufTwo << endl;
		SendData(ClientSocket, recvbufTwo);
		cout << "Wysylam do graczaI, pole graczaII: " << recvbufTwo << endl;
		cout << "int otrzymanej wiadomosci: " << (int)(recvbufTwo-48) << endl;


		

		/*
		printf ("Move the first player:\n");
		ReceiveData(ClientSocket, recvbufOne, recvbuflen);
		cout << "FirstPlayer> " << recvbufOne << endl;
		
		printf ("Move the second player:\n");
		ReceiveData(ClientSocket2, recvbufTwo, recvbuflen);
		cout << "SecondPlayer> " << recvbufTwo << endl;
		

		if(strcmp(recvbufOne, recvbufTwo)==0){
			if(strcmp(recvbufOne, "f") == 0 || strcmp(recvbufTwo, "f")==0){
				char *sendbuf = "3";
				SendData(ClientSocket, sendbuf);
				SendData(ClientSocket2, sendbuf);
			}else{
				char *sendbuf = "1";
				SendData(ClientSocket, sendbuf);
				SendData(ClientSocket2, sendbuf);
			}
		}else{
			if(strcmp(recvbufOne, "f")==0 || strcmp(recvbufTwo, "u")==0){
				char *sendbuf = "0";
				SendData(ClientSocket, sendbuf);
				char *sendbufTwo = "5";
				SendData(ClientSocket2, sendbufTwo);
			}else{
				char *sendbuf = "5";
				SendData(ClientSocket, sendbuf);
				char *sendbufTwo = "0";
				SendData(ClientSocket2, sendbufTwo);
			}
		}
		*/
		//tmp++;
		//cout << "Server>" << sendbuf << endl;
		//SendData(ClientSocket, sendbuf);
		//SendData(ClientSocket2, sendbuf);
		if(strcmp(recvbufOne, "koniec")==0 || strcmp(recvbufTwo, "koniec")==0){
			done = true;
		}
	}

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

	getch();
    return 0;
}
