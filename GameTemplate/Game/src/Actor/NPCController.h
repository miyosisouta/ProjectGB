#pragma once
#include "src/Actor/BehaviorTree.h"

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
	BossStateID attackID;			//!< 攻撃パターン
	int weight;						//!< 選ばれる重み
	bool allowRepeat = true;		//!< 直前と同じ攻撃が連続で選ばれることを許すか

	// レーザー専用：モード(単発/連発/チャージ)ごとの基礎重み。attackID != Laser のときは未使用(0)
	float laserModeWeightNormal = 0.0f;
	float laserModeWeightMult   = 0.0f;
	float laserModeWeightCharge = 0.0f;
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

	BTNodePtr staggerNode_; //!< 怯み割り込み用ツリー。Update()から毎フレーム独立にTickし、通常行動より必ず優先させる
	BTNodePtr tree_;        //!< 通常行動決定用のビヘイビアツリー（距離帯ごとの重み付き抽選）

	BossStateID lastTopLevelAction_ = BossStateID::Idle; //!< 直前に選ばれた攻撃（距離帯をまたいでボス全体で1つ）。連続回避の判定に使う

private:
	/** 距離チェックを行う。距離によって検索するルールを決める */
	DistancePhase ChackDistancePhase(float distance) const;

	/** 現在のプレイヤーとの距離帯を取得する（ビヘイビアツリーのConditionクロージャから毎回呼び直す） */
	DistancePhase GetCurrentDistancePhase() const;

	/** NPCAttackRuleParameter.jsonから、自分のボス(bossKey_)に対応する攻撃選択ルールを組み立てる */
	void BuildRulesFromParam();

	/** 怯み割り込み用のツリーを組み立てる（Condition: 怯み要求あり → Action: Hitステートへ） */
	BTNodePtr BuildStaggerNode();

	/** currentRules_を元に通常行動決定用のビヘイビアツリーを組み立てる */
	BTNodePtr BuildNormalBehaviorTree();

	/** 指定した攻撃候補リストから重み付き抽選ノードを組み立てる */
	BTNodePtr BuildWeightedAttackNode(const std::vector<AttackPattern>& patterns);

	/** 指定したBossStateIDへ遷移するActionノードを作る（開始時にChangeState、IsCurrentStateFinished()で終了判定） */
	BTNodePtr MakeStateAction(BossStateID id);

	/**
	 * 攻撃候補選択用のActionノードを作る（MakeStateActionに加えて、選ばれた瞬間にlastTopLevelAction_を更新する。
	 * Laserが選ばれた場合はモード抽選用の重みもボスへ渡す）
	 */
	BTNodePtr MakeAttackChoiceAction(const AttackPattern& pattern);


public:
	/** 選ばれているボスを設定 */
	inline void SetBossKey(const std::string& key) { bossKey_ = key; }

	/** 攻撃対象の設定 */
	inline void SetAttackTarget(Player* target) { targerPlayer_ = target; }

	/** 操作するボスを設定 */
	inline void SetBossCharacter(BossCharacter* boss) { boss_ = boss; }

	/** 操作しているボスを取得（ビヘイビアツリー構築時のクロージャから使う） */
	inline BossCharacter* GetBoss() const { return boss_; }

	/** 攻撃対象のプレイヤーを取得（ビヘイビアツリー構築時のクロージャから使う） */
	inline Player* GetPlayer() const { return targerPlayer_; }



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