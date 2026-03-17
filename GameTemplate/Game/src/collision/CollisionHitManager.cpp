#include "stdafx.h"
#include "CollisionHitManager.h"
#include "GhostBody.h"

namespace
{
	template <typename T>
	bool IsHitObject(const CollisionHitManager::Pair& pair)
	{
		if (pair.a->GetOwnerId() == T::ID()) {
			return true;
		}
		if (pair.b->GetOwnerId() == T::ID()) {
			return true;
		}
		return false;
	}

	template <typename T>
	T* GetHitObject(CollisionHitManager::Pair& pair)
	{
		if (pair.a->GetOwnerId() == T::ID()) {
			return static_cast<T*>(pair.a->GetOwner());
		}
		if (pair.b->GetOwnerId() == T::ID()) {
			return static_cast<T*>(pair.b->GetOwner());
		}
		return nullptr;
	}
}



CollisionHitManager* CollisionHitManager::instance_ = nullptr;


CollisionHitManager::CollisionHitManager()
{
	/** DEBUG: ワイヤーフレーム描画 */
	//PhysicsWorld::Get().EnableDrawDebugWireFrame();
}


CollisionHitManager::~CollisionHitManager()
{

}


void CollisionHitManager::Update()
{
	//デバッグ用現在のヒットペアの数を出力
	char countBuf[256];
	sprintf_s(countBuf, "--- Hit Pair Count: %zu ---\n", hitPairList_.size());
	OutputDebugStringA(countBuf);

	// GhostBodyのヒット処理で得たペアをもとに処理
	{
		for (auto& hitPair : hitPairList_) {
			char idBuf[256];
			sprintf_s(idBuf, "Collision! A_ID: %u, B_ID: %u\n", hitPair.a->GetOwnerId(), hitPair.b->GetOwnerId());
			OutputDebugStringA(idBuf);
		}
	}
	hitPairList_.clear();
}

void CollisionHitManager::RegisterHitPair(GhostBody* a, GhostBody* b)
{
	// ヒットペア登録
	hitPairList_.push_back(std::move(Pair(a, b)));
}