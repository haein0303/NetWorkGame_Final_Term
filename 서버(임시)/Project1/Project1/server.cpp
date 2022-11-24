#include "Common.h"
#include <iostream>
#include <fstream>
#include <chrono>
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
    SOCKET client_sock[3];
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1]; // ���� ���� ������
    HANDLE hThread[3];

    while (cnt != 3) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock[cnt] = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock[cnt] == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ������ ����
        hThread[cnt] = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock[cnt], 0, NULL);
        if (hThread == NULL) { closesocket(client_sock[cnt]); }
        else { 
            // �ʱ� ���� Ŭ�� ���̵� �۽�
            SC_Lobby_Send cl;
            cl._protocol_num = SC_lobby_send;
            cl._acc_count = cnt;
            cl._my_num = cnt;
            send(client_sock[cnt], reinterpret_cast<char*>(&cl), sizeof(cl), 0);
            cnt++;
        }
    }

    // �ӽ� �÷��̾� ������ ����
    for (int i = 0; i < 3; ++i) {
        player[i].charType = i;
        player[i].charLook = 0;
        player[i].location = { 0,0 };
        player[i].state = Idle;
        player[i].coin = false;
        player[i].skill_cooltime1 = 0;
        player[i].skill_cooltime2 = 0;
        player[i].attack_on = false;
        player[i].skill_on = false;
    }
    // end

    // ���� ���� ���� ĳ���� ���� â���� ���� �Ǵ� �κ�
    {
        // �������� ���ѹ� ���� �� Ÿ�̸� ����
        

        // �� Ŭ���̾�Ʈ�� �������� �۽�

        // for ���������� ��ü Ŭ���̾�Ʈ�� �� �ѹ�, �ʱ� �������� �۽�
    

        // �� Ŭ���̾�Ʈ�� �ΰ��� �ʱ� ������ �۽�

        // while �� ���� �ؼ� �ð� �� �ɶ����� Ŭ���̾�Ʈ ĳ���� ���� ���� �޾Ƽ� ��� ���� �Ϸ�ǰų� �ð� ���� �� ���� ���� �� ����
        // �̶� �ð� �� ������ ��� ���� Ŭ�� �������� ĳ���� ����
        
        // while�� ���� �� ���ѹ� ���� �� ���ΰ��� �ʱ�ȭ ������ �۽�
    }

    // ���� ���� �κ�
    {
        // �������� ���ѹ� ���� �� Ÿ�̸� ����
        SC_Scene_Send sc;
        sc._protocol_num = SC_scene_send;
        sc._scene_num = Main_game;
        prevTime = 100;

        // �� Ŭ���̾�Ʈ�� �������� �۽�
        for (int i = 0; i < 3; ++i) {
            send(client_sock[i], reinterpret_cast<char*>(&sc), sizeof(sc), 0);
        }

        // for ���������� ��ü Ŭ���̾�Ʈ�� �� �ѹ�, �ʱ� �������� �۽�
        SC_Ingame_Send is;
        for (int i = 0; i < 3; ++i) {
            is._player[i]._char_type = player[i].charType;
            is._player[i]._coin = player[i].coin;
            is._player[i]._location = player[i].location;
            is._player[i]._look = player[i].charLook;
            is._player[i]._skill_cooltime1 = player[i].skill_cooltime1;
            is._player[i]._skill_cooltime2 = player[i].skill_cooltime2;
            is._player[i]._state = player[i].state;
        }
        is._coin_location = { 1,1 };
        is._left_time = prevTime;
        is._protocol_num = SC_ingame_send;

        // �� Ŭ���̾�Ʈ�� �ΰ��� �ʱ� ������ �۽�
        for (int i = 0; i < 3; ++i) {
            send(client_sock[i], reinterpret_cast<char*>(&is), sizeof(is), 0);
        }

        auto start = chrono::system_clock::now();

        // while ������ ���� ���� �� ����
        while (true)
        {
            auto now = chrono::system_clock::now();

            chrono::duration<float> pip = start - now;

            elapsedTime = prevTime - pip.count();

            if (elapsedTime <= 0) {
                // ���� ���� �� �κ� ������ ��ȯ
                // �ٷ� ���� ���ϰ� ���� ������ ȭ������ ��ȯ �Ҽ�������
            }
            else {
                // �浹 ó�� �� ���� ������ ������Ʈ
                // ��Ÿ�� ������Ʈ
                // ������Ʈ �� ������ �۽�
                // ���ú� ���� �����Ͱ� ��� ��� ������Ʈ �� �۽� �ݺ�


            }
        }
    }
    // ���� �ݱ�
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}