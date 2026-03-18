#include "stdafx.h"
#include "system/system.h"

#include<InitGUID.h>
#include<dxgidebug.h>

#include "Game.h"

#include "src/collision/GhostBodyManager.h"

#include "src/Scene/SceneManager.h"
#include "src/Effect/EffectManager.h"
#include "src/Sound/SoundManager.h"



void ReportLiveObjects()
{
	IDXGIDebug* pDxgiDebug;

	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
	fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

	DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDxgiDebug);

	// 出力。
	pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
}

///////////////////////////////////////////////////////////////////
// ウィンドウプログラムのメイン関数。
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//ゲームの初期化。
	InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));
	//////////////////////////////////////
	// ここから初期化を行うコードを記述する。
	//////////////////////////////////////


	GhostBodyManager::Initialize();
	

	//Gameクラスのオブジェクトを作成。
	NewGO<Game>(0, "game");
	// SceneManagerクラスのオブジェクトを生成
	NewGO<SceneMangerObject>(0, "sceneManager");
	// EffectManagerクラスのオブジェクトを生成
	NewGO<EffectManagerObject>(20, "effect");
	//SoundManagerクラスのオブジェクトを生成
	NewGO<SoundManagerObject>(30, "sound");

	//////////////////////////////////////
	// 初期化を行うコードを書くのはここまで！！！
	//////////////////////////////////////
	
	// ここからゲームループ。
	while (DispatchWindowMessage())
	{
		if (g_pad[0]->IsTrigger(enButtonA) ){
			g_pad[0]->SetVibration(/*durationSec=*/0.5f, /*normalizedPower=*/1.0f);
		}
		K2Engine::GetInstance()->Execute();
	}

	GhostBodyManager::Finalize();

	K2Engine::DeleteInstance();

#ifdef _DEBUG
	ReportLiveObjects();
#endif // _DEBUG
	return 0;
}

