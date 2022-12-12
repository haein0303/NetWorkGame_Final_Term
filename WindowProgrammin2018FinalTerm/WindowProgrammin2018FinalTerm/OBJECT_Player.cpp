#include "stdafx.h"
#include "Object_Player.h"
#include "OBJECT_Skill.h"
#include "Framework.h"

CObject_Player::CObject_Player()
{

}

CObject_Player::~CObject_Player()
{
	free(CSkill);
}
//CObject_Player::CObject_Player(int characterNum, HDC PlayerDC)
//{
//	OnCreate(characterNum, PlayerDC);
//}

void CObject_Player::OnCreate(int characterNum)
{
	WCHAR LoadText[100];

	this->charNum = characterNum;


	for (int i = 1; i <= 4; i++)
	{
		wsprintf(LoadText, L"Graphic\\Char%d\\ATTACK\\%d.png", characterNum, i);
		this->Image.Attack[i - 1].Load(LoadText);
		wsprintf(LoadText, L"Graphic\\Char%d\\ATTACK_B\\%d.png", characterNum, i);
		this->Image.Attack_B[i - 1].Load(LoadText);
	}
	for (int i = 1; i <= 3; i++)
	{
		wsprintf(LoadText, L"Graphic\\Char%d\\SKILL_I\\%d.png", characterNum, i);
		this->Image.Skill_I[i - 1].Load(LoadText);
	}
	for (int i = 1; i <= 6; i++)
	{
		wsprintf(LoadText, L"Graphic\\Char%d\\WALK\\%d.png", characterNum, i);
		this->Image.Walk[i - 1].Load(LoadText);
		wsprintf(LoadText, L"Graphic\\Char%d\\WALK_B\\%d.png", characterNum, i);
		this->Image.Walk_B[i - 1].Load(LoadText);
	}
	for (int i = 1; i <= 2; i++)
	{
		wsprintf(LoadText, L"Graphic\\Char%d\\ATTACKED\\%d.png", characterNum, i);
		this->Image.Attacked[i - 1].Load(LoadText);
		wsprintf(LoadText, L"Graphic\\Char%d\\ATTACKED_B\\%d.png", characterNum, i);
		this->Image.Attacked_B[i - 1].Load(LoadText);
	}

	wsprintf(LoadText, L"Graphic\\Char%d\\IDLE\\1.png", characterNum);
	this->Image.Idle.Load(LoadText);
	wsprintf(LoadText, L"Graphic\\Char%d\\IDLE_B\\1.png", characterNum);
	this->Image.Idle_B.Load(LoadText);

	wsprintf(LoadText, L"Graphic\\Char%d\\DASH\\1.png", characterNum);
	this->Image.Dash.Load(LoadText);
	wsprintf(LoadText, L"Graphic\\Char%d\\DASH_B\\1.png", characterNum);
	this->Image.Dash_B.Load(LoadText);

	wsprintf(LoadText, L"Graphic\\Char%d\\WIN\\1.png", characterNum);
	this->Image.Win.Load(LoadText);
	wsprintf(LoadText, L"Graphic\\Char%d\\LOSE\\1.png", characterNum);
	this->Image.Lose.Load(LoadText);

	this->CSkill = new OBJECT_Skill(characterNum);
}

RECT* CObject_Player::getRECT()
{
	reTmp.left = x - this->Image.Idle.GetWidth() / 2;
	reTmp.top = y - this->Image.Idle.GetHeight() / 2;
	reTmp.right = x + this->Image.Idle.GetWidth() / 2;
	reTmp.bottom = y + this->Image.Idle.GetHeight() / 2;
	return &reTmp;
}

void CObject_Player::Update(float fTimeElapsed)
{
	if (this->charNum == 3 && this->isSkill == FALSE)
	{
		this->CSkill->Update(fTimeElapsed);
	}
	else if (this->isSkill)
	{
		if (this->CSkill->Update(fTimeElapsed))
		{
			this->isSkill = FALSE;
			this->isSkillEnd = TRUE;
		}
	}

}

void CObject_Player::SkillCast(int x, int y, int CharacterStatus)
{
	if (this->isSkill)
	{
		this->CSkill->OnCreate(x, y, CharacterStatus);
	}
}

void CObject_Player::Render(HDC* hdc)
{
	switch (this->CharacterStatus) // 0~1 = Idle  // 2~5 = Walk // 6~7 = Attack // 8~9 Attacked // 10~11 Dash // 12~13 Skill // 14 = Win // 15 = Lose
	{
	case CharState::IdleA:
		this->Image.Idle.Draw(*hdc, this->x - this->Image.Idle.GetWidth() / 2, this->y - 5 - this->Image.Idle.GetHeight() / 2,
			this->Image.Idle.GetWidth(), this->Image.Idle.GetHeight());
		if (this->charNum == 3)
			this->CSkill->Render(hdc);
		break;

	// 기존 case 1:
	case CharState::IdleB:
		this->Image.Idle_B.Draw(*hdc, this->x - this->Image.Idle_B.GetWidth() / 2, this->y - 5 - this->Image.Idle_B.GetHeight() / 2,
			this->Image.Idle_B.GetWidth(), this->Image.Idle_B.GetHeight());
		if (this->charNum == 3)
			this->CSkill->Render(hdc);	
		break;
	case CharState::WalkA:
		this->Image.Walk[this->WalkingImageTick % 6].Draw(*hdc, this->x - this->Image.Walk[this->WalkingImageTick % 6].GetWidth() / 2,
			this->y - 5 - this->Image.Walk[this->WalkingImageTick % 6].GetHeight() / 2, this->Image.Walk[this->WalkingImageTick % 6].GetWidth(),
			this->Image.Walk[this->WalkingImageTick % 6].GetHeight());
		if (this->charNum == 3)
			this->CSkill->Render(hdc);
		break;
	case CharState::WalkB:
		this->Image.Walk_B[this->WalkingImageTick % 6].Draw(*hdc, this->x - this->Image.Walk[this->WalkingImageTick % 6].GetWidth() / 2,
			this->y - 5 - this->Image.Walk[this->WalkingImageTick % 6].GetHeight() / 2, this->Image.Walk[this->WalkingImageTick % 6].GetWidth(),
			this->Image.Walk[this->WalkingImageTick % 6].GetHeight());
		if (this->charNum == 3)
			this->CSkill->Render(hdc);
		break;

	//case 6:
	case CharState::Attack : //d키
		this->Image.Attack[this->AttackImageTick].Draw(*hdc, this->x - this->Image.Attack[this->AttackImageTick].GetWidth() / 2,
			this->y - 5 - this->Image.Attack[this->AttackImageTick].GetHeight() / 2, this->Image.Attack[this->AttackImageTick].GetWidth(),
			this->Image.Attack[this->AttackImageTick].GetHeight());
		if (this->charNum == 3)
			this->CSkill->Render(hdc);
		break;
	//case 7:
	//	this->Image.Attack_B[this->AttackImageTick].Draw(*hdc, this->x - this->Image.Attack_B[this->AttackImageTick].GetWidth() / 2,
	//		this->y - 5 - this->Image.Attack_B[this->AttackImageTick].GetHeight() / 2, this->Image.Attack_B[this->AttackImageTick].GetWidth(),
	//		this->Image.Attack_B[this->AttackImageTick].GetHeight());
	//	if (this->charNum == 3)
	//		this->CSkill->Render(hdc);
	//	break;
	case CharState::Attacked:
		cout << "Attacked!!" << endl;
		this->Image.Attacked[WalkingImageTick % 2].Draw(*hdc, this->x - this->Image.Attacked[WalkingImageTick % 2].GetWidth() / 2,
			this->y - 5 - Image.Attacked[WalkingImageTick % 2].GetHeight() / 2, Image.Attacked[WalkingImageTick % 2].GetWidth(), Image.Attacked[WalkingImageTick % 2].GetHeight());
		if (this->charNum == 3)
			this->CSkill->Render(hdc);
		break;
	//case 9:
	//	this->Image.Attacked_B[WalkingImageTick % 2].Draw(*hdc, this->x - this->Image.Attacked_B[WalkingImageTick % 2].GetWidth() / 2,
	//		this->y - 5 - Image.Attacked_B[WalkingImageTick % 2].GetHeight() / 2, Image.Attacked_B[WalkingImageTick % 2].GetWidth(), Image.Attacked_B[WalkingImageTick % 2].GetHeight());
	//	if (this->charNum == 3)
	//		this->CSkill->Render(hdc);
	//	break;
	//case 10:
	case CharState::Dash:
		if (DashTimerTick < 7)
		{ }
		else
		this->Image.Dash.Draw(*hdc, this->x - this->Image.Dash.GetWidth() / 2, this->y - 5 - Image.Dash.GetHeight() / 2, Image.Dash.GetWidth(), Image.Dash.GetHeight());
		if (this->charNum == 3)
			this->CSkill->Render(hdc);
		break;
	case 11:
		if (DashTimerTick < 7)
		{}
		else
		this->Image.Dash_B.Draw(*hdc, this->x - this->Image.Dash_B.GetWidth() / 2, this->y - 5 - Image.Dash_B.GetHeight() / 2, Image.Dash_B.GetWidth(), Image.Dash_B.GetHeight());
		if (this->charNum == 3)
			this->CSkill->Render(hdc);
		break;
	//case 12: //스킬
	case CharState::Skill: //스킬
		this->Image.Attack[this->AttackImageTick].Draw(*hdc, this->x - this->Image.Attack[this->AttackImageTick].GetWidth() / 2,
			this->y - 5 - this->Image.Attack[this->AttackImageTick].GetHeight() / 2, this->Image.Attack[this->AttackImageTick].GetWidth(),
			this->Image.Attack[this->AttackImageTick].GetHeight());
		if (this->isSkill)
		this->CSkill->Render(hdc);
		break;
	case 13:
		this->Image.Attack_B[this->AttackImageTick].Draw(*hdc, this->x - this->Image.Attack_B[this->AttackImageTick].GetWidth() / 2,
			this->y - 5 - this->Image.Attack_B[this->AttackImageTick].GetHeight() / 2, this->Image.Attack_B[this->AttackImageTick].GetWidth(),
			this->Image.Attack_B[this->AttackImageTick].GetHeight());
		if (this->isSkill)
		this->CSkill->Render(hdc);
		break;
	case 14:
		this->Image.Win.Draw(*hdc, this->x - this->Image.Idle_B.GetWidth() / 2, this->y - 5 - this->Image.Idle_B.GetHeight() / 2,
			this->Image.Idle_B.GetWidth(), this->Image.Idle_B.GetHeight());
		break;
	case 15:
		this->Image.Lose.Draw(*hdc, this->x - this->Image.Idle_B.GetWidth() / 2, this->y - 5 - this->Image.Idle_B.GetHeight() / 2,
			this->Image.Idle_B.GetWidth(), this->Image.Idle_B.GetHeight());
		break;
	}
}


