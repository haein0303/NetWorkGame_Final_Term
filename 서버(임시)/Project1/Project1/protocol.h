#pragma once
#pragma pack(1)

#include <Windows.h>


enum Scene {
    Lobby,
    Char_sel,
    Main_game,
    End_game
};

enum CharState {
    Idle,
    Walk,
    Attack,
    Attacked,
    Dash,
    Skill,
    Win,
    Lose,
};

//ver 2.0 : add
enum SC_ProtocalInfo { //���� �����ϴ� ����
    SC_ingame_send,
    SC_lobby_send,
    SC_scene_send,
    SC_LobbytoCharsel_init,
    SC_CharseltoIngame_init,
    SC_IngametoFinish_init,
    SC_FinishtoLobby_init,
};

//ver 2.0 : add
enum CS_ProtocalInfo { //���� �����ϴ� ����
    CS_ingame_send, //or CS_ingame_send_temp
    CS_lobby_send,
};

struct fvec2 {	//�÷��̾� ��ǥ ����ü
    float	x;
    float	y;
};

//ver3.0 : add
//���뵥����------------------------------------------
float           prevTime;	//�����ð�(���ѽð����� �ʱ�ȭ)
float           elapsedTime;	//����ð�

// G_data �迭�� ���� ������ ����, character_data�� ��Ʈ��ũ �ۼ��Ž� ���
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
};

//������ ������ �ִ� ĳ������ �⺻ ����
struct char_info {
    RECT        skill_area; //��ų�� ����
    RECT        attack_area; //������ ����
};
//���뵥����------------------------------------------

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
struct SC_Ingame_Send { //�����Ӹ��� �����ϴ� ������
    Character_data  _player[3];
    fvec2           _coin_location;
    float           _left_time;
    int _protocol_num;
};

//ver 2.0 : edit
struct SC_Lobby_Send {	//���ο� acc ���� ������ ������
    int _protocol_num;
    int	_acc_count;
    int 	_my_num;
};

//����
struct SC_Scene_Send { // ��������
    int 	_scene_num;
    int _protocol_num;
};

// Ŭ���̾�Ʈ �ʱ�ȭ�� �� ����ϴ� ����ü��
struct SC_LobbytoCharsel_init { //�κ� -> ĳ ����
    int	_scene_num;
    int		_protocol_num;
    int	_char_num;  //������ ������� �÷��̾�� �ο��Ǵ� �÷��̾� �ѹ�
    int      _char_type;
    bool 	_ready;
};

struct SC_CharseltoIngame_init { //ĳ���� -> �ΰ���
    int     _scene_num;
    int     _protocol_num;
    //Gtimer  _timer; //Ÿ�̸�, ��ų ��Ÿ�� �ʱ�ȭ
    fvec2   _location;
    int     _state;
    bool    _coin;
};

struct SC_IngametoFinish_init { //�ΰ��� -> ������
    int     _scene_num;
    int     _protocol_num;
    int     _CharState;
};

struct SC_FinishtoLobby_init { //������ -> �κ�
    int 	_scene_num;
    int      _protocol_num;
};

struct CS_ingame_Send {
    int 		vkey; // GetAsyncKeyState(vkey)�� ����Ű�Է� ���θ� Ȯ���ϱ� ���� ���� 
    SHORT	pressedVkey;
};
struct CS_ingame_send_tmp {// GetAsyncKeyState(vkey)�� ����Ű�Է��� ����X�� ���
    short _horizontal_key;  // -1 : left || 0 : NULL || 1 : right
    short _vertical_key;    // -1 : down || 0 : NULL || 1 : up
    short _skill_key;       //  0 : NULL || 1 : skill || 2 : attack || 3 : dash 
};
struct CS_lobby_send {
    CS_ingame_Send      _input;
    bool		        _ready;
};