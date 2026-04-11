/**
 * Stage.h
 *
 * ステージの描画を行う
 */


#pragma once
#include "src/Stage/StageBase.h"
#include "src/Stage/StaticObject.h"

class StageManager
{
private:
	// std::unique_ptr<StageBase> stage_; //!< ギミックのあるステージを作成時に子の変数に入れる
    std::vector<StaticObject*> staticObjectList_;
    std::vector<PhysicalBody*> collisionList_;


public:
	/** tklファイルの読み込みとオブジェクト生成 */
	void StageTKLLoader(const char* path);


public:
	StageManager();
	~StageManager();

	bool Start();
	void Update();
	void Render(RenderContext& rc);

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
public:
    StageManagerObject();
    ~StageManagerObject();

    bool Start();
    void Update();
    void Render(RenderContext& rc);
};