#pragma once
#include "src/Core/ParameterManager.h"
#include "src/Emotion/EmotionSystem.h"
#include "src/Emotion/IBossStatusModifier.h"
#include <vector>

/**
 * 実体あるもののステータスをまとめたクラス
 * プレイヤーやボス、スキル等のステータスとクールダウンがまとめられている
 */


/** スキル一つ一つのクールダウン */
struct CoolDown
{
    float coolTimer_    = 0.0f;  //!< 残りクールタイム（秒）
    float coolDownTime_ = 0.0f;  //!< クールタイムの設定時間（秒）
    bool  isReadyFrame_ = false; //!< クールダウンが明けた瞬間のフレームだけ true


    /** 毎フレーム呼ぶ。タイマーを減算し、0 を下回ったら isReadyFrame_ を立てる */
    void Update()
    {
        isReadyFrame_ = false;
        if (coolTimer_ > 0.0f)
        {
            coolTimer_ -= g_gameTime->GetFrameDeltaTime();
        }
        if (coolTimer_ < 0.0f)
        {
            isReadyFrame_ = true;
            coolTimer_    = 0.0f;
        }
    }

    /** スキル使用時に呼ぶ。タイマーを設定時間にセットしてカウント開始 */
    inline void Activate(float speedMul = 1.0f) { coolTimer_ = coolDownTime_ / ((speedMul > 0.0f) ? speedMul : 1.0f); }

    /** 残りのクールタイムを取得 */
    inline float GetTimer() const { return coolTimer_; }
    /** 使用可能か */
    inline bool CanUse() const { return coolTimer_ <= 0.0f; }
    /** 明けた瞬間か */
    inline bool IsReadyFrame() const { return isReadyFrame_; }
};


/** スキル1つに必要な情報 */
struct SkillSlot
{
    std::string key_          = "";      //!<スキル識別キー（JSON の "key" と対応）
    float       motionValues_ = 0.0f;    //!< モーション倍率（本体 attack_ にかける係数）
    CoolDown    coolDown_;               //!< このスキル専用のクールダウン
    float       decreaseStamina_ = 0.0f; //!< スタミナ消費量
    float       vibrationTime_ = 0.0f;   //!< バイブレーションの時間
    float       vibrationForce_ = 0.0f;  //!< バイブレーションの強さ

    /** スキルスロットを初期化する */
    void Init(const std::string& key, float motionValues, float coolDownTime, float decreaseStamina = 0.0f, float vibTime = 0.0f, float vibForce = 0.0f)
    {
        key_                    = key;
        motionValues_           = motionValues;
        coolDown_.coolDownTime_ = coolDownTime;
        decreaseStamina_        = decreaseStamina;
        vibrationTime_          = vibTime;
        vibrationForce_         = vibForce;
    }

    /** 毎フレーム呼ぶ */
    inline void Update() { coolDown_.Update(); }

    /** スキル使用時に呼ぶ。クールダウンを開始させる */
    inline void Activate(float speedMul = 1.0f) { coolDown_.Activate(speedMul); }

    /** モーション倍率の取得 */
    inline float GetMotionValues()     const { return motionValues_; }
    /** 現在のクールダウンの取得 */
    inline float GetCoolTimer()        const { return coolDown_.GetTimer(); }
    /** 消費するスタミナの量を取得 */
    inline float GetDecreaseStamina()  const { return decreaseStamina_; }
    /** バイブレーションの時間を取得 */
    inline float GetVibrationTime()  const { return vibrationTime_; }
    /** バイブレーションの強さを取得 */
    inline float GetVibrationForce()  const { return vibrationForce_; }
    /** 使用可能か */
    inline bool  CanUse()              const { return coolDown_.CanUse(); }
    /** 使用可能か(スキルが使用可能になった1フレームのみ取得) */
    inline bool  IsReadyFrame()        const { return coolDown_.IsReadyFrame(); }
};

/** 実体あるものが初期に設定するパラメータの構造体 */
struct ActorInitParam
{
    Vector3 pos_ = Vector3::Zero;                       //!< 座標
    Quaternion rot_ = Quaternion::Identity;             //!< 回転
    Vector3 scal_ = Vector3::Zero;                      //!< 大きさ
    Vector3 collisionHeightUpValue_ = Vector3::Zero;    //!< コリジョンの高さを上げる量
    float collisionRadius_ = 0.0f;                      //!< コリジョンの半径
    float collisionHeight_ = 0.0f;                      //!< コリジョンの高さ
    float charaConRadius_ = 0.0f;                       //!< キャラコンの半径
    float charaConHeight_ = 0.0f;                       //!< キャラコンの高さ
};



/** 実体あるもののステータス */
class ActorStatus
{
protected:
    ActorInitParam initParam_;               //!< 座標などの初期パラメータ
    int   hp_                       = 0;     //!< 現在 HP
    int   maxHp_                    = 0;     //!< 最大 HP
    int   attack_                   = 0;     //!< 攻撃力
    int   critical_                 = 0;     //!< クリティカル率（%）
    float criticalDamageMultiplier_ = 0.0f;  //!< クリティカル時ダメージ倍率
    float moveSpeedBase_            = 0.0f;  //!< ベースの移動速度（ボス用）
    bool  isTakeDamage_             = false; //!< このフレームにダメージを受けたか
    bool  isDead_                   = false; //!< 死亡フラグ


public:
    /** コンストラクタ */
    ActorStatus() {}
    /** デストラクタ */
    virtual ~ActorStatus() {}

    /** 毎フレーム呼ぶ。フレームフラグをリセットする */
    virtual void Update() { isTakeDamage_ = false; }


public:
    /** 実体あるものの初期パラメータを取得 */
    inline ActorInitParam GetInitParam() const { return initParam_; }
    /** 現在のHPを取得 */
    inline int GetHP() const { return hp_; }
    /** 最大HPを取得 */
    inline int GetMaxHP() const { return maxHp_; }
    /** 基礎攻撃力の取得 */
    inline virtual int GetAttack() const { return attack_; }
    /** クリティカル率を取得 */
    inline int GetCritical() const { return critical_; }
    /** クリティカル時のダメージ倍率の取得 */
    inline float GetCriticalDamageMultiplier() const { return criticalDamageMultiplier_; }
    /** 基礎移動速度の取得 */
    inline virtual float GetMoveSpeedBase() const { return moveSpeedBase_; }
    /** 体力が0以下か */
    inline bool IsHpDepleted() const { return hp_ <= 0; } 
    /** 死亡したか */
    inline bool IsDead() const { return isDead_; } 
    /** ダメージを受けたか */
    inline bool IsTakeDamage() const { return isTakeDamage_; }


public:
    /**
     * HP減少を一時的に無効化する（チュートリアル中など、実際のHP変化を止めたい場面で使う）
     * Player・Boss共通のActorStatusにあるフラグなので、これ1つで両方のHP減少を止められる
     */
    inline static void SetDamageDisabled(bool flg) { s_damageDisabled = flg; }

    /** ダメージを与える(hpがマイナスにならないようにクランプ)。s_damageDisabled中はHPは減らさず、被弾フラグだけ立てる */
    void Damage(int damage)
    {
        if (!s_damageDisabled)
        {
            hp_ -= damage;
            if (hp_ < 0) hp_ = 0;
        }
        isTakeDamage_ = true;
    }

private:
    static bool s_damageDisabled; //!< trueの間はDamage()がHPを実際には減らさない（ActorStatus.cppで定義）

public:

    /** 回復処理(maxHp_ を超えないようにクランプする) */
    void Heal(int value)
    {
        hp_ += value;
        if (hp_ > maxHp_) hp_ = maxHp_;
    }

    /** 死亡した */
    inline void Die() { isDead_ = true; }

public:
    /** 派生型かどうか確認する */
    template <typename T>
    bool Is() const { return dynamic_cast<const T*>(this) != nullptr; }

    /** 派生型にキャストして取得する */
    template <typename T>
    T* As() { return dynamic_cast<T*>(this); }
};



/** キャラクターのステータス  */
class CharacterStatus : public ActorStatus
{
public:
    /** コンストラクタ */
    CharacterStatus() {}
    /** デストラクタ */
    virtual ~CharacterStatus() {}
    /** 更新 */
    virtual void Update() override { ActorStatus::Update(); }
};



/** プレイヤーのスキルステータス */
class PlayerSkillStatus
{
private:
    std::unordered_map<std::string, SkillSlot> slots_; //!< スキルの名前とそのスキルのパラメータを格納


public:
    PlayerSkillStatus() {}
    ~PlayerSkillStatus() {}

    /** 毎フレーム全スロットのクールダウンを更新する */
    void Update()
    {
        for (auto& it : slots_)
        {
            it.second.Update();
        }
    }


public:
    /** スキルをスロットに装備する */
    void EquipSkill(const std::string& slotName, const std::string& category, const std::string& skillKey)
    {
        // プレイヤーのスキルのパラメータを取得
        const auto* param = ParameterManager::Get().GetPlayerSkill(category, skillKey);
        if (!param) { return; }

        // パラメータの設定
        SkillSlot slot;
        slot.Init(skillKey, param->motionValues, param->cooldown, param->decreaseStamina, param->vibrationTime, param->vibrationForce);
        slots_[slotName] = slot;
    }


public:
    /** スキルを使用する。クールダウンを開始させる */
    void Activate(const std::string& slotName)
    {
        // 指定した名前でスキルを捜索、そのスキルが使用する
        auto it = slots_.find(slotName);
        if (it != slots_.end()) { it->second.Activate(); }
    }

    /** スキルが使用可能か */
    bool CanUse(const std::string& slotName) const
    {
        // 指定した名前でスキルを捜索、そのスキルが使用可能か見る
        auto it = slots_.find(slotName);
        return (it != slots_.end()) && it->second.CanUse();
    }

    /** 残りクールタイムを取得する */
    float GetCoolTimer(const std::string& slotName) const
    {
        // 指定した名前でスキルを捜索、そのスキルのクールタイムを取得
        auto it = slots_.find(slotName);
        return (it != slots_.end()) ? it->second.GetCoolTimer() : 0.0f;
    }

    /** 指定スロットのモーション倍率を取得する */
    float GetMotionValues(const std::string& slotName) const
    {
        // 指定した名前でスキルを捜索、そのスキルのモーション倍率を取得
        auto it = slots_.find(slotName);
        return (it != slots_.end()) ? it->second.GetMotionValues() : 0.0f;
    }

    /** 指定スキルのバイブレーション時間を取得する */
    float GetVibrationTimeValues(const std::string& skillKey) const
    {
        // 指定した名前でスキルを捜索、そのスキルのバイブレーション時間を取得
        auto it = slots_.find(skillKey);
        return (it != slots_.end()) ? it->second.GetVibrationTime() : 0.0f;
    }

    /** 指定スキルのバイブレーション力を取得する */
    float GetVibrationForceValues(const std::string& skillKey) const
    {
        // 指定した名前でスキルを捜索、そのスキルのバイブレーションの強さを取得
        auto it = slots_.find(skillKey);
        return (it != slots_.end()) ? it->second.GetVibrationForce() : 0.0f;
    }

    /** 指定スロットのスタミナ消費量を取得 */
    float GetDecreaseStamina(const std::string& slotName) const
    {
        // 指定した名前でスキルを捜索、そのスキルのスタミナ消費量を取得
        auto it = slots_.find(slotName);
        return (it != slots_.end()) ? it->second.GetDecreaseStamina() : 0.0f;
    }

public:
    /** 全スキルスロットを取得する */
    inline const std::unordered_map<std::string, SkillSlot>& GetAllSlots() const { return slots_; }
};



/** ボスのスキルステータス */
class BossSkillStatus
{
private:
    std::unordered_map<std::string, SkillSlot> slots_; //!< スキルの名前とそのスキルのパラメータを格納

public:
    /** コンストラクタ */
    BossSkillStatus() {}
    /** デストラクタ */
    ~BossSkillStatus() {}

    /** 毎フレーム全スロットのクールダウンを更新する */
    void Update()
    {
        for (auto& it : slots_)
        {
            it.second.Update();
        }
    }


public:
    /** 対象ボスの全スキルを一括で読み込んで初期化する */
    void Init(const std::string& characterKey)
    {
        slots_.clear();

        // キャラクターの名前から使用するスキルのパラメータをそれぞれ取得
        auto skills = ParameterManager::Get().GetBossSkillsByCategory(characterKey);

        // 取得した情報をもとにスキルそれぞれにパラメータを設定
        for (const auto* param : skills)
        {
            SkillSlot slot;
            slot.Init(param->key, param->motionValues, param->cooldown,0.0f, param->vibrationTime, param->vibrationForce);
            slots_[param->key] = slot;
        }
    }

public:
    /** スキルを使用する。クールダウンを開始させる  */
    void Activate(const std::string& skillKey, float speedMul = 1.0f)
    {
        auto it = slots_.find(skillKey);
        if (it != slots_.end()) { it->second.Activate(speedMul); }
    }

    /** スキルが使用可能か */
    bool CanUse(const std::string& skillKey) const
    {
        auto it = slots_.find(skillKey);
        return (it != slots_.end()) && it->second.CanUse();
    }

    /** 残りクールタイムを取得する */
    float GetCoolTimer(const std::string& skillKey) const
    {
        auto it = slots_.find(skillKey);
        return (it != slots_.end()) ? it->second.GetCoolTimer() : 0.0f;
    }

    /** 指定スキルのモーション倍率を取得する */
    float GetMotionValues(const std::string& skillKey) const
    {
        auto it = slots_.find(skillKey);
        return (it != slots_.end()) ? it->second.GetMotionValues() : 0.0f;
    }

    /** 指定スキルのバイブレーション時間を取得する */
    float GetVibrationTimeValues(const std::string& skillKey) const
    {
        auto it = slots_.find(skillKey);
        return (it != slots_.end()) ? it->second.GetVibrationTime() : 0.0f;
    }

    /** 指定スキルのバイブレーション力を取得する */
    float GetVibrationForceValues(const std::string& skillKey) const
    {
        auto it = slots_.find(skillKey);
        return (it != slots_.end()) ? it->second.GetVibrationForce() : 0.0f;
    }

public:
    /** 全スロットを取得する */
    inline const std::unordered_map<std::string, SkillSlot>& GetAllSlots() const { return slots_; }
};


/** プレイヤー本体のステータス */
class PlayerStatus : public CharacterStatus
{
public:
    /**
     * 無敵フラグ（ビットフラグ）
     *
     * 1つの整数の各ビットを「ON/OFF スイッチ」として使う。
     * 複数の無敵状態が同時に重なっても、それぞれ独立して管理できる。
     *
     *   ビット位置:  4           3         2          1
     *   フラグ名:    enJustAvoid enSkill   enDamage   enAvoid
     *   値(2進):     00010000    00001000  00000100   00000010
     *
     * 例: 回避中 & ジャスト回避中 → invincibleFlag_ = 00010010 (= 18)
     */
    enum class InvincibleFlags : uint32_t
    {
        enNone      = 0,
        enAvoid     = 1 << 1, //!< 回避中
        enDamage    = 1 << 2, //!< ダメージ受け中
        enSkill     = 1 << 3, //!< スキル使用中
        enJustAvoid = 1 << 4, //!< ジャスト回避ウィンドウ中
    };

    enum class StaminaState
    {
        enNone,     //!< 回復も消費もしない
        enRecover,  //!< 回復
        enDrain,    //!< 消費
        enDepletion //!< 枯渇
    };

private:
    /* スタミナ：設定値 */
    float maxStamina_                = 0.0f; //!< 最大スタミナ
    float staminaDrainPerSec_        = 0.0f; //!< 毎秒消費量（ダッシュ中）
    float staminaRecoverPerSec_      = 0.0f; //!< 毎秒回復量（通常：IdleまたはWalk中）
    float exhaustedRecoverPerSec_    = 0.0f; //!< 毎秒回復量（枯渇中）
    float exhaustedThreshold_        = 0.0f; //!< 枯渇解除に必要なスタミナ（最大値に対する割合）
    float exhaustedSpeedRate_        = 0.0f; //!< 枯渇中の移動速度倍率

    /* スタミナ：1フレームあたりの増減量 */
    float staminaDrainPerFrame_      = 0.0f; //!< 1フレームあたりの消費量
    float staminaRecoverPerFrame_    = 0.0f; //!< 1フレームあたりの回復量（通常）
    float exhaustedRecoverPerFrame_  = 0.0f; //!< 1フレームあたりの回復量（枯渇中）

    /* スタミナ：実行時の状態 */
    float           stamina_      = 0.0f;                   //!< 現在スタミナ量
    StaminaState    staminaState_ = StaminaState::enNone;   //!< 現在のスタミナ状態
    bool            isExhausted_  = false;                  //!< スタミナ枯渇フラグ
    
    /* その他 */
    float             runSpeedBase_     = 0.0f; //!< 走る時のベース移動速度
    uint32_t          invincibleFlag_   = 0;    //!< 無敵フラグ（ビット演算）
    PlayerSkillStatus skillStatus_;             //!< スキルスロット＋クールダウン管理


public:
    /** コンストラクタ */
    PlayerStatus() {}
    /** デストラクタ */
    ~PlayerStatus() {}

    /**
     * 毎フレーム呼ぶ。
     * 回復するのは Idle と Walk のみ。それ以外は増減なし
     */
    virtual void Update()
    {
        // 1フレームの時間
        const float delta = g_gameTime->GetFrameDeltaTime();

        // 1フレームあたりの増減量を毎フレーム再計算
        staminaDrainPerFrame_ = staminaDrainPerSec_ * delta;         // 消費量
        staminaRecoverPerFrame_ = staminaRecoverPerSec_ * delta;     // 通常時の回復量
        exhaustedRecoverPerFrame_ = exhaustedRecoverPerSec_ * delta; // 枯渇時の回復量

        // スタミナを消費
        if (staminaState_ == StaminaState::enDrain && !isExhausted_)
        {
            DrainStamina(staminaDrainPerFrame_);
        }
        // スタミナを回復
        else if (staminaState_ == StaminaState::enRecover || staminaState_ == StaminaState::enDepletion)
        {
            RecoverStamina(isExhausted_ ? exhaustedRecoverPerFrame_ : staminaRecoverPerFrame_);
        }

        // 枯渇フラグの更新
        if (stamina_ <= 0.0f)
        {
            isExhausted_ = true;
            staminaState_ = StaminaState::enDepletion;
        }

        // 枯渇状態から回復するために必要なスタミナ量を計算
        const float exhaustedReleaseStamina = maxStamina_ * exhaustedThreshold_;
        // 一定量回復するまでフラグを解除しない
        if (isExhausted_ && stamina_ >= exhaustedReleaseStamina)
        {
            isExhausted_ = false;
            if (staminaState_ == StaminaState::enDepletion)
            {
                staminaState_ = StaminaState::enRecover;
            }
        }

        // 更新
        skillStatus_.Update();
        CharacterStatus::Update();
    }


public:
    /** パラメータの初期化 */
    void Init(const std::string& characterKey = "Player")
    {
        // パラメータをjsonファイルから取得
        const auto* param = ParameterManager::Get().GetCharacterStatus(characterKey);
        if (!param) { return; }

        // パラメータを設定
        {
            initParam_.pos_ = param->position;                           // 座標
            initParam_.rot_ = param->rotation;                           // 回転
            initParam_.scal_ = param->scale;                             // 拡縮
            initParam_.collisionHeightUpValue_ = param->collisionPosUp;  // コリジョンを上げる量
            initParam_.collisionRadius_ = param->collisionSizeRadius;    // コリジョンの半径
            initParam_.collisionHeight_ = param->collisionSizeHeight;    // コリジョンの高さ
            initParam_.charaConRadius_ = param->charaConSizeRadius;      // キャラコンの半径
            initParam_.charaConHeight_ = param->charaConSizeHeight;      // キャラコンの高さ
            hp_ = param->hp;                                             // 現在の体力量
            maxHp_ = param->hp;                                          // 最大の体力量
            attack_ = param->attack;                                     // 基礎攻撃力量
            critical_ = param->criticalRate;                             // クリティカル率
            criticalDamageMultiplier_ = param->criticalDamageMultiplier; // クリティカル時のダメージ倍率

            // スタミナ設定値
            maxStamina_ = param->stamina.maxStamina;                         // 最大のスタミナ量
            stamina_ = maxStamina_;                                          // 現在のスタミナ量
            staminaDrainPerSec_ = param->stamina.drainPerSec;                // 1秒あたりの消費量
            staminaRecoverPerSec_ = param->stamina.recoverPerSec;            // 1秒あたりの回復量(通常)
            exhaustedRecoverPerSec_ = param->stamina.exhaustedRecoverPerSec; // 1秒あたりの回復量(枯渇)
            exhaustedThreshold_ = param->stamina.exhaustedThreshold;         // 枯渇解除に必要なスタミナ量
            exhaustedSpeedRate_ = param->stamina.exhaustedSpeedRate;         // 枯渇時の移動速度倍率

            // 移動速度
            moveSpeedBase_ = param->moveSpeedBase; // 通常の移動速度
            runSpeedBase_ = param->runSpeedBase;   // 走るときの移動速度
        }
    }

    /** スキルをスロットに装備する。Init() の後に呼ぶ */
    inline void EquipSkill(const std::string& slotName, const std::string& category, const std::string& skillKey)
    {
        skillStatus_.EquipSkill(slotName, category, skillKey);
    }


public:
    /** 現在のスタミナ量を取得 */
    inline float GetStamina()              const { return stamina_; }
    /** 最大のスタミナ量の取得 */
    inline float GetMaxStamina()           const { return maxStamina_; }
    /** スタミナ枯渇状態か */
    inline bool  IsExhausted()             const { return isExhausted_; }
    /** 現在のスタミナの状態を取得 */
    inline StaminaState GetStaminaState()  const { return staminaState_; }
    /** スタミナの状態を設定 */
    inline void SetStaminaState(const StaminaState state) { staminaState_ = state; }
    /** スタミナを最大まで回復し、枯渇状態も解除する（チュートリアル終了時など、状態を初期化したい場面で使う） */
    inline void ResetStamina() { stamina_ = maxStamina_; isExhausted_ = false; staminaState_ = StaminaState::enNone; }

    /** 毎秒のスタミナ消費量の取得 */
    inline float GetStaminaDrainPerSec()       const { return staminaDrainPerSec_; }
    /** 毎秒のスタミナ回復量の取得(通常) */
    inline float GetStaminaRecoverPerSec()     const { return staminaRecoverPerSec_; }
    /** 毎秒のスタミナ回復量の取得(枯渇) */
    inline float GetExhaustedRecoverPerSec()   const { return exhaustedRecoverPerSec_; }

    /** 1フレームあたりのスタミナの消費量を取得 */
    inline float GetStaminaDrainPerFrame()     const { return staminaDrainPerFrame_; }
    /** 1フレームあたりのスタミナの回復量を取得(通常) */
    inline float GetStaminaRecoverPerFrame()   const { return staminaRecoverPerFrame_; }
    /** 1フレームあたりのスタミナの回復量を取得(枯渇) */
    inline float GetExhaustedRecoverPerFrame() const { return exhaustedRecoverPerFrame_; }

    /**
     * 基礎攻撃力を取得 
     * DamageCalculator は ActorStatus* 経由でこれを呼ぶため virtual override が必須
     */
    inline int GetAttack() const override { return attack_; }

    /**
     * 基礎移動速度を取得
     * DamageCalculator は ActorStatus* 経由でこれを呼ぶため virtual override が必須
     */
    inline float GetMoveSpeedBase() const override { return moveSpeedBase_;}

    /** 走るときの移動速度を取得 */
    inline float GetRunSpeedBase() const { return runSpeedBase_;}
    /** 枯渇中の移動速度を取得 */
    inline float GetExhaustedSpeedRate()   const { return exhaustedSpeedRate_; }


public:
    /** スキルを使用する。クールダウンを開始させる */
    inline void  ActivateSkill(const std::string& slotName) { skillStatus_.Activate(slotName); }

    /** クールダウン開始 */
    void ExecuteSpecialAbility()
    {
        const auto& slots = skillStatus_.GetAllSlots();
        if (slots.find("SpecialAttack") != slots.end()) {
            skillStatus_.Activate("SpecialAttack");
            return;
        }
        if (slots.find("SpecialAbility") != slots.end()) {
            skillStatus_.Activate("SpecialAbility");
            return;
        }
        if (slots.find("defaultAttack") != slots.end()) {
            skillStatus_.Activate("defaultAttack");
        }
    }

    /**
     * 特殊能力が実行可能か
     */
    bool CanExecuteSpecialAbility() const
    {
        const auto& slots = skillStatus_.GetAllSlots();

        if (slots.find("SpecialAttack") != slots.end()) {
            return skillStatus_.CanUse("SpecialAttack");
        }
        if (slots.find("SpecialAbility") != slots.end()) {
            return skillStatus_.CanUse("SpecialAbility");
        }
        if (slots.find("defaultAttack") != slots.end()) {
            return skillStatus_.CanUse("defaultAttack");
        }

        return true;
    }

    /**
     * 特殊能力のクールダウン明けフレームか
     */
    bool IsReadyFrameSpecialAbility() const
    {
        const auto& slots = skillStatus_.GetAllSlots();

        auto it = slots.find("SpecialAttack");
        if (it != slots.end()) { return it->second.IsReadyFrame(); }

        it = slots.find("SpecialAbility");
        if (it != slots.end()) { return it->second.IsReadyFrame(); }

        it = slots.find("defaultAttack");
        if (it != slots.end()) { return it->second.IsReadyFrame(); }

        return false;
    }

    /** スキルが使用可能か */
    inline bool  CanUseSkill(const std::string& slotName) const { return skillStatus_.CanUse(slotName); }

    /** 残りクールタイムを取得する（UI 表示用） */
    inline float GetSkillCoolTimer(const std::string& slotName) const { return skillStatus_.GetCoolTimer(slotName); }

    /** スキルのモーション倍率を取得する */
    inline float GetSkillMotionValues(const std::string& slotName) const { return skillStatus_.GetMotionValues(slotName); }

    /** スキルのモーション倍率を取得する */
    inline float GetVibrationTimeValues(const std::string& skillKey) const { return skillStatus_.GetVibrationTimeValues(skillKey); }

    /** スキルのモーション倍率を取得する */
    inline float GetVibrationForceValues(const std::string& skillKey) const { return skillStatus_.GetVibrationForceValues(skillKey); }

    /** スキルステータス全体へのアクセス */
    inline const PlayerSkillStatus& GetSkillStatus() const { return skillStatus_; }
    inline       PlayerSkillStatus& GetSkillStatus()       { return skillStatus_; }


public:
    /** 無敵フラグ操作 */
    // |= : 指定フラグのビットを 1 にする（他のフラグはそのまま）
    inline void AddInvincible(InvincibleFlags flag)    { invincibleFlag_ |=  static_cast<uint32_t>(flag); }
    // &= ~ : 指定フラグのビットを 0 にする（~ で対象ビットだけ反転させてから AND で消す）
    inline void RemoveInvincible(InvincibleFlags flag) { invincibleFlag_ &= ~static_cast<uint32_t>(flag); }
    /** 無敵フラグを全て解除する（チュートリアル終了時など、状態を初期化したい場面で使う） */
    inline void ClearInvincible() { invincibleFlag_ = 0; }
    // != 0 : どれか1つでもフラグが立っていれば true
    inline bool IsInvincible()      const { return invincibleFlag_ != 0; }
    // & : enJustAvoid のビットだけ取り出して 0 以外なら true（ジャスト回避ウィンドウ中）
    inline bool IsJustAvoiding()    const { return (invincibleFlag_ & static_cast<uint32_t>(InvincibleFlags::enJustAvoid)) != 0; }
    // & : enAvoid のビットだけ取り出して 0 以外なら true（回避ロール中）
    inline bool IsNormalDodging()   const { return (invincibleFlag_ & static_cast<uint32_t>(InvincibleFlags::enAvoid)) != 0; }


    /*************** スタミナの増減に関する関数 ***************/
private: 
    /**
     * スタミナを減少させる。
     * @param amount 1フレームで減少するスタミナ量
     */
    void DrainStamina(float amount)
    {
        // スタミナを消費
        stamina_ -= amount;

        // スタミナが切れるなら0にする
        if (stamina_ < 0.0f){ stamina_ = 0.0f; }
    }

    /**
     * スタミナを回復させる。
     * @param amount 1フレームで回復するスタミナ量（通常 or 枯渇中で呼び分ける）
     */
    void RecoverStamina(float amount)
    {
        // スタミナ回復
        stamina_ += amount;

        // スタミナが上限を超えないよう補正
        if (stamina_ > maxStamina_)
        {
            stamina_ = maxStamina_;
        }
        // 枯渇状態なら
        if (isExhausted_)
        {
            // 枯渇状態から戻すスタミナの値を取得
            float needStaminaValue = maxStamina_ * exhaustedThreshold_;
            // 必要量を満たしているなら
            if (stamina_ >= needStaminaValue)
            {
                // 枯渇状態ではなくし、状態を初期に戻す
                isExhausted_ = false;
            }
        }
    }

public:
    /** 回避など瞬間コスト用。スタミナを直接減算し枯渇チェックも行う */
    void ConsumeStamina(float amount)
    {
        // 1フレームでコスト分スタミナを消費
        DrainStamina(amount);

        if (stamina_ <= 0.0f) {
            isExhausted_ = true;
            staminaState_ = StaminaState::enDepletion;
        }
    }

};


/**  ボス本体のステータス */
class BossStatus : public CharacterStatus
{
private:
    BossSkillStatus skillStatus_; //!< スキルスロット＋クールダウン管理
    EmotionSystem emotionSystem_; //!< 感情システム
    std::vector<IBossStatusModifier*> modifiers_; //!< ボスのステータスへの倍率を修正する際の要因を設定する


public:
    /** コンストラクタ */
    BossStatus()
    {
        // ステータスへの倍率修正の要因をここで設定
        modifiers_.push_back(&emotionSystem_);
    }
    /** デストラクタ */
    ~BossStatus() {}

    /** 毎フレーム呼ぶ。スキルのクールダウン更新と基底フラグリセットを行う */
    virtual void Update() override
    {
        skillStatus_.Update();
        CharacterStatus::Update();
    }


public:
    /** ParameterManager からキャラクターステータスと全スキルを一括初期化する */
    void Init(const std::string& characterKey)
    {
        const auto* param = ParameterManager::Get().GetCharacterStatus(characterKey);
        if (param)
        {
            initParam_.pos_ = param->position;                           //!< 座標
            initParam_.rot_ = param->rotation;                           //!< 回転
            initParam_.scal_ = param->scale;                             //!< 拡縮
            initParam_.collisionHeightUpValue_ = param->collisionPosUp;  //!< コリジョンを上にあげる
            initParam_.collisionRadius_ = param->collisionSizeRadius;    //!< コリジョンの半径
            initParam_.collisionHeight_ = param->collisionSizeHeight;    //!< コリジョンの高さ
            initParam_.charaConRadius_ = param->charaConSizeRadius;      //!< キャラコンの半径
            initParam_.charaConHeight_ = param->charaConSizeHeight;      //!< キャラコンの高さ
            hp_                       = param->hp;                       //!< 現在の体力量
            maxHp_                    = param->hp;                       //!< 最大の体力量
            attack_                   = param->attack;                   //!< 基礎攻撃力
            critical_                 = param->criticalRate;             //!< クリティカル率
            criticalDamageMultiplier_ = param->criticalDamageMultiplier; //!< クリティカル時のダメージ倍率
            moveSpeedBase_            = param->moveSpeedBase;            //!< ボスの移動速度
        }

        // 攻撃のステータス設定
        skillStatus_.Init(characterKey);

        // JSONから倍率テーブルを上書きロードする（コンストラクタ時点ではParameterManager未初期化のためここで呼ぶ）
        emotionSystem_.Init();
    }

public:
    /**
     * 基礎攻撃力を取得
     * DamageCalculator は ActorStatus* 経由でこれを呼ぶため virtual override が必須
     */
    inline int GetAttack() const override
    {
        float mul = 1.0f;
        for (auto* m : modifiers_) { mul *= m->GetAttackMul(); }
        return static_cast<int>(attack_ * mul);
    }

    /**
     * 攻撃速度倍率を取得
     * DamageCalculator は ActorStatus* 経由でこれを呼ぶため virtual override が必須
     */
    inline float GetAttackSpeedMul() const
    {
        float mul = 1.0f;
        for (auto* m : modifiers_) { mul *= m->GetAttackSpeedMul(); }
        return mul;
    }

    /** 被ダメージ倍率を取得 */
    inline float GetDamageTakenMul() const
    {
        float mul = 1.0f;
        for (auto* m : modifiers_) { mul *= m->GetDamageTakenMul(); }
        return mul;
    }

    /** アニメーション再生速度倍率を取得 */
    inline float GetAnimationSpeedMul() const
    {
        float mul = 1.0f;
        for (auto* m : modifiers_) { mul *= m->GetAnimationSpeedMul(); }
        return mul;
    }

    /** エフェクト再生速度倍率を取得 */
    inline float GetEffectSpeedMul() const
    {
        float mul = 1.0f;
        for (auto* m : modifiers_) { mul *= m->GetEffectSpeedMul(); }
        return mul;
    }

    // 感情システムへのアクセス（CollisionHitManager や BossEmotionPhaseManager から使う）
    inline EmotionSystem& GetEmotionSystem() { return emotionSystem_; }

public:
    /** スキルを使用する。クールダウンを開始させる（現在の攻撃速度倍率をクールダウンに反映する） */
    inline void  ActivateSkill(const std::string& skillKey) { skillStatus_.Activate(skillKey, GetAttackSpeedMul()); }

    /** スキルが使用可能か */
    inline bool  CanUseSkill(const std::string& skillKey)       const { return skillStatus_.CanUse(skillKey); }

    /** 残りクールタイムを取得する（UI 表示用） */
    inline float GetSkillCoolTimer(const std::string& skillKey) const { return skillStatus_.GetCoolTimer(skillKey); }

    /** スキルのモーション倍率を取得する */
    inline float GetSkillMotionValues(const std::string& skillKey) const { return skillStatus_.GetMotionValues(skillKey); }
    
    /** スキルのモーション倍率を取得する */
    inline float GetVibrationTimeValues(const std::string& skillKey) const { return skillStatus_.GetVibrationTimeValues(skillKey); }

    /** スキルのモーション倍率を取得する */
    inline float GetVibrationForceValues(const std::string& skillKey) const { return skillStatus_.GetVibrationForceValues(skillKey); }

    /** スキルステータス全体へのアクセス */
    inline const BossSkillStatus& GetSkillStatus() const { return skillStatus_; }
    inline       BossSkillStatus& GetSkillStatus()       { return skillStatus_; }
};
