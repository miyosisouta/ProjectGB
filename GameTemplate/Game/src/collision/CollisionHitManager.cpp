#include "stdafx.h"
#include "CollisionHitManager.h"
#include "src/Actor/ActorStatus.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/Player.h"
//#include "src/Actor/ActorState.h"
//#include "src/Actor/BattleCharacter.h"
//#include "src/Actor/EventCharacter.h"
//#include "src/Actor/Gimmick.h"
//#include "src/battle/BattleManager.h"
//#include "src/gimmick/WarpSystem.h"
//#include "src/util/ParallelFor.h"


namespace
{
	template <typename T>
	bool IsHitObject(const CollisionHitManager::Pair& pair, const uint32_t id)
	{
		if (pair.a->GetOwnerId() == id) {
			return true;
		}
		if (pair.b->GetOwnerId() == id) {
			return true;
		}
		return false;
	}

	template <typename T>
	T* GetHitObject(CollisionHitManager::Pair& pair, const uint32_t id)
	{
		if (pair.a->GetOwnerId() == id) {
			return static_cast<T*>(pair.a->GetOwner());
		}
		if (pair.b->GetOwnerId() == id) {
			return static_cast<T*>(pair.b->GetOwner());
		}
		return nullptr;
	}
}


CollisionHitManager* CollisionHitManager::instance_ = nullptr;


CollisionHitManager::CollisionHitManager()
{
}


CollisionHitManager::~CollisionHitManager()
{
}


void CollisionHitManager::Update()
{
	// ---- Phase 1: 全ペアの今フレームフラグをリセット ----
	for (auto it = activePairs_.begin(); it != activePairs_.end(); ++it) {
		it->second.flaggedThisFrame = false;
	}

	// ---- Phase 2: 今フレームのヒットペアを処理 ----
	//   GhostBodyManager::Update() → RegisterHitPair() で蓄積済み
	for (size_t i = 0; i < hitPairList_.size(); ++i) {
		Pair& hitPair = hitPairList_[i];
		PairKey key(hitPair.a, hitPair.b);
		auto it = activePairs_.find(key);

		if (it == activePairs_.end()) {
			// ---- 新規ペア → Enter ----
			PairInfo info;
			info.state = CollisionPairState::Enter;
			info.frameCount = 1;
			info.flaggedThisFrame = true;
			activePairs_.insert(std::make_pair(key, info));

			OnCollisionEnter(hitPair.a, hitPair.b);
		}
		else {
			// ---- 既存ペア → Stay ----
			it->second.state = CollisionPairState::Stay;
			it->second.frameCount++;
			it->second.flaggedThisFrame = true;

			OnCollisionStay(hitPair.a, hitPair.b, it->second.frameCount);
		}
	}

	// ---- Phase 3: 今フレームで検出されなかったペア → Exit ----
	for (auto it = activePairs_.begin(); it != activePairs_.end(); ) {
		if (!it->second.flaggedThisFrame) {
			OnCollisionExit(it->first.a, it->first.b);
			it = activePairs_.erase(it);
		}
		else {
			++it;
		}
	}

	hitPairList_.clear();
}


void CollisionHitManager::RegisterHitPair(GhostBody* a, GhostBody* b)
{
	hitPairList_.push_back(Pair(a, b));
}


void CollisionHitManager::OnBodyRemoved(GhostBody* body)
{
	// activePairs_ から該当ボディを含むペアを全て除去
	// ※ Exit通知は行わない（ボディが既に無効なため）
	for (auto it = activePairs_.begin(); it != activePairs_.end(); ) {
		if (it->first.a == body || it->first.b == body) {
			it = activePairs_.erase(it);
		}
		else {
			++it;
		}
	}

	// hitPairList_ からも除去（まだUpdate前に溜まっている可能性がある）
	auto newEnd = std::remove_if(hitPairList_.begin(), hitPairList_.end(),
		[body](const Pair& p) {
			return p.a == body || p.b == body;
		});
	hitPairList_.erase(newEnd, hitPairList_.end());
}


// =====================================================================
// Enter / Stay / Exit コールバック
// =====================================================================

void CollisionHitManager::OnCollisionEnter(GhostBody* a, GhostBody* b)
{
	// --- 一度だけ実行したい処理をここに書く ---
	Pair pair(a, b);

	if (ContainsPlayerAttackPair(pair)) {
		UpdatePlayerAttackPair(pair);
	}
	if (ContainsBossAttackPair(pair)) {
		UpdateBossAttackPair(pair);
	}

	//// 土管ギミック（Enter時のみ判定）
	//if (ContainsPipeGimmickPair(pair)) {
	//	UpdatePipeGimmickPair(pair);
	//}

	//// イベントキャラクター（Enter時のみ判定）
	//if (ContainsEventCharacterPair(pair)) {
	//	UpdateEventCharacterPair(pair);
	//}

	// TODO: その他のワンショット処理
	// 例: コイン取得、SE再生、ヒットエフェクト生成 など
}


void CollisionHitManager::OnCollisionStay(GhostBody* a, GhostBody* b, int frameCount)
{
	// --- 継続的に実行したい処理をここに書く ---

	// TODO: 継続ダメージなどの例
	// 例: 毒沼の継続ダメージ（Nフレームごとに適用）
	//
	// const int DAMAGE_INTERVAL = 60; // 60フレームごと
	// if (frameCount % DAMAGE_INTERVAL == 0) {
	//     ApplyContinuousDamage(a, b);
	// }
}


void CollisionHitManager::OnCollisionExit(GhostBody* a, GhostBody* b)
{
	// --- 離脱時に一度だけ実行したい処理をここに書く ---

	// TODO: バフ解除、エリア退出通知 など
}


//// =====================================================================
//// 既存のペア判定ロジック（変更なし）
//// =====================================================================

/* ============================================ */
/* プレイヤーが攻撃 */
/* ============================================ */

bool CollisionHitManager::ContainsPlayerAttackPair(const Pair& hitPair)
{
	if (!IsHitObject<Player>(hitPair, CharacterID::PlayerNormalAtkID())) {
		return false;
	}
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossID())) {
		return false;
	}
	return true;
}
void CollisionHitManager::UpdatePlayerAttackPair(Pair& hitPair)
{
	auto* bossCharacter = GetHitObject<BossCharacter>(hitPair, CharacterID::BossID());

	if (bossCharacter->GetStatus()) {
		bossCharacter->GetStatus()->Damage(1);
	}
}


/* ============================================ */
/* ボスが攻撃 */
/* ============================================ */

bool CollisionHitManager::ContainsBossAttackPair(const Pair& hitPair)
{
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossNormalAtkID())) {
		return false;
	}
	if (!IsHitObject<Player>(hitPair, CharacterID::PlayerID())) {
		return false;
	}
	return true;
}
void CollisionHitManager::UpdateBossAttackPair(Pair& hitPair)
{
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());

	player->GetStatus()->Damage(1);
}


bool CollisionHitManager::ContainsBossHitStampPair(const Pair& hitPair)
{
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossHitStampAtkID())) {
		return false;
	}
	if (!IsHitObject<Player>(hitPair, CharacterID::PlayerID())) {
		return false;
	}
	return true;
}

void CollisionHitManager::UpdateBossHitStampPair(Pair& hitPair)
{
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());

	player->GetStatus()->Damage(2);
}




//bool CollisionHitManager::ContainsPipeGimmickPair(const Pair& hitPair)
//{
//	if (!IsHitObject<PipeGimmick>(hitPair)) {
//		return false;
//	}
//	if (!IsHitObject<app::actor::BattleCharacter>(hitPair)) {
//		return false;
//	}
//	return true;
//}


//void CollisionHitManager::UpdatePipeGimmickPair(Pair& hitPair)
//{
//	auto* pipeGimmick = GetHitObject<app::actor::PipeGimmick>(hitPair);
//	auto* battleCharacter = GetHitObject<app::actor::BattleCharacter>(hitPair);

//	app::gimmick::EndpointId targetEndpointId;
//	if (!app::gimmick::WarpSystem::Get().TryResolve(pipeGimmick->GetEndpointId(), targetEndpointId)) {
//		return;
//	}

//	if (battleCharacter->GetStateMachine()->IsActionDown()) {
//		const Vector3 startPosition = pipeGimmick->GetMouthPosition();
//		const Vector3 endPosition = app::gimmick::WarpSystem::Get().FindPipe(targetEndpointId)->GetMouthPosition();
//		battleCharacter->GetStateMachine()->SetWarpPosition(startPosition, endPosition);
//	}
//}


//bool CollisionHitManager::ContainsEventCharacterPair(const Pair& hitPair)
//{
//	if (!IsHitObject<app::actor::EventCharacter>(hitPair)) {
//		return false;
//	}
//	if (!IsHitObject<app::actor::BattleCharacter>(hitPair)) {
//		return false;
//	}
//	return true;
//}


//void CollisionHitManager::UpdateEventCharacterPair(Pair& hitPair)
//{
//	auto* battleCharacter = GetHitObject<app::actor::BattleCharacter>(hitPair);
//	auto* eventCharacter = GetHitObject<app::actor::EventCharacter>(hitPair);

//	Vector3 playerPos = battleCharacter->transform.position;
//	Vector3 slimePos = eventCharacter->transform.position;

//	// パンチされたかのチェック
//	app::collision::GhostBody* colliedPlayerBody = nullptr;
//	if (hitPair.a->GetOwnerId() == app::actor::BattleCharacter::ID())
//	{
//		colliedPlayerBody = hitPair.a;
//	}
//	else if (hitPair.b->GetOwnerId() == app::actor::BattleCharacter::ID())
//	{
//		colliedPlayerBody = hitPair.b;
//	}

//	// パンチの当たり判定の時
//	if (colliedPlayerBody != nullptr
//		&& colliedPlayerBody != battleCharacter->GetGhostBody())
//	{
//		// プレイヤーからスライムへのベクトルを計算
//		Vector3 knockBackDirection = slimePos - playerPos;
//		knockBackDirection.y = 0.0f;
//		knockBackDirection.Normalize();
//		// スライムがノックバックした
//		eventCharacter->GetStateMachine()->OnKnockBack(knockBackDirection);
//	}
//	/** プレイヤー本体のゴーストと衝突した時 */
//	else
//	{
//		/** スライムからプレイヤーに向かうベクトル */
//		Vector3 toPlayer = playerPos - slimePos;
//		toPlayer.Normalize();
//		float dot = toPlayer.y;

//		bool isAbove = (dot > 0.1f);

//		// Playerが上空にいるなら
//		if (isAbove)
//		{
//			eventCharacter->GetStateMachine()->OnSquashed();
//		}
//		else
//		{
//			battleCharacter->GetStateMachine()->OnKnockBack();
//		}
//	}
//}


