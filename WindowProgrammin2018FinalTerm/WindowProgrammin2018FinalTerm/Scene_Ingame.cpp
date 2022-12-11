#include "stdafx.h"
#include "Scene_Ingame.h"
#include "OBJECT_Player.h"
#include "OBJECT_Coin.h"
#include "OBJECT_Skill.h"
#include "Framework.h"

//글로벌 키 데이터입니다.
extern CS_ingame_send_tmp gKeyData;
extern CRITICAL_SECTION g_cs;
extern SOCKET sock;
extern SC_Ingame_Send g_ingame_send;
extern SC_Scene_Send g_scene_send;
extern int gMy_num;


CIngameScene::CIngameScene()
{
}
CIngameScene::~CIngameScene()
{
	if (CoinObject != NULL)
	{
		free(CoinObject);
	}

}
CIngameScene::CIngameScene(SceneTag tag, CFramework * pFramework) : CScene(tag, pFramework) //프레임워크 포인터 활성화
{

}
bool CIngameScene::OnCreate()
{	
	C_Tile[0].Load(L"Graphic\\Tile\\Tile1.jpg");
	C_Tile[1].Load(L"Graphic\\Tile\\Tile2.png");
	C_Tile[2].Load(L"Graphic\\Tile\\Tile3.png");

	C_Angle[0].Load(L"Graphic\\UI\\1.png");
	C_Angle[1].Load(L"Graphic\\UI\\2.png");
	C_Angle[2].Load(L"Graphic\\UI\\3.png");

	C_IngameLine.Load(L"Graphic\\UI\\INGAME UI.png");

	WinC.Load(L"Graphic\\UI\\WIN.png");
	LoseC.Load(L"Graphic\\UI\\LOSE.png");

	WCHAR LoadText[100];
	for (int i = 0; i <= 9; i++)
	{
		wsprintf(LoadText, L"Graphic\\Numbers\\%d.png", i);
		this->C_Numbers[i].Load(LoadText);
	}

	char Inbuff[20000] = { 0 };
	DWORD read_size = 20000;
	DWORD c = 20000;

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

	CloseHandle(hFile);
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			C_Tile[Tileindex[j][i]].BitBlt(*m_pFramework->GetTileDC(), 64 * j, 64 * i, 64, 64, 0, 0, SRCCOPY); //타일의 가로세로크기가 64바이트
		}
	}

	RemainTime = 99;
	TimeTick = 0;
	TimerImage[0] = 9;
	TimerImage[1] = 9;
	SkillCoolTime[0] = 0;
	SkillCoolTime[1] = 0;
	BuildObjects();

	return true;
}


void CIngameScene::BuildObjects()
{
	nObjects = 0;
	CoinObject = new OBJECT_Coin(rand()%6000 + 250, rand() % 6000 + 250);
	/*CObject** ppObjects;
	int nObjects; 여기다 값넣기*/

	/*nObjects = 2;
	ppObjects[0] = m_pFramework->GetPlayer(1);
	ppObjects[1] = m_pFramework->GetPlayer(2);*/
}

void CIngameScene::KeyState()
{

	//cout << "함수 소환 테스트" << endl;

	if (keydown != TRUE)
	{
		if (isp2LockDown != TRUE)
		{
			if (GetAsyncKeyState(VK_F1) & 0x8000 && SkillCoolTime[1] <= 0) // 스킬
			{
				keydownList[4] = TRUE;

				keydown = TRUE;
				isp2LockDown = TRUE;
			}
			else if (GetAsyncKeyState(VK_F2) & 0x8000) // 공격
			{
				keydownList[5] = TRUE;

				keydown = TRUE;
				isp2LockDown = TRUE;
			}
			else if (GetAsyncKeyState(VK_F3) & 0x8000) //대시
			{
				if (m_pFramework->GetPlayer(2)->DashCoolTimer <= 0)
				{
					keydownList[6] = TRUE;

					keydown = TRUE;
					isp2LockDown = TRUE;
				}
			}
			else //p2 이동 -> p1 이동 수정
			{
				if (GetAsyncKeyState(VK_UP) & 0x8000)
				{
					keydownList[8] = TRUE;

					keydown = TRUE;
				}
				if (GetAsyncKeyState(VK_LEFT) & 0x8000)
				{
					keydownList[7] = TRUE;

					keydown = TRUE;
				}
				if (GetAsyncKeyState(VK_DOWN) & 0x8000)
				{
					keydownList[10] = TRUE;

					keydown = TRUE;
				}
				if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
				{
					keydownList[9] = TRUE;

					keydown = TRUE;
				}
			}
		}
		else
		{
			if (m_pFramework->GetPlayer(2)->isAttack)
			{
				if (m_pFramework->GetPlayer(2)->AttackTimerTick++ > 3)
				{
					m_pFramework->GetPlayer(2)->AttackTimerTick = 0;
					if (m_pFramework->GetPlayer(2)->AttackImageTick++ > 2)
					{
						m_pFramework->GetPlayer(2)->isAttack = FALSE;
						m_pFramework->GetPlayer(2)->AttackTimerTick = 0;
						m_pFramework->GetPlayer(2)->AttackImageTick = 0;
						//printf("%d", m_pFramework->GetPlayer(2)->CharacterStatus);
						if (m_pFramework->GetPlayer(2)->CharacterStatus == 6)
							m_pFramework->GetPlayer(2)->CharacterStatus = 0;
						else
							m_pFramework->GetPlayer(2)->CharacterStatus = 1;
						isp2LockDown = FALSE;
						p2key = TRUE;
					}
				}
			}
			else if (m_pFramework->GetPlayer(2)->isSkill)
			{
				if (m_pFramework->GetPlayer(2)->AttackTimerTick++ > 3)
				{
					m_pFramework->GetPlayer(2)->AttackTimerTick = 0;
					if (m_pFramework->GetPlayer(2)->AttackImageTick++ > 2)
					{
						m_pFramework->GetPlayer(2)->AttackTimerTick = 0;
						m_pFramework->GetPlayer(2)->AttackImageTick = 0;
						//printf("%d", m_pFramework->GetPlayer(2)->CharacterStatus);
					}
				}
			}
			else if (m_pFramework->GetPlayer(2)->isSkillEnd)
			{
				if (m_pFramework->GetPlayer(2)->CharacterStatus == 12)
					m_pFramework->GetPlayer(2)->CharacterStatus = 0;
				else
					m_pFramework->GetPlayer(2)->CharacterStatus = 1;
				m_pFramework->GetPlayer(2)->AttackTimerTick = 0;
				m_pFramework->GetPlayer(2)->AttackImageTick = 0;
				m_pFramework->GetPlayer(2)->isSkillEnd = FALSE;
				isp2LockDown = FALSE;
			}
			else if (m_pFramework->GetPlayer(2)->isDash)
			{
				if (m_pFramework->GetPlayer(2)->DashTimerTick++ > 10)
				{
					m_pFramework->GetPlayer(2)->DashTimerTick = 0;
					m_pFramework->GetPlayer(2)->isDash = FALSE;
					m_pFramework->GetPlayer(2)->CharacterStatus = m_pFramework->GetPlayer(2)->Old_CharStat;
					isp2LockDown = FALSE;
					p2key = TRUE;
				}
				//////////////////////////////////////////////////////////////////////

				switch (m_pFramework->GetPlayer(2)->Old_CharStat)
				{
				case 2:
				case 0:
					if (m_pFramework->GetPlayer(2)->x > 50)
						m_pFramework->GetPlayer(2)->x -= 25;
					
					break;
				case 5:
					if (m_pFramework->GetPlayer(2)->y < 6350)
						m_pFramework->GetPlayer(2)->y += 25;
					
					break; // 앞 볼 때
				case 3:
					if (m_pFramework->GetPlayer(2)->y > 50)
						m_pFramework->GetPlayer(2)->y -= 25;
					
					break;
				case 4:
				case 1:
					if (m_pFramework->GetPlayer(2)->x < 6350)
						m_pFramework->GetPlayer(2)->x += 25;
					
					break; // 뒤 볼 때
				}
			}
			else if (m_pFramework->GetPlayer(2)->isAttacked)
			{
				if (m_pFramework->GetPlayer(2)->SkillAttackedTimer-- > 0 && m_pFramework->GetPlayer(2)->AttackTimerTick > 10)
				{
					m_pFramework->GetPlayer(2)->AttackTimerTick--;
				}

				if (m_pFramework->GetPlayer(2)->AttackTimerTick++ > 20)
				{
					m_pFramework->GetPlayer(2)->AttackTimerTick = 0;
					m_pFramework->GetPlayer(2)->isAttacked = FALSE;
					m_pFramework->GetPlayer(2)->CharacterStatus = m_pFramework->GetPlayer(2)->Old_CharStat;
					isp2LockDown = FALSE;
					p2key = TRUE;
					m_pFramework->GetPlayer(2)->SkillAttackedTimer = 0;
				}
				if (m_pFramework->GetPlayer(2)->AttackTimerTick < 10)
				{


					switch (m_pFramework->GetPlayer(1)->Old_CharStat)
					{
					case 2:
					case 0:
						if (m_pFramework->GetPlayer(2)->x > 50)
							m_pFramework->GetPlayer(2)->x -= 15;
						if (coinLockDown)
						CoinObject->Setx(CoinObject->x() - 5);
						break;
					case 5:
						if (m_pFramework->GetPlayer(2)->y < 6350)
							m_pFramework->GetPlayer(2)->y += 15;
						if (coinLockDown)
						CoinObject->Setx(CoinObject->y() + 5);
						break; // 앞 볼 때
					case 3:
						if (m_pFramework->GetPlayer(2)->y > 50)
							m_pFramework->GetPlayer(2)->y -= 15;
						if (coinLockDown)
						CoinObject->Setx(CoinObject->y() - 5);
						break;
					case 4:
					case 1:
						if (m_pFramework->GetPlayer(2)->x < 6350)
							m_pFramework->GetPlayer(2)->x += 15;
						if (coinLockDown)
						CoinObject->Setx(CoinObject->x() + 5);
						break; // 뒤 볼 때
					}
				}
				else
				{
					coinLockDown = FALSE;
				}
			}
		}
		if (isp1LockDown != TRUE)
		{
			if (GetAsyncKeyState(0x41) & 0x8000 && SkillCoolTime[0] <= 0) // p1 스킬
			{
				keydownList[11] = TRUE;

				keydown = TRUE;
				isp1LockDown = TRUE;
			}
			else if (GetAsyncKeyState(0x53) & 0x8000) // p1 공격
			{
				keydownList[12] = TRUE;

				keydown = TRUE;
				isp1LockDown = TRUE;
			}
			else if (GetAsyncKeyState(0x44) & 0x8000) // p1 대시
			{
				if (m_pFramework->GetPlayer(1)->DashCoolTimer <= 0)
				{
					keydownList[13] = TRUE;

					keydown = TRUE;
					isp1LockDown = TRUE;
				}
			}
			else
			{
				if (GetAsyncKeyState(0x46) & 0x8000) // f
				{
					keydownList[0] = TRUE;

					keydown = TRUE;
				}
				if (GetAsyncKeyState(0x54) & 0x8000) // t
				{
					keydownList[1] = TRUE;

					keydown = TRUE;
				}
				if (GetAsyncKeyState(0x48) & 0x8000) // h
				{
					keydownList[2] = TRUE;

					keydown = TRUE;
				}
				if (GetAsyncKeyState(0x47) & 0x8000) // g
				{
					keydownList[3] = TRUE;

					keydown = TRUE;
				}
			}
		}
		else
		{
			if (m_pFramework->GetPlayer(1)->isAttack)
			{
				if (m_pFramework->GetPlayer(1)->AttackTimerTick++ > 3)
				{
					m_pFramework->GetPlayer(1)->AttackTimerTick = 0;
					if (m_pFramework->GetPlayer(1)->AttackImageTick++ > 2)
					{
						m_pFramework->GetPlayer(1)->isAttack = FALSE;
						m_pFramework->GetPlayer(1)->AttackTimerTick = 0;
						m_pFramework->GetPlayer(1)->AttackImageTick = 0;
						//printf("%d", m_pFramework->GetPlayer(1)->CharacterStatus);
						if (m_pFramework->GetPlayer(1)->CharacterStatus == 6)
							m_pFramework->GetPlayer(1)->CharacterStatus = 0;
						else
							m_pFramework->GetPlayer(1)->CharacterStatus = 1;
						isp1LockDown = FALSE;
						p1key = TRUE;
					}
				}
			}
			else if (m_pFramework->GetPlayer(1)->isSkill)
			{
				if (m_pFramework->GetPlayer(1)->AttackTimerTick++ > 3)
				{
					m_pFramework->GetPlayer(1)->AttackTimerTick = 0;
					if (m_pFramework->GetPlayer(1)->AttackImageTick++ > 2)
					{
						m_pFramework->GetPlayer(1)->AttackTimerTick = 0;
						m_pFramework->GetPlayer(1)->AttackImageTick = 0;
						//printf("%d", m_pFramework->GetPlayer(2)->CharacterStatus);
					}
				}
			}
			else if (m_pFramework->GetPlayer(1)->isSkillEnd)
			{
				if (m_pFramework->GetPlayer(1)->CharacterStatus == 12)
					m_pFramework->GetPlayer(1)->CharacterStatus = 0;
				else
					m_pFramework->GetPlayer(1)->CharacterStatus = 1;
				m_pFramework->GetPlayer(1)->AttackTimerTick = 0;
				m_pFramework->GetPlayer(1)->AttackImageTick = 0;
				m_pFramework->GetPlayer(1)->isSkillEnd = FALSE;
				isp1LockDown = FALSE;
			}
			else if (m_pFramework->GetPlayer(1)->isDash)
			{
				if (m_pFramework->GetPlayer(1)->DashTimerTick++ > 10)
				{
					m_pFramework->GetPlayer(1)->DashTimerTick = 0;
					m_pFramework->GetPlayer(1)->isDash = FALSE;
					m_pFramework->GetPlayer(1)->CharacterStatus = m_pFramework->GetPlayer(1)->Old_CharStat;
					p1key = true;
					isp1LockDown = FALSE;
					p1key = TRUE;
				}
				switch (m_pFramework->GetPlayer(1)->Old_CharStat)
				{
				case 2:
				case 0:
					if (m_pFramework->GetPlayer(1)->x > 50)
					m_pFramework->GetPlayer(1)->x -= 25;
					break;
				case 5:
					if (m_pFramework->GetPlayer(1)->y < 6350)
					m_pFramework->GetPlayer(1)->y += 25;
					break; // 앞 볼 때
				case 3:
					if (m_pFramework->GetPlayer(1)->y > 50)
					m_pFramework->GetPlayer(1)->y -= 25;
					break;
				case 4:
				case 1:
					if (m_pFramework->GetPlayer(1)->x < 6350)
					m_pFramework->GetPlayer(1)->x += 25;
					break; // 뒤 볼 때
				}
			}
			else if (m_pFramework->GetPlayer(1)->isAttacked)
			{
				if (m_pFramework->GetPlayer(1)->SkillAttackedTimer-- > 0 && m_pFramework->GetPlayer(1)->AttackTimerTick > 10)
				{
					m_pFramework->GetPlayer(1)->AttackTimerTick--;
				}

				if (m_pFramework->GetPlayer(1)->AttackTimerTick++ > 20)
				{
					m_pFramework->GetPlayer(1)->AttackTimerTick = 0;
					m_pFramework->GetPlayer(1)->isAttacked = FALSE;
					m_pFramework->GetPlayer(1)->CharacterStatus = m_pFramework->GetPlayer(1)->Old_CharStat;
					isp1LockDown = FALSE;
					p1key = TRUE;
					m_pFramework->GetPlayer(1)->SkillAttackedTimer = 0;
				}
				if (m_pFramework->GetPlayer(1)->AttackTimerTick < 10)
				{
					switch (m_pFramework->GetPlayer(2)->Old_CharStat)
					{
					case 2:
					case 0:
						if (m_pFramework->GetPlayer(1)->x > 50)
							m_pFramework->GetPlayer(1)->x -= 15;
						if (coinLockDown)
						CoinObject->Setx(CoinObject->x() - 5);
						break;
					case 5:
						if (m_pFramework->GetPlayer(1)->y < 6350)
							m_pFramework->GetPlayer(1)->y += 15;
						if (coinLockDown)
						CoinObject->Setx(CoinObject->y() + 5);
						break; // 앞 볼 때
					case 3:
						if (m_pFramework->GetPlayer(1)->y > 50)
							m_pFramework->GetPlayer(1)->y -= 15;
						if (coinLockDown)
						CoinObject->Setx(CoinObject->y() - 5);
						break;
					case 4:
					case 1:
						if (m_pFramework->GetPlayer(1)->x < 6350)
							m_pFramework->GetPlayer(1)->x += 15;
						if (coinLockDown)
						CoinObject->Setx(CoinObject->x() + 5);
						break; // 뒤 볼 때
					}
				}
				else
				{
					coinLockDown = FALSE;
				}

			}
		}
	}

	char s_buf[BUFSIZE + 1]; // 데이터 수신 버퍼
	int retval;
	
	if(keydown == TRUE)
	{		// 0 1 2 3 p1 이동 4 5 6 p2 스킬 공격 대시 |||  7 8 9 10 p2 이동 11 12 13 p1 스킬 공격 대시

		//struct CS_ingame_send_tmp {// GetAsyncKeyState(vkey)로 동시키입력이 동작X시 사용
		//	short _horizontal_key;  // -1 : left || 0 : NULL || 1 : right
		//	short _vertical_key;    // -1 : down || 0 : NULL || 1 : up
		//	short _skill_key;       //  0 : NULL || 1 : skill || 2 : attack || 3 : dash 
		//};

		//스킬키는 초기화
		gKeyData._skill_key = 0;
		gKeyData._horizontal_key = 0;
		gKeyData._vertical_key = 0;

		if (keydownList[7]) {//LEFT
			gKeyData._horizontal_key = -1;
		}
		if (keydownList[9]) {//RIGHT
			gKeyData._horizontal_key = 1;
		}
		if (keydownList[7]&& keydownList[9]) {//동시
			gKeyData._horizontal_key = 0;
		}
		if (keydownList[10]) {//UP
			gKeyData._vertical_key = 1;
		}
		if (keydownList[8]) {//DOWN
			gKeyData._vertical_key = -1;
		}
		if (keydownList[8] && keydownList[10]) {//동시
			gKeyData._vertical_key = 0;
		}
		if (keydownList[11]) {
			gKeyData._skill_key = 1;
			cout << "스킬!" << endl;
		}
		if (keydownList[12]) {
			gKeyData._skill_key = 2;
			cout << "공격!" << endl;
		}
		if (keydownList[13]) {
			gKeyData._skill_key = 3;
			cout << "대시!" << endl;
		}

		
		s_buf[0] = CS_ingame_send;
		retval = send(sock, s_buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			//err_display("send()");
		}
				
		// 데이터 보내기
		retval = send(sock, reinterpret_cast<char*>(&gKeyData), sizeof(gKeyData), 0);
		if (retval == SOCKET_ERROR) {
			//err_display("send()");
		}
		//cout <<"hori : " << gKeyData._horizontal_key << endl;
		//cout << "verti : " << gKeyData._vertical_key << endl;

	}
	

	
}

void CIngameScene::CharacterState()
{
	//각 플레이어들 좌표 업데이트 
	//if (m_pFramework->GetPlayer(0)) //이렇게 하면 속도 느려짐
	//{
	//	for (int i = 0; i < 3; ++i)
	//	{
	//		m_pFramework->GetPlayer(i)->x = (int)is._player[i]._location.x;
	//		m_pFramework->GetPlayer(i)->y = (int)is._player[i]._location.y;
	//	}
	//}

	cout << "Scene_ingame.cpp  588 :: " << gMy_num << endl;
	//p1 업데이트
	m_pFramework->GetPlayer(1)->x = (int)g_ingame_send._player[gMy_num]._location.x;
	m_pFramework->GetPlayer(1)->y = (int)g_ingame_send._player[gMy_num]._location.y;
	m_pFramework->GetPlayer(1)->CharacterStatus = g_ingame_send._player[gMy_num]._state;


	p1key = true;
	cout << "Scene_ingame.cpp 595:: " << g_ingame_send._player[gMy_num]._state << endl;
	

	//p2 업데이트
	m_pFramework->GetPlayer(2)->x = (int)g_ingame_send._player[calcNetId(gMy_num,1)]._location.x;
	m_pFramework->GetPlayer(2)->y = (int)g_ingame_send._player[calcNetId(gMy_num, 1)]._location.x;
	m_pFramework->GetPlayer(2)->CharacterStatus = g_ingame_send._player[calcNetId(gMy_num, 1)]._state;

	//p3 업데이트
	m_pFramework->GetPlayer(3)->x = (int)g_ingame_send._player[calcNetId(gMy_num, 2)]._location.x;
	m_pFramework->GetPlayer(3)->y = (int)g_ingame_send._player[calcNetId(gMy_num, 2)]._location.x;
	m_pFramework->GetPlayer(3)->CharacterStatus = g_ingame_send._player[calcNetId(gMy_num, 2)]._state;

	//cout << "위치: " << g_ingame_send._player[0]._location.x << ", " << g_ingame_send._player[0]._location.y << endl;
	//cout << "위치: " << tx2 << ", " << ty2 << endl;
	//cout << "위치: " << tx3 << ", " << ty3 << endl;

	// 7 8 9 10 p1 이동 // 11 12 13 p1 스킬 공격 대시 // 0 1 2 3 p2 이동 // 4 5 6 p2 스킬 공격 대시 // 
	/*
	////////////////////////// p1 이동 /////////////////////////////////
	if (keydownList[8]) //위
	{
		if (Tileindex[m_pFramework->GetPlayer(1)->x / 64][(m_pFramework->GetPlayer(1)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(1)->y > 50)
				m_pFramework->GetPlayer(1)->y -= (15 - m_pFramework->GetPlayer(1)->iHaveCoin * 4);
			m_pFramework->GetPlayer(1)->WalkingTimerTick++;
		}
		else if (Tileindex[m_pFramework->GetPlayer(1)->x / 64][(m_pFramework->GetPlayer(1)->y + 60 - 30) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(1)->y > 50)
				m_pFramework->GetPlayer(1)->y -= (10 - m_pFramework->GetPlayer(1)->iHaveCoin * 2);
		}
		p1key = true;
		m_pFramework->GetPlayer(1)->CharacterStatus = 3;
	}
	if (keydownList[7])
	{
		if (Tileindex[m_pFramework->GetPlayer(1)->x / 64][(m_pFramework->GetPlayer(1)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(1)->x > 50)
				m_pFramework->GetPlayer(1)->x -= (15 - m_pFramework->GetPlayer(1)->iHaveCoin * 4);
			m_pFramework->GetPlayer(1)->WalkingTimerTick++;
		}
		else if (Tileindex[(m_pFramework->GetPlayer(1)->x - 30) / 64][(m_pFramework->GetPlayer(1)->y + 60) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(1)->x > 50)
				m_pFramework->GetPlayer(1)->x -= (10 - m_pFramework->GetPlayer(1)->iHaveCoin * 2);
		}
		p1key = true;
		m_pFramework->GetPlayer(1)->CharacterStatus = 2;
	}
	if (keydownList[10])
	{
		if (Tileindex[m_pFramework->GetPlayer(1)->x / 64][(m_pFramework->GetPlayer(1)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(1)->y < 6350)
				m_pFramework->GetPlayer(1)->y += (15 - m_pFramework->GetPlayer(1)->iHaveCoin * 4);
			m_pFramework->GetPlayer(1)->WalkingTimerTick++;
		}
		else if (Tileindex[m_pFramework->GetPlayer(1)->x / 64][(m_pFramework->GetPlayer(1)->y + 60 + 20) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(1)->y < 6350)
				m_pFramework->GetPlayer(1)->y += (10 - m_pFramework->GetPlayer(1)->iHaveCoin * 2);
		}
		p1key = true;
		m_pFramework->GetPlayer(1)->CharacterStatus = 5;
	}
	if (keydownList[9])
	{
		if (Tileindex[m_pFramework->GetPlayer(1)->x / 64][(m_pFramework->GetPlayer(1)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(1)->x < 6350)
				m_pFramework->GetPlayer(1)->x += (15 - m_pFramework->GetPlayer(1)->iHaveCoin * 4);
			m_pFramework->GetPlayer(1)->WalkingTimerTick++;
		}
		else if (Tileindex[(m_pFramework->GetPlayer(1)->x + 30) / 64][(m_pFramework->GetPlayer(1)->y + 60) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(1)->x < 6350)
				m_pFramework->GetPlayer(1)->x += (10 - m_pFramework->GetPlayer(1)->iHaveCoin * 2);
		}
		p1key = true;
		m_pFramework->GetPlayer(1)->CharacterStatus = 4;
	}
	*/
	//////////////////// p1 스킬 공격 대시 //////////////////////////////////
	//if (keydownList[11]) // p1 스킬 A키
	//{
	//	switch (m_pFramework->GetPlayer(1)->charNum)
	//	{
	//	case 1:
	//		SkillCoolTime[0] = 8;
	//		break;
	//	case 2:
	//		SkillCoolTime[0] = 5;
	//		break;
	//	case 3:
	//		SkillCoolTime[0] = 12;
	//		break;
	//	}
	//	switch (m_pFramework->GetPlayer(1)->CharacterStatus) 
	//	{
	//	case 2:
	//	case 5:
	//	case 6:
	//	case 0:
	//		m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
	//		m_pFramework->GetPlayer(1)->isSkill = TRUE;
	//		m_pFramework->GetPlayer(1)->CharacterStatus = 12;
	//		m_pFramework->GetPlayer(1)->SkillCast(m_pFramework->GetPlayer(1)->x, m_pFramework->GetPlayer(1)->y, m_pFramework->GetPlayer(1)->Old_CharStat);
	//		break; // 앞 볼 때
	//	case 3:
	//	case 4:
	//	case 7:
	//	case 1:
	//		m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
	//		m_pFramework->GetPlayer(1)->isSkill = TRUE;
	//		m_pFramework->GetPlayer(1)->CharacterStatus = 13;
	//		m_pFramework->GetPlayer(1)->SkillCast(m_pFramework->GetPlayer(1)->x, m_pFramework->GetPlayer(1)->y, m_pFramework->GetPlayer(1)->Old_CharStat);
	//		break; // 뒤 볼 때
	//	}
	//}
	if (keydownList[12]) // p1 공격
	{
		switch (m_pFramework->GetPlayer(1)->CharacterStatus)
		{
		case 2:
		case 5:
		case 6:
		case 0:
			m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
			m_pFramework->GetPlayer(1)->isAttack = TRUE;
			m_pFramework->GetPlayer(1)->CharacterStatus = 6;
			//p2  피격처리 여기서
			if ((abs(m_pFramework->GetPlayer(1)->x - m_pFramework->GetPlayer(2)->x) < 70) &&
				(abs(m_pFramework->GetPlayer(1)->y - m_pFramework->GetPlayer(2)->y) < 50))
			{
				m_pFramework->GetPlayer(2)->Old_CharStat = m_pFramework->GetPlayer(2)->CharacterStatus;
				m_pFramework->GetPlayer(2)->isAttacked = TRUE;
				m_pFramework->GetPlayer(2)->CharacterStatus = 9;
				isp2LockDown = TRUE;
				if (m_pFramework->GetPlayer(2)->iHaveCoin)
				{
					m_pFramework->GetPlayer(2)->iHaveCoin = FALSE;
					CoinObject->OnCreate(m_pFramework->GetPlayer(2)->x, m_pFramework->GetPlayer(2)->y);
					coinLockDown = TRUE;
				}
			}
			break; // 앞 볼 때
		case 3:
		case 4:
		case 7:
		case 1:
			m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
			m_pFramework->GetPlayer(1)->isAttack = TRUE;
			m_pFramework->GetPlayer(1)->CharacterStatus = 7;
			if ((abs(m_pFramework->GetPlayer(1)->x - m_pFramework->GetPlayer(2)->x) < 70) &&
				(abs(m_pFramework->GetPlayer(1)->y - m_pFramework->GetPlayer(2)->y) < 50))
			{
				m_pFramework->GetPlayer(2)->Old_CharStat = m_pFramework->GetPlayer(2)->CharacterStatus;
				m_pFramework->GetPlayer(2)->isAttacked = TRUE;
				m_pFramework->GetPlayer(2)->CharacterStatus = 8;
				isp2LockDown = TRUE;
				if (m_pFramework->GetPlayer(2)->iHaveCoin)
				{
					m_pFramework->GetPlayer(2)->iHaveCoin = FALSE;
					CoinObject->OnCreate(m_pFramework->GetPlayer(2)->x, m_pFramework->GetPlayer(2)->y);
					coinLockDown = TRUE;
				}
			}
			break; // 뒤 볼 때
		}
	}
	if (keydownList[13]) // p1 대시
	{
		if (m_pFramework->GetPlayer(1)->DashCoolTimer <= 0)
		{
			switch (m_pFramework->GetPlayer(1)->charNum)
			{
			case 1:
				m_pFramework->GetPlayer(1)->DashCoolTimer = 410;
				break;
			case 2:
				m_pFramework->GetPlayer(1)->DashCoolTimer = 300;
				break;
			case 3:
				m_pFramework->GetPlayer(1)->DashCoolTimer = 600;
				break;
			}
	
			m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
			switch (m_pFramework->GetPlayer(1)->CharacterStatus)
			{
			case 2:
			case 5:
			case 0:
				m_pFramework->GetPlayer(1)->isDash = TRUE;
				m_pFramework->GetPlayer(1)->CharacterStatus = 10;
				break; // 앞 볼 때
			case 3:
			case 4:
			case 1:
				m_pFramework->GetPlayer(1)->isDash = TRUE;
				m_pFramework->GetPlayer(1)->CharacterStatus = 11;
				break; // 뒤 볼 때
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// p2 이동 ////////////////////////////////////////////
	//if _look 이 위쪽일때 //if (keydownList[1])
	{

		if (Tileindex[m_pFramework->GetPlayer(2)->x / 64][(m_pFramework->GetPlayer(2)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(2)->y > 50)
				m_pFramework->GetPlayer(2)->y -= (15 - m_pFramework->GetPlayer(2)->iHaveCoin * 4);
			m_pFramework->GetPlayer(2)->WalkingTimerTick++;
		}
		else if (Tileindex[m_pFramework->GetPlayer(2)->x / 64][(m_pFramework->GetPlayer(2)->y + 60 - 20) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(2)->y > 50)
				m_pFramework->GetPlayer(2)->y -= (10 - m_pFramework->GetPlayer(2)->iHaveCoin * 2);
		}
		p2key = true;
		m_pFramework->GetPlayer(2)->CharacterStatus = 3;

	}
	//if _look 이 왼쪽일때 // if (keydownList[0])
	{
		if (Tileindex[m_pFramework->GetPlayer(2)->x / 64][(m_pFramework->GetPlayer(2)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(2)->x > 50)
				m_pFramework->GetPlayer(2)->x -= (15 - m_pFramework->GetPlayer(2)->iHaveCoin * 4);
			m_pFramework->GetPlayer(2)->WalkingTimerTick++;
		}
		else if (Tileindex[((m_pFramework->GetPlayer(2)->x) - 30) / 64][(m_pFramework->GetPlayer(2)->y + 60 - 10) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(2)->x > 50)
				m_pFramework->GetPlayer(2)->x -= (10 - m_pFramework->GetPlayer(2)->iHaveCoin * 2);
		}
		p2key = true;
		m_pFramework->GetPlayer(2)->CharacterStatus = 2;

	}
	//if _look 이 아래쪽일 때 // if (keydownList[3])
	{

		if (Tileindex[m_pFramework->GetPlayer(2)->x / 64][(m_pFramework->GetPlayer(2)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(2)->y < 6350)
				m_pFramework->GetPlayer(2)->y += (15 - m_pFramework->GetPlayer(2)->iHaveCoin * 4);
			m_pFramework->GetPlayer(2)->WalkingTimerTick++;
		}
		else if (Tileindex[m_pFramework->GetPlayer(2)->x / 64][(m_pFramework->GetPlayer(2)->y + 60 + 20) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(2)->y < 6350)
				m_pFramework->GetPlayer(2)->y += (10 - m_pFramework->GetPlayer(2)->iHaveCoin * 2);
		}
		p2key = true;
		m_pFramework->GetPlayer(2)->CharacterStatus = 5;

	}
	//if _look 이 오른쪽일때 // if (keydownList[2])
	{

		if (Tileindex[m_pFramework->GetPlayer(2)->x / 64][(m_pFramework->GetPlayer(2)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(2)->x < 6350)
				m_pFramework->GetPlayer(2)->x += (15 - m_pFramework->GetPlayer(2)->iHaveCoin * 4);
			m_pFramework->GetPlayer(2)->WalkingTimerTick++;
		}
		else if (Tileindex[(m_pFramework->GetPlayer(2)->x + 30) / 64][(m_pFramework->GetPlayer(2)->y + 60) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(2)->x < 6350)
				m_pFramework->GetPlayer(2)->x += (10 - m_pFramework->GetPlayer(2)->iHaveCoin * 2);
		}
		p2key = true;
		m_pFramework->GetPlayer(2)->CharacterStatus = 4;

	}

	/////////////////////////// p2 스킬 공격 대시 /////////////////////////////
	// if charstate가 skill // if (keydownList[4]) 
	{
		switch (m_pFramework->GetPlayer(2)->charNum)
		{
		case 1:
			SkillCoolTime[1] = 8;
			break;
		case 2:
			SkillCoolTime[1] = 5;
			break;
		case 3:
			SkillCoolTime[1] = 12;
			break;
		}
		switch (m_pFramework->GetPlayer(2)->CharacterStatus)
		{
		case 2:
		case 5:
		case 6:
		case 0:
			m_pFramework->GetPlayer(2)->Old_CharStat = m_pFramework->GetPlayer(2)->CharacterStatus;
			m_pFramework->GetPlayer(2)->isSkill = TRUE;
			m_pFramework->GetPlayer(2)->CharacterStatus = 12;
			m_pFramework->GetPlayer(2)->SkillCast(m_pFramework->GetPlayer(2)->x, m_pFramework->GetPlayer(2)->y, m_pFramework->GetPlayer(2)->Old_CharStat);
			break; // 앞 볼 때
		case 3:
		case 4:
		case 7:
		case 1:
			m_pFramework->GetPlayer(2)->Old_CharStat = m_pFramework->GetPlayer(2)->CharacterStatus;
			m_pFramework->GetPlayer(2)->isSkill = TRUE;
			m_pFramework->GetPlayer(2)->CharacterStatus = 13;
			m_pFramework->GetPlayer(2)->SkillCast(m_pFramework->GetPlayer(2)->x, m_pFramework->GetPlayer(2)->y, m_pFramework->GetPlayer(2)->Old_CharStat);
			break; // 뒤 볼 때
		}
	}
	// if charstate가 attack // if (keydownList[5]) // p2 공격 (p3와의 공격체크 안돼있음)
	{
		switch (m_pFramework->GetPlayer(2)->CharacterStatus)
		{
		case 2:
		case 5:
		case 6:
		case 0:
			m_pFramework->GetPlayer(2)->Old_CharStat = m_pFramework->GetPlayer(2)->CharacterStatus;
			m_pFramework->GetPlayer(2)->isAttack = TRUE;
			m_pFramework->GetPlayer(2)->CharacterStatus = 6;
			if ((abs(m_pFramework->GetPlayer(2)->x - m_pFramework->GetPlayer(1)->x) < 70) &&
				(abs(m_pFramework->GetPlayer(2)->y - m_pFramework->GetPlayer(1)->y) < 50))
			{
				m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
				m_pFramework->GetPlayer(1)->isAttacked = TRUE;
				m_pFramework->GetPlayer(1)->CharacterStatus = 9;
				isp1LockDown = TRUE;
				if (m_pFramework->GetPlayer(1)->iHaveCoin)
				{
					m_pFramework->GetPlayer(1)->iHaveCoin = FALSE;
					CoinObject->OnCreate(m_pFramework->GetPlayer(1)->x, m_pFramework->GetPlayer(1)->y);
					coinLockDown = TRUE;
				}
			}
			break; // 앞 볼 때
		case 3:
		case 4:
		case 7:
		case 1:
			m_pFramework->GetPlayer(2)->Old_CharStat = m_pFramework->GetPlayer(2)->CharacterStatus;
			m_pFramework->GetPlayer(2)->isAttack = TRUE;
			m_pFramework->GetPlayer(2)->CharacterStatus = 7;
			if ((abs(m_pFramework->GetPlayer(2)->x - m_pFramework->GetPlayer(1)->x) < 70) &&
				(abs(m_pFramework->GetPlayer(2)->y - m_pFramework->GetPlayer(1)->y) < 50))
			{
				m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
				m_pFramework->GetPlayer(1)->isAttacked = TRUE;
				m_pFramework->GetPlayer(1)->CharacterStatus = 8;
				isp1LockDown = TRUE;
				if (m_pFramework->GetPlayer(1)->iHaveCoin)
				{
					m_pFramework->GetPlayer(1)->iHaveCoin = FALSE;
					CoinObject->OnCreate(m_pFramework->GetPlayer(1)->x, m_pFramework->GetPlayer(1)->y);
					coinLockDown = TRUE;
				}
			}
			break; // 뒤 볼 때
		}
	}
	// if charstate가 dash // if (keydownList[6]) // p2 대시
	{
		if (m_pFramework->GetPlayer(2)->DashCoolTimer <= 0)
		{
			switch (m_pFramework->GetPlayer(2)->charNum)
			{
			case 1:
				m_pFramework->GetPlayer(2)->DashCoolTimer = 410;
				break;
			case 2:
				m_pFramework->GetPlayer(2)->DashCoolTimer = 300;
				break;
			case 3:
				m_pFramework->GetPlayer(2)->DashCoolTimer = 600;
				break;
			}
			m_pFramework->GetPlayer(2)->Old_CharStat = m_pFramework->GetPlayer(2)->CharacterStatus;
			switch (m_pFramework->GetPlayer(2)->CharacterStatus)
			{
			case 2:
			case 5:
			case 0:
				m_pFramework->GetPlayer(2)->isDash = TRUE;
				m_pFramework->GetPlayer(2)->CharacterStatus = 10;
				break; // 앞 볼 때
			case 3:
			case 4:
			case 1:
				m_pFramework->GetPlayer(2)->isDash = TRUE;
				m_pFramework->GetPlayer(2)->CharacterStatus = 11;
				break; // 뒤 볼 때
			}
		}
	}

	////////////////////////////// p3 추가 /////////////////////////////////////////////
	// 추후 수정
	////////////////////////////// p3 이동 ////////////////////////////////////////////
	//if _look 이 위쪽일때 
	{
		if (Tileindex[m_pFramework->GetPlayer(3)->x / 64][(m_pFramework->GetPlayer(3)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(3)->y > 50)
				m_pFramework->GetPlayer(3)->y -= (15 - m_pFramework->GetPlayer(3)->iHaveCoin * 4);
			m_pFramework->GetPlayer(3)->WalkingTimerTick++;
		}
		else if (Tileindex[m_pFramework->GetPlayer(3)->x / 64][(m_pFramework->GetPlayer(3)->y + 60 - 20) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(3)->y > 50)
				m_pFramework->GetPlayer(3)->y -= (10 - m_pFramework->GetPlayer(3)->iHaveCoin * 2);
		}
		p2key = true;
		m_pFramework->GetPlayer(3)->CharacterStatus = 3;

	}
	//if _look 이 왼쪽일때 //
	{
		if (Tileindex[m_pFramework->GetPlayer(3)->x / 64][(m_pFramework->GetPlayer(3)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(3)->x > 50)
				m_pFramework->GetPlayer(3)->x -= (15 - m_pFramework->GetPlayer(3)->iHaveCoin * 4);
			m_pFramework->GetPlayer(3)->WalkingTimerTick++;
		}
		else if (Tileindex[((m_pFramework->GetPlayer(3)->x) - 30) / 64][(m_pFramework->GetPlayer(3)->y + 60 - 10) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(3)->x > 50)
				m_pFramework->GetPlayer(3)->x -= (10 - m_pFramework->GetPlayer(3)->iHaveCoin * 2);
		}
		p2key = true;
		m_pFramework->GetPlayer(3)->CharacterStatus = 2;



	}
	//if _look 이 아래쪽일 때 
	{
		if (Tileindex[m_pFramework->GetPlayer(3)->x / 64][(m_pFramework->GetPlayer(3)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(3)->y < 6350)
				m_pFramework->GetPlayer(3)->y += (15 - m_pFramework->GetPlayer(3)->iHaveCoin * 4);
			m_pFramework->GetPlayer(3)->WalkingTimerTick++;
		}
		else if (Tileindex[m_pFramework->GetPlayer(3)->x / 64][(m_pFramework->GetPlayer(3)->y + 60 + 20) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(3)->y < 6350)
				m_pFramework->GetPlayer(3)->y += (10 - m_pFramework->GetPlayer(3)->iHaveCoin * 2);
		}
		p2key = true;
		m_pFramework->GetPlayer(3)->CharacterStatus = 5;

	}
	//if _look 이 오른쪽일때 // if (keydownList[2])
	{
		if (Tileindex[m_pFramework->GetPlayer(3)->x / 64][(m_pFramework->GetPlayer(3)->y + 60) / 64] == 1)
		{
			if (m_pFramework->GetPlayer(3)->x < 6350)
				m_pFramework->GetPlayer(3)->x += (15 - m_pFramework->GetPlayer(3)->iHaveCoin * 4);
			m_pFramework->GetPlayer(3)->WalkingTimerTick++;
		}
		else if (Tileindex[(m_pFramework->GetPlayer(3)->x + 30) / 64][(m_pFramework->GetPlayer(3)->y + 60) / 64] == 2)
		{

		}
		else
		{
			if (m_pFramework->GetPlayer(3)->x < 6350)
				m_pFramework->GetPlayer(3)->x += (10 - m_pFramework->GetPlayer(3)->iHaveCoin * 2);
		}
		p2key = true;
		m_pFramework->GetPlayer(3)->CharacterStatus = 4;

	}

	/////////////////////////// p3 스킬 공격 대시 /////////////////////////////
	// if charstate가 skill 
	{
		switch (m_pFramework->GetPlayer(3)->charNum)
		{
		case 1:
			SkillCoolTime[2] = 8;
			break;
		case 2:
			SkillCoolTime[2] = 5;
			break;
		case 3:
			SkillCoolTime[2] = 12;
			break;
		}
		switch (m_pFramework->GetPlayer(3)->CharacterStatus)
		{
		case 2:
		case 5:
		case 6:
		case 0:
			m_pFramework->GetPlayer(3)->Old_CharStat = m_pFramework->GetPlayer(3)->CharacterStatus;
			m_pFramework->GetPlayer(3)->isSkill = TRUE;
			m_pFramework->GetPlayer(3)->CharacterStatus = 12;
			m_pFramework->GetPlayer(3)->SkillCast(m_pFramework->GetPlayer(3)->x, m_pFramework->GetPlayer(3)->y, m_pFramework->GetPlayer(3)->Old_CharStat);
			break; // 앞 볼 때
		case 3:
		case 4:
		case 7:
		case 1:
			m_pFramework->GetPlayer(3)->Old_CharStat = m_pFramework->GetPlayer(3)->CharacterStatus;
			m_pFramework->GetPlayer(3)->isSkill = TRUE;
			m_pFramework->GetPlayer(3)->CharacterStatus = 13;
			m_pFramework->GetPlayer(3)->SkillCast(m_pFramework->GetPlayer(3)->x, m_pFramework->GetPlayer(2)->y, m_pFramework->GetPlayer(3)->Old_CharStat);
			break; // 뒤 볼 때
		}
	}
	// if charstate가 attack // p3 공격
	{
		switch (m_pFramework->GetPlayer(3)->CharacterStatus)
		{
		case 2:
		case 5:
		case 6:
		case 0: //1번 플레이어와의 공격 체크만 가능한 상태
			m_pFramework->GetPlayer(3)->Old_CharStat = m_pFramework->GetPlayer(3)->CharacterStatus;
			m_pFramework->GetPlayer(3)->isAttack = TRUE;
			m_pFramework->GetPlayer(3)->CharacterStatus = 6;
			if ((abs(m_pFramework->GetPlayer(3)->x - m_pFramework->GetPlayer(1)->x) < 70) &&
				(abs(m_pFramework->GetPlayer(3)->y - m_pFramework->GetPlayer(1)->y) < 50))
			{
				m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
				m_pFramework->GetPlayer(1)->isAttacked = TRUE;
				m_pFramework->GetPlayer(1)->CharacterStatus = 9;
				isp1LockDown = TRUE;
				if (m_pFramework->GetPlayer(1)->iHaveCoin)
				{
					m_pFramework->GetPlayer(1)->iHaveCoin = FALSE;
					CoinObject->OnCreate(m_pFramework->GetPlayer(1)->x, m_pFramework->GetPlayer(1)->y);
					coinLockDown = TRUE;
				}
			}
			break; // 앞 볼 때
		case 3:
		case 4:
		case 7:
		case 1:
			m_pFramework->GetPlayer(3)->Old_CharStat = m_pFramework->GetPlayer(3)->CharacterStatus;
			m_pFramework->GetPlayer(3)->isAttack = TRUE;
			m_pFramework->GetPlayer(3)->CharacterStatus = 7;
			if ((abs(m_pFramework->GetPlayer(3)->x - m_pFramework->GetPlayer(1)->x) < 70) &&
				(abs(m_pFramework->GetPlayer(3)->y - m_pFramework->GetPlayer(1)->y) < 50))
			{
				m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
				m_pFramework->GetPlayer(1)->isAttacked = TRUE;
				m_pFramework->GetPlayer(1)->CharacterStatus = 8;
				isp1LockDown = TRUE;
				if (m_pFramework->GetPlayer(1)->iHaveCoin)
				{
					m_pFramework->GetPlayer(1)->iHaveCoin = FALSE;
					CoinObject->OnCreate(m_pFramework->GetPlayer(1)->x, m_pFramework->GetPlayer(1)->y);
					coinLockDown = TRUE;
				}
			}
			break; // 뒤 볼 때
		}
	}
	// if charstate가 dash 
	{
		if (m_pFramework->GetPlayer(3)->DashCoolTimer <= 0)
		{
			switch (m_pFramework->GetPlayer(3)->charNum)
			{
			case 1:
				m_pFramework->GetPlayer(3)->DashCoolTimer = 410;
				break;
			case 2:
				m_pFramework->GetPlayer(3)->DashCoolTimer = 300;
				break;
			case 3:
				m_pFramework->GetPlayer(3)->DashCoolTimer = 600;
				break;
			}
			m_pFramework->GetPlayer(3)->Old_CharStat = m_pFramework->GetPlayer(3)->CharacterStatus;
			switch (m_pFramework->GetPlayer(3)->CharacterStatus)
			{
			case 2:
			case 5:
			case 0:
				m_pFramework->GetPlayer(3)->isDash = TRUE;
				m_pFramework->GetPlayer(3)->CharacterStatus = 10;
				break; // 앞 볼 때
			case 3:
			case 4:
			case 1:
				m_pFramework->GetPlayer(3)->isDash = TRUE;
				m_pFramework->GetPlayer(3)->CharacterStatus = 11;
				break; // 뒤 볼 때
			}
		}
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (p1key)
	{
		m_pFramework->GetPlayer(1)->isWalk = TRUE;
		if (m_pFramework->GetPlayer(1)->WalkingTimerTick++ > 3)
		{
			m_pFramework->GetPlayer(1)->WalkingTimerTick = 0;
			m_pFramework->GetPlayer(1)->WalkingImageTick++;
		}

	}
	else
	{
		if (m_pFramework->GetPlayer(1)->isAttack != TRUE)
		{
			m_pFramework->GetPlayer(1)->isWalk = FALSE;
			switch (m_pFramework->GetPlayer(1)->CharacterStatus)
			{
			case 2:
			case 5:
			case 6:
				m_pFramework->GetPlayer(1)->CharacterStatus = 0;
				break;
			case 3:
			case 4:
			case 7:
				m_pFramework->GetPlayer(1)->CharacterStatus = 1;
				break;
			}
		}
	}
	if (p2key)
	{
		m_pFramework->GetPlayer(2)->isWalk = TRUE;
		if (m_pFramework->GetPlayer(2)->WalkingTimerTick++ > 3)
		{
			m_pFramework->GetPlayer(2)->WalkingTimerTick = 0;
			m_pFramework->GetPlayer(2)->WalkingImageTick++;
		}
	}
	else
	{
		if (m_pFramework->GetPlayer(2)->isAttack != TRUE)
		{
			m_pFramework->GetPlayer(2)->isWalk = FALSE;
			switch (m_pFramework->GetPlayer(2)->CharacterStatus)
			{
			case 2:
			case 5:
			case 6:
				m_pFramework->GetPlayer(2)->CharacterStatus = 0;
				break;
			case 3:
			case 4:
			case 7:
				m_pFramework->GetPlayer(2)->CharacterStatus = 1;
				break;
			}
		}
	}
}

void CIngameScene::Nevigator()
{
	this->distanceX = (m_pFramework->GetPlayer(2)->x - m_pFramework->GetPlayer(1)->x);
	this->distanceY = (m_pFramework->GetPlayer(2)->y - m_pFramework->GetPlayer(1)->y);
	if (distanceX < m_pFramework->GetRect().right / 4 && distanceX > -m_pFramework->GetRect().right / 4 &&
		distanceY < m_pFramework->GetRect().bottom / 2 && distanceY > -m_pFramework->GetRect().bottom / 2)
	{
		this->TotalDistance = 0;
	}
	else
	{
		this->TotalDistance = sqrt(distanceX*distanceX + distanceY*distanceY);				//거리 구하기
	}
	this->rad = atan2(distanceY, distanceX);
	this->Angle = rad * 180 / 3.14;
	this->Angle += 180;
}

void CIngameScene::Update(float fTimeElapsed)
{
	if (RemainTime <= 0)
	{
		// 추후 수정해야함 (서버에서 받은 캐릭터 정보로 코인유무 판별후 status 변경하는 걸로)
		if (m_pFramework->GetPlayer(1)->iHaveCoin == TRUE)
		{
			isGameEnd = TRUE;
			m_pFramework->GetPlayer(1)->CharacterStatus = 14;
			m_pFramework->GetPlayer(2)->CharacterStatus = 15;
		}
		else if (m_pFramework->GetPlayer(2)->iHaveCoin == TRUE)
		{
			isGameEnd = TRUE;
			m_pFramework->GetPlayer(1)->CharacterStatus = 15;
			m_pFramework->GetPlayer(2)->CharacterStatus = 14;
		}
	}
	//cout << "업데이트가 되고 있나요?" << g_scene_send._scene_num << endl;
	if(isGameEnd == FALSE) //if (g_scene_send._scene_num == Main_game)
	{
		
		KeyState();
		CharacterState();
		//Nevigator();
		CoinObject->Update(fTimeElapsed);
		m_pFramework->GetPlayer(1)->Update(fTimeElapsed);
		m_pFramework->GetPlayer(2)->Update(fTimeElapsed);
		m_pFramework->GetPlayer(3)->Update(fTimeElapsed);
		TimeTick++;

		if (m_pFramework->GetPlayer(1)->DashCoolTimer > 0)
		{
			m_pFramework->GetPlayer(1)->DashCoolTimer--;
		}
		if (m_pFramework->GetPlayer(2)->DashCoolTimer > 0)
		{
			m_pFramework->GetPlayer(2)->DashCoolTimer--;
		}
		if (m_pFramework->GetPlayer(3)->DashCoolTimer > 0)
		{
			m_pFramework->GetPlayer(3)->DashCoolTimer--;
		}
		//if (m_pFramework->GetPlayer(1)->isSkill)
		{
			if (IntersectRect(&tmp, m_pFramework->GetPlayer(2)->getRECT(), m_pFramework->GetPlayer(1)->CSkill->GetRECT())) // p2스킬피격
			{
				switch (m_pFramework->GetPlayer(1)->charNum)
				{
				case 1:
					m_pFramework->GetPlayer(2)->SkillAttackedTimer = 120;
					break;
				case 2:
					m_pFramework->GetPlayer(2)->SkillAttackedTimer = 60;
					break;
				case 3:
					m_pFramework->GetPlayer(2)->SkillAttackedTimer = 180;
					break;
				}
				//m_pFramework->GetPlayer(2)->Old_CharStat = m_pFramework->GetPlayer(2)->CharacterStatus;
				m_pFramework->GetPlayer(2)->isAttacked = TRUE;
				m_pFramework->GetPlayer(2)->CharacterStatus = 9;
				isp2LockDown = TRUE;
				if (m_pFramework->GetPlayer(2)->iHaveCoin)
				{
					m_pFramework->GetPlayer(2)->iHaveCoin = FALSE;
					CoinObject->OnCreate(m_pFramework->GetPlayer(2)->x, m_pFramework->GetPlayer(2)->y);
					coinLockDown = TRUE;
				}
			}
		}
		//if (m_pFramework->GetPlayer(2)->isSkill)
		{
			if (IntersectRect(&tmp, m_pFramework->GetPlayer(1)->getRECT(), m_pFramework->GetPlayer(2)->CSkill->GetRECT()))
			{
				switch (m_pFramework->GetPlayer(2)->charNum)
				{
				case 1:
					m_pFramework->GetPlayer(1)->SkillAttackedTimer = 120;
					break;
				case 2:
					m_pFramework->GetPlayer(1)->SkillAttackedTimer = 60;
					break;
				case 3:
					m_pFramework->GetPlayer(1)->SkillAttackedTimer = 180;
					break;
				}
				//m_pFramework->GetPlayer(1)->Old_CharStat = m_pFramework->GetPlayer(1)->CharacterStatus;
				m_pFramework->GetPlayer(1)->isAttacked = TRUE;
				m_pFramework->GetPlayer(1)->CharacterStatus = 9;
				isp1LockDown = TRUE;

				if (m_pFramework->GetPlayer(1)->iHaveCoin)
				{
					m_pFramework->GetPlayer(1)->iHaveCoin = FALSE;
					CoinObject->OnCreate(m_pFramework->GetPlayer(1)->x, m_pFramework->GetPlayer(1)->y);
					coinLockDown = TRUE;
				}
			}
		}

		if (TimeTick >= 60)
		{
			TimeTick = 0;
			if (RemainTime > 0)
			{
				RemainTime--;
				TimerImage[0] = RemainTime / 10;
				TimerImage[1] = RemainTime % 10;
			}
			if (SkillCoolTime[0] > 0)
			{
				SkillCoolTime[0]--;
			}
			if (SkillCoolTime[1] > 0)
			{
				SkillCoolTime[1]--;
			}
		}
		if (CoinObject->GetbDraw() && coinLockDown == FALSE)
		{
			if (abs(CoinObject->x() - m_pFramework->GetPlayer(1)->x) < 30 && abs(CoinObject->y() - m_pFramework->GetPlayer(1)->y) < 30)
			{
				m_pFramework->GetPlayer(1)->iHaveCoin = TRUE;
				CoinObject->SetDrawFalse();
			}
			if (abs(CoinObject->x() - m_pFramework->GetPlayer(2)->x) < 30 && abs(CoinObject->y() - m_pFramework->GetPlayer(2)->y) < 30)
			{
				m_pFramework->GetPlayer(2)->iHaveCoin = TRUE;
				CoinObject->SetDrawFalse();
			}
		}
	}
	//for (int i = 0; i < nObjects; ++i)
		//ppObjects[i]->Update(fTimeElapsed);
}

void CIngameScene::AngleRender(HDC hdc) // 아 안돼...
{
	//printf("%lf\n", Angle);
	if (TotalDistance > 0)
	{
		if (Angle > 135 && Angle <= 225) // p1가 왼쪽
		{
			if ((sin(rad) + 0.5) * m_pFramework->GetRect().right / 2 > m_pFramework->GetRect().bottom * 0.93)
				anY = m_pFramework->GetRect().bottom * 0.93;
			else if ((sin(rad) + 0.5) * m_pFramework->GetRect().right / 2 < 0)
				anY = 0;
			else
				anY= (sin(rad) + 0.5) * m_pFramework->GetRect().right / 2;
			C_Angle[m_pFramework->GetPlayer(2)->charNum - 1].Draw(hdc,m_pFramework->GetRect().right / 2 - 50,anY,50,50);
			//
			C_Angle[m_pFramework->GetPlayer(1)->charNum - 1].Draw(hdc, 10 + m_pFramework->GetRect().right / 2, m_pFramework->GetRect().bottom-50 -anY, 50, 50);
		}
		else if (Angle > 225 && Angle <= 315) // p1가 위쪽
		{
			if ((cos(rad) + 0.5) * m_pFramework->GetRect().right / 2 > m_pFramework->GetRect().right/2 * 0.93)
				anX = m_pFramework->GetRect().right/2 * 0.93;
			else if ((cos(rad) + 0.5) * m_pFramework->GetRect().right / 2 < 0)
				anX = 0;
			else
				anX = (cos(rad) + 0.5) * m_pFramework->GetRect().right / 2;
			C_Angle[m_pFramework->GetPlayer(2)->charNum - 1].Draw(hdc,anX, m_pFramework->GetRect().bottom - 55, 50, 50);
			//Ellipse(hdc, m_pFramework->GetRect().right / 4 + distanceX / 2 - 10, m_pFramework->GetRect().bottom - 65, m_pFramework->GetRect().right / 4 + distanceX/2 + 10, m_pFramework->GetRect().bottom - 45);
			C_Angle[m_pFramework->GetPlayer(1)->charNum - 1].Draw(hdc, -anX + m_pFramework->GetRect().right - 50, 20, 50, 50);
		}
		else if (Angle > 45 && Angle <= 135) // p1가 아래쪽
		{
			if ((cos(rad) + 0.5) * m_pFramework->GetRect().right / 2 > m_pFramework->GetRect().right / 2 * 0.93)
				anX = m_pFramework->GetRect().right / 2 * 0.93;
			else if ((cos(rad) + 0.5) * m_pFramework->GetRect().right / 2 < 0)
				anX = 0;
			else
				anX = (cos(rad) + 0.5) * m_pFramework->GetRect().right / 2;
			C_Angle[m_pFramework->GetPlayer(2)->charNum - 1].Draw(hdc,anX, 20,50, 50);
			//Ellipse(hdc, m_pFramework->GetRect().right / 4 + distanceX / 2 - 10, 45, m_pFramework->GetRect().right / 4 + distanceX / 2 + 10, 65);
			C_Angle[m_pFramework->GetPlayer(1)->charNum - 1].Draw(hdc, -anX + m_pFramework->GetRect().right - 20, m_pFramework->GetRect().bottom - 50, 50, 50);
		}
		else								// p1가 오른쪽
		{
			if ((sin(rad) + 0.5) * m_pFramework->GetRect().right / 2 > m_pFramework->GetRect().bottom * 0.93)
				anY = m_pFramework->GetRect().bottom * 0.93;
			else if ((sin(rad) + 0.5) * m_pFramework->GetRect().right / 2 < 0)
				anY = 0;
			else
				anY = (sin(rad) + 0.5) * m_pFramework->GetRect().right / 2;
			C_Angle[m_pFramework->GetPlayer(2)->charNum - 1].Draw(hdc, 10,anY, 50, 50);
			//Ellipse(hdc, 10, m_pFramework->GetRect().bottom / 2 + distanceY / 2 - 10, 30, m_pFramework->GetRect().bottom / 2 + distanceY / 2 + 10);
			C_Angle[m_pFramework->GetPlayer(1)->charNum - 1].Draw(hdc, m_pFramework->GetRect().right - 30, m_pFramework->GetRect().bottom - 50 -anY, 50, 50);
		}
	}
}

void CIngameScene::Render(HDC hdc)
{
	keydown = FALSE;
	for (int i = 0; i < 14; i++)
	{
		keydownList[i] = FALSE;
	}
	p1key = false;
	p2key = false;

	//타일
	BitBlt(*m_pFramework->GetPlayerDC(), m_pFramework->GetPlayer(1)->x - m_pFramework->p1.right / 2, m_pFramework->GetPlayer(1)->y - m_pFramework->p1.bottom / 2, m_pFramework->p1.right, m_pFramework->p1.bottom,
		*m_pFramework->GetTileDC(), m_pFramework->GetPlayer(1)->x - m_pFramework->p1.right / 2, m_pFramework->GetPlayer(1)->y - m_pFramework->p1.bottom / 2, SRCCOPY);
	//BitBlt(*m_pFramework->GetPlayerDC(), m_pFramework->GetPlayer(2)->x - m_pFramework->p1.right / 2, m_pFramework->GetPlayer(2)->y - m_pFramework->p1.bottom / 2, m_pFramework->p1.right, m_pFramework->p2.bottom,
	//	*m_pFramework->GetTileDC(), m_pFramework->GetPlayer(2)->x - m_pFramework->p1.right / 2, m_pFramework->GetPlayer(2)->y - m_pFramework->p2.bottom / 2, SRCCOPY);


	//플레이어
	if (m_pFramework->GetPlayer(1)->y > m_pFramework->GetPlayer(2)->y)
	{
		m_pFramework->GetPlayer(2)->Render(m_pFramework->GetPlayerDC());
		m_pFramework->GetPlayer(1)->Render(m_pFramework->GetPlayerDC());
		m_pFramework->GetPlayer(3)->Render(m_pFramework->GetPlayerDC()); //PL3 추가
	}
	else
	{
		m_pFramework->GetPlayer(1)->Render(m_pFramework->GetPlayerDC());
		m_pFramework->GetPlayer(2)->Render(m_pFramework->GetPlayerDC());
		m_pFramework->GetPlayer(3)->Render(m_pFramework->GetPlayerDC()); //PL3 추가
	}
	//코인
	CoinObject->Render(&*m_pFramework->GetPlayerDC());
	
	//Ellipse(*m_pFramework->GetPlayerDC(), m_pFramework->GetPlayer(1)->x - 5, m_pFramework->GetPlayer(1)->y - 5, m_pFramework->GetPlayer(1)->x + 5, m_pFramework->GetPlayer(1)->y + 5);
	//Ellipse(*m_pFramework->GetPlayerDC(), m_pFramework->GetPlayer(2)->x - 5, m_pFramework->GetPlayer(2)->y - 5, m_pFramework->GetPlayer(2)->x + 5, m_pFramework->GetPlayer(2)->y + 5);

	//토탈로 옮기기
	BitBlt(hdc, m_pFramework->p1.left, m_pFramework->p1.top, m_pFramework->p1.right, m_pFramework->p1.bottom, *m_pFramework->GetPlayerDC(), m_pFramework->GetPlayer(1)->x - m_pFramework->p1.right / 2, m_pFramework->GetPlayer(1)->y - m_pFramework->p1.bottom / 2, SRCCOPY);
	BitBlt(hdc, m_pFramework->p2.left, m_pFramework->p2.top, m_pFramework->p1.right, m_pFramework->p2.bottom, *m_pFramework->GetPlayerDC(), m_pFramework->GetPlayer(2)->x - m_pFramework->p1.right / 2, m_pFramework->GetPlayer(2)->y - m_pFramework->p2.bottom / 2, SRCCOPY);
	BitBlt(hdc, m_pFramework->p2.left, m_pFramework->p2.top, m_pFramework->p1.right, m_pFramework->p2.bottom, *m_pFramework->GetPlayerDC(), m_pFramework->GetPlayer(3)->x - m_pFramework->p1.right / 2, m_pFramework->GetPlayer(3)->y - m_pFramework->p2.bottom / 2, SRCCOPY);


	if (isGameEnd) //추후 PL3 추가해야함
	{
		if (m_pFramework->GetPlayer(1)->iHaveCoin == TRUE)
		{
			WinC.Draw(hdc, m_pFramework->p1.right / 2 - 100, m_pFramework->p1.bottom / 2 - 300, 200, 200);
			LoseC.Draw(hdc, m_pFramework->p2.right / 4 * 3 - 100, m_pFramework->p2.bottom / 2 - 300, 200, 200);
		}
		else
		{
			LoseC.Draw(hdc, m_pFramework->p1.right / 2 - 100, m_pFramework->p1.bottom / 2 - 300, 200, 200);
			WinC.Draw(hdc, m_pFramework->p2.right / 4 * 3 - 100, m_pFramework->p2.bottom / 2 - 300, 200, 200);
		}
	}
	else
	{
		AngleRender(hdc);
	}

	for (int i = 0; i < 3; i++)
	{
		m_pFramework->GetPlayer(1)->Image.Skill_I[i].Draw(hdc, 30 + 80 * i, windowY-120,64,64);
		m_pFramework->GetPlayer(2)->Image.Skill_I[i].Draw(hdc,windowX - 280 +( 30 + 80 * i), windowY - 120, 64, 64);
	}
	if (SkillCoolTime[0] > 0)
	{
		Rectangle(hdc, 30, windowY - 120, 30+SkillCoolTime[0]*4, windowY - 120 + 64);
	}
	if (m_pFramework->GetPlayer(1)->DashCoolTimer > 0)
	{
		Rectangle(hdc, 30 + 160, windowY - 120, 30 + 160 + (m_pFramework->GetPlayer(1)->DashCoolTimer / 10), windowY - 120 + 64);
	}
	if (SkillCoolTime[1] > 0)
	{
		Rectangle(hdc, windowX - 280 + 30 + 64 - SkillCoolTime[1]*4, windowY - 120, windowX - 280 + 30 + 64, windowY - 120 + 64);
	}
	if (m_pFramework->GetPlayer(2)->DashCoolTimer > 0)
	{
		Rectangle(hdc, windowX - 280 + 30 + 160 + 64 - (m_pFramework->GetPlayer(2)->DashCoolTimer / 10), windowY - 120, windowX - 280 + 30 + 160 + 64, windowY - 120 + 64);
	}

	C_IngameLine.Draw(hdc, 0, 0, m_pFramework->GetRect().right, m_pFramework->GetRect().bottom);
	C_Numbers[TimerImage[0]].Draw(hdc, m_pFramework->GetRect().right / 2 - 90, m_pFramework->GetRect().bottom / 15, 80, 80);
	C_Numbers[TimerImage[1]].Draw(hdc, m_pFramework->GetRect().right / 2 + 20, m_pFramework->GetRect().bottom / 15, 80, 80);

	//플레이어들이 코인을 가지고 있는지 확인
	if (m_pFramework->GetPlayer(1)->iHaveCoin)
	{
		CoinObject->Image.Draw(hdc, m_pFramework->GetRect().right / 2 - 90 - 60, windowY / 15, 50, 50);
	}
	else if (m_pFramework->GetPlayer(2)->iHaveCoin)
	{
		CoinObject->Image.Draw(hdc, m_pFramework->GetRect().right / 2 + 110, windowY / 15, 50, 50);
	}

	for (int i = 0; i < nObjects; ++i)
		ppObjects[i]->Render(*m_pFramework->GetPlayerDC());

}


