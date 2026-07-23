/**
 * BroadphaseInterface.h
 * 広域衝突判定のインターフェース
 */
#pragma once


class GhostBody;


/**
 * Broadphase（広域衝突判定）の抽象インターフェース
 */
class IBroadphase
{
protected:
	using PairCallback = std::function<void(GhostBody* a, GhostBody* b)>;


public:
	/** デストラクタ */
	virtual ~IBroadphase() = default;

	/** 判定対象に追加する */
	virtual void Add(GhostBody* body) = 0;
	/** 判定対象から除外する */
	virtual void Remove(GhostBody* body) = 0;
	/** AABBの変化を反映する */
	virtual void Update(GhostBody* body) = 0;

	/** 全ペアの当たり判定を行い、衝突しているペアをコールバックで通知する */
	virtual void Perform(PairCallback callback) = 0;
};