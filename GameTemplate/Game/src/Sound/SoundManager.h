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


/** 音量の種別 */
enum class SoundVolumeType : uint8_t
{
    Master, //!< マスター音量
    BGM,    //!< BGM音量
    SE,     //!< SE音量
    Max,
};


static constexpr uint8_t DEFAULT_VOLUME = 3;
static constexpr uint8_t MAX_VOLUME = 10;



/**
 * サウンド管理クラス
 */
class SoundManager
{
private:
    /** BGM用のサウンドソースインスタンスを保持 */
    SoundSource* bgm_ = nullptr;
    /** BGM音量スケール（ユーザー設定音量に掛け合わせる倍率） */
    float bgmVolumeScale_ = 1.0f;
    /** SE用のサウンドソースインスタンスを保持 */
    std::map<SoundHandle, SoundSource*> seList_;
    /**
     * マップで参照するようにハンドル数を保持
     * NOTE: 各サウンドソースを参照するための数値はユニークな数値になる
     */
    SoundHandle soundHandleCount_ = 0;

	uint8_t volumes_[static_cast<size_t>(SoundVolumeType::Max)] = { DEFAULT_VOLUME, DEFAULT_VOLUME, DEFAULT_VOLUME }; //!< 種別ごとの音量(0～10)
	bool isChangeVolume_ = false; //!< 音量変更があったか（再生中の音に反映するため）



private:
    /** コンストラクタ */
    SoundManager();
    /** デストラクタ */
    ~SoundManager();


public:
    /**
     * 更新処理
     * NOTE: 不要になったインスタンスの削除など
     */
    void Update();

    /**
     * 初期設定
     * タイトルに戻る時に初期化
     */
    void ResetToDefault();


public:
    /** BGM再生 */
    void PlayBGM(const int kind);
    /** BGM停止 */
    void StopBGM();
    /** 現在のBGM音量にスケールを掛ける（PlayBGM呼び出し後に使用） */
    void SetBGMVolumeScale(float scale)
    {
        bgmVolumeScale_ = scale;
        if (bgm_) bgm_->SetVolume(ComputeVolume(SoundVolumeType::BGM) * bgmVolumeScale_);
    }

    /**
     * SE再生
     * @param volumeScale 通常の音量計算(マスター×SE)に掛ける追加の倍率。
     *                    XAudio2のボイス音量は1.0を超えて指定できる（増幅）ため、
     *                    素材音源の収録音量が小さいSEだけ底上げしたい場合に使う。
     */
    SoundHandle PlaySE(const int kind, const bool isLood = false, const bool is3D = false, const float volumeScale = 1.0f);
    /** SE停止 */
    void StopSE(const SoundHandle handle);
    /** 再生中の（ループ）SEをすべて停止する。カットシーン開始時などに使用する */
    void StopAllSE();

#ifdef K2_DEBUG
    /** BGM と ループSE をその場で一時停止する */
    void PauseAll();
    /** PauseAll() で止めた音を再開する */
    void ResumeAll();
#endif


    /** ハンドルからSEのサウンドソースを探す */
    SoundSource* FindSE(const SoundHandle handle)
    {
        auto it = seList_.find(handle);
        if (it != seList_.end()) {
            return it->second;
        }
        K2_ASSERT(false, "削除済みか追加されていないSEにアクセスしようとしています。\n");
        return nullptr;
    }


    /** 種別ごとの音量を設定(0～10) */
    void SetVolume(const SoundVolumeType type, const uint8_t volume)
    {
        if (volume > MAX_VOLUME) {
            K2_ASSERT(false, "音量は0～10の範囲で指定してください。\n");
            return;
        }
        volumes_[static_cast<size_t>(type)] = volume;

        isChangeVolume_ = true;
	}


    /** 種別ごとの音量を取得 */
    inline uint8_t GetVolume(const SoundVolumeType type) const
    {
        return volumes_[static_cast<size_t>(type)];
	}


    /** マスター×指定種別の実際の音量係数(0.0～1.0)を計算する */
    float ComputeVolume(const SoundVolumeType type) const
    {
        // マスターボリュームを基準に、BGMとSEのボリュームを掛け合わせる
		const float masterVolume = static_cast<float>(volumes_[static_cast<size_t>(SoundVolumeType::Master)]) / static_cast<float>(MAX_VOLUME);
		const float typeVolume = static_cast<float>(volumes_[static_cast<size_t>(type)]) / static_cast<float>(MAX_VOLUME);
		return masterVolume * typeVolume;
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
    /** コンストラクタ */
    SoundManagerObject();
    /** デストラクタ */
    ~SoundManagerObject();


public:
    /** スタート処理 */
    bool Start()override;
    /** 更新処理 */
    void Update()override;

#ifdef K2_DEBUG
private:
    static bool s_isUpdatePaused; //!< 更新の一時停止フラグ
public:
    /** デバッグ用: 更新の一時停止を設定 */
    static inline void SetUpdatePaused(bool isPaused) { s_isUpdatePaused = isPaused; }
    /** デバッグ用: 一時停止中か */
    static inline bool IsUpdatePaused() { return s_isUpdatePaused; }
#endif
};