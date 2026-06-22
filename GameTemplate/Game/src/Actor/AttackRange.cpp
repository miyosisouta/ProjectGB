#include "stdafx.h"
#include "AttackRange.h"

const char* AttackRange::GetMeshPath(Type type)
{
    switch (type)
    {
    case Type::enCircle: return MESH_CIRCLE;
    case Type::enBox:    return MESH_BOX;
    case Type::enLine:   return MESH_LINE;
    default:           return MESH_CIRCLE;
    }
}

const wchar_t* AttackRange::GetTexturePath(Type type, Character character)
{
    if (character == Character::Boss)
    {
        switch (type)
        {
        case Type::enCircle: return TEX_BOSS_CIRCLE;
        case Type::enBox:    return TEX_BOSS_BOX;
        case Type::enLine:   return TEX_BOSS_LINE;
        default:           return TEX_BOSS_CIRCLE;
        }
    }
    else
    {
        switch (type)
        {
        case Type::enCircle: return TEX_PLAYER_CIRCLE;
        case Type::enBox:    return TEX_PLAYER_BOX;
        case Type::enLine:   return TEX_PLAYER_LINE;
        default:           return TEX_PLAYER_CIRCLE;
        }
    }
}

bool AttackRange::Start()
{
    // 攻撃インジケーターはシャドウキャスト不要・レイトレース対象外
    model_.SetShadowCasterFlag(false);
    model_.SetRaytracingWorld(false);
    const char* shaderPath = (initParam_.type == Type::enBox)  ? SHADER_PATH_BOX  :
                             (initParam_.type == Type::enLine) ? SHADER_PATH_LINE :
                                                                 SHADER_PATH;
    model_.SetGBufferFxOverride(shaderPath);
    model_.Init(GetMeshPath(initParam_.type), nullptr, 0, enModelUpAxisZ, false);

    // TKM に焼き込まれた Player テクスチャを Boss/Player DDS に差し替える
    texture_.InitFromDDSFile(GetTexturePath(initParam_.type, initParam_.character), DirectX::DDS_LOADER_FORCE_SRGB);
    if (texture_.IsValid())
        model_.ChangeGBufferAlbedoMap(texture_);

    // b2 経由でシェーダーパラメータを渡す (param0=emissiveIntensity, param1=lineThreshold)
    model_.SetSplatColorParams(initParam_.emissiveIntensity, initParam_.lineThreshold, 0.0f, 0.0f);

    // Line mesh extends along local X; rotate +90deg around Y to align X with world Z
    if (initParam_.type == Type::enLine)
    {
        Quaternion offset;
        offset.SetRotationY(-1.5707963f);
        transform_.localRotation = transform_.localRotation * offset;
    }

    // 初期トランスフォームをモデルに反映
    transform_.UpdateTransform();
    model_.SetTRS(transform_.position, transform_.rotation, transform_.scale);
    model_.Update();

    isInited_ = true;
    return true;
}

void AttackRange::Update()
{
    if (!isInited_) return;

    // タイプ別にシェーダーパラメータを更新する
    if (initParam_.type == Type::enBox)
    {
        // ボックス 9スライス: param0=tilingU, param1=tilingV (スケール / 基準サイズ で自動計算)
        float tilingU = (initParam_.baseSizeX > 0.0f)
                        ? transform_.localScale.x / initParam_.baseSizeX : 1.0f;
        float tilingV = (initParam_.baseSizeZ > 0.0f)
                        ? transform_.localScale.z / initParam_.baseSizeZ : 1.0f;
        model_.SetSplatColorParams(tilingU, tilingV, 0.0f, 0.0f);
    }
    else if (initParam_.type == Type::enLine)
    {
        // Line: param2=drawProgress, param3=uvTilingY (localScale.x / baseSizeX, length axis is X)
        float uvTilingY = (initParam_.baseSizeX > 0.0f)
                          ? transform_.localScale.x / initParam_.baseSizeX : 1.0f;
        model_.SetSplatColorParams(
            initParam_.emissiveIntensity,
            initParam_.lineThreshold,
            lineDrawProgress_,
            uvTilingY
        );
    }
    else
    {
        // サークル (デフォルト): param2=時間 (パルスリング用), param3=pulseSpeed
        // 表示中のみ加算（非表示中にズレるのを防ぐ）
        if (isDraw_) pulseTime_ += g_gameTime->GetFrameDeltaTime();
        model_.SetSplatColorParams(
            initParam_.emissiveIntensity,
            initParam_.lineThreshold,
            pulseTime_,
            initParam_.pulseSpeed
        );
    }

    transform_.UpdateTransform();
    model_.SetTRS(transform_.position, transform_.rotation, transform_.scale);
    model_.Update();
}

void AttackRange::Render(RenderContext& rc)
{
    if (!isDraw_ || !isInited_) return;
    model_.Draw(rc);
}

void AttackRange::SetScale(const Vector3& scale)
{
    transform_.localScale = scale;
}
