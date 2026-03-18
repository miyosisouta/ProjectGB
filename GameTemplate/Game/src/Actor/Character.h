#pragma once
#include "Actor.h"
#include "StateMachine.h"
#include "src/Skill/NormalAttack/NormalAtatck.h"
#include "src/Skill/SpecialAbility/AbilityBase.h"
#include "src/Skill/Utility/Utility.h"

class Character : public Actor
{
protected:
	AllocatedArray<AnimationClip> animationClipList_; //!< アニメーションクリップのリスト
	StateMachine stateMachine_;

	/* スキルの実態を持つ */
	std::unique_ptr<NormalAttackBase> activeNormalAttack_;
	std::unique_ptr<AbilityBase>      activeAbility_;
	std::unique_ptr<UtilityBase>      activeUtility_;

	Vector3 moveVelocity_ = Vector3::Zero; //!< 毎フレームの移動速度を保持する変数

public:
	/* 通常攻撃枠のスキルを取得 */
	NormalAttackBase* GetNormalAttackSkill() const { return activeNormalAttack_.get(); }
	/* 特殊能力枠のスキルを取得 */
	AbilityBase* GetAbilitySkill() const { return activeAbility_.get(); }
	/* 汎用能力枠のスキルを取得 */
	UtilityBase* GetUtilitySkill() const { return activeUtility_.get(); }

	/* ステートマシーンの取得 */
	StateMachine* GetStateMachine() { return  &stateMachine_; }

	/* 移動速度の設定 */
	inline void SetMoveVelocity(const Vector3& velocity) { moveVelocity_ = velocity; }
	/* 移動速度を取得 */
	inline Vector3 GetMoveVelocity() { return moveVelocity_; }

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
	/**
	 * アニメーションの再生。
	 * id : ステートのIDによってアニメーションのインデックスを決める
	 */
	virtual void PlayAnimation(int id) {}

	/* キャラクターごとのスキル作成 */
	virtual void CreateSkill(NormalAttackType nAttackType, AbilityType abilityType, UtilityType utilityType) {}
};