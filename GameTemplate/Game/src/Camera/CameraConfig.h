/**
 * CameraConfig.h
 * カメラ設定（感度・距離・操作反転）のシングルトン管理
 * KeyConfig と同じパターン。設定画面で値を書き込み、バトル開始時に読み出す。
 */
#pragma once


class CameraConfig
{
private:
    float sensitivity_        = 0.0f;  //!< 感度
    float distance_           = 0.0f;  //!< 距離
    bool  invert_             = false; //!< 操作を反転させるか
    float defaultSensitivity_ = 0.0f;  //!< 感度の初期値
    float defaultDistance_    = 0.0f;  //!< 距離の初期値


public:
    /** 感度の取得 */
    inline float GetSensitivity() const { return sensitivity_; }
    /** 距離の取得 */
    inline float GetDistance()    const { return distance_; }
    /** 反転設定の取得 */
    inline bool  GetInvert()      const { return invert_; }

    /** 感度の設定 */
    inline void  SetSensitivity(float v) { sensitivity_ = v; }
    /** 距離の設定 */
    inline void  SetDistance(float v)    { distance_ = v; }
    /** 反転設定の設定 */
    inline void  SetInvert(bool v)       { invert_ = v; }

    /** BattleCommonParameter.json から初期値をロードする（main.cpp から呼ぶ） */
    void Initialize();

    /** 初期値に戻す */
    void ResetToDefault()
    {
        sensitivity_ = defaultSensitivity_;
        distance_    = defaultDistance_;
        invert_      = false;
    }


    /*========================================*/
    /* シングルトンアクセス                    */
    /*========================================*/

private:
    /** コンストラクタ */
    CameraConfig() = default;
    /** デストラクタ */
    ~CameraConfig() = default;

    static CameraConfig* instance_; //!< インスタンス

public:
    /** インスタンスを生成 */
    static void CreateInstance()
    {
        if (instance_ == nullptr)
        {
            instance_ = new CameraConfig();
        }
    }

    /** インスタンスを取得 */
    static CameraConfig& Get()
    {
        return *instance_;
    }

    /** インスタンスを破棄 */
    static void Finalize()
    {
        if (instance_ != nullptr)
        {
            delete instance_;
            instance_ = nullptr;
        }
    }
};
