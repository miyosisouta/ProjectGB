/**
 * DebugScene.h
 *
 * デバックの描画
 */

#pragma once
#include "IScene.h"
#include "src/Stage/GrassBendManager.h"


class StageManagerObject;
class DebugGrassScene : public IScene
{
	Scene(DebugGrassScene);


private:
	/** 草1本分のトランスフォーム */
	struct GrassTransform
	{
		Vector3    position = Vector3::Zero;		//!< 座標
		Quaternion rotation = Quaternion::Identity; //!< 回転
		Vector3    scale    = Vector3::One;			//!< 拡縮
	};

	StageManagerObject*            stage_ = nullptr; //!< ステージ
	std::unique_ptr<TaskSchedulerSystem> task_;		 //!< タイマー付きタスクの管理
	ModelRender                    grassRenderer_;	 //!< 草のインスタンシングレンダラー
	std::vector<GrassTransform>    grassTransforms_; //!< 配置した草のトランスフォームリスト

private:
	/** 草をランダムに作成・配置 */
	void GenarateGrass();
	/** jsonに出力 */
	void ExportJson();

public:
	/** コンストラクタ */
	DebugGrassScene();
	/** デストラクタ */
	~DebugGrassScene();


public:
	/** スタート処理 */
	bool Start() override;
	/** 更新処理 */
	void Update() override;
	/** 描画処理 */
	void Render(RenderContext& rc) override;
	/** シーン遷移要求 */
	bool RequestScene(uint32_t& id) override;
};
