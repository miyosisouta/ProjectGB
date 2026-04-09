#include "stdafx.h"
#include "CollisionHitManager.h"
#include "src/Actor/ActorStatus.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/Player.h"
#include "src/Util/DamageCalculator.h"


namespace
{
	/* テンプレート */
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

	/* 定数 */
	constexpr uint8_t NORMAL_ATTACK_DAMAGE = 10;
	constexpr uint8_t SKILL_ATTACK_DAMAGE = 15;
	constexpr float SPIN_KNOCK_BACK = 300.0f;
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

void CollisionHitManager::UpdateTakeHitSound()
{
	SoundManager::Get().PlaySE(enSoundKind_Player_TakeHit);
}

void CollisionHitManager::UpdateAttackHitSound()
{
	SoundManager::Get().PlaySE(enSoundKind_Player_AttackHit);
}


// =====================================================================
// Enter / Stay / Exit コールバック
// =====================================================================

void CollisionHitManager::OnCollisionEnter(GhostBody* a, GhostBody* b)
{
	// --- 一度だけ実行したい処理をここに書く ---
	Pair pair(a, b);

	// プレイヤーの攻撃
	{
		// 通常攻撃
		if (ContainsPlayerNormalAttackPair(pair)) {
			UpdatePlayerNormalAttackPair(pair);
		}
		// スキル攻撃
		if (ContainsPlayerSkillAttackPair(pair)) {
			UpdatePlayerSkillAttackPair(pair);
		}
	}

	// ボスの攻撃
	{
		// 通常攻撃
		if (ContainsBossAttackPair(pair)) {
			UpdateBossAttackPair(pair);
		}
		// ヒットスタンプ
		if (ContainsBossHitStampPair(pair)) {
			UpdateBossHitStampPair(pair);
		}
		// 回転攻撃
		if (ContainsBossSpinPair(pair)) {
			UpdateBossSpinPair(pair);
		}
	}
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

bool CollisionHitManager::ContainsPlayerNormalAttackPair(const Pair& hitPair)
{
	if (!IsHitObject<Player>(hitPair, CharacterID::PlayerNormalAtkID())) {
		return false;
	}
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossID())) {
		return false;
	}
	return true;
}
void CollisionHitManager::UpdatePlayerNormalAttackPair(Pair& hitPair)
{
	// 通常攻撃
	auto* playerCharacter = GetHitObject<Player>(hitPair, CharacterID::PlayerNormalAtkID());
	auto* bossCharacter = GetHitObject<BossCharacter>(hitPair, CharacterID::BossID());
	if (!bossCharacter || !playerCharacter || !playerCharacter->GetStatus() || !bossCharacter->GetStatus()) return;


	auto playerStatus = playerCharacter->GetStatus()->As<PlayerStatus>();

	if (bossCharacter->GetStatus()) {
		// ダメージ計算
		float motion = playerStatus->GetSkillMotionValues("NormalAttack");
		int damage = Calculate(playerStatus, motion);
		bossCharacter->GetStatus()->Damage(damage);
		UpdateAttackHitSound(); // 攻撃が当たったSEを流す
	}
}



bool CollisionHitManager::ContainsPlayerSkillAttackPair(const Pair& hitPair)
{
	if (!IsHitObject<Player>(hitPair, CharacterID::PlayerSkillAtkID())) {
		return false;
	}
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossID())) {
		return false;
	}
	return true;
}
void CollisionHitManager::UpdatePlayerSkillAttackPair(Pair& hitPair)
{
	// スキル攻撃
	auto* playerCharacter = GetHitObject<Player>(hitPair, CharacterID::PlayerSkillAtkID());
	auto* bossCharacter = GetHitObject<BossCharacter>(hitPair, CharacterID::BossID());
	if (!bossCharacter || !playerCharacter || !playerCharacter->GetStatus() || !bossCharacter->GetStatus()) return;


	auto* playerStatus = playerCharacter->GetStatus()->As<PlayerStatus>();

	if (bossCharacter->GetStatus()) {
		// "SpecialAttack" スロットの威力を取得してダメージ計算
		float motion = playerStatus->GetSkillMotionValues("SpecialAttack");
		int damage = Calculate(playerStatus, motion);
		bossCharacter->GetStatus()->Damage(damage);
		UpdateAttackHitSound(); // 攻撃が当たったSEを流す
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
	auto* boss = GetHitObject<BossCharacter>(hitPair, CharacterID::BossNormalAtkID());
	auto bossStatus = boss->GetStatus()->As<BossStatus>();
	

	// プレイヤーがいるか、プレイヤーのステータスがあるか
	if (player == nullptr || player->GetStatus() == nullptr) { return; }
	// プレイヤーが無敵フラグを持っているか
	PlayerStatus* status = player->GetStatus()->As<PlayerStatus>();
	if (status && status->IsInvincible()) { return; }

	// ボスのステータスがあるならダメージ処理
	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("NormalAttack");
		int damage = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(damage);
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
	}
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
	auto* boss = GetHitObject<BossCharacter>(hitPair, CharacterID::BossHitStampAtkID());
	auto bossStatus = boss->GetStatus()->As<BossStatus>();

	// プレイヤーがいるか、もしくはプレイヤーのステータスがあるか
	if (player == nullptr || player->GetStatus() == nullptr) { return; }
	// プレイヤーが無敵フラグを持っているか
	PlayerStatus* status = player->GetStatus()->As<PlayerStatus>();
	if (status && status->IsInvincible()){ return; }

	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("HitStamp");
		int damage = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(damage); // ダメージを与える
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
	}
}



bool CollisionHitManager::ContainsBossSpinPair(const Pair& hitPair)
{
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossSpinAtkID())) {
		return false;
	}
	if (!IsHitObject<Player>(hitPair, CharacterID::PlayerID())) {
		return false;
	}
	return true;
}
void CollisionHitManager::UpdateBossSpinPair(Pair& hitPair)
{
	// Pairの二人のクラスを取得
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());
	auto* boss = GetHitObject<BossCharacter>(hitPair, CharacterID::BossSpinAtkID());
	auto bossStatus = boss->GetStatus()->As<BossStatus>();

	// プレイヤーがいるかつプレイヤーのステータスがあるかつボスがいいなら続ける
	if (player == nullptr || player->GetStatus() == nullptr || boss == nullptr) { return; }
	PlayerStatus* status = player->GetStatus()->As<PlayerStatus>();
	// プレイヤーが無敵フラグを持っているか
	if (status && status->IsInvincible()){ return; }

	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("SpinAttack");
		int damage = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(damage);
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
	}
	

	// 当たった二人の座標を取得
	Vector3 playerPos = player->GetTransformPosition();
	Vector3 bossPos = boss->GetTransformPosition();

	// 方向を取得
	Vector3 dir= playerPos - bossPos;
	dir.Normalize();

	// ノックバック
	Vector3 knockBackVelocity = dir * SPIN_KNOCK_BACK;
}


