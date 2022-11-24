#include "Common.h"
#include <iostream>
#include <fstream>
#include "protocol.h"

using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    512

G_data player[3];

DWORD WINAPI ProcessClient(LPVOID arg)
{
    // ������ ��ſ� ����� ����
    int retval;
    SOCKET client_sock = (SOCKET)arg;
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1]; // ���� ���� ������

    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    while (1) {

        // ������ ũ�� �ޱ�
        // ������ Ÿ�� Ȯ��

        // Ŭ���̾�Ʈ�� ������ ���
        while (1) {
            // ������ �ޱ�(���� ����)
            retval = recv(client_sock, buf, BUFSIZE, MSG_WAITALL);
            if (retval == SOCKET_ERROR) {
                err_display("recv()");
                break;
            }
            else if (retval == 0)
                break;

            // ������ ���� �� ���� ������ ���� �����Ϳ� ������Ʈ
            // ���� �ݺ�
        }
        // closesocket()
        closesocket(client_sock);
        return 0;
    }
}

int main(int argc, char* argv[])
{
    int retval;
    int cnt = 0;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // ���� ����
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1]; // ���� ���� ������
    HANDLE hThread[3];

    while (cnt != 3) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ������ ����
        hThread[cnt] = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) { closesocket(client_sock); }
        else { 
            // �ʱ� ���� Ŭ�� ���̵� �۽�
            
            cnt++;
        }
    }

    // ���� ���� ���� ĳ���� ���� â���� ���� �Ǵ� �κ�
    {
        // �������� ���ѹ� ���� �� Ÿ�̸� ����
        // for ���������� ��ü Ŭ���̾�Ʈ�� �� �ѹ�, �ʱ� �������� �۽�

        {
            // while �� ���� �ؼ� �ð� �� �ɶ����� Ŭ���̾�Ʈ ĳ���� ���� ���� �޾Ƽ� ��� ���� �Ϸ�ǰų� �ð� ���� �� ���� ���� �� ����
            // �̶� �ð� �� ������ ��� ���� Ŭ�� �������� ĳ���� ����
        }
        
        // while�� ���� �� ���ѹ� ���� �� ���ΰ��� �ʱ�ȭ ������ �۽�
    }

    // ���� ���� �κ�
    {
        // while ������ ���� ���� �� ����
        // �̶� �� �������� ���ú���� �����Ͱ� ���� �� ��� �ٲ� �����ͷ� �۽�
        // ��Ÿ���� ��� ���ú� ���� �ð��� ���� �ð� üũ -> ���ú� ���� ������ ���� �ð� ���� �� ���� ->
        // ���� ������Ʈ �ÿ� �ð� ��� �� ���ŵ� ���� �۽�

        // ���ú� ���� �����Ͱ� ��� ��� ������Ʈ �� �۽� �ݺ�

        // ���� ���� �ð� 0�� ��� ������ Ȯ�� �� �� ���� �۽� �� ĳ���� �߾Ӻ� �̵�
    }
    // ���� �ݱ�
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}