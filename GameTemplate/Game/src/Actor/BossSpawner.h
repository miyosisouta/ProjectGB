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
	Player* attackTarget_ = nullptr;					//!< NPCコントローラーへ渡すため
	PlayerController* playerController_ = nullptr;	//!< プレイヤーコントローラー
	NPCController* bossController_ = nullptr;		//!< ボスコントローラー
	
private:
	/* ボスを作る際に必要なデータを作成する */
	BossParam CreateBossData(BossType type, GameModeType mode);


public:
	/* 
	 * コントローラーの設定
	 * isPlayerControl = 実装確認のためtrueの場合、プレイヤーで動かせるようにする
	 */
	void SpawnBoss(bool isPlayerControl = false);

	/** ボスの攻撃対象を設定するための中間管理 */
	void SetAttackTarger(Player* target) { attackTarget_ = target; }


public:
	/* コンストラクタ */
	BossSpawner();
	/* デストラクタ */
	~BossSpawner();

	/* 更新 */
	void Update(); 

};