/**
 * CameraController.h
 * カメラコントローラー群
 */
#pragma once
#include "CameraCommon.h"


 /**
  * ゲーム内で外部から状態をセットして使うタイプのカメラコントローラー
  */
class GameCamera : public ICameraController
{
    appCameraController(GameCamera);


private:
    CameraData data_; //!< カメラのデータ
	bool isUpdate_ = true; //!< 更新の可否状態


public:
    /**
     * 外部から状態をセットする
     * NOTE: BattleManagerなどが呼ぶ
     */
    inline void SetState(const CameraData& data)
    {
        data_ = data;
    }

    /** 視野角の設定 */
    inline void SetFovDeg(const float deg)
    {
        data_.fov = Math::DegToRad(deg);
    }

    /** 視野角の取得 */
    inline float GetFovDeg() const
    {
        return Math::RadToDeg(data_.fov);
    }

    /** 更新処理 */
    void Update() override
    {
        // 基本何もしない。
        // 必要ならここで画面揺れの処理を入れるかも
    }

    /** カメラデータの取得 */
    inline const CameraData& GetCameraData() const override { return data_; }
};




#if defined(_DEBUG)
/**
 * デバッグ用カメラコントローラー
 */
class DebugCamera : public ICameraController
{
    appCameraController(DebugCamera);


private:
    CameraData cameraData_; //!< カメラのデータ


public:
    /** 始まりの処理 */
    void OnEnter() override;

    /** 更新処理 */
    void Update() override;

    /** カメラデータの取得 */
    inline const CameraData& GetCameraData() const override { return cameraData_; }
};
#endif // _DEBUG