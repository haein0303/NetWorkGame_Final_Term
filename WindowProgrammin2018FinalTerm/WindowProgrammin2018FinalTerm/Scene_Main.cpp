#include "stdafx.h"
#include "Scene_Main.h"
#include "Framework.h"

CMainScene::CMainScene()
{

}


CMainScene::~CMainScene()
{
}
CMainScene::CMainScene(SceneTag tag, CFramework * pFramework) : CScene(tag, pFramework)
{

}


void CMainScene::OnDestroy() {
	MainTitle.Destroy();

}

bool CMainScene::OnCreate()
{
	//기본적으론 false
	CheckKey = false;
	SceneNum = 0;
	finish = false;

	MainTitle.Load(L"Graphic\\UI\\MAINTITLE.jpg");

	return false;
}

void CMainScene::BuildObjects()
{


}

void CMainScene::Render(HDC hdc)
{
	switch (SceneNum)
	{
	case 0:
		MainTitle.Draw(hdc, 0, 0, windowX, windowY);
		break;
	case 1:
		break;
	case 2:
		break;
	default:
		MainTitle.Draw(hdc, 0, 0, windowX, windowY);
		break;
	}	
}

void CMainScene::Update(float fTimeElapsed)
{
	KeyState();

	if (finish)
	{
		m_pFramework->ChangeScene(CScene::SceneTag::Select_Char);
		m_pFramework->curSceneCreate();
		CMainScene::OnDestroy();
	}
}

void CMainScene::KeyState() {
	if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		if (!CheckKey) {
			switch (SceneNum) {
			case 0:
				//SceneNum = 3;
				finish = true;
				break;
			//case 1:
			//	SceneNum = 2;
			//	break;
			//case 2:
			//	SceneNum = 3;
			//	break;
			case 3:
				finish = true;
				break;

			}
		}
		CheckKey = true;
	}
	else {
		CheckKey = false;
	}
}
