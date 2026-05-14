/**
 * Stage.h
 *
 * ステージの描画を行う
 */


#pragma once
#include "src/Stage/StageBase.h"
#include "src/Stage/StaticObject.h"
#include "src/Stage/StageCullingSystem.h"

class StageManager
{
private:
    std::vector<StaticObject*> staticObjectList_; // 静的オブジェクトのリスト
    std::vector<StaticObject*> grassObjectList_; // 草の静的オブジェクトリスト
    std::vector<PhysicalBody*> collisionList_; // コリジョン用のリスト

    std::unique_ptr<StageCullingSystem> stageCullingSystem_; // ステージのカリングシステム

#if defined(_DEBUG)
    bool isDisableGlass = false;
#endif


private:
    Vector3 grassAreaPos_[2]; //!< 草を生成する範囲を計算するために必要な座標の配列

public:
    /** 草を生成する範囲を計算するために必要な座標を取得 */
    Vector3 GetGrassAreaPos(int index) const { return grassAreaPos_[index]; }

public:
	/** tklファイルの読み込みとオブジェクト生成 */
	void StageTKLLoader(const char* path);


public:
	StageManager();
	~StageManager();

	bool Start();
	void Update();
	void Render(RenderContext& rc);

#if defined(_DEBUG)
    void DisableGlass() { isDisableGlass = false; }
#endif

/**
 * シングルトン関連
 */
private:
    static StageManager* instance_;


public:
    static void Initialize()
    {
        if (instance_ == nullptr) {
            instance_ = new StageManager();
        }
    }
    static StageManager& Get() { return *instance_; }

    static void Finalize()
    {
        if (instance_ != nullptr) {
            delete instance_;
            instance_ = nullptr;
        }
    }
};

class StageManagerObject : public IGameObject
{
private:
	bool isUpdate_ = true;  //!< 更新するか否か

public:
    StageManagerObject();
    ~StageManagerObject();

    bool Start();
    void Update();
    void Render(RenderContext& rc);

	/** 更新の可否を設定する */
	void SetUpdate(const bool flg) { isUpdate_ = flg; }
};