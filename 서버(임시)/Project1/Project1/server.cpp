#include "Common.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "protocol.h"

using namespace std;

#define TILE_SIZE 64

#define SERVERPORT 9000
#define BUFSIZE    512

G_data player[2];
Coin coin;

//HANDLE hRcev1Event, hRcev2Event, hRcev3Event; // �̺�Ʈ
HANDLE hRecvEvent[2];
HANDLE hRootEvent;

//�ڽ��� ���� �������� ��ȣ�� ��ȯ
int calc_next_thread(int i) {
    return (i + 1) % 2;
}
//�ڽ��� ���� �������� ��ȣ�� ��ȯ
int calc_prev_thread(int i) {
    return (i + 2) % 2;
}

//Recv ������
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

    int my_num = 0; //�ڱ� �ڽ��� ���� ��ȣ�� ����

    for (int i = 0; i < 2; i++) {
        if (player[i].my_num == GetCurrentThreadId()) {
            cout << player[i].my_num << "   " << GetCurrentThreadId() << endl;
            my_num = i;
            break;
        }
    }

    while (1) {
        while (1) {
            CS_ingame_send_tmp ingame_key;
            DWORD retval;
            int protocol_num;
            // �ӽ� ���� ����
            retval = recv(client_sock, buf, BUFSIZE, 0);

            protocol_num = (int)buf[0];

            switch (protocol_num) {
            case CS_ingame_send: // ingame_key_send
                retval = recv(client_sock, reinterpret_cast<char*>(&ingame_key), sizeof(ingame_key), MSG_WAITALL);
                WaitForSingleObject(hRecvEvent[my_num], INFINITE);
                player[my_num].ingame_key = ingame_key;
                //cout << my_num << " : " << ingame_key._horizontal_key << "|" << ingame_key._vertical_key << endl;
                SetEvent(hRootEvent);
                break;
            }

            //�����˻�
            if (retval == SOCKET_ERROR) {
                err_display("recv()");
                break;
            }
            else if (retval == 0) {
                break;
            }
        }

        break;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int retval;
    int cnt = 0;
    DWORD num;

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
    SOCKET client_sock[2];
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1]; // ���� ���� ������
    HANDLE hThread[2];

    ZeroMemory(buf, BUFSIZE);

    DWORD event_retval; //��� �����

    //�̺�Ʈ ����
    for (int i = 0; i < 2; ++i) {
        hRecvEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    hRootEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    char Inbuff[20000] = { 0 };
    DWORD read_size = 20000;
    DWORD c = 20000;

    // �� ������ �ε�
    HANDLE hFile;
    int Tileindex[100][100] = { 0 };

    hFile = CreateFile(L"map.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
    memset(Inbuff, 0, 99 * sizeof(WCHAR));
    ReadFile(hFile, Inbuff, c, &read_size, NULL); // hFile���� size ��ŭ �о� InBuff�� ����
    //Inbuff[c - 1] = '\0';
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            if (Inbuff[j + i * 100] >= 48 && Inbuff[j + i * 100] <= 50)
                Tileindex[j][i] = Inbuff[j + i * 100] - 48;
            switch (Tileindex[j][i])
            {
            case 0:
                Tileindex[j][i] = 2;
                break;
            case 1:
                Tileindex[j][i] = 0;
                break;
            case 2:
                Tileindex[j][i] = 1;
                break;
            }
        }
    }

    CloseHandle(hFile);

    coin.init = true;
    coin.location = { 34 * 64, 15 * 64 };

    while (cnt < 2) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock[cnt] = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock[cnt] == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // ������ ����
        hThread[cnt] = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock[cnt], 0, &num);
        
        if (hThread[cnt] == NULL) { closesocket(client_sock[cnt]); }
        else {
            // �ʱ� ���� Ŭ�� ���̵� �۽�
            buf[0] = SC_lobby_send;
            SC_Lobby_Send cl;
            cl._acc_count = cnt;
            player[cnt].my_num = num;

            for (int i = 0; i <= cnt; ++i) {
                send(client_sock[cnt], buf, BUFSIZE, 0);
                send(client_sock[cnt], reinterpret_cast<char*>(&cl), sizeof(cl), 0);
            }
            cnt++;
        }
    }

    ZeroMemory(buf, BUFSIZE);

    // �ӽ� �÷��̾� ������ ����
    for (int i = 0; i < 2; ++i) {
        player[i].charType = i+1;
        player[i].charLook = 0;
        player[i].location = { 40 * TILE_SIZE, (i + 15) * TILE_SIZE };
        player[i].state = IdleA;
        player[i].coin = false;
        player[i].skill_cooltime1 = 0;
        player[i].skill_cooltime2 = 0;
        player[i].attack_on = false;
        player[i].skill_on = false;
        switch (player[i].charType)
        {
        case 0:
            // ��ų ����
            player[i].char_info.skill_area.left = 0;
            player[i].char_info.skill_area.right = 552;
            player[i].char_info.skill_area.top = 0;
            player[i].char_info.skill_area.bottom = 310;
            // ���� ����
            player[i].char_info.skill_area.left = 0;
            player[i].char_info.skill_area.right = 70;
            player[i].char_info.skill_area.top = 0;
            player[i].char_info.skill_area.bottom = 50;
            break;
        case 1:
            // ��ų ����
            player[i].char_info.skill_area.left = 0;
            player[i].char_info.skill_area.right = 323;
            player[i].char_info.skill_area.top = 0;
            player[i].char_info.skill_area.bottom = 128;
            // ���� ����
            player[i].char_info.skill_area.left = 0;
            player[i].char_info.skill_area.right = 70;
            player[i].char_info.skill_area.top = 0;
            player[i].char_info.skill_area.bottom = 50;
            break;
        case 2:
            // ��ų ����
            player[i].char_info.skill_area.left = 0;
            player[i].char_info.skill_area.right = 413;
            player[i].char_info.skill_area.top = 0;
            player[i].char_info.skill_area.bottom = 223;
            // ���� ����
            player[i].char_info.skill_area.left = 0;
            player[i].char_info.skill_area.right = 70;
            player[i].char_info.skill_area.top = 0;
            player[i].char_info.skill_area.bottom = 50;
            break;
        }
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


        // for ���������� ��ü Ŭ���̾�Ʈ�� �� �ѹ�, �ʱ� �������� �۽�
        SC_Ingame_Send is;
        for (int i = 0; i < cnt; ++i) {
            is._player[i]._char_type = player[i].charType;
            is._player[i]._coin = player[i].coin;
            is._player[i]._location = player[i].location;
            is._player[i]._look = player[i].charLook;
            is._player[i]._skill_cooltime1 = player[i].skill_cooltime1;
            is._player[i]._skill_cooltime2 = player[i].skill_cooltime2;
            is._player[i]._state = player[i].state;
        }
        is._coin_location = coin.location;
        is._left_time = -1.0;
        buf[0] = SC_ingame_send;

        // �� Ŭ���̾�Ʈ�� �ΰ��� �ʱ� ������ �۽�
        for (int i = 0; i < cnt; ++i) {
            send(client_sock[i], buf, BUFSIZE, 0);
            send(client_sock[i], reinterpret_cast<char*>(&is), sizeof(is), 0);
        }

        ZeroMemory(buf, BUFSIZE);

        // �������� ���ѹ� ���� �� Ÿ�̸� ����
        SC_Scene_Send sc;
        buf[0] = SC_scene_send;
        sc._scene_num = Main_game;
        prevTime = 100;

        this_thread::sleep_for(1000ms);

        // �� Ŭ���̾�Ʈ�� �������� �۽�
        for (int i = 0; i < cnt; ++i) {
            send(client_sock[i], buf, BUFSIZE, 0);
            send(client_sock[i], reinterpret_cast<char*>(&sc), sizeof(sc), 0);
        }

        ZeroMemory(buf, BUFSIZE);

        auto start = chrono::system_clock::now();

        int _x = 10;
        int _y = 10;

        // while ������ ���� ���� �� ����
        while (true)
        {
            //for (int i = 0; i < 3; ++i) {
            //    //���
            //    SetEvent(hRecvEvent[i]);
            //    //��ȯ
            //    event_retval = WaitForSingleObject(hRootEvent, INFINITE);
            //    if (event_retval != WAIT_OBJECT_0) break;
            //}

            for (int i = 0; i < cnt; ++i) {
                //���ú� ������ �����
                SetEvent(hRecvEvent[i]);
                //cout << "main Thread : " << i << " is Work" << endl;
                //�� ��ٸ���
                WaitForSingleObject(hRootEvent, 10);//������ٸ� �ʿ�� ����,
            }

            auto now = chrono::system_clock::now();

            chrono::duration<float> pip = now - start;

            elapsedTime = prevTime - pip.count();


            if (elapsedTime <= 0) {
                // ���� ���� �� �κ� ������ ��ȯ
                // �ٷ� ���� ���ϰ� ���� ������ ȭ������ ��ȯ �Ҽ�������
            }

            else { // ���� ������Ʈ
                // �浹 ó�� �� ���� ������ ������Ʈ
                for (int i = 0; i < cnt; ++i) {
                    // ��ų �浹
                    if (player[i].ingame_key._skill_key == 1 && player[i].skill_cooltime2 <= 0) { // 1�� ��ų
                        if (player[i].ingame_key._horizontal_key == 1) {
                            RECT tmp;
                            tmp.left = player[i].location.x;
                            tmp.right = player[i].location.x + player[i].char_info.skill_area.right;
                            tmp.top = player[i].location.y - player[i].char_info.skill_area.bottom / 2;
                            tmp.bottom = player[i].location.y + player[i].char_info.skill_area.bottom / 2;

                            if (i == 0)
                            {
                                if (player[1].location.x >= tmp.left &&
                                    player[1].location.x <= tmp.right &&
                                    player[1].location.y >= tmp.top &&
                                    player[1].location.y <= tmp.bottom)
                                {
                                    player[1].state = Attacked;
                                    if (player[1].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[1].location.x + 20;
                                        coin.location.y = player[1].location.y + 20;
                                        player[1].coin = false;
                                    }
                                }
                            }
                            else
                            {
                                if (player[0].location.x >= tmp.left &&
                                    player[0].location.x <= tmp.right &&
                                    player[0].location.y >= tmp.top &&
                                    player[0].location.y <= tmp.bottom)
                                {
                                    player[0].state = Attacked;
                                    if (player[0].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[0].location.x + 20;
                                        coin.location.y = player[0].location.y + 20;
                                        player[0].coin = false;
                                    }
                                }
                            }
                            player[i].charLook = 4;
                        }
                        else if (player[i].ingame_key._horizontal_key == -1) {
                            RECT tmp;
                            tmp.left = player[i].location.x - player[i].char_info.skill_area.right;
                            tmp.right = player[i].location.x;
                            tmp.top = player[i].location.y - player[i].char_info.skill_area.bottom / 2;
                            tmp.bottom = player[i].location.y + player[i].char_info.skill_area.bottom / 2;

                            if (i == 0)
                            {
                                if (player[1].location.x >= tmp.left &&
                                    player[1].location.x <= tmp.right &&
                                    player[1].location.y >= tmp.top &&
                                    player[1].location.y <= tmp.bottom)
                                {
                                    player[1].state = Attacked;
                                    if (player[1].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[1].location.x + 20;
                                        coin.location.y = player[1].location.y + 20;
                                        player[1].coin = false;
                                    }
                                }
                            }
                            else
                            {
                                if (player[0].location.x >= tmp.left &&
                                    player[0].location.x <= tmp.right &&
                                    player[0].location.y >= tmp.top &&
                                    player[0].location.y <= tmp.bottom)
                                {
                                    player[0].state = Attacked;
                                    if (player[0].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[0].location.x + 20;
                                        coin.location.y = player[0].location.y + 20;
                                        player[0].coin = false;
                                    }
                                }
                            }
                            player[i].charLook = 2;
                        }
                        else if (player[i].ingame_key._vertical_key == 1) {
                            RECT tmp;
                            tmp.left = player[i].location.x - player[i].char_info.skill_area.bottom / 2;
                            tmp.right = player[i].location.x + player[i].char_info.skill_area.bottom / 2;
                            tmp.top = player[i].location.y - player[i].char_info.skill_area.right;
                            tmp.bottom = player[i].location.y;

                            if (i == 0)
                            {
                                if (player[1].location.x >= tmp.left &&
                                    player[1].location.x <= tmp.right &&
                                    player[1].location.y >= tmp.top &&
                                    player[1].location.y <= tmp.bottom)
                                {
                                    player[1].state = Attacked;
                                    if (player[1].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[1].location.x + 20;
                                        coin.location.y = player[1].location.y + 20;
                                        player[1].coin = false;
                                    }
                                }
                            }
                            else
                            {
                                if (player[0].location.x >= tmp.left &&
                                    player[0].location.x <= tmp.right &&
                                    player[0].location.y >= tmp.top &&
                                    player[0].location.y <= tmp.bottom)
                                {
                                    player[0].state = Attacked;
                                    if (player[0].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[0].location.x + 20;
                                        coin.location.y = player[0].location.y + 20;
                                        player[0].coin = false;
                                    }
                                }
                            }
                            player[i].charLook = 3;
                        }
                        else if (player[i].ingame_key._vertical_key == -1) {
                            RECT tmp;
                            tmp.left = player[i].location.x - player[i].char_info.skill_area.bottom / 2;
                            tmp.right = player[i].location.x + player[i].char_info.skill_area.bottom / 2;
                            tmp.top = player[i].location.y;
                            tmp.bottom = player[i].location.y + player[i].char_info.skill_area.right;

                            if (i == 0)
                            {
                                if (player[1].location.x >= tmp.left &&
                                    player[1].location.x <= tmp.right &&
                                    player[1].location.y >= tmp.top &&
                                    player[1].location.y <= tmp.bottom)
                                {
                                    player[1].state = Attacked;
                                    player[1].coin = false;
                                }
                            }
                            else
                            {
                                if (player[0].location.x >= tmp.left &&
                                    player[0].location.x <= tmp.right &&
                                    player[0].location.y >= tmp.top &&
                                    player[0].location.y <= tmp.bottom)
                                {
                                    player[0].state = Attacked;
                                    player[0].coin = false;
                                }
                            }
                            player[i].charLook = 5;
                        }
                        cout << "��ų" << endl;
                        player[i].state = Skill;
                        switch (player[i].charType) {
                        case 1:
                            player[i].skill_cooltime2 = 120;
                            break;
                        case 2:
                            player[i].skill_cooltime2 = 60;
                            break;
                        case 3:
                            player[i].skill_cooltime2 = 180;
                            break;
                        }
                        }
                    // ���� �浹
                    else if (player[i].ingame_key._skill_key == 2) { // ����
                        if (player[i].ingame_key._horizontal_key == 1) {
                            RECT tmp;
                            tmp.left = player[i].location.x;
                            tmp.right = player[i].location.x + player[i].char_info.attack_area.right;
                            tmp.top = player[i].location.y - player[i].char_info.attack_area.bottom / 2;
                            tmp.bottom = player[i].location.y + player[i].char_info.attack_area.bottom / 2;

                            if (i == 0)
                            {
                                if (player[1].location.x >= tmp.left &&
                                    player[1].location.x <= tmp.right &&
                                    player[1].location.y >= tmp.top &&
                                    player[1].location.y <= tmp.bottom)
                                {
                                    player[1].state = Attacked;
                                    if (player[1].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[1].location.x + 20;
                                        coin.location.y = player[1].location.y + 20;
                                        player[1].coin = false;
                                    }
                                }
                            }
                            else
                            {
                                if (player[0].location.x >= tmp.left &&
                                    player[0].location.x <= tmp.right &&
                                    player[0].location.y >= tmp.top &&
                                    player[0].location.y <= tmp.bottom)
                                {
                                    player[0].state = Attacked;
                                    if (player[0].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[0].location.x + 20;
                                        coin.location.y = player[0].location.y + 20;
                                        player[0].coin = false;
                                    }
                                }
                            }
                            player[i].charLook = 4;
                        }
                        else if (player[i].ingame_key._horizontal_key == -1) {
                            RECT tmp;
                            tmp.left = player[i].location.x - player[i].char_info.attack_area.right;
                            tmp.right = player[i].location.x;
                            tmp.top = player[i].location.y - player[i].char_info.attack_area.bottom / 2;
                            tmp.bottom = player[i].location.y + player[i].char_info.attack_area.bottom / 2;

                            if (i == 0)
                            {
                                if (player[1].location.x >= tmp.left &&
                                    player[1].location.x <= tmp.right &&
                                    player[1].location.y >= tmp.top &&
                                    player[1].location.y <= tmp.bottom)
                                {
                                    player[1].state = Attacked;
                                    player[1].coin = false;
                                }
                            }
                            else
                            {
                                if (player[0].location.x >= tmp.left &&
                                    player[0].location.x <= tmp.right &&
                                    player[0].location.y >= tmp.top &&
                                    player[0].location.y <= tmp.bottom)
                                {
                                    player[0].state = Attacked;
                                    if (player[0].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[0].location.x + 20;
                                        coin.location.y = player[0].location.y + 20;
                                        player[0].coin = false;
                                    }
                                }
                            }
                            player[i].charLook = 2;
                        }
                        else if (player[i].ingame_key._vertical_key == 1) {
                            RECT tmp;
                            tmp.left = player[i].location.x - player[i].char_info.attack_area.bottom / 2;
                            tmp.right = player[i].location.x + player[i].char_info.attack_area.bottom / 2;
                            tmp.top = player[i].location.y - player[i].char_info.attack_area.right;
                            tmp.bottom = player[i].location.y;

                            if (i == 0)
                            {
                                if (player[1].location.x >= tmp.left &&
                                    player[1].location.x <= tmp.right &&
                                    player[1].location.y >= tmp.top &&
                                    player[1].location.y <= tmp.bottom)
                                {
                                    player[1].state = Attacked;
                                    if (player[1].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[1].location.x + 20;
                                        coin.location.y = player[1].location.y + 20;
                                        player[1].coin = false;
                                    }
                                }
                            }
                            else
                            {
                                if (player[0].location.x >= tmp.left &&
                                    player[0].location.x <= tmp.right &&
                                    player[0].location.y >= tmp.top &&
                                    player[0].location.y <= tmp.bottom)
                                {
                                    player[0].state = Attacked;
                                    if (player[0].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[0].location.x + 20;
                                        coin.location.y = player[0].location.y + 20;
                                        player[0].coin = false;
                                    }
                                }
                            }
                            player[i].charLook = 3;
                        }
                        else if (player[i].ingame_key._vertical_key == -1) {
                            RECT tmp;
                            tmp.left = player[i].location.x - player[i].char_info.attack_area.bottom / 2;
                            tmp.right = player[i].location.x + player[i].char_info.attack_area.bottom / 2;
                            tmp.top = player[i].location.y;
                            tmp.bottom = player[i].location.y + player[i].char_info.attack_area.right;

                            if (i == 0)
                            {
                                if (player[1].location.x >= tmp.left &&
                                    player[1].location.x <= tmp.right &&
                                    player[1].location.y >= tmp.top &&
                                    player[1].location.y <= tmp.bottom)
                                {
                                    player[1].state = Attacked;
                                    if (player[1].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[1].location.x + 20;
                                        coin.location.y = player[1].location.y + 20;
                                        player[1].coin = false;
                                    }
                                }
                            }
                            else
                            {
                                if (player[0].location.x >= tmp.left &&
                                    player[0].location.x <= tmp.right &&
                                    player[0].location.y >= tmp.top &&
                                    player[0].location.y <= tmp.bottom)
                                {
                                    player[0].state = Attacked;
                                    if (player[0].coin == true) {
                                        coin.init = true;
                                        coin.location.x = player[0].location.x + 20;
                                        coin.location.y = player[0].location.y + 20;
                                        player[0].coin = false;
                                    }
                                }
                            }
                            player[i].charLook = 5;
                        }
                        cout << "����" << endl;
                        player[i].state = Attack;
                        }

                    if (player[i].state == Attacked) continue;
                    else 
                    {
                        // �̵�
                        if (player[i].ingame_key._skill_key == 0) { // ��ų x �̵���
                            if (player[i].ingame_key._horizontal_key == 1 && player[i].location.x < 6350) {
                                if (Tileindex[(player[i].location.x + 15) / 64][player[i].location.y / 64] == 1) // ���ӹ���
                                    player[i].location.x += _x + 5;
                                else if (Tileindex[(player[i].location.x + 10) / 64][player[i].location.y / 64] == 0) {
                                    player[i].location.x += _x;
                                } // �Ϲ� ����
                                player[i].state = WalkB;
                                player[i].charLook = 4;
                            }
                            else if (player[i].ingame_key._horizontal_key == -1 && player[i].location.x > 50) {
                                if (Tileindex[(player[i].location.x - 15) / 64][player[i].location.y / 64] == 1) // ���ӹ���
                                    player[i].location.x -= (_x + 5);
                                else if (Tileindex[(player[i].location.x - 10) / 64][player[i].location.y / 64] == 0) {
                                    player[i].location.x -= _x;
                                } // �Ϲ� ����
                                player[i].state = WalkA;
                                player[i].charLook = 2;
                            }

                            if (player[i].ingame_key._vertical_key == -1 && player[i].location.y > 50) {
                                if (Tileindex[player[i].location.x / 64][(player[i].location.y - 15) / 64] == 1) // ���ӹ���
                                    player[i].location.y -= (_y + 5);
                                else if (Tileindex[player[i].location.x / 64][(player[i].location.y - 10) / 64] == 0) {
                                    player[i].location.y -= _y;
                                } // �Ϲ� ����
                                player[i].state = WalkB;
                                player[i].charLook = 3;
                            }
                            else if (player[i].ingame_key._vertical_key == 1 && player[i].location.y < 6350) {
                                if (Tileindex[player[i].location.x / 64][(player[i].location.y + 15) / 64] == 1) // ���ӹ���
                                    player[i].location.y += _y + 5;
                                else if (Tileindex[player[i].location.x / 64][(player[i].location.y + 10) / 64] == 0) {
                                    player[i].location.y += _y;
                                } // �Ϲ� ����
                                player[i].state = WalkA;
                                player[i].charLook = 5;
                            }
                            //player[i].state = Walk;
                        }
                        // �뽬
                        else if (player[i].ingame_key._skill_key == 3 && player[i].skill_cooltime1 <= 0) { // �뽬
                            if (player[i].ingame_key._horizontal_key == 1) player[i].location.x += _x * 5;
                            else if (player[i].ingame_key._horizontal_key == -1) player[i].location.x -= _x * 5;

                            if (player[i].ingame_key._vertical_key == 1) player[i].location.y += _y * 5;
                            else if (player[i].ingame_key._vertical_key == -1) player[i].location.y -= _y * 5;

                            player[i].state = Dash;
                            switch (player[i].charType) {
                            case 1:
                                player[i].skill_cooltime1 = 410;
                                break;
                            case 2:
                                player[i].skill_cooltime1 = 300;
                                break;
                            case 3:
                                player[i].skill_cooltime1 = 600;
                                break;
                            }
                        }

                        if (player[i].location.x == coin.location.x && player[i].location.y && coin.location.y) {
                            coin.init = false;
                            coin.location = { 0, 0 };
                            player[i].coin = true;
                        }
                    }
                }

                // ��Ÿ�� ������Ʈ
                for (int i = 0; i < 2; ++i) {
                    if (player[i].skill_cooltime1 > 0) {
                        player[i].skill_cooltime1 -= pip.count();
                    }
                    if (player[i].skill_cooltime2 > 0) {
                        player[i].skill_cooltime2 -= pip.count();
                    }                 

                    // �ð� �� Ȯ�� �� ���� �ð� ��ŭ ���� ��Ÿ�ӿ��� ����
                }

                // ������Ʈ �� ������ �۽�
                SC_Ingame_Send _is;
                for (int i = 0; i < 2; ++i) {
                    _is._player[i]._char_type = player[i].charType;
                    _is._player[i]._coin = player[i].coin;
                    _is._player[i]._location = player[i].location;
                    _is._player[i]._look = player[i].charLook;
                    _is._player[i]._skill_cooltime1 = player[i].skill_cooltime1;
                    _is._player[i]._skill_cooltime2 = player[i].skill_cooltime2;
                    _is._player[i]._state = player[i].state;
                }
                _is._left_time = elapsedTime;
                _is._coin_location = coin.location; // ���Ŀ� ���� �ʿ�

                /*for (int i = 0; i < cnt; ++i) {
                    cout << i << " X : " << player[i].location.x << " Y : " << player[i].location.y << endl;
                }*/


                buf[0] = SC_ingame_send;

                for (int i = 0; i < cnt; ++i) {
                    send(client_sock[i], buf, BUFSIZE, 0);
                    send(client_sock[i], reinterpret_cast<char*>(&_is), sizeof(_is), 0);
                }

                for (int i = 0; i < cnt; i++) {
                    if (!(player[i].ingame_key._horizontal_key == 0 && player[i].ingame_key._vertical_key == 0)) {
                        if (player[i].ingame_key._horizontal_key == 1 || player[i].ingame_key._vertical_key == -1) {
                            player[i].state = IdleB;
                        }
                        else {
                            player[i].state = IdleA;
                        }
                    }                                            
                    player[i].ingame_key._horizontal_key = 0;
                    player[i].ingame_key._skill_key = 0;
                    player[i].ingame_key._vertical_key = 0;
                }

                ZeroMemory(buf, BUFSIZE);
            }
        }

        // ���â
    }
    // ���� �ݱ�
    closesocket(listen_sock);

    // �̺�Ʈ ����
    for (int i = 0; i < 2; ++i) {
        CloseHandle(hRecvEvent[i]);
    }
    CloseHandle(hRootEvent);
    // ���� ����
    WSACleanup();
    return 0;
}