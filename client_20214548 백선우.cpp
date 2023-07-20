#include "../common.h"
#include <process.h>
#include <errno.h>

#define BUF_SIZE 512
#define SERVERPORT 9000

char* SERVERIP = (char*)"127.0.0.1";

unsigned WINAPI Send_msg(void* arg);        // ������ �����Լ�
unsigned WINAPI Receive_msg(void* arg);   // ������ �����Լ�
void SendFile(SOCKET sock, const char* message); // ���� ���� �Լ�

char message[BUF_SIZE];     // �Է��� �޼���
char id[10] = "client1";


int main(int argc, char* argv[]) {
    WSADATA wsaData;
    HANDLE send_Thread, recv_Thread;


    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    // ���� ����
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sock)
        err_quit("socket()");

    //����ü ����
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(SERVERPORT); // ���� ��Ʈ 9000

    // connect() : ������ ���� ���� ��û
    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        err_quit("connect()");

    printf("[USER ID] %s  \t\t\t\t   Exit : 00\n", &id);
    printf("\t\t\t\t\t\t   file=���ϰ��\n");
    printf("----------------------------------------------------------------\n");


    send_Thread = (HANDLE)_beginthreadex(NULL, 0, Send_msg, (void*)&sock, 0, NULL);//�޽��� ���ۿ� �����尡 ����ȴ�.
    recv_Thread = (HANDLE)_beginthreadex(NULL, 0, Receive_msg, (void*)&sock, 0, NULL);//�޽��� ���ſ� �����尡 ����ȴ�.

    //WaitForSingleObject : �����尡 ������ ������ ��� (INFINITE : ���� ��� �ð�)
    WaitForSingleObject(send_Thread, INFINITE);
    WaitForSingleObject(recv_Thread, INFINITE);

    // ����
    closesocket(sock);
    WSACleanup();

    return 0;
}


unsigned WINAPI Send_msg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);      //������ ���� ����
    char ID_MSG[200];   // ID + msg
    int retval = 0;

    while (1) {

        scanf("%s", &message);

        if (!strcmp(message, "00")) { // ���� ����
            closesocket(sock);
            exit (0);
        }
        printf("\n");

        if (strncmp(message, "file=", 5) == 0) { // �޼��� ������ file=�̸� ���� ����
            SendFile(sock, message); // ���� ���� �Լ� ȣ��
        }
        else {
            sprintf(ID_MSG, "%s : %s\n", id, message);          // id : �޼��� ���·� ���
            send(sock, ID_MSG, strlen(ID_MSG), 0);      //������ ����
        }

    }
    return 0;
}

unsigned WINAPI Receive_msg(void* arg) {
    SOCKET sock = *((SOCKET*)arg);      //������ ���� ����
    char recv_msg[BUF_SIZE];
    int retval;

    while (1) {
        retval = recv(sock, recv_msg, BUF_SIZE - 1, 0);     //�����κ��� �޽��� ����
        if (retval == -1)
            return -1;

        recv_msg[retval] = 0;       // ���ڿ� ���� ����
        printf("[recv] ");
        fputs(recv_msg, stdout); // ���� �޼��� ���
        printf("\n");
    }
    return 0;
}

void SendFile(SOCKET sock, const char* message) {
    FILE* fp;
    char filepath[50];
    int question;

    strcpy(filepath, message + 5);  //��θ� ����
    fp = fopen(filepath, "r");

    if (fp == NULL) 
    {
        printf("[%s] �ش� ������ �������� �ʽ��ϴ�.\n\n", strerror(errno)); //errno : ���� ���� Ȯ��
        return;
    }

    send(sock, message, strlen(message), 0);
   
    fclose(fp);
    printf("[>]  ���� ���� �Ϸ�\n\n");

 }

