



#pragma once
class SceneManager
{
	/** シーンのマップ */
	using SceneMap = std::map<uint32_t, std::function<IScene* ()>>;


private:
	SceneManager();
	~SceneManager();


public:
	void Update();


	/******************************************************************************************/
	//シングルトン用
	/******************************************************************************************/


private:
	/** 自身のインスタンス */
	static SceneManager* m_myInstance;


public:
	/** インスタンス作成 */
	static inline void CreateInstance()
	{
		if (!m_myInstance) m_myInstance = new SceneManager;
	}

	/** インスタンス削除 */
	static inline void DeleteInstance()
	{
		if (m_myInstance) {
			delete m_myInstance;
			m_myInstance = nullptr;
		}
	}

	/** インスタンスを取得 */
	static inline SceneManager* GetInstance()
	{
		if (m_myInstance) return m_myInstance;
	}
};

