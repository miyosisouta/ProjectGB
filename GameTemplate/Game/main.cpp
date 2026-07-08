#include "stdafx.h"
#include "system/system.h"

#include<InitGUID.h>
#include<dxgidebug.h>

#include "Game.h"
#include "src/CharacterDataBase.h"
#include "src/collision/GhostBodyManager.h"
#include "src/collision/CollisionHitManager.h"

#include "src/Scene/SceneManager.h"
#include "src/Sound/SoundManager.h"
#include "src/Stage/StageManager.h"
#include "src/Effect/EffectManager.h"

#include "src/UI/UIAnimationParameter.h"

#include "src/Camera/CameraManager.h"
#include "src/Camera/CameraController.h"
#include "src/Camera/CameraConfig.h"


void ReportLiveObjects()
{
	IDXGIDebug* pDxgiDebug;

	typedef HRESULT(__stdcall* fPtr)(const IID&, void**);
	HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
	fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

	DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDxgiDebug);

	// �o�́B
	pDxgiDebug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
}

///////////////////////////////////////////////////////////////////
// �E�B���h�E�v���O�����̃��C���֐��B
///////////////////////////////////////////////////////////////////
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//�Q�[���̏������B
	InitGame(hInstance, hPrevInstance, lpCmdLine, nCmdShow, TEXT("Game"));
	//////////////////////////////////////
	// �������珉�������s���R�[�h���L�q����B
	//////////////////////////////////////

	g_renderingEngine->DisableRaytracing();

	// �R���W����
	CollisionHitManager::Initialize();
	GhostBodyManager::Initialize();
	GhostBodyManager::Get().RegisterCallback([](GhostBody* a, GhostBody* b)
		{
			CollisionHitManager::Get().RegisterHitPair(a, b);
		});

	// �L�����N�^�[�f�[�^
	CharacterDataBase::CreateInstance();

	// �p�����[�^
	ParameterManager::CreateInstance();
	ParameterManager::Get().LoadCharacterStatusData("Assets/Objects/CharacterData/CharacterStatusData.json");
	ParameterManager::Get().LoadPlayerSkillStatusData("Assets/Objects/CharacterData/PlayerSkillStatus.json");
	ParameterManager::Get().LoadBossSkillStatusData("Assets/Objects/CharacterData/BossSkillStatus.json");
	ParameterManager::Get().LoadBattleCommonParamData("Assets/Parameter/BattleCommonParameter.json");
	ParameterManager::Get().LoadGrassBendParamData("Assets/Objects/Stage/Forest/ObjectData/grass_attack_params.json");
	ParameterManager::Get().LoadMissionParamData("Assets/Parameter/MissionParameter.json");
	ParameterManager::Get().LoadCollisionHitParamData("Assets/Parameter/CollisionHitParameter.json");
	ParameterManager::Get().LoadPlayerStateParamData("Assets/Parameter/PlayerStateParameter.json");
	ParameterManager::Get().LoadNPCControllerParamData("Assets/Parameter/NPCControllerParameter.json");
	ParameterManager::Get().LoadBossSpawnerParamData("Assets/Parameter/BossSpawnerParameter.json");
	ParameterManager::Get().LoadSkillParamData("Assets/Parameter/SkillParameter.json");
	ParameterManager::Get().LoadAttackObjectParamData("Assets/Parameter/AttackObjectParameter.json");
	ParameterManager::Get().LoadStageManagerParamData("Assets/Parameter/StageManagerParameter.json");
	ParameterManager::Get().LoadEmotionParamData("Assets/Parameter/EmotionParameter.json");
	ParameterManager::Get().LoadBossStateParamData("Assets/Parameter/BossStateParameter.json");
	ParameterManager::Get().LoadBossPhaseCutSceneParamData("Assets/Parameter/BossPhaseCutSceneParameter.json");

	// カメラコンフィグ
	CameraConfig::CreateInstance();
	CameraConfig::Get().Initialize();

	// �L�[�R���t�B�O
	KeyConfig::CreateInstance();
	KeyConfig::Get().ResetToDefault();

	// �J����
	CameraManager::Initialize();
	CameraManager::Get().Setup(g_camera3D);

	// UIAnimation�N���X�̐���
	UIAnimationParameter::Get().Load("Assets/ui/uiAnimation/UIAnimation.json");
	//Game�N���X�̃I�u�W�F�N�g���쐬�B
	NewGO<Game>(0, "game");	
	// EffectManager�N���X�̃I�u�W�F�N�g�𐶐�
	NewGO<EffectManagerObject>(20, "effect");
	//SoundManager�N���X�̃I�u�W�F�N�g�𐶐�
	NewGO<SoundManagerObject>(30, "sound");
	// SceneManager�N���X�̃I�u�W�F�N�g�𐶐�
	NewGO<SceneMangerObject>(0, "sceneManager");

	//////////////////////////////////////
	// ���������s���R�[�h�������̂͂����܂ŁI�I�I
	//////////////////////////////////////
	
	// ��������Q�[�����[�v�B
	while (DispatchWindowMessage())
	{
		K2Engine::GetInstance()->Execute();

		CameraManager::Get().Update(g_gameTime->GetFrameDeltaTime());


		// �f�o�b�O�p�J�����̐؂�ւ�
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
	CharacterDataBase::Finalize();
	GhostBodyManager::Finalize();
	CollisionHitManager::Finalize();
	KeyConfig::Finalize();
	CameraConfig::Finalize();

	K2Engine::DeleteInstance();

#ifdef _DEBUG
	ReportLiveObjects();
#endif // _DEBUG
	return 0;
}

