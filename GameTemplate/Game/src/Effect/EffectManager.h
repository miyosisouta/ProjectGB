/**
 * EffectManager.h
 * 
 * エフェクト管理
 * 必要なエフェクトファイルを読み込んだり再生したりなど管理する
 */


#pragma once
#include "src/effect/Types.h"

    
 /** エフェクト用のハンドル名 */
using EffectHandle = uint32_t;
/** ハンドル無効値 */
static constexpr EffectHandle INVALID_EFFECT_HANDLE = 0xffffffff;


/**
 * エフェクト管理クラス
 */
class EffectManager
{
private:
    /**
     * エフェクトインスタンスを保持
     * NOTE: ループだけ管理
     *       ループではない場合、K2Engine側で削除処理をしているため
     */
    std::map<EffectHandle, EffectEmitter*> effectList_;
    /**
     * マップで参照するようにハンドル数を保持
     * NOTE: 各サウンドソースを参照するための数値はユニークな数値になる
     */
    EffectHandle effectHandleCount_ = 0;


private:
    EffectManager();
    ~EffectManager();


public:
    /**
     * 更新処理
     * NOTE: 不要になったインスタンスの削除など
     */
    void Update();


public:
    /** エフェクト再生 */
    EffectHandle PlayEffect(const int kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale);
    /** エフェクト停止 */
    void StopEffect(const EffectHandle handle);

    /**
     * 追従エフェクトの位置を更新する
     * NOTE: ループエフェクトをボスなどに追従させるために毎フレーム呼ぶ
     */
    void SetEffectPosition(const EffectHandle handle, const Vector3& pos);
    /**
     * 追従エフェクトの回転を更新する
     */
    void SetEffectRotation(const EffectHandle handle, const Quaternion& rot);
    /**
     * 追従エフェクトの大きさを更新する
     * NOTE: ループエフェクトをボスなどに追従させるために毎フレーム呼ぶ
     */
    void SetEffectScale(const EffectHandle handle, const Vector3& pos);
    

    EffectEmitter* FindEffect(const EffectHandle handle)
    {
        auto it = effectList_.find(handle);
        if (it != effectList_.end()) {
            return it->second;
        }
        K2_ASSERT(false, "削除済みか追加されていないエフェクトにアクセスしようとしています。\n");
        return nullptr;
    }



    /**
     * シングルトン用
     */
public:
    /** 自身のインスタンス */
    static EffectManager* myInstance_;

    /**
     * インスタンスを作る
     */
    static void CreateInstance()
    {
        if (!myInstance_)  myInstance_ = new EffectManager();
    }


    /**
     * インスタンスを取得
     */
    static EffectManager& Get()
    {
        return *myInstance_;
    }


    /**
     * インスタンスを破棄
     */
    static void DestroyInstance()
    {
        if (myInstance_ != nullptr)
        {
            delete myInstance_;
            myInstance_ = nullptr;
        }
    }
};


/******************************************************************************/


/** エフェクトマネージャーを管理するゲームオブジェクト */
class EffectManagerObject :public IGameObject
{
public:
    EffectManagerObject();
    ~EffectManagerObject();


public:
    bool Start()override;
    void Update()override;
};