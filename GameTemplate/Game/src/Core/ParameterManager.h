/**
 * ParameterManager.h
 * パラメーター管理
 * ステータスなどの数値を外部ファイルから読み込んで使用する
 */
#pragma once
#include <iostream>
#include <fstream>

 /**
  * NOTE: すべてのパラメーターに付ける
  */
#define appParameter(name)\
public:\
 static constexpr uint32_t ID() { return Hash32(#name); }


  /** 基底クラス。必ず継承すること！ */
struct IMasterParameter {};

/**
 * キャラクターのステータス
 * CharacterStatusData.json の "Character" 配列から読み込む
 * "key" : フィールドでキャラクターを識別する
 */
struct MasterCharacterStatusParameter : public IMasterParameter
{
	appParameter(MasterCharacterStatusParameter);

	struct Stamina 
	{
		float maxStamina;				//!< スタミナの最大値
		float drainPerSec;				//!< 1秒間に消費するスタミナ量
		float recoverPerSec;			//!< 通常状態に回復する1秒間のスタミナ量
		float exhaustedRecoverPerSec;	//!< 枯渇状態に回復する1秒間のスタミナ量
		float exhaustedThreshold;		//!< 枯渇解除に必要なスタミナ（最大値に対する割合）
		float exhaustedSpeedRate;		//!< 枯渇中の移動速度倍率
	};

	std::string key;						//!< キャラクター識別キー (例: "Player", "Gorilla", "Turtle")
	Vector3		position;					//!< 座標
	Quaternion	rotation;					//!< 回転
	Vector3		scale;						//!< 大きさ
	Vector3		collisionPosUp;				//!< コリジョンの座標を上にあげる量
	float		collisionSizeRadius;		//!< コリジョンの半径
	float		collisionSizeHeight;		//!< コリジョンの高さ
	float		charaConSizeRadius;			//!< キャラコンの半径
	float		charaConSizeHeight;			//!< キャラコンの高さ
	int         hp;							//!< HP
	int         attack;						//!< 攻撃力
	int         criticalRate;				//!< クリティカル率 (%)
	float       criticalDamageMultiplier;	//!< クリティカルダメージ倍率
	Stamina     stamina;					//!< スタミナ
	float		moveSpeedBase;				//!< 通常のベース速度
	float		runSpeedBase;				//!< 走るときのベース速度
};

/**
 * キャラクターマスター（敵の見た目データの置き場）
 * CharacterMaster.json の "CharacterMaster" 配列から読み込む
 * "key" フィールドでキャラクターを識別する (例: "Gorilla", "Turtle")
 */
struct MasterCharacterMasterParameter : public IMasterParameter
{
	appParameter(MasterCharacterMasterParameter);

	std::string key;               //!< キャラクター識別キー (例: "Gorilla", "Turtle")
	std::string category;          //!< 敵の区分 ("Boss" または "Normal")
	std::string modelPath;         //!< モデルファイル(.tkm)のパス
	std::string animationBasePath; //!< アニメーションファイル(.tka)が入っているフォルダのパス（AnimationMasterの"fileName"と連結して使う）
	std::string modelAxis;         //!< モデルの上方向 ("Y" または "Z")
};

/**
 * アニメーションマスター（アニメーションクリップの登録）
 * AnimationMaster.json から読み込む（トップレベルのキーがキャラクター識別キーになる＝category）
 */
struct MasterAnimationMasterParameter : public IMasterParameter
{
	appParameter(MasterAnimationMasterParameter);

	std::string category;      //!< キャラクター識別キー (AnimationMaster.jsonのトップレベルキー。例: "Gorilla")
	std::string key;           //!< アニメーション種類キー (例: "Idle", "Run")
	std::string fileName;      //!< アニメーションファイル名(.tka)
	bool        isLoop = false;//!< ループ再生するか
};

/**
 * プレイヤースキルのステータス
 * PlayerSkillStatus.json から読み込む
 */
struct MasterPlayerSkillParameter : public IMasterParameter
{
	appParameter(MasterPlayerSkillParameter);

	std::string category;			//!< スキルカテゴリ (例: "NormalAttack", "SpecialAttack", "Utility")
	std::string key;				//!< スキル識別キー  (例: "Bite", "Bomb", "Dodge")
	float       motionValues;		//!< 攻撃力
	float       cooldown;			//!< クールダウン
	float		decreaseStamina;	//!< スタミナ消費量
	float		vibrationTime;		//!< バイブレーション時間
	float		vibrationForce;		//!< バイブレーション力
};

/**
 * ボススキルのステータス
 * BossSkillStatus.json から読み込む
 */
struct MasterBossSkillParameter : public IMasterParameter
{
	appParameter(MasterBossSkillParameter);

	std::string category;		//!< ボス名カテゴリ
	std::string key;			//!< スキル識別キー
	float       motionValues;   //!< 攻撃力
	float       cooldown;		//!< クールダウン
	float		vibrationTime;	//!< バイブレーション時間
	float		vibrationForce; //!< バイブレーション力
	std::string correspondingState; //!< 対応するC++のステートクラス名（ドキュメント用途）
	std::string stateId;            //!< 対応するBossStateID名（ドキュメント用途）
};

struct MasterBattleCommonParameter : public IMasterParameter
{
	appParameter(MasterBattleCommonParameter);


	struct CameraParam
	{
		float nearClip = 0.0f;		//!< 近平面
		float farClip = 0.0f;		//!< 遠平面
		float fovy = 0.0f;			//!< 視野角
		float height = 0.0f;		//!< 高さ
		float distance = 0.0f;		//!< 距離
		float rotSpeed = 0.0f;		//!< 回転速度
		bool invert = false;		//!< 反転させるか否か
		float sensitivity = 0.0f;	//!< 感度
		float lookAtOffsetY = 0.0f;	//!< 注視点のY軸オフセット
	};
	
	struct GameTimerParam
	{
		float limitTime = 0.0f;		//!< 制限時間
		float warningTime = 0.0f;	//!< 警告時間
	};

	struct CutSceneParam 
	{
		float firstCutTime = 0.0f;	//!< 最初のカットの処理を始める時間
		float secondCutTime = 0.0f;	//!< 2番目のカットの処理を始める時間
		float thirdCutTime = 0.0f;	//!< 3番目のカットの処理を始める時間
		float endCutTime = 0.0f;	//!< 最後のカットの処理を始める時間
		Vector3 firstCutCameraPos = Vector3::Zero;	//!< 最初のカットのカメラの座標
		Vector3 secondCutCameraPos = Vector3::Zero;	//!< 2番目のカットのカメラの座標
		Vector3 thirdCutCameraPos = Vector3::Zero;	//!< 3番目のカットのカメラの座標
		Vector3 cutSceneTargetPos = Vector3::Zero;	//!< 注視点
	};

	std::string category;			//!< ボス名カテゴリ
	std::string key;				//!< スキル識別キー
	CameraParam cameraParam;		//!< カメラのパラメータ
	GameTimerParam gameTimeParam;	//!< ゲームタイマーのパラメータ
	CutSceneParam cutSceneParam;	//!< カットシーンのパラメータ
	float skyCubeScale = 0.0f;		//!< スカイキューブの大きさ

};

/**
 * 草曲げパラメーター
 * grass_attack_params.json の "attacks" 配列から読み込む
 */
struct MasterGrassBendParameter : public IMasterParameter
{
	appParameter(MasterGrassBendParameter);

	std::string key;          //!< 攻撃種別キー (例: "NormalAttack", "ThrowRock")
	float       force;        //!< 最大曲げ量 (ワールド単位)
	float       radius;       //!< 影響半径 (ワールド単位)
	float       duration;     //!< 曲げ持続時間 (秒)
	float       recoverySpeed;//!< 将来の回復速度拡張用
};

/**
 * プレイヤーステートパラメーター
 * PlayerStateParameter.json の "PlayerState" 配列から読み込む
 */
struct MasterPlayerStateParameter : public IMasterParameter
{
	appParameter(MasterPlayerStateParameter);

	float walkSeInterval;     //!< 歩きSEの再生間隔
	float walkLoopTime;       //!< ループシーケンスの基準時間
	float walkEffectScale;    //!< 歩きエフェクトスケール
	float runSeInterval;      //!< 走りSEの再生間隔
	float deathAnimationTime; //!< 死亡アニメーション終了までの時間
	float moveThreshold;      //!< 移動入力の最小閾値
};

/**
 * NPCコントローラーパラメーター
 * NPCControllerParameter.json の "NPCController" 配列から読み込む
 */
struct MasterNPCControllerParameter : public IMasterParameter
{
	appParameter(MasterNPCControllerParameter);

	float shortDistance;    //!< 近距離判定の距離閾値
	float midDistance;      //!< 中距離判定の距離閾値
	float longDistance;     //!< 遠距離判定の距離閾値
};

/**
 * NPCの攻撃選択ルール（ボスごとの距離帯別・攻撃抽選の重みテーブル）
 * NPCAttackRuleParameter.json から読み込む（トップレベルのキーがキャラクター識別キーになる＝category）
 */
struct MasterNPCAttackRuleParameter : public IMasterParameter
{
	appParameter(MasterNPCAttackRuleParameter);

	std::string category; //!< キャラクター識別キー (NPCAttackRuleParameter.jsonのトップレベルキー。例: "Gorilla")
	std::string distance;  //!< 距離帯 ("Short"/"Mid"/"Long"/"OutRange")
	std::string stateId;   //!< 選ばれた時に遷移するBossStateID名 (例: "Attack", "Spin"...)
	int         weight = 0;//!< 抽選の重み
	bool        allowRepeat = true; //!< 直前と同じ攻撃が連続で選ばれることを許すか

	// レーザー専用：モード(単発/連発/チャージ)ごとの基礎重み。stateId != "Laser" の行では未使用
	float laserModeWeightNormal = 0.0f;
	float laserModeWeightMult   = 0.0f;
	float laserModeWeightCharge = 0.0f;
};

/**
 * ボススポーナーパラメーター
 * BossSpawnerParameter.json の "BossSpawner" 配列から読み込む
 */
struct MasterBossSpawnerParameter : public IMasterParameter
{
	appParameter(MasterBossSpawnerParameter);

	float modeAttackMultiplier; //!< 高攻撃モードの攻撃力倍率
	float modeHpMultiplier;     //!< タイムアタックモードのHP倍率
};

/**
 * スキルパラメーター
 * SkillParameter.json の "Skill" 配列から読み込む
 * "key" フィールドでスキルを識別する (例: "Bite", "Avoid", "FireMagic")
 */
struct MasterSkillParameter : public IMasterParameter
{
	appParameter(MasterSkillParameter);

	std::string key; //!< スキル識別キー

	// Bite (通常攻撃)
	float collisionRadius;           //!< コリジョンの半径
	float collisionForward;          //!< コリジョンの前方オフセット
	float collisionHeight;           //!< コリジョンの高さオフセット

	// Avoid (回避)
	float targetPosForward;          //!< 回避の目標位置 (前方距離)
	float avoidMoveSpeed;            //!< 回避の移動速度
	float avoidEffectRotation;       //!< 回避エフェクトの回転角度
	float avoidEffectScale;          //!< 回避エフェクトスケール 
	float avoidStartTime;            //!< 回避処理の開始タイミング
	float avoidInvincibleStartTime;  //!< 無敵フラグONのタイミング
	float avoidJustStartTime;        //!< ジャスト回避ウィンドウ開始
	float avoidJustEndTime;          //!< ジャスト回避ウィンドウ終了
	float avoidEndTime;              //!< 回避処理の終了タイミング

	// FireMagic (炎魔法)
	float effectScaleFactor;         //!< エフェクトスケール倍率
	float fireMagicCollisionSize;    //!< 炎魔法コリジョンの縦幅・横幅
	float fireMagicCollisionDepth;   //!< 炎魔法コリジョンの奥行き
	float fireMagicCollisionForward; //!< 炎魔法コリジョンの前方オフセット
	float fireMagicCollisionHeight;  //!< 炎魔法コリジョンの高さオフセット
	int   fireMagicHitCount;         //!< 炎魔法の多段攻撃回数
	float fireMagicHitInterval;      //!< 炎魔法の攻撃間隔
	float fireMagicAttackStartTime;  //!< 炎魔法の攻撃判定開始時間
	float fireMagicAttackEndTime;    //!< 炎魔法の攻撃判定終了時間
};

/**
 * 攻撃オブジェクトパラメーター
 * AttackObjectParameter.json の "AttackObject" 配列から読み込む
 */
struct MasterAttackObjectParameter : public IMasterParameter
{
	appParameter(MasterAttackObjectParameter);

	float landmineModelScale;          //!< 地雷モデルスケール
	float landmineCollisionSize;       //!< 地雷コリジョンサイズ
	float landmineIndicatorRadius;     //!< 地雷インジケーター半径
	float effectScaleFactorDamageLing; //!< ダメージエフェクトスケール倍率
	float effectScaleFactorExplode;    //!< 爆発エフェクトスケール倍率
};

/**
 * ステージマネージャーパラメーター
 * StageManagerParameter.json の "StageManager" 配列から読み込む
 */
struct MasterStageManagerParameter : public IMasterParameter
{
	appParameter(MasterStageManagerParameter);

	float ditheringAlphaGround;   //!< 地面のディザリングα値
	float ditheringAlphaFence;    //!< 柵のディザリングα値
	float ditheringAlphaTree;     //!< 木のディザリングα値
	float splatMapGrassLuminance; //!< 草の明るさ倍率
	float splatMapClayLuminance;  //!< 土の明るさ倍率
	float splatMapHumusLuminance; //!< 腐葉土の明るさ倍率
	float splatMapWholeLuminance; //!< 全体の明るさ倍率
	float splatMapSaturation;     //!< 彩度
	float collisionUp;            //!< コリジョンの高さオフセット
	float meterToCentimeter;      //!< メートル→センチメートル変換係数
};

/**
 * コリジョンヒットパラメーター
 * CollisionHitParameter.json の "CollisionHit" 配列から読み込む
 */
struct MasterCollisionHitParameter : public IMasterParameter
{
	appParameter(MasterCollisionHitParameter);

	float spinKnockBack;                 //!< スピン攻撃のノックバック力
	float effectYCap;                    //!< プレイヤーヒットエフェクトのY座標上限
	float playerNormalAttackEffectScale; //!< プレイヤー通常攻撃ヒットエフェクトスケール
	float playerSkillAttackEffectScale;  //!< プレイヤースキル攻撃ヒットエフェクトスケール
	float bossNormalAttackEffectScale;   //!< ボス通常攻撃ヒットエフェクトスケール
	float bossHitStampEffectScale;       //!< ボスヒットスタンプヒットエフェクトスケール
	float bossSpinEffectScale;           //!< ボス回転攻撃ヒットエフェクトスケール
	float bossThrowRockEffectScale;      //!< ボス岩投げヒットエフェクトスケール
	float bossLaserWeakEffectScale;      //!< ボス弱レーザーヒットエフェクトスケール
	float bossLaserStrongEffectScale;    //!< ボス強レーザーヒットエフェクトスケール
};

/**
 * チュートリアルパラメーター
 * TutorialParameter.json の "Tutorial" 配列から読み込む
 */
struct MasterTutorialParameter : public IMasterParameter
{
	appParameter(MasterTutorialParameter);

	int   requiredNormalAttackCount;   //!< 通常攻撃段階：次に進むために必要な通常攻撃回数
	float moveTimeoutDuration;         //!< 移動段階：ダッシュしなくても強制的にクリア扱いにするまでの秒数
	float transitionDuration;          //!< 「アニメーション」表示から次の段階名表示に切り替わるまでの秒数
	float bossIdleDurationInTeaching;  //!< かいひ/ジャスト段階中、ボスがIdleで待機する秒数
	float bossIdleDurationInFree;      //!< 自由段階中、ボスがIdleで待機する秒数
};

/**
 * ミッションパラメーター
 * MissionParameter.json の "Mission" 配列から読み込む
 * "key" フィールドでボスを識別する (例: "Gorilla", "Turtle")
 */
struct MasterMissionParameter : public IMasterParameter
{
	appParameter(MasterMissionParameter);

	std::string key;                    //!< ボス識別キー (例: "Gorilla", "Turtle")
	float       targetTime;             //!< 目標タイム (秒)
	uint8_t     abilityTargetCount;     //!< 特殊スキル目標使用回数
	uint8_t     utilityTargetCount;     //!< 汎用スキル目標使用回数
	uint8_t     normalAttackTargetCount;//!< 通常攻撃目標使用回数
	float       goalHpRate;             //!< 目標HP残存率
};

/**
 * タイトル背景パラメーター
 * TitleBackgroundParameter.json の "TitleBackground" 配列から読み込む
 */
struct MasterTitleBGParameter : public IMasterParameter
{
	appParameter(MasterTitleBGParameter);

	float treeSpeed          = 0.0f;	 //!< 木のスクロール速度
	float grassSpeed         = 0.0f;	 //!< 草のスクロール速度
	float fenceSpeed         = 0.0f;	 //!< 柵のスクロール速度
	float treeSpacing        = 0.0f;	 //!< 木の基本間隔
	float treeZ              = 0.0f;	 //!< 手前側の木のZ座標
	float treeZ2             = 0.0f;	 //!< 奥側の木のZ座標
	float grassSpacing       = 0.0f;	 //!< 草の基本間隔
	float grassZ             = 0.0f;	 //!< 草のZ座標
	float fenceSpacing       = 0.0f;	 //!< 柵の基本間隔
	float fenceZ             = 0.0f;	 //!< 柵のZ座標
	float groundY            = 0.0f;	 //!< 地面のY座標
	float groundScale        = 0.0f;	 //!< 地面のスケール
	float cullingMargin      = 0.0f;	 //!< 画面外判定のマージン
	float spawnX             = 0.0f;	 //!< 木・草・柵の出現X座標
	float treeSpawnX         = 0.0f;	 //!< 木の出現X座標
	float playerX            = 0.0f;	 //!< プレイヤーのX座標
	float playerY            = 0.0f;	 //!< プレイヤーのY座標
	float playerZ            = 0.0f;	 //!< プレイヤーのZ座標
	float playerRotYDeg      = 0.0f;	 //!< プレイヤーのY軸回転角度
	float camPosX            = 0.0f;	 //!< カメラ座標X
	float camPosY            = 0.0f;	 //!< カメラ座標Y
	float camPosZ            = 0.0f;	 //!< カメラ座標Z
	float camTargetX         = 0.0f;	 //!< カメラ注視点X
	float camTargetY         = 0.0f;	 //!< カメラ注視点Y
	float camTargetZ         = 0.0f;	 //!< カメラ注視点Z
	float camFovDeg          = 0.0f;	 //!< カメラ視野角(度)
	float camNear            = 0.0f;	 //!< ニアクリップ
	float camFar             = 0.0f;	 //!< ファークリップ
	float skyCubeScale       = 0.0f;	 //!< スカイキューブのスケール
	float playerRunSpeed     = 0.0f;	 //!< プレイヤーの走行アニメーション速度
	int   treeMaxConsecutive  = 0;		 //!< 木の最大連続出現数
	float treeMinGap          = 0.0f;	 //!< 木の最小間隔
	float treeBaseGap         = 0.0f;	 //!< 木の基本間隔
	float treeMaxGap          = 0.0f;	 //!< 木の最大間隔倍率
	int   grassMaxConsecutive = 0;		 //!< 草の最大連続出現数
	float grassMinGap         = 0.0f;	 //!< 草の最小間隔
	float grassBaseGap        = 0.0f;	 //!< 草の基本間隔
	float grassMaxGap         = 0.0f;	 //!< 草の最大間隔倍率
	int   fenceMaxConsecutive = 0;		 //!< 柵の最大連続出現数
	float fenceMinGap         = 0.0f;	 //!< 柵の最小間隔
	float fenceBaseGap        = 0.0f;	 //!< 柵の基本間隔
	float fenceMaxGap         = 0.0f;	 //!< 柵の最大間隔倍率
};

/**
 * 感情システムパラメーター
 * EmotionParameter.json の "Emotion" 配列から読み込む
 * 各段階の倍率テーブルとエフェクト位置オフセットを保持する
 */
struct MasterEmotionParameter : public IMasterParameter
{
	appParameter(MasterEmotionParameter);

	// 各段階の攻撃力・攻撃速度・被ダメージ倍率
	// インデックス 0=Debuff3, 1=Debuff2, 2=Debuff1, 3=Normal, 4=Buff1, 5=Buff2, 6=Buff3
	struct ModifierEntry
	{
		float attackMul          = 0.0f;  //!< 攻撃力倍率
		float attackSpeedMul     = 0.0f;  //!< 攻撃速度倍率（taskSchedulerの秒数・攻撃間隔に反映。演出速度とは独立）
		float damageTakenMul     = 0.0f;  //!< 被ダメージ倍率
		float animationSpeedMul  = 0.0f;  //!< アニメーション再生速度倍率（攻撃速度とは独立に調整できる演出用の値）
		float effectSpeedMul     = 0.0f;  //!< エフェクト再生速度倍率（攻撃速度とは独立に調整できる演出用の値）
	};

	ModifierEntry modifiers[7];					//!< 7段階分の倍率テーブル
	float         buffEffectScale   = 0.0f;		//!< Buffエフェクトのスケール（一様）
	float         debuffEffectScale = 0.0f;		//!< Debuffエフェクトのスケール（一様）
	float         buffEffectOffsetY   = 0.0f;	//!< Buffエフェクトのボス位置からのYオフセット（XZはボス座標のまま）
	float         debuffEffectOffsetY = 0.0f;	//!< Debuffエフェクトのボス位置からのYオフセット（XZはボス座標のまま）
	float         buffSeVolumeBoost   = 0.0f;	//!< BuffSEの追加音量倍率（通常のマスター×SE計算値に掛ける。素材音源の収録音量差を補うためのもの）
	float         debuffSeVolumeBoost = 0.0f;   //!< DebuffSEの追加音量倍率（同上）
};

/**
 * ボスの行動パラメーター（BossState.cpp の各ステートが参照する調整値）
 * BossStateParameter.json の "BossState" 配列から読み込む（ボス種別によらず共通の1セット）
 * Idle/Run/Death は「攻撃」ではないため攻撃速度の影響を受けない。
 */
struct MasterBossStateParameter : public IMasterParameter
{
	appParameter(MasterBossStateParameter);

	// 共通（複数ステートで使う値、または他システム(NPCController)と足並みを揃えるための値）
	struct Common
	{
		float shortDistance         = 0.0f;  //!< 近距離判定 (NPCControllerと同じ値。BossRunStateの目標距離計算で使用)
		float midDistance           = 0.0f;  //!< 中距離判定
		float longDistance          = 0.0f;  //!< 遠距離判定
		float rotateSpeed           = 0.0f;  //!< ボスがターゲット方向へ旋回する速さ（各攻撃の予備動作で共通使用）
		float damageRingEffectScale = 0.0f;  //!< 円形攻撃エフェクトのスケール係数（通常攻撃・回転攻撃・レーザーの着弾エフェクトで共通使用）
	} common;

	// 待機 (BossIdleState)
	struct Idle
	{
		float endTime = 0.0f; //!< 待機を終えるまでの秒数
	} idle;

	// 走る (BossRunState)
	struct Run
	{
		float   moveSpeed          = 0.0f; //!< 走行時のベース移動速度
		float   seLoopInterval     = 0.0f;	 //!< 足音SEを鳴らすループ間隔(秒)
		float   effectLoopInterval = 0.0f;	 //!< 疾走エフェクトを再生するループ間隔(秒)
		Vector3 effectScale        = Vector3::Zero;	//!< 疾走エフェクトのスケール
	} run;

	// 通常攻撃 (BossAttackState) ※攻撃速度で秒数が変化する
	struct NormalAttack
	{
		float beginTime          = 0.0f;   //!< 攻撃判定・アニメーションが発生するまでの秒数
		float collisionResetTime = 0.0f;   //!< 攻撃判定を消すまでの秒数
		float endTime            = 0.0f;   //!< ステートを終えるまでの秒数
		float collisionForward   = 0.0f;   //!< 判定をボス前方へずらすオフセット量
		float collisionHeight    = 0.0f;   //!< 判定の高さオフセット量
		float collisionSize      = 0.0f;   //!< 判定球の半径
	} normalAttack;

	// ヒットスタンプ (HitStampState) ※攻撃速度で秒数が変化する
	struct HitStamp
	{
		float   upBeginTime         = 0.0f;    //!< 飛び上がり開始までの秒数
		float   overheadMoveTime    = 0.0f;    //!< プレイヤー頭上へ移動を終えるまでの秒数
		float   fallBeginTime       = 0.0f;    //!< 落下を開始するまでの秒数
		float   rangeSize           = 0.0f;    //!< 攻撃範囲インジケーター(着地地点の予告円)の半径
		Vector3 jumpHeight          = Vector3::Zero; //!< [movement] 飛び上がる高さ（現在地からの相対値）
		float   verticalVelocity    = 0.0f;    //!< 上昇時に設定する垂直速度
		float   upSpeed             = 0.0f;    //!< 上昇中の水平移動速度
		float   downSpeed           = 0.0f;    //!< 落下中の水平移動速度
		float   gravityPower        = 0.0f;    //!< 落下中にかかる重力の強さ
		float   effectScaleBasis   = 0.0f;     //!< 着地エフェクトのスケール算出に使う基準サイズ（実際の攻撃判定半径はBossState.cpp内の固定値を使用しており本値の対象外）
		float   smokeEffectScale    = 0.0f;    //!< 着地時の煙エフェクトのスケール係数（effectScaleBasisに掛ける）
		float   shockWaveEffectScale = 0.0f;   //!< 着地時の衝撃波エフェクトのスケール係数（effectScaleBasisに掛ける）
		float   impactAnimLeadTime  = 0.0f;    //!< 着地インパクトアニメーション(enAnimJumpImpact)を、落下速度から概算した残り落下時間がこの秒数を下回った時点（＝着地直前）で再生する
		float   stateExitDelay      = 0.0f;    //!< 着地してからステートを終えるまでの秒数（インパクトアニメーションを最後まで見せるための猶予）
	} hitStamp;

	// 回転攻撃 (SpinState) ※攻撃速度で秒数が変化する
	struct Spin
	{
		float attackStartTime    = 0.0f;   //!< 予測表示を終え突進を開始するまでの秒数
		float attackEndTime      = 0.0f;   //!< 強制終了までの秒数
		float seLoopInterval     = 0.0f;   //!< 回転SEを鳴らすループ間隔(秒)
		float effectLoopInterval = 0.0f;   //!< 回転エフェクトのループ間隔(秒)（現状コード内では未使用。将来の追従エフェクト用に予約）
		float moveSpeed          = 0.0f;   //!< 突進時の移動速度
		float overMoveDistance   = 0.0f;   //!< 目標地点からさらに進む距離
		float effectScaleBasis   = 0.0f;   //!< 突進エフェクトのスケール算出に使う基準サイズ（実際の攻撃判定半径はBossState.cpp内の固定値を使用しており本値の対象外）
		float indicatorRangeSize = 0.0f;   //!< 攻撃予測ラインの太さ
		float indicatorLength    = 0.0f;   //!< 攻撃予測ラインの長さ（見た目のみ、攻撃距離とは無関係）
		float indicatorForward   = 0.0f;   //!< 攻撃予測ライン中心の前方オフセット
		float jumpLeadTime       = 0.0f;   //!< 突進開始(attackStartTime)の何秒前からジャンプ演出を始めるか
		float jumpHeight         = 0.0f;   //!< ジャンプ演出で上昇するY座標の高さ
	} spin;

	// 岩を投げる攻撃 (ThrowRockState) ※攻撃速度で秒数が変化する
	struct ThrowRock
	{
		float beginTime         = 0.0f;   //!< 岩を投げるまでの秒数
		float endTime           = 0.0f;   //!< ステートを終えるまでの秒数
		float overMoveDistance  = 0.0f;   //!< プレイヤーのいる位置からさらに奥に投げ込む距離
		float indicatorLength   = 0.0f;   //!< 攻撃予測ラインの長さ（見た目のみ、攻撃距離とは無関係）
		float indicatorBaseSize = 0.0f;   //!< 攻撃予測ラインのタイリング基準サイズ（長さ軸）
		float indicatorForward  = 0.0f;   //!< 攻撃予測ライン中心の前方オフセット
		float indicatorRangeSize = 0.0f;  //!< 攻撃予測ラインの太さ
		float rockCollisionSize = 0.0f;   //!< 投げた岩の攻撃判定サイズ
		float clickedBlendTime  = 0.0f;   //!< ワインドアップ→投擲アニメーションへ滑らかに繋げるためのブレンド秒数
		float windUpSwayAmplitudeDeg   = 0.0f;  //!< ワインドアップ中に左右へ揺さぶる角度(度)
		float windUpSwaySegmentDuration = 0.0f; //!< ワインドアップの揺さぶりが片側(中央→端)へ動くのにかける秒数
		float windUpSpawnHeight = 0.0f;		//!< ワインドアップ開始時に岩が出現するY座標（投げる瞬間までに本来の高さへ近づく）
		float dustEffectScale = 0.0f;			//!< 岩を準備している間に足元(Y=0)で再生する土ぼこりエフェクトのスケール（一様）
	} throwRock;

	// レーザー攻撃 (LaserState) ※攻撃速度で秒数が変化する
	struct Laser
	{
		float   initialShotTime       = 0.0f;  //!< 最初の予測タイマーが発火するまでの秒数
		float   attackTime            = 0.0f;  //!< 発射開始から判定・エフェクトを消すまでの秒数
		float   shotIntervalNormal    = 0.0f;  //!< 予測→発射までの秒数（通常/連発モード）
		float   shotIntervalCharge    = 0.0f;  //!< 予測→発射までの秒数（チャージモード）
		uint8_t shotCountNormal       = 0;     //!< 発射本数（通常/チャージモード）
		uint8_t shotCountMult         = 0;     //!< 発射本数（連発モード）
		Vector3 chargeScale           = Vector3::Zero; //!< チャージモード時のエフェクト・判定スケール倍率
		float   collisionScale        = 0.0f;  //!< 発射時の攻撃判定スケール基準値
		float   indicatorRadiusNormal = 0.0f;   //!< 予測サークルの半径（通常/連発モード）
		float   indicatorRadiusCharge = 0.0f;   //!< 予測サークルの半径（チャージモード）
		float   effectScaleFactor     = 0.0f;   //!< レーザーエフェクトのサイズ調整係数（現状コード内では未使用）
		float   multiJumpHeight         = 0.0f;   //!< 連発攻撃前の予備動作で上下するY座標の高さ
		float   multiJumpDuration       = 0.0f;   //!< 予備ジャンプ1回（上昇→着地）にかける秒数
		uint8_t multiJumpCount          = 0;      //!< 予備ジャンプを繰り返す回数
		Vector3 chargeBodyScale         = Vector3::Zero; //!< チャージ攻撃前の予備動作で拡大するボス本体のScale（絶対値。通常時のScaleはCharacterStatusData.json側で管理）
		float   chargeIndicatorDelay    = 0.0f;   //!< チャージ攻撃：振り向き終了後、予測線が表示されるまでの「溜め」の秒数（他モードのinitialShotTimeに相当する値をチャージ専用に長く取ったもの）
		float   chargeScaleDownDuration = 0.0f;   //!< チャージ攻撃後、Scaleを通常Scaleへ戻すのにかける秒数
		float   multFavorMultiplierAfterNormal = 1.0f; //!< 直前が単発モードだった時、連発モードの重みに掛ける倍率（技の繋がり演出用。モード自体の基礎重みはNPCAttackRuleParameter.json側）
	} laser;

	// 死亡 (BossDeathState)
	struct Death
	{
		float animationTime = 0.0f; //!< [timing] 死亡アニメーション再生後、実際に死亡扱いにするまでの秒数
	} death;

	// ダメージリアクション・怯み (BossStaggerState) ※攻撃速度の影響は受けない
	struct Stagger
	{
		float duration = 0.0f; //!< 怯み状態が続く秒数
	} stagger;
};

/**
 * ボスHP閾値カットシーンのパラメーター（ボスの種類ごとに設定）
 * BossPhaseCutSceneParameter.json の "BossPhaseCutScene" 配列から読み込む
 * "key" フィールドでボスを識別する (例: "Gorilla", "Turtle")
 */
struct MasterBossPhaseCutSceneParameter : public IMasterParameter
{
	appParameter(MasterBossPhaseCutSceneParameter);

	std::string key;					//!< ボス識別キー (例: "Gorilla", "Turtle")
	float angryHpRatio = 0.0f;			//!< 「怒り状態」カットシーンに入るHP割合の閾値
	float tiredHpRatio = 0.0f;			//!< 「疲れ状態」カットシーンに入るHP割合の閾値
	float cameraDistance = 0.0f;		//!< ボス正面からのカメラ距離
	float cameraHeight = 0.0f;			//!< 注視点・カメラ位置の高さオフセット
	float cameraEaseDuration = 0.0f;	//!< カメラが通常視点⇔ボス正面をイージングで移動する秒数（往復とも同じ秒数）
	float cutsceneDuration = 0.0f;		//!< カメラをボスの正面に合わせてから、通常視点に戻すまでの秒数
	float bossIdleHoldDuration = 0.0f;	//!< 視点が戻ってからボスAIを再開するまでの待機秒数（戻った瞬間に攻撃が当たって見えるのを防ぐ）
	float iconOffsetY = 0.0f;			//!< 怒り/疲れマーク(angry1/2, tired1/2/3)のY座標に加えるオフセット。ボスごとに立ち位置・高さが違うための調整値
	float tiredIconOffsetX = 0.0f;		//!< 疲れマーク(tired1/2/3)を外側(X方向)にずらす量。tired1/3は+X、tired2は-X側へ適用する

	// HP25%演出：ボスの前後傾き（X軸回転）
	float pitchForwardDeg         = 0.0f;	//!< 前方向へ傾ける角度
	float pitchForwardEaseDuration = 0.0f;	//!< 0度→pitchForwardDegへイージングする秒数
	float pitchHoldDuration        = 0.0f;	//!< 前傾きのまま静止する秒数（左右揺れ・疲れマークの演出もこの間に行う）
	float pitchBackEaseDuration    = 0.0f;	//!< pitchForwardDeg→0度へイージングで戻す秒数

	// HP25%演出：ボスの左右揺れ（Y軸回転）。静止時間(pitchHoldDuration)を4等分し、
	// 0→-A→+A→-A→0度と往復させる（Aがこの振れ幅）
	float yawWobbleAmplitudeDeg = 0.0f;	//!< 左右揺れの振れ幅（度）

	// HP25%演出：疲れマーク(tired1/2/3)の弧移動
	float tiredArcRiseDuration     = 0.0f;	//!< 弧の上昇にかける秒数
	float tiredArcDriftDuration    = 0.0f;	//!< 弧の下降（横移動）にかける秒数
	float tiredStaggerInterval     = 0.0f;	//!< tired1→2→3を開始する間隔（秒）

	// カットシーン専用のバフ/デバフエフェクト位置オフセット（通常のインゲーム中のオフセットはEmotionParameter.json側）
	float debuffEffectOffsetYOverride = 0.0f;	//!< HP25%演出中のDebuffエフェクトのYオフセット上書き値
	float buffEffectOffsetYOverride   = 0.0f;	//!< HP50%演出中のBuffエフェクトのYオフセット上書き値

	// HP50%演出：ボスのジャンプ演出・怒りマークのスケールパルス
	int   bossJumpCount          = 0;		//!< ジャンプ→着地を繰り返す回数
	float iconScaleDownDuration   = 0.0f;	//!< 怒りマークが縮むのにかける秒数
	float iconScaleUpDuration     = 0.0f;	//!< 怒りマークが元の大きさに戻るのにかける秒数
	float icon2PulseDelay         = 0.0f;	//!< 2枚目の怒りマークのパルスを1枚目からずらす秒数
	float angry1BaseScale         = 0.0f;	//!< 怒りマーク1の基準スケール（BossPhaseCutSceneUI.jsonのscaleと合わせること。パルス停止時に戻す値）
	float angry2BaseScale         = 0.0f;	//!< 怒りマーク2の基準スケール（BossPhaseCutSceneUI.jsonのscaleと合わせること。パルス停止時に戻す値）
};

/** defineの使用終了 */
#undef appParameter


/**
 * パラメーター管理クラス
 */
class ParameterManager
{
private:
	/** 複数パラメーターがあっても良いように */
	using ParameterVector = std::vector<IMasterParameter*>;
	/** 各パラメーターごとに保持する */
	using ParameterMap = std::map<uint32_t, ParameterVector>;

private:
	ParameterMap parameterMap_;  //!< パラメーターを保持

private:
	/** コンストラクタ */
	ParameterManager();
	/** デストラクタ */
	~ParameterManager();

public:
	/* 各種パラメーターJSONの読み込み（呼び出し元は main.cpp のロード処理） */
	void LoadCharacterMasterData(const char* path);
	void LoadAnimationMasterData(const char* path);
	void LoadCharacterStatusData(const char* path);
	void LoadPlayerSkillStatusData(const char* path);
	void LoadBossSkillStatusData(const char* path);
	void LoadBattleCommonParamData(const char* path);
	void LoadGrassBendParamData(const char* path);
	void LoadMissionParamData(const char* path);
	void LoadCollisionHitParamData(const char* path);
	void LoadPlayerStateParamData(const char* path);
	void LoadNPCControllerParamData(const char* path);
	void LoadNPCAttackRuleData(const char* path);
	void LoadBossSpawnerParamData(const char* path);
	void LoadSkillParamData(const char* path);
	void LoadAttackObjectParamData(const char* path);
	void LoadStageManagerParamData(const char* path);
	void LoadTitleBGParamData(const char* path);
	void LoadEmotionParamData(const char* path);
	void LoadBossStateParamData(const char* path);
	void LoadBossPhaseCutSceneParamData(const char* path);
	void LoadTutorialParamData(const char* path);

public:
	/**
	 * パラメーター読み込み
	 * NOTE: Unloadも呼ぶことを忘れないように
	 * 第2引数のラムダ式でテンプレートで指定する型の情報に変換する
	 */
	template <typename T>
	void LoadParameter(const char* path, const std::function<void(const nlohmann::json& json, T& p)>& func)
	{
		std::ifstream file(path);
		if (!file.is_open()) {
			return;
		}

		nlohmann::json jsonRoot;
		file >> jsonRoot;

		std::vector<IMasterParameter*> parameters;
		for (const auto& j : jsonRoot) {
			T* parameter = new T();
			func(j, *parameter);
			parameters.push_back(static_cast<IMasterParameter*>(parameter));
		}

		parameterMap_.emplace(T::ID(), parameters);
	}

	/**
	 * ネストされた配列からパラメーター読み込み
	 * NOTE: Unloadも呼ぶことを忘れないように
	 * <param name="path">JSONファイルパス</param>
	 * <param name="arrayKey">配列のキー名 (例: "Character")</param>
	 * <param name="func">各要素を型Tに変換するラムダ式</param>
	 */
	template <typename T>
	void LoadParameterFromArray(
		const char* path,
		const char* arrayKey,
		const std::function<void(const nlohmann::json& json, T& p)>& func)
	{
		std::ifstream file(path);
		if (!file.is_open()) {
			return;
		}

		nlohmann::json jsonRoot;
		file >> jsonRoot;

		// 指定したキーの配列が存在するか確認
		if (!jsonRoot.contains(arrayKey) || !jsonRoot[arrayKey].is_array()) {
			return;
		}

		std::vector<IMasterParameter*> parameters;
		for (const auto& j : jsonRoot[arrayKey]) {
			// オブジェクトでない要素（文字列など）は読み込み対象外
			if (!j.is_object()) { continue; }

			// "key"が"__EXAMPLE__"の行はコピペ用テンプレートなので読み込まない
			if (j.value("key", "") == "__EXAMPLE__") { continue; }

			T* parameter = new T();
			func(j, *parameter);
			parameters.push_back(static_cast<IMasterParameter*>(parameter));
		}

		parameterMap_.emplace(T::ID(), parameters);
	}


	/**
	 * 2段階ネストされた配列からパラメーター読み込み
	 */
	template <typename T>
	void LoadParameterFromNestedArray(const char* path, const std::function<void(const std::string& category, const nlohmann::json& json, T& p)>& func)
	{
		std::ifstream file(path);
		if (!file.is_open()) {
			return;
		}

		nlohmann::json jsonRoot;
		file >> jsonRoot;

		// 既存エントリがあれば追記（同じ型を複数ファイルから読む場合に備える）
		auto& dest = parameterMap_[T::ID()];

		// トップレベルのキーをカテゴリとして走査
		for (auto it = jsonRoot.begin(); it != jsonRoot.end(); ++it)
		{
			const std::string& category = it.key();
			const auto& array = it.value();

			// "_README"のような説明用キーはカテゴリではないので読み飛ばす
			if (category == "_README") { continue; }

			if (!array.is_array()) { continue; }

			for (const auto& j : array)
			{
				// オブジェクトでない要素（文字列など）は読み込み対象外
				if (!j.is_object()) { continue; }

				// 空オブジェクト {} はスキップ
				if (j.empty()) { continue; }

				// "key"が"__EXAMPLE__"の行はコピペ用テンプレートなので読み込まない
				if (j.value("key", "") == "__EXAMPLE__") { continue; }

				T* parameter = new T();
				func(category, j, *parameter);
				dest.push_back(static_cast<IMasterParameter*>(parameter));
			}
		}
	}



	/** 読み込んだパラメーターを解放する */
	template <typename T>
	void UnloadParameter()
	{
		auto it = parameterMap_.find(T::ID());
		if (it != parameterMap_.end()) {
			auto& parameters = it->second;
			for (auto* p : parameters) {
				delete p;
			}
			parameterMap_.erase(it);
		}
	}


	/** 1つだけパラメーターを取得する */
	template <typename T>
	const T* GetParameter(const int index = 0) const
	{
		const auto parameters = GetParameters<T>();
		if (parameters.size() == 0) { return nullptr; }
		if (parameters.size() <= index) { return nullptr; }
		return parameters[index];
	}


	/** 複数パラメーターを取得する */
	template <typename T>
	inline const std::vector<T*> GetParameters() const
	{
		std::vector<T*> parameters;
		auto it = parameterMap_.find(T::ID());
		if (it != parameterMap_.end()) {
			for (auto* parameter : it->second) {
				parameters.push_back(static_cast<T*>(parameter));
			}
		}
		return parameters;
	}


	/** パラメーターをラムダ式で回すForEach */
	template <typename T>
	void ForEach(std::function<void(const T&)> func) const
	{
		const std::vector<T*> parameters = GetParameters<T>();
		for (const T* paramter : parameters) {
			func(*paramter);
		}
	}

	/** パラメーターをラムダ式で探すFind */
	template <typename T>
	const T* FindParameter(std::function<bool(const T&)> func)
	{
		const std::vector<T*> parameters = GetParameters<T>();
		for (const T* paramter : parameters) {
			if (func(*paramter)) {
				return paramter;
			}
		}
		return nullptr;
	}

	/** キーでキャラクターのステータスを取得するショートカット */
	inline const MasterCharacterStatusParameter* GetCharacterStatus(const std::string& key)
	{
		return FindParameter<MasterCharacterStatusParameter>(
			[&key](const MasterCharacterStatusParameter& p) {
				return p.key == key;
			}
		);
	}


	/** カテゴリ＋keyでプレイヤースキルを取得するショートカット */
	inline const MasterPlayerSkillParameter* GetPlayerSkill(const std::string& category, const std::string& key)
	{
		return FindParameter<MasterPlayerSkillParameter>(
			[&](const MasterPlayerSkillParameter& p) {
				return p.category == category && p.key == key;
			}
		);
	}

	/** カテゴリ(ボス名)＋keyでボススキルを取得するショートカット */
	inline const MasterBossSkillParameter* GetBossSkill(const std::string& category, const std::string& key)
	{
		return FindParameter<MasterBossSkillParameter>(
			[&](const MasterBossSkillParameter& p) {
				return p.category == category && p.key == key;
			}
		);
	}

	/** キーでキャラクターマスター（敵の見た目データ）を取得するショートカット */
	inline const MasterCharacterMasterParameter* GetCharacterMaster(const std::string& key)
	{
		return FindParameter<MasterCharacterMasterParameter>(
			[&key](const MasterCharacterMasterParameter& p) {
				return p.key == key;
			}
		);
	}

	/** カテゴリ(キャラクター)＋keyでアニメーションマスターを取得するショートカット */
	inline const MasterAnimationMasterParameter* GetAnimationMaster(const std::string& category, const std::string& key)
	{
		return FindParameter<MasterAnimationMasterParameter>(
			[&](const MasterAnimationMasterParameter& p) {
				return p.category == category && p.key == key;
			}
		);
	}

	/** カテゴリ(キャラクター)で絞り込んで全アニメーションマスターを取得するショートカット */
	std::vector<const MasterAnimationMasterParameter*> GetAnimationsByCharacter(const std::string& category)
	{
		std::vector<const MasterAnimationMasterParameter*> result;
		ForEach<MasterAnimationMasterParameter>(
			[&](const MasterAnimationMasterParameter& p) {
				if (p.category == category) { result.push_back(&p); }
			}
		);
		return result;
	}

	/** カテゴリ(キャラクター)で絞り込んで全NPC攻撃選択ルールを取得するショートカット */
	std::vector<const MasterNPCAttackRuleParameter*> GetNPCAttackRulesByCharacter(const std::string& category)
	{
		std::vector<const MasterNPCAttackRuleParameter*> result;
		ForEach<MasterNPCAttackRuleParameter>(
			[&](const MasterNPCAttackRuleParameter& p) {
				if (p.category == category) { result.push_back(&p); }
			}
		);
		return result;
	}

	/** キーでキャラクターのステータスを取得するショートカット */
	inline const MasterBattleCommonParameter* GetBattleCommonParam(const std::string& key)
	{
		return FindParameter<MasterBattleCommonParameter>(
			[&key](const MasterBattleCommonParameter& p) {
				return p.key == key;
			}
		);
	}

	/** keyで草曲げパラメーターを取得するショートカット */
	inline const MasterGrassBendParameter* GetGrassBendParam(const std::string& key)
	{
		return FindParameter<MasterGrassBendParameter>(
			[&key](const MasterGrassBendParameter& p) {
				return p.key == key;
			}
		);
	}

	/** keyでミッションパラメーターを取得するショートカット */
	inline const MasterMissionParameter* GetMissionParam(const std::string& key)
	{
		return FindParameter<MasterMissionParameter>(
			[&key](const MasterMissionParameter& p) {
				return p.key == key;
			}
		);
	}

	/** コリジョンヒットパラメーターを取得するショートカット */
	inline const MasterCollisionHitParameter* GetCollisionHitParam()
	{
		return GetParameter<MasterCollisionHitParameter>(0);
	}

	/** プレイヤーステートパラメーターを取得するショートカット */
	inline const MasterPlayerStateParameter* GetPlayerStateParam()
	{
		return GetParameter<MasterPlayerStateParameter>(0);
	}

	/** NPCコントローラーパラメーターを取得するショートカット */
	inline const MasterNPCControllerParameter* GetNPCControllerParam()
	{
		return GetParameter<MasterNPCControllerParameter>(0);
	}

	/** ボススポーナーパラメーターを取得するショートカット */
	inline const MasterBossSpawnerParameter* GetBossSpawnerParam()
	{
		return GetParameter<MasterBossSpawnerParameter>(0);
	}

	/** チュートリアルパラメーターを取得するショートカット */
	inline const MasterTutorialParameter* GetTutorialParam()
	{
		return GetParameter<MasterTutorialParameter>(0);
	}

	/** keyでスキルパラメーターを取得するショートカット */
	inline const MasterSkillParameter* GetSkillParam(const std::string& key)
	{
		return FindParameter<MasterSkillParameter>(
			[&key](const MasterSkillParameter& p) {
				return p.key == key;
			}
		);
	}

	/** 攻撃オブジェクトパラメーターを取得するショートカット */
	inline const MasterAttackObjectParameter* GetAttackObjectParam()
	{
		return GetParameter<MasterAttackObjectParameter>(0);
	}

	/** ステージマネージャーパラメーターを取得するショートカット */
	inline const MasterStageManagerParameter* GetStageManagerParam()
	{
		return GetParameter<MasterStageManagerParameter>(0);
	}

	/** タイトル背景パラメーターを取得するショートカット */
	inline const MasterTitleBGParameter* GetTitleBGParam()
	{
		return GetParameter<MasterTitleBGParameter>(0);
	}

	/** 感情システムパラメーターを取得するショートカット */
	inline const MasterEmotionParameter* GetEmotionParam() const
	{
		return GetParameter<MasterEmotionParameter>(0);
	}

	/** ボスの行動パラメーターを取得するショートカット */
	inline const MasterBossStateParameter* GetBossStateParam() const
	{
		return GetParameter<MasterBossStateParameter>(0);
	}

	/** キー(ボス名)でボスHP閾値カットシーンパラメーターを取得するショートカット */
	inline const MasterBossPhaseCutSceneParameter* GetBossPhaseCutSceneParam(const std::string& key)
	{
		return FindParameter<MasterBossPhaseCutSceneParameter>(
			[&key](const MasterBossPhaseCutSceneParameter& p) {
				return p.key == key;
			}
		);
	}



	/** カテゴリ名で絞り込んで全スキルを取得するショートカット */
	std::vector<const MasterPlayerSkillParameter*> GetPlayerSkillsByCategory(const std::string& category)
	{
		std::vector<const MasterPlayerSkillParameter*> result;
		ForEach<MasterPlayerSkillParameter>(
			[&](const MasterPlayerSkillParameter& p) {
				if (p.category == category) { result.push_back(&p); }
			}
		);
		return result;
	}

	/** ボス名で絞り込んで全スキルを取得するショートカット */
	std::vector<const MasterBossSkillParameter*> GetBossSkillsByCategory(const std::string& category)
	{
		std::vector<const MasterBossSkillParameter*> result;
		ForEach<MasterBossSkillParameter>(
			[&](const MasterBossSkillParameter& p) {
				if (p.category == category) { result.push_back(&p); }
			}
		);
		return result;
	}


	


private:
	static ParameterManager* instance_; //!< インスタンス


public:
	/** インスタンスを作る */
	static void CreateInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new ParameterManager();
		}
	}

	/** インスタンスを取得 */
	static ParameterManager& Get()
	{
		return *instance_;
	}

	/** インスタンスを破棄 */
	static void DestroyInstance()
	{
		if (instance_ != nullptr)
		{
			delete instance_;
			instance_ = nullptr;
		}
	}
};
