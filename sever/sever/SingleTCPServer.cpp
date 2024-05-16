#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>

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

//void 

struct sClient
{
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    HANDLE hThread;
    UINT threadaddr;
    int clientId;
    bool isThread;
}client;

struct Data
{
    int Id;
    char buf[BUFSIZE + 1];
};

UINT WINAPI hThreadClient(LPVOID lparam) 
{
    sClient* pClient = (sClient*)lparam;
    sClient client;
    Data data;
    for (int i = 0; i < 10; i++) {
        if (pClient[i].isThread == false) {
            pClient[i].isThread = true;
            client = pClient[i];
            break;
        }
    }
    int buflen;
    char buf[BUFSIZE + 1];
    char bufdata[BUFSIZE + 1];
    char output[] = "from server : ";
    int retval;
    int clientId = client.clientId;
    printf("%d", client.clientId);
    
    //u_long mode = 1; // ����ŷ ���
    //int result = ioctlsocket(client.client_sock, FIONBIO, &mode);
    //if (result == SOCKET_ERROR) {
    //    printf("ioctlsocket failed: %d\n", WSAGetLastError());
    //    closesocket(client.client_sock);
    //    WSACleanup();
    //    return 1;
    //}

    // Ŭ���̾�Ʈ�� ������ ���
    while (1) {
        // ������ �ޱ�
        retval = recv(client.client_sock, (char*) & data, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;

        strcpy(buf, data.buf);

        // ���� ������ ���
        buf[strlen(buf)] = '\0';
        printf("[TCP/%s:%d] %s\n", inet_ntoa(client.clientaddr.sin_addr),
            ntohs(client.clientaddr.sin_port), buf);

        // ���� �����͸� ã��(from cliect�� ������)
        char* ptr = strstr(buf, " : ");
        ptr += 3;
        for (int i = 0; i < BUFSIZE + 1; i++) {
            if (ptr[i] == '\0')
                break;
            bufdata[i] = ptr[i];
        }

        // ���� ������ �����(from server�� ������)
        buflen = strlen(output);

        for (int i = 0; i < buflen + 1; i++) {
            if (output[i] == '\0')
                break;
            buf[i] = output[i];
        }

        for (int i = 0; i < BUFSIZE + 1; i++) {
            if (bufdata[i] == '\0') {
                buf[buflen + i] = '\0';
                break;
            }
            buf[buflen + i] = bufdata[i];
        }

        // ������ ������
       /* for (int i = 0; i < 10; i++) {
            if (pClient[i].isThread == false) {
                break;
            }
            retval = send(pClient[i].client_sock, buf, retval, 0);
            if (retval == SOCKET_ERROR) {
                err_display("send()");
                break;
            }
        
        }*/
        retval = send(client.client_sock, buf, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
    }

    CloseHandle(client.hThread);
    // closesocket()
    closesocket(client.client_sock);
    printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
        inet_ntoa(client.clientaddr.sin_addr), ntohs(client.clientaddr.sin_port));

    return 0;
}



int main(int argc, char* argv[])
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // ������ ��ſ� ����� ����
    
    int addrlen;
    struct sClient client[10];
    int i = 0;

    while (1) {

        // accept()
        addrlen = sizeof(client[i].clientaddr);
        client[i].client_sock = accept(listen_sock, (SOCKADDR*)&client[i].clientaddr, &addrlen);
        if (client[i].client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            inet_ntoa(client[i].clientaddr.sin_addr), ntohs(client[i].clientaddr.sin_port));

        client[i].clientId = i;
        client[i].isThread = false;
        client[i].hThread = (HANDLE)_beginthreadex(NULL, 0, hThreadClient, &client, 0, &client[i].threadaddr);
        
        i++;
    }

    // closesocket()
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}
