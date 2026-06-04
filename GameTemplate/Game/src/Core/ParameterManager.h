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
 * プレイヤースキルのステータス
 * PlayerSkillStatus.json から読み込む
 */
struct MasterPlayerSkillParameter : public IMasterParameter
{
	appParameter(MasterPlayerSkillParameter);

	std::string category;			//!< スキルカテゴリ (例: "NormalAttack", "SpecialAttack", "Utility")
	std::string key;				//!< スキル識別キー  (例: "Bite", "Bomb", "Dodge")
	float       motionValues;		//!< 攻撃力
	float       cooldown;			//!< クールダウン (秒)
	float		decreaseStamina;	//!< スタミナ消費量
	float		vibrationTime;	//!< バイブレーション時間
	float		vibrationForce; //!< バイブレーション力
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
	float       cooldown;		//!< クールダウン (秒)
	float		vibrationTime;	//!< バイブレーション時間
	float		vibrationForce; //!< バイブレーション力
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

	float walkSeInterval;     //!< 歩きSEの再生間隔 (秒)
	float walkLoopTime;       //!< ループシーケンスの基準時間 (秒)
	float walkEffectScale;    //!< 歩きエフェクトスケール (一様)
	float runSeInterval;      //!< 走りSEの再生間隔 (秒)
	float deathAnimationTime; //!< 死亡アニメーション終了までの時間 (秒)
	float moveThreshold;      //!< 移動入力の最小閾値
};

/**
 * NPCコントローラーパラメーター
 * NPCControllerParameter.json の "NPCController" 配列から読み込む
 */
struct MasterNPCControllerParameter : public IMasterParameter
{
	appParameter(MasterNPCControllerParameter);

	int   attackLotteryMax; //!< 攻撃行動抽選の合計確率 (各フェーズの重み合計と一致させること)
	float shortDistance;    //!< 近距離判定の距離閾値
	float midDistance;      //!< 中距離判定の距離閾値
	float longDistance;     //!< 遠距離判定の距離閾値
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
	float collisionForward;          //!< コリジョンの前方オフセット
	float collisionHeight;           //!< コリジョンの高さオフセット

	// Avoid (回避)
	float targetPosForward;          //!< 回避の目標位置 (前方距離)
	float avoidMoveSpeed;            //!< 回避の移動速度
	float avoidEffectRotation;       //!< 回避エフェクトの回転角度 (度)
	float avoidEffectScale;          //!< 回避エフェクトスケール (一様)
	float avoidStartTime;            //!< 回避処理の開始タイミング (秒)
	float avoidEndTime;              //!< 回避処理の終了タイミング (秒)

	// FireMagic (炎魔法)
	float effectScaleFactor;         //!< エフェクトスケール倍率
	float fireMagicCollisionSize;    //!< 炎魔法コリジョンの縦幅・横幅
	float fireMagicCollisionDepth;   //!< 炎魔法コリジョンの奥行き
	float fireMagicCollisionForward; //!< 炎魔法コリジョンの前方オフセット
	float fireMagicCollisionHeight;  //!< 炎魔法コリジョンの高さオフセット
	float fireMagicAttackStartTime;  //!< 炎魔法の攻撃判定開始時間 (秒)
	float fireMagicAttackEndTime;    //!< 炎魔法の攻撃判定終了時間 (秒)
};

/**
 * 攻撃オブジェクトパラメーター
 * AttackObjectParameter.json の "AttackObject" 配列から読み込む
 */
struct MasterAttackObjectParameter : public IMasterParameter
{
	appParameter(MasterAttackObjectParameter);

	float landmineModelScale;          //!< 地雷モデルスケール (一様)
	float landmineCollisionSize;       //!< 地雷コリジョンサイズ (爆発半径)
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
	float       goalHpRate;             //!< 目標HP残存率 (0.0〜1.0)
};

/** defineの使用終了 */
#undef appParameter


/**
 * パラメーター管理クラス
 */
class ParameterManager
{
private:
	/* 複数パラメーターがあっても良いように */
	using ParameterVector = std::vector<IMasterParameter*>;
	/* 各パラメーターごとに保持する */
	using ParameterMap = std::map<uint32_t, ParameterVector>;

private:
	ParameterMap parameterMap_;  //!< パラメーターを保持

private:
	/* コンストラクタ */
	ParameterManager();
	/* デストラクタ */
	~ParameterManager();

public:
	void LoadCharacterStatusData(const char* path);
	void LoadPlayerSkillStatusData(const char* path);
	void LoadBossSkillStatusData(const char* path);
	void LoadBattleCommonParamData(const char* path);
	void LoadGrassBendParamData(const char* path);
	void LoadMissionParamData(const char* path);
	void LoadCollisionHitParamData(const char* path);
	void LoadPlayerStateParamData(const char* path);
	void LoadNPCControllerParamData(const char* path);
	void LoadBossSpawnerParamData(const char* path);
	void LoadSkillParamData(const char* path);
	void LoadAttackObjectParamData(const char* path);
	void LoadStageManagerParamData(const char* path);

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
			T* parameter = new T();
			func(j, *parameter);
			parameters.push_back(static_cast<IMasterParameter*>(parameter));
		}

		parameterMap_.emplace(T::ID(), parameters);
	}


	/*
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

			if (!array.is_array()) { continue; }

			for (const auto& j : array)
			{
				// 空オブジェクト {} はスキップ
				if (j.empty()) { continue; }

				T* parameter = new T();
				func(category, j, *parameter);
				dest.push_back(static_cast<IMasterParameter*>(parameter));
			}
		}
	}



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
	const MasterCharacterStatusParameter* GetCharacterStatus(const std::string& key)
	{
		return FindParameter<MasterCharacterStatusParameter>(
			[&key](const MasterCharacterStatusParameter& p) {
				return p.key == key;
			}
		);
	}


	/** カテゴリ＋keyでプレイヤースキルを取得するショートカット */
	const MasterPlayerSkillParameter* GetPlayerSkill(const std::string& category, const std::string& key)
	{
		return FindParameter<MasterPlayerSkillParameter>(
			[&](const MasterPlayerSkillParameter& p) {
				return p.category == category && p.key == key;
			}
		);
	}

	/** カテゴリ(ボス名)＋keyでボススキルを取得するショートカット */
	const MasterBossSkillParameter* GetBossSkill(const std::string& category, const std::string& key)
	{
		return FindParameter<MasterBossSkillParameter>(
			[&](const MasterBossSkillParameter& p) {
				return p.category == category && p.key == key;
			}
		);
	}

	/** キーでキャラクターのステータスを取得するショートカット */
	const MasterBattleCommonParameter* GetBattleCommonParam(const std::string& key)
	{
		return FindParameter<MasterBattleCommonParameter>(
			[&key](const MasterBattleCommonParameter& p) {
				return p.key == key;
			}
		);
	}

	/** keyで草曲げパラメーターを取得するショートカット */
	const MasterGrassBendParameter* GetGrassBendParam(const std::string& key)
	{
		return FindParameter<MasterGrassBendParameter>(
			[&key](const MasterGrassBendParameter& p) {
				return p.key == key;
			}
		);
	}

	/** keyでミッションパラメーターを取得するショートカット */
	const MasterMissionParameter* GetMissionParam(const std::string& key)
	{
		return FindParameter<MasterMissionParameter>(
			[&key](const MasterMissionParameter& p) {
				return p.key == key;
			}
		);
	}

	/** コリジョンヒットパラメーターを取得するショートカット */
	const MasterCollisionHitParameter* GetCollisionHitParam()
	{
		return GetParameter<MasterCollisionHitParameter>(0);
	}

	/** プレイヤーステートパラメーターを取得するショートカット */
	const MasterPlayerStateParameter* GetPlayerStateParam()
	{
		return GetParameter<MasterPlayerStateParameter>(0);
	}

	/** NPCコントローラーパラメーターを取得するショートカット */
	const MasterNPCControllerParameter* GetNPCControllerParam()
	{
		return GetParameter<MasterNPCControllerParameter>(0);
	}

	/** ボススポーナーパラメーターを取得するショートカット */
	const MasterBossSpawnerParameter* GetBossSpawnerParam()
	{
		return GetParameter<MasterBossSpawnerParameter>(0);
	}

	/** keyでスキルパラメーターを取得するショートカット */
	const MasterSkillParameter* GetSkillParam(const std::string& key)
	{
		return FindParameter<MasterSkillParameter>(
			[&key](const MasterSkillParameter& p) {
				return p.key == key;
			}
		);
	}

	/** 攻撃オブジェクトパラメーターを取得するショートカット */
	const MasterAttackObjectParameter* GetAttackObjectParam()
	{
		return GetParameter<MasterAttackObjectParameter>(0);
	}

	/** ステージマネージャーパラメーターを取得するショートカット */
	const MasterStageManagerParameter* GetStageManagerParam()
	{
		return GetParameter<MasterStageManagerParameter>(0);
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
