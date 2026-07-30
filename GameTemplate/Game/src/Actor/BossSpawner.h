#pragma once
class ActorStatus;
class Character;
class Player;
class PlayerController;
class BossCharacter;
class NPCController;
class BossSpawner
{
private:
	ActorStatus* status_ = nullptr;					//!< ステータス
	BossCharacter* boss_ = nullptr;					//!< ボス
	Player* attackTarget_ = nullptr;				//!< NPCコントローラーへ渡すため
	PlayerController* playerController_ = nullptr;	//!< プレイヤーコントローラー
	NPCController* bossController_ = nullptr;		//!< ボスコントローラー
	
private:
	/** ボスを作る際に必要なデータを作成する */
	BossParam CreateBossData(const std::string& key);

public:
	/** 更新のグループを取得 */
	inline const uint32_t GetUpdateGroup() const { return UpdateGroup::Boss; }

public:
	/** ボスの生成 */
	void SpawnBoss(bool isPlayerControl = false);

	/** ボスの攻撃対象を設定するための中間管理 */
	inline void SetAttackTarger(Player* target) { attackTarget_ = target; }

	/**
	 * ボス本体の取得
	 * BossEmotionPhaseManager や BattleManager が HP 監視のためにボス本体を参照するために使う
	 */
	inline BossCharacter* GetBoss() const { return boss_; }

	/** 更新の可否状態を設定 */
	void SetUpdate(const bool flg);

	/** ボスコントローラーと移動制御を有効/無効にする */
	void SetControlEnabled(const bool flg);


public:
	/** コンストラクタ */
	BossSpawner();
	/** デストラクタ */
	~BossSpawner();

	/** 更新 */
	void Update();
};