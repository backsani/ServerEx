#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
//#define SERVERIP   "172.16.34.19"
#define SERVERPORT 9000
#define BUFSIZE    512

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        ptr += received;
    }

    return (len - left);
}

struct Data
{
    int Id;
    char buf[BUFSIZE + 1];
};

int main(int argc, char* argv[])
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    // ������ ��ſ� ����� ����
    char inputBuf[BUFSIZE + 1];
    int len;
    struct Data data;
    data.Id = 1;

    // ������ ������ ���
    while (1) {
        // ������ �Է�
        printf("\n[���� ������] ");
        if (fgets(inputBuf, BUFSIZE + 1, stdin) == NULL)
            break;

        strcpy(data.buf, "from client : ");
        
        len = strlen(data.buf);
        if (data.buf[len - 1] == '\n')
            data.buf[len - 1] = '\0';
        if (data.buf[len] == '\0')
            data.buf[len] = NULL;
        if (strlen(data.buf) == 0)
            break;

        for (int i = 0; i < strlen(inputBuf) + 1; i++) {
            if (inputBuf[i] == '\0') {
                data.buf[len + i] = '\0';
                break;
            }
                
            data.buf[len + i] = inputBuf[i];
        }


        // '\n' ���� ����
        len = strlen(data.buf);
        if (data.buf[len - 1] == '\n')
            data.buf[len - 1] = '\0';
        if (strlen(data.buf) == 0)
            break;



        // ������ ������
        retval = send(sock, (char*) & data, sizeof(struct Data), 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);

        // ������ �ޱ�
        retval = recvn(sock, data.buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;

        // ���� ������ ���
        data.buf[retval] = '\0';
        printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
        printf("[���� ������] %s\n", data.buf);
    }

    // closesocket()
    closesocket(sock);

    // ���� ����
    WSACleanup();
    return 0;
}
