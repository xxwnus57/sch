#include "../common.h"
#include <process.h>
#include <errno.h>

#define BUF_SIZE 512
#define SERVERPORT 9000

char* SERVERIP = (char*)"127.0.0.1";

unsigned WINAPI Send_msg(void* arg);        // 스레드 전송함수
unsigned WINAPI Receive_msg(void* arg);   // 스레드 수신함수
void SendFile(SOCKET sock, const char* message); // 파일 전송 함수

char message[BUF_SIZE];     // 입력할 메세지
char id[10] = "client1";


int main(int argc, char* argv[]) {
    WSADATA wsaData;
    HANDLE send_Thread, recv_Thread;


    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sock)
        err_quit("socket()");

    //구조체 생성
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(SERVERPORT); // 서버 포트 9000

    // connect() : 서버에 소켓 연결 요청
    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        err_quit("connect()");

    printf("[USER ID] %s  \t\t\t\t   Exit : 00\n", &id);
    printf("\t\t\t\t\t\t   file=파일경로\n");
    printf("----------------------------------------------------------------\n");


    send_Thread = (HANDLE)_beginthreadex(NULL, 0, Send_msg, (void*)&sock, 0, NULL);//메시지 전송용 쓰레드가 실행된다.
    recv_Thread = (HANDLE)_beginthreadex(NULL, 0, Receive_msg, (void*)&sock, 0, NULL);//메시지 수신용 쓰레드가 실행된다.

    //WaitForSingleObject : 스레드가 종료할 때까지 대기 (INFINITE : 무한 대기 시간)
    WaitForSingleObject(send_Thread, INFINITE);
    WaitForSingleObject(recv_Thread, INFINITE);

    // 종료
    closesocket(sock);
    WSACleanup();

    return 0;
}


unsigned WINAPI Send_msg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);      //서버용 소켓 전달
    char ID_MSG[200];   // ID + msg
    int retval = 0;

    while (1) {

        scanf("%s", &message);

        if (!strcmp(message, "00")) { // 종료 조건
            closesocket(sock);
            exit (0);
        }
        printf("\n");

        if (strncmp(message, "file=", 5) == 0) { // 메세지 시작이 file=이면 파일 전송
            SendFile(sock, message); // 파일 전송 함수 호출
        }
        else {
            sprintf(ID_MSG, "%s : %s\n", id, message);          // id : 메세지 형태로 출력
            send(sock, ID_MSG, strlen(ID_MSG), 0);      //서버로 전송
        }

    }
    return 0;
}

unsigned WINAPI Receive_msg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);      //서버용 소켓 전달
    char recv_msg[BUF_SIZE];
    int retval;

    while (1) {
        retval = recv(sock, recv_msg, BUF_SIZE - 1, 0);     //서버로부터 메시지 수신
        if (retval == -1)
            return -1;

        recv_msg[retval] = 0;       // 문자열 끝을 구분
        printf("[recv] ");
        fputs(recv_msg, stdout); // 받은 메세지 출력
        printf("\n");
    }
    return 0;
}

void SendFile(SOCKET sock, const char* message) {
    FILE* fp;
    char filepath[50];
    int question;

    strcpy(filepath, message + 5);  //경로를 복사
    fp = fopen(filepath, "r");

    if (fp == NULL) 
    {
        printf("[%s] 해당 파일이 존재하지 않습니다.\n\n", strerror(errno)); //errno : 파일 오류 확인
        return;
    }

    send(sock, message, strlen(message), 0);
   
    fclose(fp);
    printf("[>]  파일 전송 완료\n\n");

 }

