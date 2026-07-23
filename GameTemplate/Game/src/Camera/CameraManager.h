/**
 * CameraManager.h
 * カメラ管理
 */
#pragma once
#include "CameraCommon.h"


/** カメラ感度の変化 11段階（段階1~11) */
static constexpr int SENSITIVITY_MIN_STAGE = 1;
static constexpr int SENSITIVITY_MAX_STAGE = 11;
static constexpr int SENSITIVITY_DEFAULT_STAGE = 6; // 初期値

static constexpr float SENSITIVITY_STEP = 0.29f; //0.1f~3.0fを作るときに間が0.29になってしまう
static constexpr float SENSITIVITY_BASE = -0.19f; // 段階1で感度が0.1fになるように調整するための数値
static constexpr float SENSITIVITY_MIN = SENSITIVITY_BASE + (SENSITIVITY_MIN_STAGE * SENSITIVITY_STEP); // 0.1f
static constexpr float SENSITIVITY_MAX = SENSITIVITY_BASE + (SENSITIVITY_MAX_STAGE * SENSITIVITY_STEP); // 3.0f
static constexpr float SENSITIVITY_DEFAULT = SENSITIVITY_BASE + (SENSITIVITY_DEFAULT_STAGE * SENSITIVITY_STEP); // 1.55f

/** カメラ距離オプション (3段階) */
static constexpr float DISTANCE_NEAR = 300.0f; // ちかい
static constexpr float DISTANCE_NORMAL = 600.0f; // ふつう
static constexpr float DISTANCE_FAR = 900.0f; // とおい

/** カメラ反転オプション */
static constexpr bool INVERT_OFF = false; // 反転OFF(初期値)
static constexpr bool INVERT_ON = true; // 反転ON

class CameraManager
{
private:
    std::map<uint32_t, std::shared_ptr<ICameraController>> controllers_; //!< コントローラーの一覧
    std::shared_ptr<ICameraController> current_; //!< 現在アクティブなコントローラ
    std::shared_ptr<ICameraController> next_; //!< 次アクティブになるコントローラ

#if defined(_DEBUG)
    std::shared_ptr<ICameraController> prev_; //!< ひとつ前のコントローラー
#endif


    /** ブレンド用 */
    bool       isBlending_    = false; //!< ブレンド中か
    float      blendDuration_ = 0.0f;  //!< ブレンドにかける時間
    float      blendTimer_    = 0.0f;  //!< ブレンド経過時間
    CameraData blendStartData_;        //!< ブレンド開始時点のカメラデータ

    /** 描画で使用されるカメラ */
    nsK2EngineLow::Camera* engineCamera_ = nullptr; //!< エンジン側の実体カメラ

    /** カメラオプション設定値 */
    int   sensitivityStage_ = SENSITIVITY_DEFAULT_STAGE; //!< 感度の段階(1~11)
    float distance_         = DISTANCE_NORMAL;           //!< 距離(ちかい・ふつう・とおい)
    bool  invert_           = INVERT_OFF;                //!< 反転on/off

private:
    /** コンストラクタ */
    CameraManager();
    /** デストラクタ */
    ~CameraManager();


public:
    /**
     * 初期化
     * エンジン側の実体カメラを設定
     */
    void Setup(nsK2EngineLow::Camera* engineCamera);

    /**
     * 初期設定
     * タイトルに戻る時に初期化
     */
    void ResetToDefault();

    /**
     * 更新処理
     * ブレンド計算とエンジンカメラへの反映
     */
    void Update(const float deltaTime);

    /** コントローラーの登録・解除 */
    void Register(const uint32_t nameHash, RefCameraController controller);
    void Unregister(const uint32_t nameHash);

    /**
     * カメラ切り替え
     * NOTE: blendTime秒かけて遷移し0なら即時。
     */
    void SwitchCamera(const uint32_t nameHash, const float blendTime = 0.0f);
    void SwitchCamera(RefCameraController controller, const float blendTime = 0.0f);

#if defined(_DEBUG)
    /** デバッグ用: 前のカメラに戻す */
    void SwitchPrevCamera(const float blendTime = 0.0f)
    {
        if (prev_) {
            SwitchCamera(prev_, blendTime);
        }
    }
#endif // _DEBUG

    /** 現在のカメラデータを取得 */
    const CameraData& GetCurrentCameraData() const
    {
        if (current_) {
            return current_->GetCameraData();
        }
        static CameraData defaultData;
        return defaultData;
    }


public:
    /** カメラ感度 */
    void SetSensitivityStage(const int stage)
    {
        if (stage < SENSITIVITY_MIN_STAGE || stage > SENSITIVITY_MAX_STAGE) { return; }
        sensitivityStage_ = stage;
    }
    inline int GetSensitivityStage() const { return sensitivityStage_; }
    inline float GetSensitivityValue() const { return SENSITIVITY_BASE + (sensitivityStage_ * SENSITIVITY_STEP); }

    /** カメラ距離 */
    inline void SetDistance(const float distance) { distance_ = distance; }
    inline float GetDistance() const { return distance_; }

    /** カメラ反転 */
    inline void SetInvert(const bool flg) { invert_ = flg; }
    inline bool GetInvert() const { return invert_; }


    /**
     * シングルトン関連
     */
private:
    static CameraManager* instance_; //!< インスタンス


public:
    /** インスタンスを生成 */
    static void Initialize()
    {
        if (instance_ == nullptr) {
            instance_ = new CameraManager();
        }
    }
    /** インスタンスを取得 */
    static CameraManager& Get() { return *instance_; }

    /** インスタンスを破棄 */
    static void Finalize()
    {
        if (instance_ != nullptr) {
            delete instance_;
            instance_ = nullptr;
        }
    }
};