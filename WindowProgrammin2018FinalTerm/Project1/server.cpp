#include "Common.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include "protocol.h"

using namespace std;

#define TILE_SIZE 64

#define SERVERPORT 9000
#define BUFSIZE    512

G_data player[3];

//HANDLE hRcev1Event, hRcev2Event, hRcev3Event; // 이벤트
HANDLE hRecvEvent[3];
HANDLE hRootEvent;

//자신의 다음 쓰레드의 번호를 반환
int calc_next_thread(int i) {
    return (i + 1) % 3;
}
//자신의 이전 쓰레드의 번호를 반환
int calc_prev_thread(int i) {
    return (i + 2) % 3;
}

//Recv 쓰레드
DWORD WINAPI ProcessClient(LPVOID arg)
{
    // 데이터 통신에 사용할 변수
    int retval;
    SOCKET client_sock = (SOCKET)arg;
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1]; // 가변 길이 데이터

    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    int my_num = 0; //자기 자신의 배정 번호를 저장

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
            // 임시 에코 서버
            retval = recv(client_sock, buf, BUFSIZE, 0);

            protocol_num = (int)buf[0];
            //cout << "프로토콜 넘버" << protocol_num << endl;

            switch (protocol_num) {
            case CS_ingame_send: // ingame_key_send
                retval = recv(client_sock, reinterpret_cast<char*>(&ingame_key), sizeof(ingame_key), MSG_WAITALL);
                player[my_num].ingame_key = ingame_key;
                if (retval == SOCKET_ERROR) {
                    err_display("recv()");
                    break;
                }
                else if (retval == 0) {
                    break;
                }
                break;

            }
        }

        // 클라이언트와 데이터 통신
        //while (1) {
        //    DWORD retval;

        //    // 데이터 받기(가변 길이)
        //    retval = recv(client_sock, buf, BUFSIZE, MSG_WAITALL);
        //    if (retval == SOCKET_ERROR) {
        //        err_display("recv()");
        //        break;
        //    }
        //    else if (retval == 0)
        //        break;

        //    //이전 쓰레드가 자신에게 허락할때까지 대기
        //    retval = WaitForSingleObject(hRecvEvent[my_num], INFINITE);
        //    if (retval != WAIT_OBJECT_0) break;


        //    // 데이터 받은 후 받은 데이터 공용 데이터에 업데이트
        //    
        //    //root 이벤트를 돌려줌
        //    SetEvent(hRootEvent);

        //    // 이후 반복


        //}

        break;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int retval;
    int cnt = 0;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // 소켓 생성
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

    // 데이터 통신에 사용할 변수
    SOCKET client_sock[3];
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1]; // 가변 길이 데이터
    HANDLE hThread[3];

    DWORD event_retval; //결과 저장용

    //이벤트 생성
    for (int i = 0; i < 3; ++i) {
        hRecvEvent[i] = CreateEvent(NULL, FALSE, TRUE, NULL);
    }

    char Inbuff[20000] = { 0 };
    DWORD read_size = 20000;
    DWORD c = 20000;

    HANDLE hFile;
    int Tileindex[100][100] = { 0 };

    hFile = CreateFile(L"map.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
    memset(Inbuff, 0, 99 * sizeof(WCHAR));
    ReadFile(hFile, Inbuff, c, &read_size, NULL); // hFile에서 size 만큼 읽어 InBuff에 저장
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

    while (cnt != 1) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock[cnt] = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock[cnt] == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 스레드 생성
        hThread[cnt] = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock[cnt], 0, NULL);
        if (hThread == NULL) { closesocket(client_sock[cnt]); }
        else {
            // 초기 설정 클라 아이디 송신
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

    // 임시 플레이어 데이터 세팅
    for (int i = 0; i < 3; ++i) {
        player[i].charType = i;
        player[i].charLook = 0;
        player[i].location = { (float)35 * TILE_SIZE,(float)(i + 15) * TILE_SIZE };
        player[i].state = Idle;
        player[i].coin = false;
        player[i].skill_cooltime1 = 0;
        player[i].skill_cooltime2 = 0;
        player[i].attack_on = false;
        player[i].skill_on = false;
    }
    // end

    // 세명 접속 이후 캐릭터 선택 창으로 변경 되는 부분
    {
        // 서버에서 씬넘버 변경 후 타이머 설정


        // 각 클라이언트에 씬데이터 송신

        // for 루프문으로 전체 클라이언트에 씬 넘버, 초기 설정값등 송신


        // 각 클라이언트에 인게임 초기 데이터 송신

        // while 문 설정 해서 시간 다 될때까지 클라이언트 캐릭터 선택 정보 받아서 모두 선택 완료되거나 시간 종료 시 매인 게임 문 실행
        // 이때 시간 다 지났을 경우 현재 클라 설정으로 캐릭터 세팅

        // while문 종료 시 씬넘버 변경 후 메인게임 초기화 데이터 송신
    }

    // 메인 게임 부분
    {
        // 서버에서 씬넘버 변경 후 타이머 설정
        SC_Scene_Send sc;
        buf[0] = SC_scene_send;
        sc._scene_num = Main_game;
        prevTime = 100;

        // 각 클라이언트에 씬데이터 송신
        for (int i = 0; i < cnt; ++i) {
            send(client_sock[i], buf, BUFSIZE, 0);
            send(client_sock[i], reinterpret_cast<char*>(&sc), sizeof(sc), 0);
        }

        // for 루프문으로 전체 클라이언트에 씬 넘버, 초기 설정값등 송신
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
        is._coin_location = { 34 * 64, 15 * 64 };
        is._left_time = prevTime;
        buf[0] = SC_ingame_send;

        // 각 클라이언트에 인게임 초기 데이터 송신
        for (int i = 0; i < cnt; ++i) {
            send(client_sock[i], buf, BUFSIZE, 0);
            send(client_sock[i], reinterpret_cast<char*>(&is), sizeof(is), 0);
        }

        auto start = chrono::system_clock::now();

        float _x = 1;
        float _y = 1;

        // while 문에서 매인 게임 문 실행
        while (true)
        {
            auto now = chrono::system_clock::now();

            chrono::duration<float> pip = now - start;

            elapsedTime = prevTime - pip.count();

            //cout << elapsedTime << endl;
            //cout << pip.count() << endl;

            //for (int i = 0; i < 3; ++i) {
            //    //대기
            //    SetEvent(hRecvEvent[i]);
            //    //소환
            //    event_retval = WaitForSingleObject(hRootEvent, INFINITE);
            //    if (event_retval != WAIT_OBJECT_0) break;
            //}


            if (elapsedTime <= 0) {
                // 게임 종료 및 로비 씬으로 전환
                // 바로 종료 안하고 게임 끝나는 화면으로 전환 할수도있음
            }

            else {
                // 충돌 처리 및 공용 데이터 업데이트
                for (int i = 0; i < 3; ++i) {
                    if (player[i].ingame_key._skill_key == 0) { // 스킬 x 이동만

                        if (player[i].ingame_key._horizontal_key == 1) player[i].location.x += _x;
                        else if (player[i].ingame_key._horizontal_key == -1) player[i].location.x -= _x;

                        if (player[i].ingame_key._vertical_key == 1) player[i].location.y += _y;
                        else if (player[i].ingame_key._vertical_key == -1) player[i].location.y -= _y;

                        player[i].state = Walk;
                    }
                    else if (player[i].ingame_key._skill_key == 1) { // 1번 스킬

                    }
                    else if (player[i].ingame_key._skill_key == 2) { // 공격

                    }
                    else if (player[i].ingame_key._skill_key == 3) { // 대쉬
                        if (player[i].ingame_key._horizontal_key == 1) player[i].location.x += _x * 10;
                        else if (player[i].ingame_key._horizontal_key == -1) player[i].location.x -= _x * 10;

                        if (player[i].ingame_key._vertical_key == 1) player[i].location.y += _y * 10;
                        else if (player[i].ingame_key._vertical_key == -1) player[i].location.y -= _y * 10;

                        player[i].state = Dash;
                    }
                    // 맵 충돌 확인

                    // 공격 충돌 확인
                }

                // 쿨타임 업데이트
                for (int i = 0; i < 3; ++i) {
                    // 시간 값 확인 후 지난 시간 만큼 남은 쿨타임에서 감소
                }

                // 업데이트 된 데이터 송신
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
                _is._coin_location = { 34 * 64 ,15 * 64 }; // 추후에 수정 필요

                for (int i = 0; i < 1; ++i) {
                    cout << i << " X : " << player[i].location.x << " Y : " << player[i].location.y << endl;

                }


                buf[0] = SC_ingame_send;

                for (int i = 0; i < cnt; ++i) {
                    send(client_sock[i], buf, BUFSIZE, 0);
                    send(client_sock[i], reinterpret_cast<char*>(&_is), sizeof(_is), 0);
                }
            }
        }

        // 결과창
    }
    // 소켓 닫기
    closesocket(listen_sock);

    // 이벤트 제거
    for (int i = 0; i < 3; ++i) {
        CloseHandle(hRecvEvent[i]);
    }

    // 윈속 종료
    WSACleanup();
    return 0;
}