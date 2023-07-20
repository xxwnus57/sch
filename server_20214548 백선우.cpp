#include "../common.h"
#include <process.h>

#define SERVERPORT 9000
#define BUF_SIZE    512
#define CLIENT 100

unsigned WINAPI recv_Client(void* arg);	//쓰레드 함수
void SendMsg(char* msg, int len, SOCKET exclude);	// 클라이언트로 메세지 전달
void ReceiveFile(SOCKET clientSock, const char* filename); // 파일 저장

int clientCount = 0;
SOCKET clientSocket[CLIENT];	//클라이언트 소켓 
HANDLE hMutex;

char addr[INET_ADDRSTRLEN] = { 0, };

int main(int argc, char* argv[])
{
	SOCKADDR_IN serveraddr, clientaddr;
	SOCKET serversock, clientsock;

	int retval = -1;
	int clientaddrsize;


	// 윈속 초기화
	WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
		return 1;
	printf("[TCP Server] WSAStartup Success.\n");

	// 뮤텍스 생성
	hMutex = CreateMutex(NULL, FALSE, NULL);


	//	create socket
	serversock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == serversock)
		err_quit("socket()");

	// 소켓 주소 구조체 초기화
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET; //IPv4
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	// bind()  : 로컬 네트워크 주소와 소켓 결합
	retval = bind(serversock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");
	printf("[TCP Server] bind() Success.\n");

	// listen() : 연결 요청 대기 상태로 소켓 설정
	retval = listen(serversock, SOMAXCONN);
	if (SOCKET_ERROR == retval)
		err_quit("listen()");
	printf("[TCP Server] listen() Success.\n");



	while (1)
	{
		clientaddrsize = sizeof(clientaddr);
		clientsock = accept(serversock, (SOCKADDR*)&clientaddr, &clientaddrsize);//서버에게 전달된 클라이언트 소켓을 clientSock에 전달

		WaitForSingleObject(hMutex, INFINITE);		// 뮤텍스 실행
		clientSocket[clientCount++] = clientsock;		// 클라이언트 소켓에 소켓 주소 전달
		ReleaseMutex(hMutex);		 // ReleaseMutex : 뮤텍스 해제

		//HandleClient 실행 , beginthreadex : 스레드 생성 함수
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, recv_Client, (void*)&clientsock, 0, NULL);

		// 클라이언트 ip, 포트 출력
		printf("\n[TCP SERVER] Client IP : %s, Port = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}


	// 종료
	closesocket(serversock);
	WSACleanup();

	return 0;
}

unsigned WINAPI recv_Client(void* arg) {
	SOCKET clientSock = *((SOCKET*)arg);			// 클라이언트 소켓을 전달
	int retval = 0;
	char msg[BUF_SIZE+1];
	char filename[BUF_SIZE];

	// recv()
	while ((retval = recv(clientSock, msg, BUF_SIZE, 0)) != 0) {
		char filename[BUF_SIZE];
		msg[retval] = '\0'; //문자열 끝 구분 -> 예외처리 오류

		if (strncmp(msg, "file=", 5)==0) {
			strcpy(filename, msg + 5);
			ReceiveFile(clientSock, filename);
		}
		else {
			SendMsg(msg, retval, clientSock); //메세지 전달
			printf("\n[recv] %s\n", msg);
		}
	}
	
	ReleaseMutex(hMutex);
	closesocket(clientSock);


	return 0;
}

// 클라이언트로 메세지 전송
void SendMsg(char* msg, int len, SOCKET clientSock) {
	WaitForSingleObject(hMutex, INFINITE); // 뮤텍스 실행
	for (int i = 0; i < clientCount; i++) {
		if (clientSocket[i] != clientSock) // 메세지를 전송한 클라이언트를 제외한 모든 클라이언트에게 전달
			send(clientSocket[i], msg, len, 0); // 메시지 전달
	}
	ReleaseMutex(hMutex);        // 뮤텍스 해제
}

void ReceiveFile(SOCKET clientSock, const char* filename) {
	int filelength;
	printf("[받은 파일] %s\n", filename);

	FILE* recvfile = fopen(filename, "r");

	if (recvfile == NULL) {
		printf("[파일 열기 실패] %s\n", filename);
		return;
	}

	char str[BUF_SIZE] = {0,};
	fread(str, 1, BUF_SIZE, recvfile);
	printf("[파일 내용]\n%s\n", str);
	printf("[>] 파일 수신 완료\n");

	fclose(recvfile);

}