#include "stdafx.h"
#include "system/system.h"

#include<InitGUID.h>
#include<dxgidebug.h>

#include "Game.h"
#include "src/CharacterDataBase.h"
#include "src/collision/GhostBodyManager.h"
#include "src/collision/CollisionHitManager.h"

#include "src/Scene/SceneManager.h"
#include "src/Effect/EffectManager.h"
#include "src/Sound/SoundManager.h"

#include "src/UI/UIAnimationParameter.h"

#include "src/Camera/CameraManager.h"
#include "src/Camera/CameraController.h"


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

	// コリジョン
	CollisionHitManager::Initialize();
	GhostBodyManager::Initialize();
	GhostBodyManager::Get().RegisterCallback([](GhostBody* a, GhostBody* b)
		{
			CollisionHitManager::Get().RegisterHitPair(a, b);
		});

	// キャラクターデータ
	CharacterDataBase::CreateInstance();

	// パラメータ
	ParameterManager::CreateInstance();
	ParameterManager::Get().LoadCharacterStatusData("Assets/Objects/CharacterData/CharacterStatusData.json");
	ParameterManager::Get().LoadPlayerSkillStatusData("Assets/Objects/CharacterData/PlayerSkillStatus.json");
	ParameterManager::Get().LoadBossSkillStatusData("Assets/Objects/CharacterData/BossSkillStatus.json");


	// カメラ
	CameraManager::Initialize();
	CameraManager::Get().Setup(g_camera3D);

	// UIAnimationクラスの生成
	UIAnimationParameter::Get().Load("Assets/ui/uiAnimation/UIAnimation.json");
	//Gameクラスのオブジェクトを作成。
	NewGO<Game>(0, "game");	
	// EffectManagerクラスのオブジェクトを生成
	NewGO<EffectManagerObject>(20, "effect");
	//SoundManagerクラスのオブジェクトを生成
	NewGO<SoundManagerObject>(30, "sound");
	// SceneManagerクラスのオブジェクトを生成
	NewGO<SceneMangerObject>(0, "sceneManager");

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

		CameraManager::Get().Update(g_gameTime->GetFrameDeltaTime());


		// デバッグ用カメラの切り替え
#if defined(_DEBUG)
		static bool isTriggerDebugCameraKey = false;
		if (GetAsyncKeyState(VK_F2)) {
			if (!isTriggerDebugCameraKey) {
				static bool isUseDebugCamera = false;
				if (!isUseDebugCamera) {
					RefCameraController debugCamera = std::make_shared<DebugCamera>();
					CameraManager::Get().SwitchCamera(debugCamera);
				}
				else {
					CameraManager::Get().SwitchPrevCamera();
				}
				isUseDebugCamera = !isUseDebugCamera;
				isTriggerDebugCameraKey = true;
			}
		}
		else {
			isTriggerDebugCameraKey = false;
		}
#endif// APP_DEBUG
	}

	CameraManager::Finalize();

	CharacterDataBase::DestroyInstance();
	GhostBodyManager::Finalize();
	CollisionHitManager::Finalize();

	K2Engine::DeleteInstance();

#ifdef _DEBUG
	ReportLiveObjects();
#endif // _DEBUG
	return 0;
}

