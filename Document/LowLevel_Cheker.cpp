
enum Scene {
    Lobby,
    Char_sel,
    Main_game,
    End_game
}

enum CharState {
    Idle,
    Walk,
    Attack,
    Attacked,
    Dash,
    Skill,
    Win,
    Lose,
}
// ver 1.0
//enum ProtocalInfo{ //현재 전송하는 정보
//	Timer,
//	PlayerData,
//	Scene_Data
//	//….
//}

//send -> ProtocolInfo + struct  조합으로 데이터 전송

//ver 2.0 : add
enum SC_ProtocalInfo { //현재 전송하는 정보
    SC_Ingame_send,
    SC_lobby_send,
    SC_scene_send,
    SC_LobbytoCharsel_init,
    SC_CharseltoIngame_init,
    SC_IngametoFinish_init,
    SC_FinishtoLobby_init,
}

//ver 2.0 : add
enum CS_ProtocalInfo { //현재 전송하는 정보
    CS_ingame_send, //or CS_ingame_send_temp
    CS_lobby_send,
}
/////////////////////////////////////////////

struct fvec2{	//플레이어 좌표 구조체
	float	x;
	float	y;
}

// ver 2.0 del
//struct Gtimer{
//	float 	_left_time;
//	float 	_skill_cooltime1;
//	float 	_skill_cooltime2;
//}
 // ver 2.0 del
//struct player_data{
//	int 		_char_type;	
//	fvec2 		_location;
//	int 		_look;
//	int 		_state;
//  bool 		_coin;
//}

//ver 2.0 : add
struct Character_data {
    int 		_char_type;
    fvec2       _location;
    int 		_look;
    int 		_state;
    bool 		_coin;
    float 	    _skill_cooltime1;
    float 	    _skill_cooltime2;
};

//ver 2.0 : add
struct SC_Ingame_send { //프레임마다 전송하는 데이터
    Character_data  _player[3];
    fvec2           _coin_location;
    float           _left_time;
};

//ver 2.0 : edit
struct SC_lobby_send{	//새로운 acc 있을 때마다 보내줌 
	int	    _acc_count;  
	//int 	_my_num; // ver 2.0 del
}


//공통
struct SC_scene_send{ // 씬데이터
    int 	_scene_num;
    int     _protocol_num;
}

// 클라이언트 초기화할 때 사용하는 구조체들
struct SC_LobbytoCharsel_init{ //로비 -> 캐 선택
    int 	_scene_num;
    int     _protocol_num;
    int     _char_num;  //접속한 순서대로 플레이어에게 부여되는 플레이어 넘버
    int     _char_type;
    bool 	_ready;
}

struct SC_CharseltoIngame_init { //캐선택 -> 인게임
    int 	_scene_num;
    int     _protocol_num;
    Gtimer  _timer; //타이머, 스킬 쿨타임 초기화
    fvec2   _location;
    int 	_state;
    bool 	_coin;
}

struct SC_IngametoFinish_init{ //인게임 -> 겜종료
    int 	_scene_num;
    int     _protocol_num;
    int     _CharState;
}

struct SC_FinishtoLobby_init{ //겜종료 -> 로비
    int 	_scene_num;
    int     _protocol_num;
}



struct CS_ingame_send{
	int 		vkey; // GetAsyncKeyState(vkey)로 동시키입력 여부를 확인하기 위한 변수 
	SHORT		pressedVkey;
}

struct CS_ingame_send_temp{
    short _horizontal_key;  // -1 : left || 0 : NULL || 1 : right
    short _vertical_key;    // -1 : down || 0 : NULL || 1 : up
    short _skill_key;       //  0 : NULL || 1 : skill || 2 : attack || 3 : dash 
}

struct CS_lobby_send {
	CS_ingame_send 	_input;
	bool 			_ready;
}



 

/* 1.	서버 네트워크  */

int InitServer()                                // 소켓을 생성한다
void SendData(player_data&) 	                // 클라이언트에게 플레이어 데이터를 전송한다.
void WaitForClientConnect() 	                // 접속한 순서대로 클라이언트에게 넘버를 부여하고 클라이언트가 3명이 모두 접속 했는지 확인한다.
static DWORD WINAPI RecvCLData (LPVOID)         // 클라이언트로부터 데이터를 받고 업데이트한 정보를 클라이언트에 전송한다.

//void SVSendTimer()	                        // 타이머(탑 화면의 시간, 스킬 쿨타임)를 전송한다.
DWORD UpdateTimer()	                            // 타이머 업데이트
void SVSendTotal() 	                            // 현재 전송하는 Protocal Info와 접속한 Player Data를 전송한다.
void SVSendScene()                              // 다음 넘길 Scene을 전송한다.
void SVSendCharSel()	                        // 캐릭터 선택창에서의 플레이어 데이터(선택한 캐릭터, Ready 여부)를 전송한다.
void SVSendMainGm()                             // 메인 게임 내에서 플레이어 데이터(선택한 캐릭터, 위치, 방향, 상태, 코인 여부) 전송한다.
void UpdatePLInfo()	                            // 플레이어 데이터(상태, 좌표값, 방향, 코인여부)를 업데이트한다.
void UpdateScene()	                            // Scene 정보를 업데이트한다.
void PLCollision()		                        // 플레이어와 플레이어, 플레이어와 코인, 플레이어와 장애물 사이의 충돌여부에 따라 플레이어 상태를 업데이트한다.
void PLDamaged()	                            // 스킬 키를 입력이 들어왔을 때 플레이어가 스킬에 의한 데미지를 입었는지 검사하고 플레이어 상태를 업데이트한다.
//추가할 여지가 있음 ->  클라이언트 인게임 위치 초기화 함수


/* 2.	클라이언트 네트워크 */

void CreateSocket()	                            // 소켓을 생성한다
bool CLConnect()		                        // 네트워크에 연결하고 연결 성공 유무에 따라 값을 반환한다.
void CLSendData (LPVOID)                        // 서버로 패킷을 전송한다.
//void CLLobbySend(CS_lobby_send )                // 로비에서의 데이터(레디유무,선택캐릭터)를 서버에게 보낸다.
//void SendKeyInfo(CS_ingame_send& )              // 입력한 키값을 서버에게 보낸다.
//void RecvTimer()		                        // 탑 화면 시간과 스킬 쿨타임 정보를 서버로부터 받는다.

void UpdatePL(player_data )                     // 서버로부터 넘겨받은 플레이어 정보로 해당 값을 업데이트한다.
void UpdateScene() 	                            // Scene을 업데이트한다.
