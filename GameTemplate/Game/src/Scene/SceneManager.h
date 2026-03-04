/**
 * SceneManager.h
 * 
 * シーンの遷移を管理
 */

#pragma once
#include "src/Scene/IScene.h"


class SceneManager
{
	/** シーンのマップ */
	using SceneMap = std::map<uint32_t, std::function<IScene* ()>>;

private:
	/** 切り替えたいシーンのID */
	uint32_t requestSceneID_ = 0;
	/** 現在のシーン */
	IScene* currentScene_ = nullptr;
	/** シーンのマップ */
	SceneMap sceneMap_;


private:
	SceneManager();
	~SceneManager();


public:
	void Update();


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

