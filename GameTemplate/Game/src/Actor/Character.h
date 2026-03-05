#pragma once
#include "Actor.h"


class Character : public Actor
{
protected:
	AllocatedArray<AnimationClip> m_animationClipList; //!< アニメーションクリップのリスト


public:
	/* コンストラクタ */
	Character() {}
	/* デストラクタ */
	~Character() {}

	/* スタート処理 */
	virtual bool Start() override;
	/* 更新処理 */
	virtual void Update() override;
	/* 描画処理 */
	virtual void Render(RenderContext& rc) override;
};