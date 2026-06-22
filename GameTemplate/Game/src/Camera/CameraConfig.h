/**
 * CameraConfig.h
 * カメラ設定（感度・距離・操作反転）のシングルトン管理
 * KeyConfig と同じパターン。設定画面で値を書き込み、バトル開始時に読み出す。
 */
#pragma once


class CameraConfig
{
public:
    float GetSensitivity() const { return sensitivity_; }
    float GetDistance()    const { return distance_; }
    bool  GetInvert()      const { return invert_; }

    void  SetSensitivity(float v) { sensitivity_ = v; }
    void  SetDistance(float v)    { distance_ = v; }
    void  SetInvert(bool v)       { invert_ = v; }

    /** BattleCommonParameter.json から初期値をロードする（main.cpp から呼ぶ） */
    void Initialize();

    void ResetToDefault()
    {
        sensitivity_ = defaultSensitivity_;
        distance_    = defaultDistance_;
        invert_      = false;
    }


private:
    float sensitivity_        = 0.0f;
    float distance_           = 0.0f;
    bool  invert_             = false;
    float defaultSensitivity_ = 0.0f;
    float defaultDistance_    = 0.0f;


    /*========================================*/
    /* シングルトンアクセス                    */
    /*========================================*/

private:
    CameraConfig() = default;
    ~CameraConfig() = default;

    static CameraConfig* instance_;

public:
    static void CreateInstance()
    {
        if (instance_ == nullptr)
        {
            instance_ = new CameraConfig();
        }
    }

    static CameraConfig& Get()
    {
        return *instance_;
    }

    static void Finalize()
    {
        if (instance_ != nullptr)
        {
            delete instance_;
            instance_ = nullptr;
        }
    }
};
