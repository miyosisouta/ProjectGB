#pragma once

/* ボスの攻撃ID */
enum class BossStateID
{
	Idle,
	Run,
	Jump,
	Attack,
	Hit,
	Death,
	Spin,
	Clicked
};



/** ======================================================================================= */
/** 距離と距離ごとの攻撃リストの設定 */
/** ======================================================================================= */

/** 距離ごとの攻撃パターンのリストを設定するためのもの */
enum class DistancePhase{ enShortAttackType, enMidAttackType, enLongAttackType, enOutRange, enMax };

/** 攻撃ごとの確率用構造体 */
struct AttackPattern
{
	BossStateID attackID;	//!< 攻撃パターン
	int weight;				//!< 選ばれる重み
};

/** 距離ごとの攻撃パターンとその確率 */
struct DistanceRule
{
	DistancePhase distanceType_;			//!< 距離タイプ
	std::vector<AttackPattern> attackList;	//!< 距離ごとの攻撃種類
};



/** ======================================================================================= */
/** AI用クラス */
/** ======================================================================================= */

class Player;
class BossCharacter;
class NPCController : public IGameObject
{
private:
	BossType bossType_ = BossType::enNone; //!< 自分がどのボスか
	std::array<DistanceRule, static_cast<int>(DistancePhase::enMax)> currentRules_;

	BossCharacter* boss_ = nullptr; //!< 操作するボス
	Player* targerPlayer_ = nullptr; //!< 攻撃対象になるプレイヤー

private:
	/* 行動ノードを取得 */
	void SelectActionNode();

	/** 距離チェックを行う。距離によって検索するルールを決める */
	DistancePhase ChackDistancePhase(float distance);


public:
	/* 選ばれているボスを設定 */
	void SetBossType(BossType type) { bossType_ = type; }

	/** 攻撃対象の設定 */
	void SetAttackTarget(Player* target) { targerPlayer_ = target; }

	/** 操作するボスを設定 */
	inline void SetBossCharacter(BossCharacter* boss) { boss_ = boss; }



public:
	NPCController();
	~NPCController();

	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc)override {} /* 描画するものがないので必要なし */
};