#pragma once
#include "src/Core/ParameterManager.h"


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
    void Activate() { coolTimer_ = coolDownTime_; }

    float GetTimer()    const { return coolTimer_; }            //!< 残り時間取得
    bool  CanUse()      const { return coolTimer_ <= 0.0f; }    //!< 使用可能か
    bool  IsReadyFrame() const { return isReadyFrame_; }        //!< 明けた瞬間か
};


/** スキル1つに必要な情報 */
struct SkillSlot
{
    std::string key_          = "";      //!< スキル識別キー（JSON の "key" と対応）
    float       motionValues_ = 0.0f;    //!< モーション倍率（本体 attack_ にかける係数）
    CoolDown    coolDown_;               //!< このスキル専用のクールダウン
    float       decreaseStamina_ = 0.0f; //!< スタミナ消費量

    /** スキルスロットを初期化する */
    void Init(const std::string& key, float motionValues, float coolDownTime, float decreaseStamina = 0.0f)
    {
        key_                    = key;
        motionValues_           = motionValues;
        coolDown_.coolDownTime_ = coolDownTime;
        decreaseStamina_ = decreaseStamina;
    }

    /** 毎フレーム呼ぶ */
    void Update() { coolDown_.Update(); }

    /** スキル使用時に呼ぶ。クールダウンを開始させる */
    void Activate() { coolDown_.Activate(); }

    float GetMotionValues()     const { return motionValues_; }
    float GetCoolTimer()        const { return coolDown_.GetTimer(); }
    float GetDecreaseStamina()  const { return decreaseStamina_; }
    bool  CanUse()              const { return coolDown_.CanUse(); }
    bool  IsReadyFrame()        const { return coolDown_.IsReadyFrame(); }
};

struct ActorInitParam
{
    Vector3 pos_ = Vector3::Zero; //!< 座標
    Quaternion rot_ = Quaternion::Identity; //!< 回転
    Vector3 scal_ = Vector3::Zero; //!< 大きさ
    Vector3 collisionHeightUpValue_ = Vector3::Zero; //!< コリジョンの高さを上げる量
    float collisionRadius_ = 0.0f;  //!< コリジョンの半径
    float collisionHeight_ = 0.0f;  //!< コリジョンの高さ
    float charaConRadius_ = 0.0f;  //!< キャラコンの半径
    float charaConHeight_ = 0.0f;  //!< キャラコンの高さ
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


public:
    ActorStatus()          {}
    virtual ~ActorStatus() {}

    /** 毎フレーム呼ぶ。フレームフラグをリセットする */
    virtual void Update() { isTakeDamage_ = false; }


public:
    ActorInitParam GetInitParam()       const { return initParam_; }
    int   GetHP()                       const { return hp_; }
    int   GetMaxHP()                    const { return maxHp_; }
    int   GetAttack()                   const { return attack_; }
    int   GetCritical()                 const { return critical_; }
    float GetCriticalDamageMultiplier() const { return criticalDamageMultiplier_; }
    float GetMoveSpeedBase()            const { return moveSpeedBase_; }
    bool  IsDead()                      const { return hp_ <= 0; }
    bool  IsTakeDamage()                const { return isTakeDamage_; }


public:
    /** ダメージを与える。HP を減算し isTakeDamage_ を立てる */
    void Damage(int damage)
    {
        hp_ -= damage;
        if (hp_ < 0) hp_ = 0;
        isTakeDamage_ = true;
    }

    /** 回復する。maxHp_ を超えないようにクランプ */
    void Heal(int value)
    {
        hp_ += value;
        if (hp_ > maxHp_) hp_ = maxHp_;
    }


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
    CharacterStatus()          {}
    virtual ~CharacterStatus() {}

    virtual void Update() override { ActorStatus::Update(); }
};


/** プレイヤーのスキルステータス */
class PlayerSkillStatus
{
private:
    std::unordered_map<std::string, SkillSlot> slots_; //!< スロット名 → SkillSlot


public:
    PlayerSkillStatus()  {}
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
    void EquipSkill(const std::string& slotName,
                    const std::string& category,
                    const std::string& skillKey)
    {
        const auto* param = ParameterManager::Get().GetPlayerSkill(category, skillKey);
        if (!param) { return; }

        SkillSlot slot;
        slot.Init(skillKey, param->motionValues, param->cooldown, param->decreaseStamina);
        slots_[slotName] = slot;
    }


public:
    /** スキルを使用する。クールダウンを開始させる */
    void Activate(const std::string& slotName)
    {
        auto it = slots_.find(slotName);
        if (it != slots_.end()) { it->second.Activate(); }
    }

    /** スキルが使用可能か */
    bool CanUse(const std::string& slotName) const
    {
        auto it = slots_.find(slotName);
        return (it != slots_.end()) && it->second.CanUse();
    }

    /** 残りクールタイムを取得する */
    float GetCoolTimer(const std::string& slotName) const
    {
        auto it = slots_.find(slotName);
        return (it != slots_.end()) ? it->second.GetCoolTimer() : 0.0f;
    }

    /** 指定スロットのモーション倍率を取得する */
    float GetMotionValues(const std::string& slotName) const
    {
        auto it = slots_.find(slotName);
        return (it != slots_.end()) ? it->second.GetMotionValues() : 0.0f;
    }

    /** 指定スロットのスタミナ消費量を取得 */
    float GetDecreaseStamina(const std::string& slotName) const
    {
        auto it = slots_.find(slotName);
        return (it != slots_.end()) ? it->second.GetDecreaseStamina() : 0.0f;
    }

public:
    /** 全スロットを取得する */
    const std::unordered_map<std::string, SkillSlot>& GetAllSlots() const { return slots_; }
};


/* ============================================================ */
/*  BossSkillStatus                                              */
/*  ボスの全スキルスロットを管理する                             */
/*  JSON のスキルキーをそのままスロットキーとして保持する        */
/*  Init() で対象ボスの全スキルを一括読み込みする               */
/*  クールダウンは各 SkillSlot が 1 対 1 で持つ                 */
/* ============================================================ */
class BossSkillStatus
{
private:
    std::unordered_map<std::string, SkillSlot> slots_; //!< スキルキー

public:
    BossSkillStatus()  {}
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
    /** ParameterManager から対象ボスの全スキルを一括で読み込んで初期化する */
    void Init(const std::string& characterKey)
    {
        slots_.clear();

        auto skills = ParameterManager::Get().GetBossSkillsByCategory(characterKey);
        for (const auto* param : skills)
        {
            SkillSlot slot;
            slot.Init(param->key, param->motionValues, param->cooldown);
            slots_[param->key] = slot;
        }
    }

public:
    /** スキルを使用する。クールダウンを開始させる */
    void Activate(const std::string& skillKey)
    {
        auto it = slots_.find(skillKey);
        if (it != slots_.end()) { it->second.Activate(); }
    }

    /** スキルが使用可能か */
    bool CanUse(const std::string& skillKey) const
    {
        auto it = slots_.find(skillKey);
        return (it != slots_.end()) && it->second.CanUse();
    }

    /** 残りクールタイムを取得する（UI 表示用） */
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

public:
    /** 全スロットを取得する */
    const std::unordered_map<std::string, SkillSlot>& GetAllSlots() const { return slots_; }
};


/** プレイヤー本体のステータス */
class PlayerStatus : public CharacterStatus
{
public:
    /* 無敵フラグ */
    enum class InvincibleFlags : uint32_t
    {
        enNone   = 0,
        enAvoid  = 1 << 1, //!< 回避中
        enDamage = 1 << 2, //!< ダメージ受け中
        enSkill  = 1 << 3, //!< スキル使用中
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
    float           stamina_                = 0.0f; //!< 現在スタミナ
    StaminaState    staminaState_           = StaminaState::enNone; //!< スタミナ状態
    bool            isExhausted_            = false; //!< スタミナ枯渇フラグ
    

    /* 移動速度 */
    float runSpeedBase_              = 0.0f; //!< 走り時のベース移動速度

    /* その他 */
    uint32_t          invincibleFlag_ = 0;   //!< 無敵フラグ（ビット演算）
    PlayerSkillStatus skillStatus_;          //!< スキルスロット＋クールダウン管理


public:
    PlayerStatus()  
    {

    }
    ~PlayerStatus() {}

    /**
     * 毎フレーム呼ぶ。
     * 回復するのは Idle と Walk のみ。それ以外（攻撃など）は増減なし
     */
    virtual void Update()
    {
        // 1フレームの時間
        const float delta = g_gameTime->GetFrameDeltaTime();

        // 1フレームあたりの増減量を毎フレーム再計算
        staminaDrainPerFrame_ = staminaDrainPerSec_ * delta;
        staminaRecoverPerFrame_ = staminaRecoverPerSec_ * delta;
        exhaustedRecoverPerFrame_ = exhaustedRecoverPerSec_ * delta;

        if (staminaState_ == StaminaState::enDrain && !isExhausted_)
        {
            DrainStamina(staminaDrainPerFrame_);
        }
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

        skillStatus_.Update();
        CharacterStatus::Update();
    }


public:
    /** ParameterManager からキャラクターステータスを読み込んで初期化する */
    void Init(const std::string& characterKey = "Player")
    {
        // パラメータをjsonファイルから取得
        const auto* param = ParameterManager::Get().GetCharacterStatus(characterKey);
        if (!param) { return; }

        // パラメータを設定
        {
            initParam_.pos_ = param->position;
            initParam_.rot_ = param->rotation;
            initParam_.scal_ = param->scale;
            initParam_.collisionHeightUpValue_ = param->collisionPosUp;
            initParam_.collisionRadius_ = param->collisionSizeRadius;
            initParam_.collisionHeight_ = param->collisionSizeHeight;
            initParam_.charaConRadius_ = param->charaConSizeRadius;
            initParam_.charaConHeight_ = param->charaConSizeHeight;
            hp_ = param->hp;
            maxHp_ = param->hp;
            attack_ = param->attack;
            critical_ = param->criticalRate;
            criticalDamageMultiplier_ = param->criticalDamageMultiplier;

            // スタミナ設定値
            maxStamina_ = param->stamina.maxStamina;
            stamina_ = maxStamina_;
            staminaDrainPerSec_ = param->stamina.drainPerSec;
            staminaRecoverPerSec_ = param->stamina.recoverPerSec;
            exhaustedRecoverPerSec_ = param->stamina.exhaustedRecoverPerSec;
            exhaustedThreshold_ = param->stamina.exhaustedThreshold;
            exhaustedSpeedRate_ = param->stamina.exhaustedSpeedRate;

            // 移動速度
            moveSpeedBase_ = param->moveSpeedBase;
            runSpeedBase_ = param->runSpeedBase;
        }
    }

    /** スキルをスロットに装備する。Init() の後に呼ぶ */
    void EquipSkill(const std::string& slotName,
                    const std::string& category,
                    const std::string& skillKey)
    {
        skillStatus_.EquipSkill(slotName, category, skillKey);
    }


public:
    /* スタミナ取得 */
    float GetStamina()              const { return stamina_; }
    float GetMaxStamina()           const { return maxStamina_; }
    bool  IsExhausted()             const { return isExhausted_; }
    StaminaState GetStaminaState()  const { return staminaState_; }
    void SetStaminaState(const StaminaState state) { staminaState_ = state; }

    /* スタミナ設定値取得（UIなどデバッグ用） */
    float GetStaminaDrainPerSec()       const { return staminaDrainPerSec_; }
    float GetStaminaRecoverPerSec()     const { return staminaRecoverPerSec_; }
    float GetExhaustedRecoverPerSec()   const { return exhaustedRecoverPerSec_; }

    /* 1フレームあたりの増減量取得 */
    float GetStaminaDrainPerFrame()     const { return staminaDrainPerFrame_; }
    float GetStaminaRecoverPerFrame()   const { return staminaRecoverPerFrame_; }
    float GetExhaustedRecoverPerFrame() const { return exhaustedRecoverPerFrame_; }

    /* 移動速度取得 */
    float GetRunSpeedBase()         const { return runSpeedBase_; }
    float GetExhaustedSpeedRate()   const { return exhaustedSpeedRate_; }


public:
    /** スキルを使用する。クールダウンを開始させる */
    void  ActivateSkill(const std::string& slotName) { skillStatus_.Activate(slotName); }

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
    bool  CanUseSkill(const std::string& slotName)       const { return skillStatus_.CanUse(slotName); }

    /** 残りクールタイムを取得する（UI 表示用） */
    float GetSkillCoolTimer(const std::string& slotName) const { return skillStatus_.GetCoolTimer(slotName); }

    /** スキルのモーション倍率を取得する */
    float GetSkillMotionValues(const std::string& slotName) const { return skillStatus_.GetMotionValues(slotName); }

    /** スキルステータス全体へのアクセス */
    const PlayerSkillStatus& GetSkillStatus() const { return skillStatus_; }
          PlayerSkillStatus& GetSkillStatus()       { return skillStatus_; }


public:
    /* 無敵フラグ */
    void AddInvincible(InvincibleFlags flag)    { invincibleFlag_ |=  static_cast<uint32_t>(flag); }
    void RemoveInvincible(InvincibleFlags flag) { invincibleFlag_ &= ~static_cast<uint32_t>(flag); }
    bool IsInvincible() const                  { return invincibleFlag_ != 0; }


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


public:
    BossStatus()  {}
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
            initParam_.pos_ = param->position;
            initParam_.rot_ = param->rotation;
            initParam_.scal_ = param->scale;
            initParam_.collisionHeightUpValue_ = param->collisionPosUp;
            initParam_.collisionRadius_ = param->collisionSizeRadius;
            initParam_.collisionHeight_ = param->collisionSizeHeight;
            initParam_.charaConRadius_ = param->charaConSizeRadius;
            initParam_.charaConHeight_ = param->charaConSizeHeight;
            hp_                       = param->hp;
            maxHp_                    = param->hp;
            attack_                   = param->attack;
            critical_                 = param->criticalRate;
            criticalDamageMultiplier_ = param->criticalDamageMultiplier;
            moveSpeedBase_            = param->moveSpeedBase; //!< ボスの移動速度
        }

        skillStatus_.Init(characterKey);
    }

public:
    /** スキルを使用する。クールダウンを開始させる */
    void  ActivateSkill(const std::string& skillKey) { skillStatus_.Activate(skillKey); }

    /** スキルが使用可能か */
    bool  CanUseSkill(const std::string& skillKey)       const { return skillStatus_.CanUse(skillKey); }

    /** 残りクールタイムを取得する（UI 表示用） */
    float GetSkillCoolTimer(const std::string& skillKey) const { return skillStatus_.GetCoolTimer(skillKey); }

    /** スキルのモーション倍率を取得する */
    float GetSkillMotionValues(const std::string& skillKey) const { return skillStatus_.GetMotionValues(skillKey); }

    /** スキルステータス全体へのアクセス */
    const BossSkillStatus& GetSkillStatus() const { return skillStatus_; }
          BossSkillStatus& GetSkillStatus()       { return skillStatus_; }
};
