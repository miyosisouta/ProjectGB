/**
 * Stage.cpp
 * ステージの描画を行う
 */


#include "stdafx.h"
#include <fstream>
#include "StageManager.h"
#include "StaticObject.h"
#include "GrassBendManager.h"
#include "src/CharacterDataBase.h"
#include "src/collision/PhysicalBody.h"

StageManager* StageManager::instance_        = nullptr;
bool          StageManager::s_disableGrassLoad_ = false;


namespace
{
	/* ディザリング時の透明度 */
	constexpr float DITHERING_ALPHA_GROUND = 1.0f;
	constexpr float DITHERING_ALPHA_FENCE = 0.5f;
	constexpr float DITHERING_ALPHA_TREE = 0.2f;

	/* その他 */
	constexpr float COLLISION_UP = 600.0f;
	constexpr float METER_TO_CENTIMETER = 100.0f;

	/* 草のLODモデルパス [0]=near [1]=mid [2]=far */
	const char* GRASS_LOD_MODEL_PATHS[StageManager::GRASS_LOD_COUNT] = {
		"Assets/Objects/Stage/Forest/ObjectData/grass.tkm",
		"Assets/Objects/Stage/Forest/ObjectData/grassLOD1.tkm",
		"Assets/Objects/Stage/Forest/ObjectData/grassLOD2.tkm",
	};

	/* 草LODの切り替え距離 (ワールド単位) */
	constexpr float GRASS_LOD0_MAX_DIST = 1900.0f; // これ以下 → LOD0 (grass,   near)
	constexpr float GRASS_LOD1_MAX_DIST = 2500.0f; // これ以下 → LOD1 (grassLOD1, mid)
	                                                // これより遠い → LOD2 (grassLOD2, far)

	/* 草の配置JSONパス */
	const char* GRASS_JSON_PATH = "Assets/Objects/Stage/Forest/ObjectData/grass_placement.json";
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


void StageManager::LoadGrassFromJson(const char* path)
{
	std::ifstream ifs(path);
	if (!ifs.is_open()) { return; }

	nlohmann::json root;
	try { ifs >> root; }
	catch (...) { return; }

	if (!root.contains("grass")) { return; }

	const auto& arr = root["grass"];
	const int count = (int)arr.size();
	if (count == 0) { return; }

	// LOD0 (near) と LOD1 (mid) は草曲げシェーダーを適用
	// LOD2 (far) は遠すぎるためデフォルトシェーダーで描画
	for (int lod = 0; lod < GRASS_LOD_COUNT; lod++)
	{
		if (lod < 2 && GrassBendManager::IsInitialized())
		{
			grassRenderer_[lod].SetGBufferFxOverride("Assets/shader/grass.fx");
			grassRenderer_[lod].SetVSEntryOverride("VSGrass");
			grassRenderer_[lod].SetExtraGBufferSRV(0, &GrassBendManager::Get().GetStructuredBuffer());
		}
		grassRenderer_[lod].Init(GRASS_LOD_MODEL_PATHS[lod], nullptr, 0, enModelUpAxisZ, true, count);
		grassRenderer_[lod].SetDitherAlpha(1.0f);
	}

	// カリング用ローカルAABBをLOD0から計算
	grassTemplateBounds_.Compute(grassRenderer_[0].GetModel());

	grassTransforms_.reserve(count);
	for (int i = 0; i < count; i++)
	{
		const auto& e = arr[i];

		GrassTransform t;

		const auto& p = e["position"];
		t.position = Vector3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());

		const auto& r = e["rotation"];
		t.rotation = Quaternion(r[0].get<float>(), r[1].get<float>(), r[2].get<float>(), r[3].get<float>());

		const auto& s = e["scale"];
		t.scale = Vector3(s[0].get<float>(), s[1].get<float>(), s[2].get<float>());

		grassTransforms_.push_back(t);

		// 初期値: LOD0をアクティブ、LOD1/2はscale=0
		grassRenderer_[0].UpdateInstancingData(i, t.position, t.rotation, t.scale);
		for (int lod = 1; lod < GRASS_LOD_COUNT; lod++)
		{
			grassRenderer_[lod].UpdateInstancingData(i, t.position, t.rotation, Vector3::Zero);
		}
	}
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

	if (!s_disableGrassLoad_)
	{
#if defined(_DEBUG)
		if (!isDisableGlass)
#endif
		{
			LoadGrassFromJson(GRASS_JSON_PATH);
		}
	}

	return true;
}


void StageManager::Update()
{
	stageCullingSystem_->Update(staticObjectList_);

	// 草のフラスタムカリング + LOD選択 + インスタンシングデータ更新
	if (!grassTransforms_.empty())
	{
		Frustum frustum;
		frustum.BuildFromViewProjectionMatrix(g_camera3D->GetViewProjectionMatrix());
		const Vector3 camPos = g_camera3D->GetPosition();

		const float dist0Sq = GRASS_LOD0_MAX_DIST * GRASS_LOD0_MAX_DIST;
		const float dist1Sq = GRASS_LOD1_MAX_DIST * GRASS_LOD1_MAX_DIST;

		for (int i = 0; i < (int)grassTransforms_.size(); i++)
		{
			const auto& t = grassTransforms_[i];

			Bounds wb;
			wb.minPoint = grassTemplateBounds_.minPoint + t.position;
			wb.maxPoint = grassTemplateBounds_.maxPoint + t.position;

			// フラスタム外はすべてのLODを非表示
			if (!frustum.IsVisible(wb))
			{
				for (int lod = 0; lod < GRASS_LOD_COUNT; lod++)
				{
					grassRenderer_[lod].UpdateInstancingData(i, t.position, t.rotation, Vector3::Zero);
				}
				continue;
			}

			// カメラとの距離でアクティブなLODを決定
			float distSq = (Vector3(t.position.x - camPos.x, 0.0f, t.position.z - camPos.z)).LengthSq();
			int activeLod;
			if      (distSq < dist0Sq) { activeLod = 0; }
			else if (distSq < dist1Sq) { activeLod = 1; }
			else                       { activeLod = 2; }

			// アクティブなLODに実スケール、他はscale=0
			for (int lod = 0; lod < GRASS_LOD_COUNT; lod++)
			{
				grassRenderer_[lod].UpdateInstancingData(
					i, t.position, t.rotation,
					(lod == activeLod) ? t.scale : Vector3::Zero
				);
			}
		}
	}
}


void StageManager::Render(RenderContext& rc)
{
	for (auto* object : staticObjectList_)
	{
		object->Render(rc);
	}

	if (!grassTransforms_.empty())
	{
		for (int lod = 0; lod < GRASS_LOD_COUNT; lod++)
		{
			grassRenderer_[lod].Draw(rc);
		}
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
