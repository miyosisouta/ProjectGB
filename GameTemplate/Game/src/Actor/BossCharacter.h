#pragma once
#include "src/Actor/Character.h"


class BossStatus;
/* ボスキャラクターの基底クラス */
class BossCharacter : public Character
{
protected:
	BossStatus* status_ = nullptr;
	BossParam param_;

protected:
	/*
	 * アニメーションの再生
	 */
	void PlayAnimation(const int id);

public:
	BossCharacter();
	virtual ~BossCharacter();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc)override;

	/* ステータス設定 */
	inline void SetupStatus(BossStatus* status) { status_ = status; }
	inline void SetupParam(BossParam& param) { param_ = param; }
};