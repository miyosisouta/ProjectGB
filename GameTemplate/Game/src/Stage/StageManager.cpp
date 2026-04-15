/**
 * Stage.cpp
 *
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
			// 見た目のオブジェクトを作る必要がないので return true
			return true;
		}


		// ファイルパスがあるなら作る
		if (!assetPath.empty()) {
			auto* staticObject = new StaticObject();
			staticObject->Init(
				assetPath.c_str(),
				data.position,
				data.rotation,
				data.scale
			);
			staticObjectList_.push_back(staticObject);
		}

		return true;
	});
}


StageManager::StageManager()
{
}
StageManager::~StageManager()
{

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
	PhysicsWorld::Get().EnableDrawDebugWireFrame();
#endif

	return true;
}

void StageManagerObject::Update()
{
	StageManager::Get().Update();
}

void StageManagerObject::Render(RenderContext& rc)
{
	StageManager::Get().Render(rc);
}
