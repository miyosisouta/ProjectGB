/**
 * GhostBodyManager.h
 * ゴーストボディ管理関連
 */
#pragma once
#include "BroadphaseInterface.h"



/**
 * ゴーストボディ管理クラス
 */
class GhostBodyManager
{
private:
	using RegisterPairCallback = std::function<void(GhostBody* a, GhostBody* b)>;


private:
	/** Broadphaseアルゴリズム（抽象化） */
	std::unique_ptr<IBroadphase> broadphase_;

	/** 管理リスト（Dirtyチェック用） */
	std::vector<GhostBody*> bodyList_;

	/** 衝突ペア登録コールバック */
	RegisterPairCallback registerPairCallback_ = nullptr;


private:
	/** コンストラクタ */
	GhostBodyManager();
	/** デストラクタ */
	~GhostBodyManager();

	/** 衝突ペアの処理 */
	void ProcessCollisionPair(GhostBody* a, GhostBody* b);

	/** Bulletを使った詳細判定 */
	bool CheckCollisionBullet(GhostBody* a, GhostBody* b);


public:
	/** 更新処理 */
	void Update();

	/** 判定対象に登録する */
	void AddBody(GhostBody* body);
	/** 判定対象から除外する */
	void RemoveBody(GhostBody* body);

	/** 衝突ペア登録コールバックを設定 */
	inline void RegisterCallback(const RegisterPairCallback& callback) { registerPairCallback_ = callback; }
	/** 衝突ペア登録コールバックを解除 */
	inline void ClearCallback() { registerPairCallback_ = nullptr; }



	/**
	 * シングルトン関連
	 */
private:
	static GhostBodyManager* m_instance; //!< インスタンス


public:
	/** インスタンスを生成 */
	static void Initialize() { if (!m_instance) m_instance = new GhostBodyManager(); }
	/** インスタンスを破棄 */
	static void Finalize() { if (m_instance) { delete m_instance; m_instance = nullptr; } }
	/** インスタンスを取得 */
	static GhostBodyManager& Get() { return *m_instance; }
	/** インスタンスが利用可能か */
	static bool IsAvailable() { return m_instance != nullptr; }
};


