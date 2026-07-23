#pragma once

/**
 * 攻撃範囲インジケータークラス
 *
 * 3DsMax で作った板ポリ(tkm)を使い、上からテクスチャを貼って攻撃予測を可視化する。
 * 現在エフェクトで行っていた攻撃予測を置き換えることを目的とする。
 *
 * 使い方:
 *   attackRange_ = NewGO<AttackRange>(0, "attackRange");
 *   AttackRange::InitParam param;
 *   param.type        = AttackRange::Type::Circle;
 *   param.texturePath = L"Assets/Objects/AttackRange/Texture/Boss/DamageZone_Circl_Boss.DDS";
 *   attackRange_->SetInitParam(param);
 *   attackRange_->SetScale(Vector3(radius, 1.0f, radius)); // XZ でスケール
 * 
 *   // --- 攻撃予測を表示したいとき ---
 *   attackRange_->SetPosition(pos);
 *   attackRange_->SetDraw(true);
 * 
 *   // --- 攻撃開始で非表示 ---
 *   attackRange_->SetDraw(false);
 * 
 *   // --- ステート終了時 ---
 *   DeleteGO(attackRange_);
 */
class AttackRange : public IGameObject
{
public:
    /** メッシュの種類 */
    enum class Type { enNone, enCircle, enBox, enLine };
    /** テクスチャの対象キャラクター */
    enum class Character { Boss, Player };

    /** 初期化パラメータ */
    struct InitParam
    {
        Type      type              = Type::enNone;
        Character character         = Character::Boss;
        float     emissiveIntensity = 1.5f; //!< パルスリングの明るさの倍率
        float     lineThreshold     = 0.5f; //!< この alpha 以上のピクセルを「線」として DDS 色で描画する
        float     pulseSpeed        = 0.5f; //!< Circle 専用： パルスリングの速さ (cycles/秒)。0.5 = 2秒で中央→外側を1往復
        float     baseSizeX         = 100.0f; //!< box・Circle 専用：ベースとなるXのスケール
        float     baseSizeZ         = 100.0f; //!< box・Circle 専用：ベースとなるZのスケール
    };

private:
    /** 使用するファイルのパスをコンパイル時に設定 */
    static constexpr const char* SHADER_PATH            = "Assets/shader/attackRange.fx";
    static constexpr const char* SHADER_PATH_BOX        = "Assets/shader/attackRangeBox.fx";
    static constexpr const char* SHADER_PATH_LINE       = "Assets/shader/attackRangeLine.fx";
    static constexpr const char* SHADER_PATH_LINE_SLICE = "Assets/shader/attackRangeLine9Slice.fx";
    static constexpr const char* MESH_CIRCLE  = "Assets/Objects/AttackRange/Mesh/DamageZone_Circle.tkm";
    static constexpr const char* MESH_BOX     = "Assets/Objects/AttackRange/Mesh/DamageZone_Box.tkm";
    static constexpr const char* MESH_LINE    = "Assets/Objects/AttackRange/Mesh/DamageZone_Line.tkm";

    static constexpr const wchar_t* TEX_BOSS_CIRCLE  = L"Assets/Objects/AttackRange/Texture/Boss/DamageZone_Circle_Boss.DDS";
    static constexpr const wchar_t* TEX_BOSS_BOX     = L"Assets/Objects/AttackRange/Texture/Boss/DamageZone_Box_Boss.DDS";
    static constexpr const wchar_t* TEX_BOSS_LINE    = L"Assets/Objects/AttackRange/Texture/Boss/DatameZome_Line_Boss.DDS";
    static constexpr const wchar_t* TEX_PLAYER_CIRCLE = L"Assets/Objects/AttackRange/Texture/Player/DamageZone_Circle_Player.DDS";
    static constexpr const wchar_t* TEX_PLAYER_BOX    = L"Assets/Objects/AttackRange/Texture/Player/DamageZone_Box_Player.DDS";
    static constexpr const wchar_t* TEX_PLAYER_LINE   = L"Assets/Objects/AttackRange/Texture/Player/DatameZome_Line_Player.DDS";


    ModelRender model_;     //!< 板ポリのモデル
    Texture     texture_;   //!< 差し替え用テクスチャ
    Transform   transform_; //!< トランスフォーム
    InitParam   initParam_; //!< 初期化パラメータ
    float       pulseTime_        = 0.0f; //!< パルスリング用の経過時間
    float       lineDrawProgress_ = 0.0f; //!< [Line専用] 描画進捗
    bool        isDraw_           = false; //!< 表示中か
    bool        isInited_         = false; //!< 初期化済みか


private:
    /** メッシュ種類からメッシュパスを取得 */
    static const char*    GetMeshPath(Type type);
    /** メッシュ種類とキャラクターからテクスチャパスを取得 */
    static const wchar_t* GetTexturePath(Type type, Character character);

public:
    /**
     * [Circle 専用] カウントダウン時間を渡すと、その秒数でパルスリングが
     * 外円にちょうど到達するよう pulseSpeed を自動設定する。SetDraw(true) の前に呼ぶこと。
     */
    void SetPulseCountdown(float durationSeconds)
    {
        if (durationSeconds > 0.0f)
            initParam_.pulseSpeed = 1.0f / durationSeconds;
    }
    /** 現在の表示状態を取得する。 */
    inline bool GetDraw() const { return isDraw_; }
    /**
     * [Line 専用] 描画進捗を設定する (0.0=非表示, 1.0=全表示)。
     * SpinState の Update() から毎フレーム呼ぶ。
     */
    inline void SetLineDrawProgress(float progress) { lineDrawProgress_ = progress; }


public:
    /** 初期化パラメータを設定する (NewGO の直後に呼ぶ。Start() より前でよい)。 */
    inline void SetInitParam(const InitParam& param) { initParam_ = param; }

    /** スタート処理 */
    bool Start()                override;
    /** 更新処理 */
    void Update()               override;
    /** 描画処理 */
    void Render(RenderContext& rc) override;

    /** ワールド座標を設定する。 */
    inline void SetPosition(const Vector3& pos) { transform_.localPosition = pos; }
    /** 回転を設定する。 */
    inline void SetRotation(const Quaternion& rot) { transform_.localRotation = rot; }
    /**
     * スケールを設定する。
     * Box/Line では中央部タイリングのため、スケール変更時に自動でタイリングを更新する。
     */
    void SetScale(const Vector3& scale);

    /**
     * 表示・非表示を切り替える。
     * true にした瞬間 pulseTime_ をリセットし、パルスリングが中央から再開する。
     */
    void SetDraw(bool isDraw)
    {
        if (isDraw && !isDraw_) pulseTime_ = 0.0f;
        isDraw_ = isDraw;
    }
};
