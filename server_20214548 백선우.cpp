#include "../common.h"
#include <process.h>

#define SERVERPORT 9000
#define BUF_SIZE    512
#define CLIENT 100

unsigned WINAPI recv_Client(void* arg);	//������ �Լ�
void SendMsg(char* msg, int len, SOCKET exclude);	// Ŭ���̾�Ʈ�� �޼��� ����
void ReceiveFile(SOCKET clientSock, const char* filename); // ���� ����

int clientCount = 0;
SOCKET clientSocket[CLIENT];	//Ŭ���̾�Ʈ ���� 
HANDLE hMutex;

char addr[INET_ADDRSTRLEN] = { 0, };

int main(int argc, char* argv[])
{
	SOCKADDR_IN serveraddr, clientaddr;
	SOCKET serversock, clientsock;

	int retval = -1;
	int clientaddrsize;


	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
		return 1;
	printf("[TCP Server] WSAStartup Success.\n");

	// ���ؽ� ����
	hMutex = CreateMutex(NULL, FALSE, NULL);


	//	create socket
	serversock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == serversock)
		err_quit("socket()");

	// ���� �ּ� ����ü �ʱ�ȭ
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET; //IPv4
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	// bind()  : ���� ��Ʈ��ũ �ּҿ� ���� ����
	retval = bind(serversock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");
	printf("[TCP Server] bind() Success.\n");

	// listen() : ���� ��û ��� ���·� ���� ����
	retval = listen(serversock, SOMAXCONN);
	if (SOCKET_ERROR == retval)
		err_quit("listen()");
	printf("[TCP Server] listen() Success.\n");



	while (1)
	{
		clientaddrsize = sizeof(clientaddr);
		clientsock = accept(serversock, (SOCKADDR*)&clientaddr, &clientaddrsize);//�������� ���޵� Ŭ���̾�Ʈ ������ clientSock�� ����

		WaitForSingleObject(hMutex, INFINITE);		// ���ؽ� ����
		clientSocket[clientCount++] = clientsock;		// Ŭ���̾�Ʈ ���Ͽ� ���� �ּ� ����
		ReleaseMutex(hMutex);		 // ReleaseMutex : ���ؽ� ����

		//HandleClient ���� , beginthreadex : ������ ���� �Լ�
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, recv_Client, (void*)&clientsock, 0, NULL);

		// Ŭ���̾�Ʈ ip, ��Ʈ ���
		printf("\n[TCP SERVER] Client IP : %s, Port = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}


	// ����
	closesocket(serversock);
	WSACleanup();

	return 0;
}

unsigned WINAPI recv_Client(void* arg) {
	SOCKET clientSock = *((SOCKET*)arg);			// Ŭ���̾�Ʈ ������ ����
	int retval = 0;
	char msg[BUF_SIZE+1];
	char filename[BUF_SIZE];

	// recv()
	while ((retval = recv(clientSock, msg, BUF_SIZE, 0)) != 0) {
		char filename[BUF_SIZE];
		msg[retval] = '\0'; //���ڿ� �� ���� -> ����ó�� ����

		if (strncmp(msg, "file=", 5)==0) {
			strcpy(filename, msg + 5);
			ReceiveFile(clientSock, filename);
		}
		else {
			SendMsg(msg, retval, clientSock); //�޼��� ����
			printf("\n[recv] %s\n", msg);
		}
	}
	
	ReleaseMutex(hMutex);
	closesocket(clientSock);


	return 0;
}

// Ŭ���̾�Ʈ�� �޼��� ����
void SendMsg(char* msg, int len, SOCKET clientSock) {
	WaitForSingleObject(hMutex, INFINITE); // ���ؽ� ����
	for (int i = 0; i < clientCount; i++) {
		if (clientSocket[i] != clientSock) // �޼����� ������ Ŭ���̾�Ʈ�� ������ ��� Ŭ���̾�Ʈ���� ����
			send(clientSocket[i], msg, len, 0); // �޽��� ����
	}
	ReleaseMutex(hMutex);        // ���ؽ� ����
}

void ReceiveFile(SOCKET clientSock, const char* filename) {
	int filelength;
	printf("[���� ����] %s\n", filename);

	FILE* recvfile = fopen(filename, "r");

	if (recvfile == NULL) {
		printf("[���� ���� ����] %s\n", filename);
		return;
	}

	char str[BUF_SIZE] = {0,};
	fread(str, 1, BUF_SIZE, recvfile);
	printf("[���� ����]\n%s\n", str);
	printf("[>] ���� ���� �Ϸ�\n");

	fclose(recvfile);

}