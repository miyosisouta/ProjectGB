#include "stdafx.h"
#include "AttackObject.h"
#include "AttackRange.h"
#include "src/Actor/Character.h"
#include "src/Actor/BossCharacter.h"
#include "src/Stage/GrassBendManager.h"

/*=======================================*/
/* 攻撃用オブジェクトの基底クラス */
/*=======================================*/

bool AttackObjectBase::OnGround()
{
    if (transform_.position.y <= 0.0f) {
        return true;
    }
    return false;
}

AttackObjectBase::AttackObjectBase()
{
}
AttackObjectBase::~AttackObjectBase()
{
}

bool AttackObjectBase::Start()
{
    return true;
}

void AttackObjectBase::Update()
{
}

void AttackObjectBase::Render(RenderContext& rc)
{
}


/*=======================================*/
/* 岩を投げる攻撃 */
/*=======================================*/


void ThrowRockObject::Setup(const Vector3& startPos, const Vector3& direction, float collisionSize, float speed, float launchAngle, float gravity, float spawnHeight)
{
    // 投げる瞬間の座標と、出現直後の座標を覚えておく
    growTargetPos_ = startPos;      // 投げる瞬間
    growStartPos_ = startPos;       // 出現直後
    growStartPos_.y = spawnHeight;

    transform_.localPosition = growStartPos_;   // 出現直後は低い位置から始める（本来の高さへ近づけていく）
    transform_.localScale = startScale_;        // 出現直後は小さいスケールから始める（大きくしていく）
    collisionSize_ = collisionSize;
    flySpeed_ = speed;
    launchAngle_ = launchAngle;
    gravity_ = -gravity;                        // 内部では負の値として持つ（下向きに働くため）
    transform_.UpdateTransform();

    // 水平方向の速度ベクトル
    float rad = Math::DegToRad(launchAngle_);   // 角度計算
    float hSpeed = speed * cos(rad);            // 速度を計算
    flatVelocity_ = direction * hSpeed;         // 飛ぶ方向と速度を計算

    // 垂直方向の初速（仰角から計算）
    verticalVelocity_ = speed * sin(rad);
}

void ThrowRockObject::Culculate()
{
    // 岩が壊れる処理をここに書く
    if (OnGround()) { isFinished_ = true; return; } 

    float time = g_gameTime->GetFrameDeltaTime();

    // 重力を計算
    verticalVelocity_ += gravity_ * time;

    // 1フレームに飛ぶ量を計算。flatVelocityがすでに方向と速度を持っている
    Vector3 moveThisFrame = flatVelocity_ * time;
    moveThisFrame.y += verticalVelocity_ * time;

    // 座標を更新
    transform_.localPosition += moveThisFrame;
    transform_.UpdateTransform();

    // 飛んだ距離を累計
    traveledDist_ += Vector3(moveThisFrame.x, 0.0f, moveThisFrame.z).Length();

    // モデルとコリジョンの座標を同期
    model_.SetPosition(transform_.position);
    model_.Update();
    attackHitBox_->SetPosition(transform_.position);
}



ThrowRockObject::ThrowRockObject()
{
}

ThrowRockObject::~ThrowRockObject()
{
}

bool ThrowRockObject::Start()
{
    // モデルの読み込み
    model_.Init("Assets/Objects/Skill/ThrowRock/throwRock.tkm");
    model_.SetPosition(transform_.position);
    model_.SetScale(transform_.scale); // 出現直後は小さいスケールで表示する

    model_.Update();
	return true;
}

void ThrowRockObject::SetGrowProgress(float t)
{
    if (isLaunched_) return; // 投げた後は呼ばれても無視する

    // 1.0にクランプ
    const float clampedT = max(0.0f, min(t, 1.0f));

    // 大きさの線形補完
    Vector3 curScale;
    curScale.Lerp(clampedT, startScale_, targetScale_);
    transform_.localScale = curScale;

    // 座標の線形補完
    Vector3 curPos;
    curPos.Lerp(clampedT, growStartPos_, growTargetPos_);
    transform_.localPosition = curPos;

    // 値を設定
    transform_.UpdateTransform();
    model_.SetPosition(transform_.position);
    model_.SetScale(transform_.scale);
    model_.Update();
}

void ThrowRockObject::Launch()
{
    if (isLaunched_) return;
    isLaunched_ = true;

    // 値を設定(最終の値が必ず合うようにする)
    transform_.localPosition = growTargetPos_;
    transform_.localScale = targetScale_;
    transform_.UpdateTransform();
    model_.SetPosition(transform_.position);
    model_.SetScale(transform_.scale);

    // コリジョンの作成（投げるまでは当たり判定を発生させない）
    attackHitBox_ = std::make_unique<GhostBody>();
    attackHitBox_->CreateSphere(
        boss_,
        CharacterID::BossThrowRockAtkID(),
        collisionSize_,
        ghost::CollisionAttribute::BossAtk, // 自分の属性
        ghost::CollisionAttributeMask::BossAtk // 当たる相手の属性
    );
    attackHitBox_->SetPosition(transform_.position);

    // 草を曲げる（投げた瞬間に発生させる）
    if (GrassBendManager::IsInitialized())
    {
        if (const auto* gp = ParameterManager::Get().GetGrassBendParam("ThrowRock"))
        {
            GrassBendManager::AttackParams params{ gp->force, gp->radius, gp->duration, gp->recoverySpeed };
            GrassBendManager::Get().AddSource(transform_.position, params);
        }
    }
}

void ThrowRockObject::Update()
{
    if (!isLaunched_) { model_.Update(); return; }

    Culculate();

    transform_.UpdateTransform();
    model_.SetPosition(transform_.position);
    model_.Update();
    attackHitBox_->SetPosition(transform_.position);
}
void ThrowRockObject::Render(RenderContext& rc)
{
    model_.Draw(rc);
}



/*=======================================*/
/* 地雷攻撃 */
/*=======================================*/

void LandmineObject::Setup(Vector3 targetPos, float motionValue)
{
    motionValue_ = motionValue;                                                                  // ダメージ倍率
    transform_.localPosition = targetPos;                                                        // 設置座標
    const float modelScale = ParameterManager::Get().GetAttackObjectParam()->landmineModelScale; // 大きさ
    transform_.localScale = Vector3(modelScale);
    transform_.UpdateTransform();

    predictionEffectHandle_ = INVALID_EFFECT_HANDLE; // エフェクトの無効の値
    phase_ = Phase::enWaiting;
}

LandmineObject::LandmineObject()
{
}

LandmineObject::~LandmineObject()
{
    if (attackRangeIndicator_) {
        DeleteGO(attackRangeIndicator_);
        attackRangeIndicator_ = nullptr;
    }
}

bool LandmineObject::Start()
{
    // モデルの読み込み
    model_.Init("Assets/Objects/Skill/Landmine/GameData/Landmine.tkm");
    model_.SetPosition(transform_.position);
    model_.SetScale(transform_.scale);
    model_.Update();

    // エフェクトサイズ
    const auto* ap = ParameterManager::Get().GetAttackObjectParam();
    Vector3 damageZoneEffectScale = ap->landmineCollisionSize * ap->effectScaleFactorDamageLing;
    Vector3 exploadEffectScale    = ap->landmineCollisionSize * ap->effectScaleFactorExplode;
    float collisionScale          = ap->landmineCollisionSize;


    // タスクスケジューラーを作成
    taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

    // 攻撃範囲の可視化
    taskScheduler_->AddTimer(3.0f, [this]() {
        attackRangeIndicator_ = NewGO<AttackRange>(0, "landmineRange");
        AttackRange::InitParam param;
        param.type       = AttackRange::Type::enCircle;
        param.character  = AttackRange::Character::Boss;
        param.pulseSpeed = 1.0f / 3.0f; 
        attackRangeIndicator_->SetInitParam(param);
        attackRangeIndicator_->SetPosition(transform_.position);
        const float indicatorRadius = ParameterManager::Get().GetAttackObjectParam()->landmineIndicatorRadius;
        attackRangeIndicator_->SetScale(Vector3(indicatorRadius, 1.0f, indicatorRadius));
        attackRangeIndicator_->SetDraw(true);

        phase_ = Phase::enWarning;
        });


    // 爆発
    taskScheduler_->AddTimer(6.0f, [this, exploadEffectScale,collisionScale]() {
        // 攻撃予測インジケーターを削除
        if (attackRangeIndicator_) {
            DeleteGO(attackRangeIndicator_);
            attackRangeIndicator_ = nullptr;
        }

        // 爆発エフェクトを再生
        predictionEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_Expload,
            transform_.position,
            Quaternion::Identity,
            exploadEffectScale
        );

        // 効果音
        SoundManager::Get().PlaySE(enSoundKind_Player_Landmine);

        // 当たり判定の生成
        attackHitBox_ = std::make_unique<GhostBody>();
        attackHitBox_->CreateSphere(
            owner_,
            CharacterID::CharaLandmineAtkID(),
            collisionScale,
            ghost::CollisionAttribute::CharacterAtk, // キャラクター誰でも攻撃可能
            ghost::CollisionAttributeMask::CharacterAtk // キャラクター全員ダメージを受ける
        );
        attackHitBox_->SetPosition(transform_.position);

        // 草を曲げる
        if (GrassBendManager::IsInitialized())
        {
            // jsonファイルからLandmineの草に関する値を取得
            if (const auto* gp = ParameterManager::Get().GetGrassBendParam("Landmine"))
            {
                // 値を設定、反映
                GrassBendManager::AttackParams params{ gp->force, gp->radius, gp->duration, gp->recoverySpeed };
                GrassBendManager::Get().AddSource(transform_.position, params);
            }
        }

        phase_ = Phase::enExpload;
        });

    // 削除
    taskScheduler_->AddTimer(6.1f, [&]() {
        attackHitBox_.reset();
        phase_ = Phase::enDone;
        });
    return true;
}

void LandmineObject::Update()
{
    // タスクスケジューラーの時間を更新
    if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

    // 処理が終了したら
    if (phase_ == Phase::enDone) 
    {
        isFinished_ = true;
    }

    model_.Update();
}

void LandmineObject::Render(RenderContext& rc)
{
    model_.Draw(rc);
}
