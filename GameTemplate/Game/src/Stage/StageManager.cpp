/**
 * Stage.cpp
 * ステージの描画を行う
 */


#include "stdafx.h"
#include "StageManager.h"
#include "StaticObject.h"
#include "src/CharacterDataBase.h"
#include "src/collision/PhysicalBody.h"

StageManager* StageManager::instance_ = nullptr;


namespace 
{
	/* ディザリング時の透明度 */
	constexpr float DITHERING_ALPHA_GROUND = 1.0f;
	constexpr float DITHERING_ALPHA_FENCE = 0.5f;
	constexpr float DITHERING_ALPHA_TREE = 0.2f;

	/* その他 */
	constexpr float COLLISION_UP = 600.0f;
	constexpr float METER_TO_CENTIMETER = 100.0f;
}

void StageManager::StageTKLLoader(const char* path)
{
	LevelRender stage;

	stage.Init(path, [&](LevelObjectData& data)
	{
		// ファイルパス
		std::string assetPath;

		/** ここで名前に応じたファイルパスを取得・設定 */
		if (data.ForwardMatchName(L"ground")){
			assetPath = "Assets/Objects/Stage/Forest/ObjectData/ground.tkm";
		}

		else if (data.ForwardMatchName(L"fence")){
			assetPath = "Assets/Objects/Stage/Forest/ObjectData/fence.tkm";
		}

		else if (data.ForwardMatchName(L"Tree")){
			assetPath = "Assets/Objects/Stage/Forest/ObjectData/tree.tkm";
		}

		else if (data.ForwardMatchName(L"collisionBox")) {
			auto collision = new PhysicalBody();
			// collisionの位置をcollisionの大きさによって上げる
			Vector3 pos = Vector3(data.position.x, data.position.y + COLLISION_UP, data.position.z);
			// コリジョンの作成
			collision->CreateBox(
				data.scale * METER_TO_CENTIMETER,
				pos,
				enCollisionAttr_Ground
			);
			collisionList_.push_back(collision);
			// 見た目のオブジェクトを作る必要がないので処理をか
			return true;
		}

		else if (data.ForwardMatchName(L"GrassArea")) {
			if (data.EqualObjectName(L"GrassArea01")) {
				grassAreaPos_[0] = data.position;
			}
			else if (data.EqualObjectName(L"GrassArea02")) {
				grassAreaPos_[1] = data.position;
			}
		}


		if (!assetPath.empty()) {
			auto* staticObject = new StaticObject();
			staticObject->Init(assetPath.c_str(), data.position, data.rotation, data.scale);

			// ← ここでアルファ値を設定
			if (data.ForwardMatchName(L"ground")) {
				staticObject->SetDitherAlpha(DITHERING_ALPHA_GROUND);   // 床：透明にしない
			}
			else if (data.ForwardMatchName(L"fence")) {
				staticObject->SetDitherAlpha(DITHERING_ALPHA_FENCE);   // フェンス：半透明
			}
			else if (data.ForwardMatchName(L"Tree")) {
				staticObject->SetDitherAlpha(DITHERING_ALPHA_TREE);   // 木：やや透明
			}

			staticObjectList_.push_back(staticObject);
		}
	});
}


StageManager::StageManager()
{
}
StageManager::~StageManager()
{
	// 見た目のあるオブジェクトの削除
	for (auto* obj : staticObjectList_) {
		delete obj;
	}
	staticObjectList_.clear();

	// コリジョンの削除
	for (auto* obj : collisionList_) {
		delete obj;
	}
	collisionList_.clear();

	// 草のオブジェクトを削除
	for (auto* obj : grassObjectList_) {
		delete obj;
	}
	grassObjectList_.clear();

}


bool StageManager::Start()
{
	// 選択されたボスを取得
	//BossType stageKind = CharacterDataBase::Get().GetGameParam().stageType_;
	BossType stageKind = BossType::enGorilla;

	// ボスによってステージを作成
	switch (stageKind)
	{
	case BossType::enGorilla:
	{
		StageTKLLoader("Assets/Objects/Stage/Forest/tkl/Stage_Gollira.tkl");
		break;
	}

	case BossType::enTurtle:
	{
		StageTKLLoader("Assets/Objects/Stage/Forest/tkl/Stage_Turtle.tkl");
		break;
	}

	default:
		K2_ASSERT(true,"ボスタイプが設定されていません");
		break;
	}

	stageCullingSystem_ = std::make_unique<StageCullingSystem>();

#if defined(_DEBUG)
	if (!isDisableGlass)
#endif
	{
		// json読み込み
		// 草をnewgo
	}

	return true;
}


void StageManager::Update()
{
	stageCullingSystem_->Update(staticObjectList_);
}


void StageManager::Render(RenderContext& rc)
{
	for (auto* object : staticObjectList_) 
	{
		object->Render(rc);
	}
	for (auto* object : grassObjectList_)
	{
		object->Render(rc);
	}
}


/******************* ステージマネージャーオブジェクト ********************************/

StageManagerObject::StageManagerObject()
{
}

StageManagerObject::~StageManagerObject()
{
	StageManager::Get().Finalize();
}

bool StageManagerObject::Start()
{
	StageManager::Initialize();
	StageManager::Get().Start();

	// TODO : 当たり判定の可視化
#ifdef K2_DEBUG 
	//PhysicsWorld::Get().EnableDrawDebugWireFrame();
#endif

	return true;
}

void StageManagerObject::Update()
{
	if (!isUpdate_) return;
	StageManager::Get().Update();
}

void StageManagerObject::Render(RenderContext& rc)
{
	StageManager::Get().Render(rc);
}
