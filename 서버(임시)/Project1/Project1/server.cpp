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
    // 데이터 통신에 사용할 변수
    int retval;
    SOCKET client_sock = (SOCKET)arg;
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1]; // 가변 길이 데이터

    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

    while (1) {

        // 데이터 크기 받기
        // 데이터 타입 확인

        // 클라이언트와 데이터 통신
        while (1) {
            // 데이터 받기(가변 길이)
            retval = recv(client_sock, buf, BUFSIZE, MSG_WAITALL);
            if (retval == SOCKET_ERROR) {
                err_display("recv()");
                break;
            }
            else if (retval == 0)
                break;

            // 데이터 받은 후 받은 데이터 공용 데이터에 업데이트
            // 이후 반복
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

    while (cnt != 3) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock[cnt] = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock[cnt] == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 스레드 생성
        hThread[cnt] = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock[cnt], 0, NULL);
        if (hThread == NULL) { closesocket(client_sock[cnt]); }
        else { 
            // 초기 설정 클라 아이디 송신
            SC_Lobby_Send cl;
            cl._protocol_num = SC_lobby_send;
            cl._acc_count = cnt;
            cl._my_num = cnt;
            send(client_sock[cnt], reinterpret_cast<char*>(&cl), sizeof(cl), 0);
            cnt++;
        }
    }

    // 임시 플레이어 데이터 세팅
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
        sc._protocol_num = SC_scene_send;
        sc._scene_num = Main_game;
        prevTime = 100;

        // 각 클라이언트에 씬데이터 송신
        for (int i = 0; i < 3; ++i) {
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
        is._coin_location = { 1,1 };
        is._left_time = prevTime;
        is._protocol_num = SC_ingame_send;

        // 각 클라이언트에 인게임 초기 데이터 송신
        for (int i = 0; i < 3; ++i) {
            send(client_sock[i], reinterpret_cast<char*>(&is), sizeof(is), 0);
        }

        auto start = chrono::system_clock::now();

        // while 문에서 매인 게임 문 실행
        while (true)
        {
            auto now = chrono::system_clock::now();

            chrono::duration<float> pip = start - now;

            elapsedTime = prevTime - pip.count();

            if (elapsedTime <= 0) {
                // 게임 종료 및 로비 씬으로 전환
                // 바로 종료 안하고 게임 끝나는 화면으로 전환 할수도있음
            }
            else {
                // 충돌 처리 및 공용 데이터 업데이트
                // 쿨타임 업데이트
                // 업데이트 된 데이터 송신
                // 리시브 받은 데이터가 없어도 계속 업데이트 후 송신 반복


            }
        }
    }
    // 소켓 닫기
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}