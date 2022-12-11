// ���� ���α׷��� �������� �����մϴ�.
//
#include "stdafx.h"
#include "WindowProgrammin2018FinalTerm.h"
#include "Framework.h"
#include "Common.h"
#include <chrono>
//#include "protocol.h"

//�۷ι� Ű �������Դϴ�.
CS_ingame_send_tmp gKeyData;

SC_Scene_Send g_scene_send;
SC_Ingame_Send g_ingame_send;
SC_Lobby_Send g_lobby_send;
G_data gPldata;

CRITICAL_SECTION g_cs;

int gMy_num = -1;


#define MAX_LOADSTRING	100
#define CLIENT_WIDTH	1920
#define CLIENT_HEIGHT	1080

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.
int windowX = ::GetSystemMetrics(SM_CXSCREEN);  //����� x����
int windowY = ::GetSystemMetrics(SM_CYSCREEN);  //����� y����

CFramework myFramework;

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);



SOCKET sock; // ����

G_data player;


// TCP Ŭ���̾�Ʈ ���� �κ�
DWORD WINAPI ClientMain(LPVOID arg)
{
	
	InitializeCriticalSection(&g_cs);
	int retval;
	char r_buf[BUFSIZE + 1]; // ������ ���� ����
	bool b_ingame_setting_checker_toggle = false;

	// ���� ����
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");


	{
		g_ingame_send._player[0]._location = { 35 * 64, (0 + 15) * 64 };
		g_ingame_send._player[1]._location = { 35 * 64, (1 + 15) * 64 };
		g_ingame_send._player[2]._location = { 35 * 64, (2 + 15) * 64 };
	}

	// ������ ������ ���
	while (1) {
		int protocol_num;
		SC_Ingame_Send init_setting;
		int user_num[3] = { -1,-1,-1 };
		// ������ �ޱ�
		retval = recv(sock, r_buf, BUFSIZE, 0);
		//cout << "���ú� �ϱ� �ϴ�?" << endl;
		protocol_num = (int)r_buf[0];
		//cout << "�Ѱܹ��� ��ȣ : " << protocol_num << endl;
		switch (protocol_num){
		case SC_ProtocalInfo::SC_lobby_send: //��� �����ߴ��� Ȯ��, �� ��ȣ�� Ȯ�� ����
			
			retval = recv(sock, reinterpret_cast<char*>(&g_lobby_send), sizeof(g_lobby_send), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0) break;
			if (gMy_num == -1) {
				gMy_num = g_lobby_send._acc_count;
				cout << "�ѹ� �޾Ҵ�!" << gMy_num << endl;//���� ���� Ȯ�οϷ� 20221202 0120
			}
			cout << "�� ���� ������ : " << g_lobby_send._acc_count + 1 << endl;

			break;
		case SC_ProtocalInfo::SC_scene_send: //scene ���� ���� ��
			retval = recv(sock, reinterpret_cast<char*>(&g_scene_send), sizeof(g_scene_send), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0) break;
			//Scene�� �Ѱ��ִ� �Է°��� Ȯ���ϰ�, ���� �����մϴ�.
			//g_pFramework < m_pFramework �־���� ��
			//���� ������������ ������, �����ʿ�
			cout << "�Ѱ��ִ� SCENE : " << g_scene_send._scene_num << endl;
			switch (g_scene_send._scene_num)//sc._scene_num
			{
			case Scene::Char_sel:
				myFramework.ChangeScene(CScene::SceneTag::Select_Char);
				break;
			case Scene::Main_game:
				myFramework.updateCTR(1);
				myFramework.ChangeScene(CScene::SceneTag::Ingame);
				myFramework.curSceneCreate();
				//myFramework.BuildPlayer(1, 2, 3);
				cout << "126 :: main :: " <<
					init_setting._player[0]._char_type <<
					init_setting._player[1]._char_type<<
					endl;
				myFramework.BuildPlayer(init_setting._player[gMy_num]._char_type+1,
					init_setting._player[calcNetId(gMy_num, 1)]._char_type+1,
					3);
				myFramework.updateCTR(0);
				break;
			case Scene::Lobby:
				myFramework.ChangeScene(CScene::SceneTag::Main_Lobby);
				break;
			default:
				break;
			}  
				

			break;

		case SC_ProtocalInfo::SC_ingame_send: //in game
			SC_Ingame_Send tmp_send;
			retval = recv(sock, reinterpret_cast<char*>(&tmp_send), sizeof(tmp_send), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0) break;
			//cout << "0�� ���� ���� X : " << tmp_send._player[0]._location.x << " Y : " << tmp_send._player[0]._location.y << endl;
			//cout << "1�� ���� ���� X : " << tmp_send._player[1]._location.x << " Y : " << tmp_send._player[1]._location.y << endl;
			//cout << "2�� ���� ���� X : " << tmp_send._player[2]._location.x << " Y : " << tmp_send._player[2]._location.y << endl;
			
			
			//�ʱ�ȭ ������ �˻�
			if (tmp_send._left_time < 0) {
				init_setting = tmp_send;
				user_num[0] = gMy_num;
				user_num[1] = calcNetId(gMy_num, 1);
				user_num[2] = calcNetId(gMy_num, 2);
				break;
			}
			::EnterCriticalSection(&g_cs);
				g_ingame_send = tmp_send;						
			::LeaveCriticalSection(&g_cs);		
			

			break;
		}
		

		//���� ������ ������ ���� (�������ݹޱ�)
		
	}

	return 0;
}



///////////////////////////////////////////////////////////////////////////////////////////

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ���⿡ �ڵ带 �Է��մϴ�.
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	
	


	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDOWPROGRAMMIN2018FINALTERM, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);	// Ŭ������ ����Ѵٴ� ���̴�. ������ Ŭ������ ���¸� �����ش�. 


								// ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// ���� ��� ������ ����
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWPROGRAMMIN2018FINALTERM));

	MSG msg;
	// �⺻ �޽��� �����Դϴ�.
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))	// GetMessage(&msg, nullptr, 0, 0)
														// PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)
														// ���� ������ ����������� ����������, ���� Ŭ���Ҷ����� �����´�.
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			if (msg.message == WM_QUIT) break; // esc ������ ������ ����.
			if (msg.message != WM_MOUSEMOVE && msg.message != WM_CHAR)
			{
				TranslateMessage(&msg);	// �޽��� �̵�
				DispatchMessage(&msg);	// �޽��� �İ�
			}
		}
		// ���⼭ �������� ���� ���α׷��� ������ �Ѵ�.
		// FrameAdvance�� ���� ���α׷��� ������.
		myFramework.FrameAdvance();


	}

	return (int)msg.wParam;
}

//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;	// WND CLASS EX W == '������ Ŭ���� Ȯ�� �����ڵ�' ��� ���� ����ü�̴�.
						// { style , WndProc, hInstance, ClassName, return RegisterClassW }
						// style�� msdn ���� ���� �˾Ƽ� ������ �Ѵ�.
						// WndProc�� �ݹ� �Լ��̴�. Ȱ��ȭ�� Input���� Getmsg�� �޾Ƽ� �ҷ��´�.
						// hInstance�� main �ܼ��Լ������� �ִ�. GetModuleHandle(NULL)�Լ��ε� ������ �� �ִ�.
						// hInstance�� ���μ����� ���̵��̴�. ���̵� �����ͼ� ���α׷��� ���� �ĺ���ȣ�� ����Ѵ�.
						// ClassName�� �� �״�� Ŭ������ �̸��� �����Ѵ�.

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style =
		CS_HREDRAW		// Ŭ���̾�Ʈ�� �ʺ� �����ϸ�, ��ü �����츦 �ٽ� �׸��� �Ѵ�.	WM{_SIZE�� ���� ����.
		| CS_VREDRAW	// Ŭ���̾�Ʈ�� ���̸� �����ϸ�, ��ü �����츦 �ٽ� �׸��� �Ѵ�.
						//  | CS_DBLCLKS	// �ش� �����쿡�� ����Ŭ���� ����ؾ� �Ѵٸ� �߰��ؾ� �Ѵ�.
		;
	wcex.lpfnWndProc = CFramework::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_WINDOWPROGRAMMIN2018FINALTERM));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL; // MAKEINTRESOURCEW(IDC_CSTUDYWINDOW);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// ���� ������ �ڵ�
	HWND hWnd;

	// ������ ��Ÿ��
	DWORD dwStyle =
		WS_OVERLAPPED			// ����Ʈ ������. Ÿ��Ʋ �ٿ� ũ�� ������ �ȵǴ� ��輱�� ������. �ƹ��� ��Ÿ�ϵ� ���� ������ �� ��Ÿ���� ����ȴ�.
		| WS_CAPTION			// Ÿ��Ʋ �ٸ� ���� �����츦 ����� WS_BORDER ��Ÿ���� �����Ѵ�.
		| WS_SYSMENU			// �ý��� �޴��� ���� �����츦 �����.
		| WS_MINIMIZEBOX		// �ּ�ȭ ��ư�� �����.
		| WS_BORDER				// �ܼ����� �� ��輱�� ����� ũ�� ������ �� �� ����.
								//	  | WS_THICKFRAME		// ũ�� ������ ������ �β��� ��輱�� ������. WS_BORDER�� ���� ����� �� ����.
		;						// �߰��� �ʿ��� ������ ��Ÿ���� http://www.soen.kr/lecture/win32api/reference/Function/CreateWindow.htm ����

	hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

	RECT getWinSize;
	GetWindowRect(GetDesktopWindow(), &getWinSize);

	// Ŭ���̾�Ʈ ������
	RECT rc;
	
	rc.left = rc.top = 0;
	rc.right = windowX;
	rc.bottom = windowY;
	// ������ ����� ������ �߰��Ǵ� (ĸ��, �ܰ��� ��) ũ�⸦ ����.
	AdjustWindowRect(&rc, dwStyle, FALSE);

	// Ŭ���̾�Ʈ ���� ��ǥ(left, top)
	// ����ũ���� �߾ӿ� Ŭ���̾�Ʈ�� ��ġ�ϵ��� ����
	POINT ptClientWorld;
	ptClientWorld.x = (getWinSize.right - windowX) / 2;
	ptClientWorld.y = (getWinSize.bottom - windowY) / 2;

	/*
	HWND hWnd = CreateWindowW(
	szWindowClass			// ������ Ŭ���� ��
	, szTitle				// ĸ�� ǥ�� ���ڿ�
	, WS_OVERLAPPEDWINDOW	// ������ ��Ÿ��
	, CW_USEDEFAULT			// �θ� ������ ��� ������ ������ǥ : x
	, 0						// �θ� ������ ��� ������ ������ǥ : y
	, CW_USEDEFAULT			// ������ ������ : width
	, 0						// ������ ������ : height
	, nullptr				// �θ� ������.
	, nullptr				// �޴� �ڵ�
	, hInstance				// �ν��Ͻ� �ڵ�
	, nullptr);				// �߰� �Ķ��Ÿ : NULL
	*/

	hWnd = CreateWindowW(
		szWindowClass			// ������ Ŭ���� ��
		, szTitle				// ĸ�� ǥ�� ���ڿ�
		, dwStyle				// ������ ��Ÿ��
		, ptClientWorld.x		// �θ� ������ ��� ������ ������ǥ : x
		, ptClientWorld.y		// �θ� ������ ��� ������ ������ǥ : y
		, windowX		// ������ ������ : width
		, windowY		// ������ ������ : height
		, nullptr				// �θ� ������.
		, nullptr				// �޴� �ڵ�
		, hInstance				// �ν��Ͻ� �ڵ�
		, nullptr);				// �߰� �Ķ��Ÿ : NULL

								// CreateWindow �Լ���? 
								// ���� ������ �����찡 �ִµ�, �̴�� ����ڴ� ��� ���̴�. 
								// ��������Ŭ������ ������ �İ�, �� �Լ��� ù ���ڷ� ������ ã�´�.
								// 2��° ���ڴ� ĸ������, Ÿ��Ʋ �̸��̴�.
								// 3��° �÷��״� msdn ����
								// ȭ���� �»�� x
								// ȭ���� �»�� y
								// ���� ����� â�� ���� ũ��
								// ���� ����� â�� ���� ũ��
								// �� �ڷδ� null null hInstance(���μ��� �ּ�) null

								// ���� ���н� ���α׷� ����   
	if (!hWnd) return FALSE;
	if (!myFramework.OnCreate(hInstance, hWnd, rc)) return FALSE;
	// ������ ǥ��
	ShowWindow(hWnd, nCmdShow); // ������ â�� �����ش�. �̰� ������ ������ â�� �� ���̱� ������ Ȱ��ȭ�� �ȵǹǷ� �ƹ��͵� ���Ѵ�. ���� ������!
								// Ȯ�� : WnbdProc�� default msg handler�� DefWindowProc
								// �Լ��� ��ȯ�ϴ°�?
	ShowCursor(FALSE);
	UpdateWindow(hWnd);

	// ���� ��ȯ
	return TRUE;
}

