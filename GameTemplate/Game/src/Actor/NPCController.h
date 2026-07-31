#pragma once

/** ボスの攻撃ID */
enum class BossStateID
{
	Idle,			//!< 待機
	Run,			//!< 走る
	HitStamp,		//!< ヒットスタンプ攻撃
	Attack,			//!< 通常攻撃
	Hit,			//!< ダメージリアクション
	Spin,			//!< 回転攻撃
	ThrowRock,		//!< 岩投げ攻撃
	Laser,			//!< レーザー攻撃
	Death			//!< 死亡
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
	std::string bossKey_; //!< 自分がどのボスか（CharacterMaster.json等のkeyと一致）
	std::array<DistanceRule, static_cast<int>(DistancePhase::enMax)> currentRules_; //!< 現在のボス種類に対応する距離別攻撃ルール

	BossCharacter* boss_ = nullptr; //!< 操作するボス
	Player* targerPlayer_ = nullptr; //!< 攻撃対象になるプレイヤー

private:
	/** 行動ノードを取得 */
	void SelectActionNode();

	/** 距離チェックを行う。距離によって検索するルールを決める */
	DistancePhase ChackDistancePhase(float distance);

	/** NPCAttackRuleParameter.jsonから、自分のボス(bossKey_)に対応する攻撃選択ルールを組み立てる */
	void BuildRulesFromParam();


public:
	/** 選ばれているボスを設定 */
	inline void SetBossKey(const std::string& key) { bossKey_ = key; }

	/** 攻撃対象の設定 */
	inline void SetAttackTarget(Player* target) { targerPlayer_ = target; }

	/** 操作するボスを設定 */
	inline void SetBossCharacter(BossCharacter* boss) { boss_ = boss; }



public:
	/** コンストラクタ */
	NPCController();
	/** デストラクタ */
	~NPCController();

	/** スタート処理 */
	bool Start() override;
	/** 更新処理 */
	void Update() override;
	/** 描画処理 */
	void Render(RenderContext& rc)override {} /* 描画するものがないので必要なし */
};