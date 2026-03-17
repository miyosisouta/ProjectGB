/**
 * CollisionHitManager.h
 * 衝突ヒット管理
 */
#pragma once


class GhostBody;


/**
 * 衝突ヒット管理クラス
 */
class CollisionHitManager
{
private:
	struct Pair
	{
		GhostBody* a = nullptr;
		GhostBody* b = nullptr;
		//
		Pair(GhostBody* bodyA, GhostBody* bodyB)
			: a(bodyA), b(bodyB)
		{
		}
	};


private:
	std::vector<Pair> hitPairList_;

private:
	CollisionHitManager();
	~CollisionHitManager();


public:
	/** 更新 */
	void Update();

	/** 衝突ペア登録 */
	void RegisterHitPair(GhostBody* a, GhostBody* b);


	/**
	 * シングルトン関連
	 */
private:
	static CollisionHitManager* instance_;


public:
	static void Initialize()
	{
		if (!instance_) {
			instance_ = new CollisionHitManager();
		}
	}
	static CollisionHitManager& Get() { return *instance_; }
	static void Finalize()
	{
		if (instance_) {
			delete instance_;
			instance_ = nullptr;
		}
	}
};