#pragma once
#include "Character.h"


class State;
class Player : public Character
{
public:
	/** 遷移ルールのセットアップ */
	void SetUpTranslateRulu();

	/** 
	 * アニメーションの再生。
	 * id : ステートのIDによってアニメーションのインデックスを決める
	 */
	void PlayAnimation(StateID id);


public:
	/* コンストラクタ */
	Player();
	/* デストラクタ */
	~Player() {}

	/* スタート処理 */
	virtual bool Start() override;
	/* 更新処理 */
	virtual void Update() override;
	/* 描画処理 */
	virtual void Render(RenderContext& rc) override;
};