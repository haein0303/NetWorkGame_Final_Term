#pragma once
#pragma pack(1)

#include <Windows.h>


enum Scene {
    Lobby = 1,
    Char_sel = 3,
    Main_game = 4,
    End_game
};

enum CharState {
    Idle,
    IdleA,
    IdleB,
    Walk,
    WalkA, // 위, 오른쪽
    WalkB, // 아래, 왼쪽
    Attack,
    Attacked,
    Dash,
    Skill,
    Win,
    Lose,
};

//ver 2.0 : add
enum SC_ProtocalInfo { //현재 전송하는 정보
    SC_ingame_send,
    SC_lobby_send,
    SC_scene_send,
    SC_LobbytoCharsel_init,
    SC_CharseltoIngame_init,
    SC_IngametoFinish_init,
    SC_FinishtoLobby_init,
};

//ver 2.0 : add
enum CS_ProtocalInfo { //현재 전송하는 정보
    CS_ingame_send, //or CS_ingame_send_temp
    CS_lobby_send,
};

struct fvec2 {	//플레이어 좌표 구조체
    int	x;
    int	y;
};

struct Coin {
    fvec2 location;
    bool init;
};

//ver3.0 : add
//공용데이터------------------------------------------
float           prevTime;	//이전시간(제한시간으로 초기화)
float           elapsedTime;	//현재시간

//서버가 가지고 있는 캐릭터의 기본 정보
struct char_info {
    RECT        skill_area; //스킬의 범위
    RECT        attack_area; //공격의 범위
};

struct CS_ingame_send_tmp {// GetAsyncKeyState(vkey)로 동시키입력이 동작X시 사용
    short _horizontal_key;  // -1 : left || 0 : NULL || 1 : right
    short _vertical_key;    // -1 : down || 0 : NULL || 1 : up
    short _skill_key;       //  0 : NULL || 1 : skill || 2 : attack || 3 : dash 
};

// G_data 배열로 실제 데이터 관리, character_data는 네트워크 송수신시 사용
struct G_data {
    char_info        char_info;
    int 	    		charType;
    int              charLook;
    fvec2            location;
    int 	state;
    bool 	coin;
    float 	    skill_cooltime1;
    float 	    skill_cooltime2;
    bool	    attack_on;
    bool   	skill_on;
    DWORD my_num;
    CS_ingame_send_tmp ingame_key;
};


//공용데이터------------------------------------------

//ver 2.0 : add
struct Character_data {
    int 		_char_type;
    fvec2        _location;
    int 		_look;
    int 		_state;
    bool 	_coin;
    float 	_skill_cooltime1;
    float 	_skill_cooltime2;
};

//ver 2.0 : add
struct SC_Ingame_Send { //프레임마다 전송하는 데이터
    Character_data  _player[2];
    fvec2           _coin_location;
    float           _left_time;
};

//ver 2.0 : edit
struct SC_Lobby_Send {	//새로운 acc 있을 때마다 보내줌
    int	_acc_count;
};

//공통
struct SC_Scene_Send { // 씬데이터
    int 	_scene_num;
};

// 클라이언트 초기화할 때 사용하는 구조체들
struct SC_LobbytoCharsel_init { //로비 -> 캐 선택
    int	_scene_num;
    int	_char_num;  //접속한 순서대로 플레이어에게 부여되는 플레이어 넘버
    int      _char_type;
    bool 	_ready;
};

struct SC_CharseltoIngame_init { //캐선택 -> 인게임
    int     _scene_num;
    //Gtimer  _timer; //타이머, 스킬 쿨타임 초기화
    fvec2   _location;
    int     _state;
    bool    _coin;
};

struct SC_IngametoFinish_init { //인게임 -> 겜종료
    int     _scene_num;
    int     _CharState;
};

struct SC_FinishtoLobby_init { //겜종료 -> 로비
    int 	_scene_num;
};

struct CS_ingame_Send {
    int 		vkey; // GetAsyncKeyState(vkey)로 동시키입력 여부를 확인하기 위한 변수 
    SHORT	pressedVkey;
};

struct CS_lobby_send {
    CS_ingame_Send      _input;
    bool		        _ready;
};