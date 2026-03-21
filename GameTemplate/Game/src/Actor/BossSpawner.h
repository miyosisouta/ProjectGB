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
	BossCharacter* boss_ = nullptr;					//!< ボス
	ActorStatus* status_ = nullptr;					//!< ステータス
	PlayerController* playerController_ = nullptr;	//!< プレイヤーコントローラー
	NPCController* bossController_ = nullptr;		//!< ボスコントローラー
	
private:
	BossParam CreateBossData(BossType type, GameModeType mode);


public:
	/* 
	 * コントローラーの設定
	 * 実装確認のためプレイヤーで動かせるようにする
	 */
	void SpawnBoss(Character* chara);


public:
	/* コンストラクタ */
	BossSpawner();
	/* デストラクタ */
	~BossSpawner();

	/* 更新 */
	void Update(); 

};