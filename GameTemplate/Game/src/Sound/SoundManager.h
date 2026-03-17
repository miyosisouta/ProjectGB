/**
 * SoundManager.h
 * 
 * サウンド管理
 * 必要なサウンドファイルを読み込んだり再生したりなど管理する
 */


#pragma once
#include "src/Sound/Types.h"


 /** サウンド用のハンドル名 */
using SoundHandle = uint32_t;
/** ハンドル無効値 */
static constexpr SoundHandle INVALID_SOUND_HANDLE = 0xffffffff;


/**
 * サウンド管理クラス
 */
class SoundManager
{
private:
    /** BGM用のサウンドソースインスタンスを保持 */
    SoundSource* bgm_ = nullptr;
    /** SE用のサウンドソースインスタンスを保持 */
    std::map<SoundHandle, SoundSource*> seList_;
    /**
     * マップで参照するようにハンドル数を保持
     * NOTE: 各サウンドソースを参照するための数値はユニークな数値になる
     */
    SoundHandle soundHandleCount_ = 0;


private:
    SoundManager();
    ~SoundManager();


public:
    /**
     * 更新処理
     * NOTE: 不要になったインスタンスの削除など
     */
    void Update();


public:
    /** BGM再生 */
    void PlayBGM(const int kind);
    /** BGM停止 */
    void StopBGM();

    /** SE再生 */
    SoundHandle PlaySE(const int kind, const bool isLood = false, const bool is3D = false);
    /** SE停止 */
    void StopSE(const SoundHandle handle);


    SoundSource* FindSE(const SoundHandle handle)
    {
        auto it = seList_.find(handle);
        if (it != seList_.end()) {
            return it->second;
        }
        K2_ASSERT(false, "削除済みか追加されていないSEにアクセスしようとしています。\n");
        return nullptr;
    }



private:
    /** 自身のインスタンス */
    static SoundManager* myInstance_;


public:
    /**
     * インスタンスを作る
     */
    static void CreateInstance()
    {
        if (!myInstance_) myInstance_ = new SoundManager();
    }


    /**
     * インスタンスを取得
     */
    static SoundManager& Get()
    {
        return *myInstance_;
    }


    /**
     * インスタンスを破棄
     */
    static void DestroyInstance()
    {
        if (myInstance_) {
            delete myInstance_;
            myInstance_ = nullptr;
        }
    }
};


/******************************************************************************/


/** サウンドマネージャーを管理するためのオブジェクト */
class SoundManagerObject :public IGameObject
{
public:
    SoundManagerObject();
    ~SoundManagerObject();


public:
    bool Start()override;
    void Update()override;
};