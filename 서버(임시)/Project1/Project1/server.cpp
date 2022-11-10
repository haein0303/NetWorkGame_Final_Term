#include "Common.h"
#include <iostream>
#include <fstream>

using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    512

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
            // 충돌 처리를 어디서 할 지는 몰?루
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
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE + 1]; // 가변 길이 데이터
    HANDLE hThread[3];

    while (cnt != 3) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 접속한 클라이언트 정보 출력
        printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        // 스레드 생성
        hThread[cnt] = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) { closesocket(client_sock); }
        else { 
            // 초기 설정 클라 아이디 송신 해줘야 함
            cnt++;
        }
    }

    // 세명 접속 이후 캐릭터 선택 창으로 변경 되는 부분
    {
        // 서버에서 씬넘버 변경 후 타이머 설정
        // for 루프문으로 전체 클라이언트에 씬 넘버, 초기 설정값등 송신

        {
            // while 문 설정 해서 시간 다 될때까지 클라이언트 캐릭터 선택 정보 받아서 모두 선택 완료되거나 시간 종료 시 매인 게임 문 실행
            // 이때 시간 다 지났을 경우 현재 클라 설정으로 캐릭터 세팅
        }
        
        // while문 종료 시 씬넘버 변경 후 메인게임 초기화 데이터 송신
    }

    // 메인 게임 부분
    {
        // while 문에서 매인 게임 문 실행
        // 이때 매 루프마다 리시브받은 데이터가 존재 할 경우 바뀐 데이터로 송신
        // 쿨타임의 경우 리시브 받은 시간을 토대로 시간 체크 -> 리시브 받은 순간에 절대 시간 측정 후 저장 ->
        // 다음 업데이트 시에 시간 계산 후 갱신된 정보 송신

        // 리시브 받은 데이터가 없어도 계속 업데이트 후 송신 반복

        // 매인 게임 시간 0일 경우 데이터 확인 후 씬 변경 송신 및 캐릭터 중앙부 이동
    }
    // 소켓 닫기
    closesocket(listen_sock);

    // 윈속 종료
    WSACleanup();
    return 0;
}