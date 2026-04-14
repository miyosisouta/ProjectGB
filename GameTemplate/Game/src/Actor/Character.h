#pragma once
#include "Actor.h"
#include "StateMachine.h"
#include "src/Skill/NormalAttack/NormalAtatck.h"
#include "src/Skill/SpecialAbility/AbilityBase.h"
#include "src/Skill/Utility/Utility.h"

namespace CharacterID
{
	/* キャラクターの体のID */
	inline uint32_t CharacterID() { return Hash32("Character"); }
	inline uint32_t PlayerID() { return Hash32("Player"); }
	inline uint32_t BossID() { return Hash32("Boss"); }

	/* プレイヤーの攻撃用ID */
	inline uint32_t PlayerNormalAtkID() { return Hash32("PlayerNormalAttack"); }
	inline uint32_t PlayerSkillAtkID() { return Hash32("PlayerSkillAttack"); }
	inline uint32_t PlayerSkillAvoidID() { return Hash32("PlayerSkillAvoid"); }
	
	/* ボスの攻撃用ID */
	inline uint32_t BossNormalAtkID() { return Hash32("BossNormalAttack"); }
	inline uint32_t BossHitStampAtkID() { return Hash32("BossHitStamp"); }
	inline uint32_t BossSpinAtkID() { return Hash32("BossSpin"); }
	inline uint32_t BossThrowRockAtkID() { return Hash32("BossThrowRock"); }

	/** キャラクター全体に影響を与える */
	inline uint32_t CharaLandmineAtkID() { return Hash32("CharaLandmine"); }
}


class Character : public Actor
{
protected:
	AllocatedArray<AnimationClip> animationClipList_; //!< アニメーションクリップのリスト
	std::unique_ptr<GhostBody> damageBody_; //!< ダメージ用コリジョン
	StateMachine stateMachine_;
	CharacterController charaCon_;

	/* スキルの実態を持つ */
	std::unique_ptr<NormalAttackBase> activeNormalAttack_;
	std::unique_ptr<AbilityBase>      activeAbility_;
	std::unique_ptr<UtilityBase>      activeUtility_;

	Vector3 moveVelocity_ = Vector3::Zero; //!< 毎フレームの移動速度を保持する変数

	bool isUpdate_ = true; //!< 更新するかどうかのフラグ

public:
	/* 通常攻撃枠のスキルを取得 */
	NormalAttackBase* GetNormalAttackSkill() const { return activeNormalAttack_.get(); }
	/* 特殊能力枠のスキルを取得 */
	AbilityBase* GetAbilitySkill() const { return activeAbility_.get(); }
	/* 汎用能力枠のスキルを取得 */
	UtilityBase* GetUtilitySkill() const { return activeUtility_.get(); }

	/* ステートマシーンの取得 */
	StateMachine* GetStateMachine() { return  &stateMachine_; }
	/* キャラクターコントローラの取得 */
	CharacterController* GetCharaCon() { return &charaCon_; }

	/* 移動速度の設定 */
	inline void SetMoveVelocity(const Vector3& velocity) { moveVelocity_ = velocity; }
	/* 移動速度を取得 */
	inline Vector3 GetMoveVelocity() { return moveVelocity_; }

	/** 更新の可否状態を設定 */
	inline void SetUpdate(const bool flg) { isUpdate_ = flg; }

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