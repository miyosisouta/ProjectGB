/**
 * DebugScene.cpp
 *
 * デバックの描画
 */

#include "stdafx.h"
#include <random>
#include <fstream>
#include "DebugGrassScene.h"
#include "src/Stage/StageManager.h"
#include "src/Stage/GrassObject.h"
#include "src/collision/BoundingVolume.h"

namespace
{
	/* 優先度 */
	constexpr uint8_t PRIORITY_STAGE = 0;	// ステージ

	/* 配列 */
	constexpr uint8_t AREA_MIN_POS_ARRAY_NUM = 0;
	constexpr uint8_t AREA_MAX_POS_ARRAY_NUM = 1;

	/* 草を作る個数 */
	constexpr int CREATE_GRASS_NUM = 300;

	/* 草を削除する時間の猶予 : 草を作り始めるまでの時間 */
	constexpr float CREATE_GRASS_TIME = 1.0f;

	/* 作った際のy座標の位置 */
	constexpr float CREATE_INIT_POS_Y = 10.0f;
}



void DebugGrassScene::GenarateGrass()
{
	// 現在のインスタンスをすべてリセット
	for (int i = 0; i < (int)grassTransforms_.size(); i++) { grassRenderer_.RemoveInstance(i); }
	grassTransforms_.clear();

	// アセットパスの設定
	std::string assetPath = "Assets/Objects/Stage/Forest/ObjectData/grass.tkm";

	// エリアを計算するために必要な座標を取得
	Vector3 areaMinPos = StageManager::Get().GetGrassAreaPos(AREA_MIN_POS_ARRAY_NUM);
	Vector3 areaMaxPos = StageManager::Get().GetGrassAreaPos(AREA_MAX_POS_ARRAY_NUM);

	// エリアを計算
	float minX = min(areaMinPos.x, areaMaxPos.x);
	float maxX = max(areaMinPos.x, areaMaxPos.x);
	float minZ = min(areaMinPos.z, areaMaxPos.z);
	float maxZ = max(areaMinPos.z, areaMaxPos.z);

	// ランダムで座標を出す
	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> distX(minX, maxX);
	std::uniform_real_distribution<float> distZ(minZ, maxZ);

	// 前の草が消えるまでの時間の猶予を持たせる
	task_->AddTimer(CREATE_GRASS_TIME, [this, rng, distX, distZ, assetPath]() mutable
		{
			// モデルのローカルAABBをテンプレートとして1回だけ計算
			Bounds templateBounds;
			{
				GrassObject tempGrass;
				tempGrass.Init(assetPath.c_str(), Vector3::Zero, Quaternion::Identity, Vector3::One);
				templateBounds.Compute(tempGrass.GetModel()->GetModel());
			}

			// 配置済みワールドAABBのリスト
			std::vector<Bounds> placedBounds;
			placedBounds.reserve(CREATE_GRASS_NUM);

			// 再抽選の上限(エリアが狭い時の無限ループ防止)
			constexpr int MAX_RETRY = 100;

			// インスタンス番号
			int instanceNo = 0;

			// 作りたい数だけ回す
			for (int i = 0; i < CREATE_GRASS_NUM; i++)
			{
				for (int retry = 0; retry < MAX_RETRY; retry++)
				{
					// 座標をランダムで取得
					float x = distX(rng);
					float z = distZ(rng);
					Vector3 initPos = Vector3(x, CREATE_INIT_POS_Y, z);

					// この位置のワールドAABBを計算
					Bounds worldBounds;
					worldBounds.minPoint = templateBounds.minPoint + initPos;
					worldBounds.maxPoint = templateBounds.maxPoint + initPos;

					// 既存の草AABBと重なるかチェック
					bool overlaps = false;
					for (const auto& pb : placedBounds)
					{
						if (worldBounds.minPoint.x <= pb.maxPoint.x &&
							worldBounds.maxPoint.x >= pb.minPoint.x &&
							worldBounds.minPoint.z <= pb.maxPoint.z &&
							worldBounds.maxPoint.z >= pb.minPoint.z)
						{
							overlaps = true;
							break;
						}
					}

					if (!overlaps)
					{
						// インスタンスデータを登録
						GrassTransform t;
						t.position = initPos;
						t.rotation = Quaternion::Identity;
						t.scale    = Vector3::One;
						grassTransforms_.push_back(t);

						grassRenderer_.UpdateInstancingData(instanceNo, t.position, t.rotation, t.scale);
						instanceNo++;

						placedBounds.push_back(worldBounds);
						break;
					}
				}
			}
		});
}

void DebugGrassScene::ExportJson()
{
	nlohmann::json grassArray = nlohmann::json::array();

	for (const auto& t : grassTransforms_)
	{
		nlohmann::json entry;
		entry["position"] = { t.position.x, t.position.y, t.position.z };
		entry["rotation"] = { t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w };
		entry["scale"]    = { t.scale.x, t.scale.y, t.scale.z };
		grassArray.push_back(entry);
	}

	nlohmann::json root;
	root["grass"] = grassArray;

	std::ofstream ofs("Assets/Objects/Stage/Forest/ObjectData/grass_placement.json");
	ofs << root.dump(4);
}



DebugGrassScene::DebugGrassScene()
{
}


DebugGrassScene::~DebugGrassScene()
{
	DeleteGO(stage_);
}


bool DebugGrassScene::Start()
{
	// パラメータの取得
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);

	// DebugGrassSceneでは草JSONを読み込まない
	StageManager::SetDisableGrassLoad(true);

	// ステージ
	stage_ = NewGO<StageManagerObject>(PRIORITY_STAGE, "stage");

	// タスクスケジューラを作成
	task_ = std::make_unique<TaskSchedulerSystem>();

	// 草のインスタンシングレンダラーを初期化
	grassRenderer_.Init(
		"Assets/Objects/Stage/Forest/ObjectData/grass.tkm",
		nullptr, 0, enModelUpAxisZ, true, CREATE_GRASS_NUM);

	g_ditherCBData.isEnable = 0.0f;

	return true;
}


void DebugGrassScene::Update()
{
	// タスクスケジューラの時間を進める
	if (task_) { task_->Update(g_gameTime->GetFrameDeltaTime()); }

	// Aボタンを押したら草を再生成
	if (g_pad[0]->IsTrigger(enButtonA))
	{
		GenarateGrass();
	}

	// Bボタンならjsonに出力
	else if (g_pad[0]->IsTrigger(enButtonB))
	{
		ExportJson();
	}

	// 草のインスタンシングデータを更新
	for (int i = 0; i < (int)grassTransforms_.size(); i++)
	{
		const auto& t = grassTransforms_[i];
		grassRenderer_.UpdateInstancingData(i, t.position, t.rotation, t.scale);
	}
}


void DebugGrassScene::Render(RenderContext& rc)
{
	grassRenderer_.Draw(rc);
}


bool DebugGrassScene::RequestScene(uint32_t& id)
{
	return false;
}
