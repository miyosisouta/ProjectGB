#pragma once
#include "Character.h"
#include "src/CharacterDataBase.h"

class State;
class NormalAttackBase;
class AbilityBase;
class UtilityBase;
class Player : public Character
{
	/*======================================*/
	/* スキル関連 */
	/*======================================*/
private:
	/** 通常攻撃スキルを装備 */
	void EquipNormalAttack(NormalAttackType type);
	/** 特殊能力スキルを装備 */
	void EquipAbility(AbilityType type);
	/** 汎用スキルを装備 */
	void EquipUtility(UtilityType type);

	/** 特殊能力が使えるか */
	bool CanSpecialAbility();

public:
	/** 遷移ルールのセットアップ */
	void SetUpTranslateRulu();
	/** スキルの設定、作成 */
	void CreateSkill(NormalAttackType nAttackType, AbilityType abilityType, UtilityType utilityType)override;

	/**
	 * アニメーションの再生。
	 * id : ステートのIDによってアニメーションのインデックスを決める
	 */
	void PlayAnimation(int id)override;

	/** 回避を使えるか */
	bool IsExhausted() const;

public:
	/** コンストラクタ */
	Player();
	/** デストラクタ */
	~Player();

	/** スタート処理 */
	virtual bool Start() override;
	/** 更新処理 */
	virtual void Update() override;
	/** 描画処理 */
	virtual void Render(RenderContext& rc) override;
};