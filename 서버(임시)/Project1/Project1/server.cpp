#include "Common.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include "protocol.h"

using namespace std;

#define TILE_SIZE 64

#define SERVERPORT 9000
#define BUFSIZE    10

G_data player[3];

//HANDLE hRcev1Event, hRcev2Event, hRcev3Event; // �̺�Ʈ
HANDLE hRecvEvent[3];
HANDLE hRootEvent;

//�ڽ��� ���� �������� ��ȣ�� ��ȯ
int calc_next_thread(int i) {
    return (i + 1) % 3;
}
//�ڽ��� ���� �������� ��ȣ�� ��ȯ
int calc_prev_thread(int i) {
    return (i + 2) % 3;
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

    for (int i = 0; i < 3; i++) {
        if (player[i].my_num == GetCurrentThreadId()) {
            my_num = i;
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
            //cout << "�������� �ѹ�" << protocol_num << endl;

            switch (protocol_num) {
            case CS_ingame_send: // ingame_key_send
                retval = recv(client_sock, reinterpret_cast<char*>(&ingame_key), sizeof(ingame_key), MSG_WAITALL);
                //cout << "Recv Thread : " << my_num << " is Work" << endl;
                WaitForSingleObject(hRecvEvent[my_num], INFINITE);
                player[my_num].ingame_key = ingame_key;
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

        // Ŭ���̾�Ʈ�� ������ ���
        //while (1) {
        //    DWORD retval;

        //    // ������ �ޱ�(���� ����)
        //    retval = recv(client_sock, buf, BUFSIZE, MSG_WAITALL);
        //    if (retval == SOCKET_ERROR) {
        //        err_display("recv()");
        //        break;
        //    }
        //    else if (retval == 0)
        //        break;

        //    //���� �����尡 �ڽſ��� ����Ҷ����� ���
        //    retval = WaitForSingleObject(hRecvEvent[my_num], INFINITE);
        //    if (retval != WAIT_OBJECT_0) break;


        //    // ������ ���� �� ���� ������ ���� �����Ϳ� ������Ʈ
        //    
        //    //root �̺�Ʈ�� ������
        //    SetEvent(hRootEvent);

        //    // ���� �ݺ�


        //}
       
        break;
    }
    return 0;
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

    DWORD event_retval; //��� �����

    //�̺�Ʈ ����
    for (int i = 0; i < 3; ++i) {
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

    while (cnt != 1) {
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
        hThread[cnt] = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock[cnt], 0, NULL);
        if (hThread == NULL) { closesocket(client_sock[cnt]); }
        else { 
            // �ʱ� ���� Ŭ�� ���̵� �۽�
            buf[0] = SC_lobby_send;
            SC_Lobby_Send cl;
            cl._acc_count = cnt;
            player[cnt].my_num = GetCurrentThreadId();

            for (int i = 0; i < cnt; ++i) {
                send(client_sock[cnt], buf, BUFSIZE, 0);
                send(client_sock[cnt], reinterpret_cast<char*>(&cl), sizeof(cl), 0);
            }
            cnt++;
        }
    }

    // �ӽ� �÷��̾� ������ ����
    for (int i = 0; i < 3; ++i) {
        player[i].charType = i;
        player[i].charLook = 0;
        player[i].location = { 35 * TILE_SIZE, (i+15) * TILE_SIZE};
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
        buf[0] = SC_scene_send;
        sc._scene_num = Main_game;
        prevTime = 100;

        // �� Ŭ���̾�Ʈ�� �������� �۽�
        for (int i = 0; i < cnt; ++i) {
            send(client_sock[i], buf, BUFSIZE, 0);
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
        is._coin_location = { 34 * 64, 15* 64 };
        is._left_time = prevTime;
        buf[0] = SC_ingame_send;

        // �� Ŭ���̾�Ʈ�� �ΰ��� �ʱ� ������ �۽�
        for (int i = 0; i < cnt; ++i) {
            send(client_sock[i], buf, BUFSIZE, 0);
            send(client_sock[i], reinterpret_cast<char*>(&is), sizeof(is), 0);
        }

        auto start = chrono::system_clock::now();

        int _x = 10;
        int _y = 10;

        // while ������ ���� ���� �� ����
        while (true)
        {
            auto now = chrono::system_clock::now();

            chrono::duration<float> pip = now - start;

            elapsedTime = prevTime - pip.count();

            //cout << elapsedTime << endl;
            //cout << pip.count() << endl;

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


            if (elapsedTime <= 0) {
                // ���� ���� �� �κ� ������ ��ȯ
                // �ٷ� ���� ���ϰ� ���� ������ ȭ������ ��ȯ �Ҽ�������
            }

            else { // ���� ������Ʈ
                // �浹 ó�� �� ���� ������ ������Ʈ
                for (int i = 0; i < cnt; ++i) {
                    if (player[i].ingame_key._skill_key == 0) { // ��ų x �̵���
                        if (player[i].ingame_key._horizontal_key == 1 && player[i].location.x < 6350) {
                            if (Tileindex[player[i].location.x + 60 / 64][player[i].location.y / 64] == 1) // ���ӹ���
                                player[i].location.x += _x + 5;
                            else if (Tileindex[player[i].location.x / 64][player[i].location.y / 64] == 0) {} // ��
                            else
                                player[i].location.x += _x;

                            player[i].charLook = 4;
                        }
                        else if (player[i].ingame_key._horizontal_key == -1 && player[i].location.x > 50) {
                            if (Tileindex[player[i].location.x - 60 / 64][player[i].location.y / 64] == 1) // ���ӹ���
                                player[i].location.x -= _x - 5;
                            else if (Tileindex[player[i].location.x / 64][player[i].location.y / 64] == 0) {} // ��
                            else // �Ϲ� ����
                                player[i].location.x -= _x;

                            player[i].charLook = 2;
                        }
                        if (player[i].ingame_key._vertical_key == 1 && player[i].location.y > 50) {
                            if (Tileindex[player[i].location.x / 64][player[i].location.y - 60 / 64] == 1) // ���ӹ���
                                player[i].location.y -= _y - 5;
                            else if (Tileindex[player[i].location.x / 64][player[i].location.y / 64] == 0) {} // ��
                            else // �Ϲ� ����
                                player[i].location.y -= _y;

                            player[i].charLook = 3;
                        }
                        else if (player[i].ingame_key._vertical_key == -1 && player[i].location.y < 6350) {
                            if (Tileindex[player[i].location.x / 64][player[i].location.y + 60 / 64] == 1) // ���ӹ���
                                player[i].location.y += _y + 5;
                            else if (Tileindex[player[i].location.x / 64][player[i].location.y / 64] == 0) {} // ��
                            else // �Ϲ� ����
                                player[i].location.y += _y;

                            player[i].charLook = 5;
                        }
                        player[i].state = Walk;
                    }
                    else if (player[i].ingame_key._skill_key == 1) { // 1�� ��ų
                        if (player[i].ingame_key._horizontal_key == 1) {
                            player[i].charLook = 4;
                        }
                        else if (player[i].ingame_key._horizontal_key == -1) {
                            player[i].charLook = 2;
                        }
                        if (player[i].ingame_key._vertical_key == 1) {
                            player[i].charLook = 3;
                        }
                        else if (player[i].ingame_key._vertical_key == -1) {
                            player[i].charLook = 5;
                        }

                        player[i].state = Skill;
                    }
                    else if (player[i].ingame_key._skill_key == 2) { // ����
                        if (player[i].ingame_key._horizontal_key == 1) {
                            player[i].charLook = 4;
                        }
                        else if (player[i].ingame_key._horizontal_key == -1) {
                            player[i].charLook = 2;
                        }
                        if (player[i].ingame_key._vertical_key == 1) {
                            player[i].charLook = 3;
                        }
                        else if (player[i].ingame_key._vertical_key == -1) {
                            player[i].charLook = 5;
                        }

                        player[i].state = Attack;
                    }
                    else if (player[i].ingame_key._skill_key == 3) { // �뽬
                        if (player[i].ingame_key._horizontal_key == 1) player[i].location.x += _x * 5;
                        else if (player[i].ingame_key._horizontal_key == -1) player[i].location.x -= _x * 5;

                        if (player[i].ingame_key._vertical_key == 1) player[i].location.y += _y * 5;
                        else if (player[i].ingame_key._vertical_key == -1) player[i].location.y -= _y * 5;

                        player[i].state = Dash;
                    }

                    // ���� �浹 Ȯ��
                }

                // ��Ÿ�� ������Ʈ
                for (int i = 0; i < 3; ++i) {
                    // �ð� �� Ȯ�� �� ���� �ð� ��ŭ ���� ��Ÿ�ӿ��� ����
                }

                // ������Ʈ �� ������ �۽�
                SC_Ingame_Send _is;
                for (int i = 0; i < 3; ++i) {
                    _is._player[i]._char_type = player[i].charType;
                    _is._player[i]._coin = player[i].coin;
                    _is._player[i]._location = player[i].location;
                    _is._player[i]._look = player[i].charLook;
                    _is._player[i]._skill_cooltime1 = player[i].skill_cooltime1;
                    _is._player[i]._skill_cooltime2 = player[i].skill_cooltime2;
                    _is._player[i]._state = player[i].state;
                }
                _is._left_time = elapsedTime;
                _is._coin_location = { 34 * 64 ,15 * 64 }; // ���Ŀ� ���� �ʿ�

                for (int i = 0; i < cnt; ++i) {
                    cout << i << " X : " << player[i].location.x << " Y : " << player[i].location.y << endl;
                }


                buf[0] = SC_ingame_send;

                for (int i = 0; i < cnt; ++i) {
                    send(client_sock[i], buf, BUFSIZE, 0);
                    send(client_sock[i], reinterpret_cast<char*>(&_is), sizeof(_is), 0);
                }

                for (int i = 0; i < cnt; i++) {
                    player[i].ingame_key._horizontal_key = 0;
                    player[i].ingame_key._skill_key = 0;
                    player[i].ingame_key._vertical_key = 0;
                    player[i].state = Idle;
                }
            }
        }

        // ���â
    }
    // ���� �ݱ�
    closesocket(listen_sock);

    // �̺�Ʈ ����
    for (int i = 0; i < 3; ++i) {
        CloseHandle(hRecvEvent[i]);
    }
    CloseHandle(hRootEvent);
    // ���� ����
    WSACleanup();
    return 0;
}