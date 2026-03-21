/**
 * SceneManager.h
 * 
 * シーンの遷移を管理
 */

#pragma once
#include "src/Scene/IScene.h"
#include <map>
#include <functional>


class LoadingScreen;
	

class SceneManager
{
	/** シーンのマップ */
	using SceneMap = std::map<uint32_t, std::function<IScene* ()>>;

private:
	/** 切り替えたいシーンのID */
	uint32_t requestSceneID_ = 0;
	/** タスクシステムのポインタ */
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_;
	/** 現在のシーン */
	IScene* currentScene_ = nullptr;
	/** シーンのマップ */
	SceneMap sceneMap_;
	/** ロード画面クラスのポインタ */
	LoadingScreen* loadingScreen_ = nullptr;
	/** シーン生成のフラグが経ってからの時間 */
	float changeSceneElapsed = 0.0f;
	/** 次のシーン井切り替えた */
	bool isNextScene_ = false;


private:
	SceneManager();
	~SceneManager();


public:
	void Update();
	void Render(RenderContext& rc);


private:
	/** 次のシーンへの切り替え */
	void GnangeNextScene(const uint32_t id);


private:
	/** シーン追加 */
	template <typename T>
	inline void AddSceneMap()
	{
		sceneMap_.emplace(T::ID(), []()
			{
				return new T();
			});
	}
	/** シーン追加 */
	void CreateScene(const uint32_t id);


	/******************************************************************************************/
	//シングルトン用
	/******************************************************************************************/


private:
	/** 自身のインスタンス */
	static SceneManager* myInstance_;


public:
	/** インスタンス作成 */
	static inline void CreateInstance()
	{
		if (!myInstance_) myInstance_ = new SceneManager;
	}

	/** インスタンス削除 */
	static inline void DeleteInstance()
	{
		if (myInstance_) {
			delete myInstance_;
			myInstance_ = nullptr;
		}
	}

	/** インスタンスを取得 */
	static inline SceneManager* GetInstance()
	{
		if (myInstance_) return myInstance_;
	}
};



/******************************************************************************************************/


class SceneMangerObject : public IGameObject
{
public:
	SceneMangerObject();
	~SceneMangerObject();


private:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};