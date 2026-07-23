/**
 * CameraCommon.h
 * カメラの基礎となるデータ構造とインターフェース群
 */
#pragma once


/** NOTE: すべてのカメラコントローラーに付ける */
#define appCameraController(name)\
public:\
	static constexpr uint32_t ID() { return Hash32(#name); }



 /**
  * カメラの姿勢データ
  */
struct CameraData
{
    Vector3 position = Vector3(0.0f, 50.0f, -100.0f); //!< カメラの座標
    Vector3 target = Vector3::Zero; //!< 注視点
    Vector3 up = Vector3::Up; //!< 上方向ベクトル
    float fov = Math::DegToRad(120.0f); //!< 視野角(ラジアン)
    float nearClip = 0.01f; //!< ニアクリップ
    float farClip = 5000.0f; //!< ファークリップ

    /** 線形補間（ブレンド用） */
    static CameraData Lerp(const float t, const CameraData& start, const CameraData& end)
    {
        CameraData result;
        result.position.Lerp(t, start.position, end.position);
        result.target.Lerp(t, start.target, end.target);
        result.up.Lerp(t, start.up, end.up);
        result.fov = Math::Lerp(t, start.fov, end.fov);
        result.nearClip = Math::Lerp(t, start.nearClip, end.nearClip);
        result.farClip = Math::Lerp(t, start.farClip, end.farClip);
        return result;
    }
};




/**
 * カメラコントローラーの基底インターフェース
 */
class ICameraController
{
public:
    /** コンストラクタ */
    ICameraController() = default;
    /** デストラクタ */
    virtual ~ICameraController() = default;

    /** アクティブになった瞬間に呼ばれる */
    virtual void OnEnter() {}

    /** 毎フレーム呼ばれる更新処理 */
    virtual void Update() = 0;

    /** 計算結果を返す */
    virtual const CameraData& GetCameraData() const = 0;

    /** 派生型かどうか確認する */
    template <typename T>
    bool Is() const { return dynamic_cast<T*>(this); }
    /** 派生型にキャストして取得する */
    template <typename T>
    T* As() { return dynamic_cast<T*>(this); }
};
using RefCameraController = std::shared_ptr<ICameraController>; //!< カメラコントローラーの参照型