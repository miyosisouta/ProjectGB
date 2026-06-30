#include "stdafx.h"
#include "CollisionHitManager.h"
#include "src/Actor/ActorStatus.h"
#include "src/Actor/Character.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/Player.h"
#include "src/Actor/AttackObject.h"
#include "src/Core/ParameterManager.h"
#include "src/Util/DamageCalculator.h"
#include "src/Emotion/EmotionSystem.h"


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
	T* GetHitObject(const CollisionHitManager::Pair& pair, const uint32_t id)
	{
		if (pair.a->GetOwnerId() == id) {
			return static_cast<T*>(pair.a->GetOwner());
		}
		if (pair.b->GetOwnerId() == id) {
			return static_cast<T*>(pair.b->GetOwner());
		}
		return nullptr;
	}

	GhostBody* GetGhostBodyById(CollisionHitManager::Pair& pair, const uint32_t id)
	{
		if (pair.a->GetOwnerId() == id) return pair.a;
		if (pair.b->GetOwnerId() == id) return pair.b;
		return nullptr;
	}

	// 攻撃ボディ位置からボスのコリジョン表面座標を計算する
	// 攻撃体はヒット検出時点でボス表面付近に位置するため、その座標をそのまま使う
	Vector3 CalcBossSurfacePos(GhostBody* attackBody, GhostBody* bossBody)
	{
		return attackBody->GetPosition();
	}

	// ボス攻撃ボディからプレイヤーのGhostBodyにRayを飛ばしてヒット地点を返す
	Vector3 CalcPlayerHitPos(GhostBody* bossAtkBody, GhostBody* playerBody)
	{
		Vector3 start = bossAtkBody->GetPosition();
		Vector3 end   = playerBody->GetPosition();

		nsK2EngineLow::RaycastHit hit;
		bool isHit = PhysicsWorld::Get().Raycast(
			start, end, hit, 0xFFFFFFFF,
			[playerBody](const btCollisionObject& obj) {
				return &obj == playerBody->GetBulletObject();
			}
		);

		Vector3 effectPos = isHit ? hit.point : end;
		// 攻撃範囲が広い場合にY座標が高くなりすぎるのを防ぐ
		const float yCap = ParameterManager::Get().GetCollisionHitParam()->effectYCap;
		if (effectPos.y > yCap) {
			effectPos.y = yCap;
		}
		return effectPos;
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
		if (IsJustAvoidPair(pair)) {
			// ジャスト回避: 専用の演出・報酬処理を行い、ダメージはすべてスキップ
			OnJustAvoid(pair);
		}
		else if (IsPlayerInvinciblePair(pair)) {
			// 通常回避（無敵中）: 何もせずダメージをすべてスキップ
		}
		else {
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
			// 岩を投げる
			if (ContainsBossThrowRockPair(pair)) {
				UpdateBossThrowRockPair(pair);
			}
			// レーザー
			if (ContainsBossLaserWeakPair(pair)) {
				UpdateBossLaserWeakPair(pair);
			}
			if (ContainsBossLaserStrongPair(pair)) {
				UpdateBossLaserStrongPair(pair);
			}
		}
	}

	// キャラクター全体
	{
		// 地雷（対プレイヤー）: 無敵中はスキップ
		if (ContainsCharacterLandminePlayerPair(pair) && !IsPlayerInvinciblePair(pair)) {
			UpdateCharacterLandminePlayerPair(pair);
		}
		if (ContainsCharacterLandmineBossPair(pair)) {
			UpdateCharacterLandmineBossPair(pair);
		}
	}
}


void CollisionHitManager::OnCollisionStay(GhostBody* a, GhostBody* b, int frameCount)
{
	Pair pair(a, b);

	// スキル攻撃の多段ヒット
	// Enter で1回目のダメージ済み（frameCount=1）
	// その後 fireMagicHitInterval 秒ごとに追加ヒット
	if (ContainsPlayerSkillAttackPair(pair))
	{
		const auto* sp          = ParameterManager::Get().GetSkillParam("FireMagic");
		const int intervalFrames = static_cast<int>(sp->fireMagicHitInterval * 60.0f);
		const int maxHits        = sp->fireMagicHitCount;

		// frameCount=1 が Enter（1回目）なので、elapsed=frameCount-1 で正規化
		int elapsed  = frameCount - 1;
		int hitIndex = elapsed / intervalFrames; // 1回目の追加ヒット=1, 2回目=2, ...

		if (elapsed > 0 && elapsed % intervalFrames == 0 && hitIndex < maxHits)
		{
			UpdatePlayerSkillAttackPair(pair);
		}
	}
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
		DamageResult result = Calculate(playerStatus, motion);
		bossCharacter->GetStatus()->Damage(result.damage);
		if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::AttackHit, result.isCritical); // ダメージの情報を通知
		UpdateAttackHitSound(); // 攻撃が当たったSEを流す
	}

	// エフェクト: 攻撃判定からボスのコリジョン表面座標を求めて再生
	GhostBody* attackBody = GetGhostBodyById(hitPair, CharacterID::PlayerNormalAtkID());
	GhostBody* bossBody   = GetGhostBodyById(hitPair, CharacterID::BossID());
	Vector3 effectPos = CalcBossSurfacePos(attackBody, bossBody);
	float scale = ParameterManager::Get().GetCollisionHitParam()->playerNormalAttackEffectScale;
	EffectManager::Get().PlayEffect(enEffectKind_AttackHitBoss, effectPos, Quaternion::Identity, Vector3(scale));
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
		DamageResult result = Calculate(playerStatus, motion);
		bossCharacter->GetStatus()->Damage(result.damage);
		if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::AttackHit, result.isCritical); // ダメージの情報を通知
		UpdateAttackHitSound(); // 攻撃が当たったSEを流す
	}

	// エフェクト: 攻撃判定からボスのコリジョン表面座標を求めて再生
	GhostBody* attackBody = GetGhostBodyById(hitPair, CharacterID::PlayerSkillAtkID());
	GhostBody* bossBody   = GetGhostBodyById(hitPair, CharacterID::BossID());
	Vector3 effectPos = CalcBossSurfacePos(attackBody, bossBody);
	float scale = ParameterManager::Get().GetCollisionHitParam()->playerSkillAttackEffectScale;
	EffectManager::Get().PlayEffect(enEffectKind_AttackHitBoss, effectPos, Quaternion::Identity, Vector3(scale));
}



/* ============================================ */
/* ボスが攻撃 */
/* ============================================ */

//// =====================================================================
//// ジャスト回避
//// =====================================================================

bool CollisionHitManager::IsJustAvoidPair(const Pair& hitPair)
{
	// プレイヤーが衝突に含まれているか
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());
	if (!player || !player->GetStatus()) return false;

	// ボスの攻撃が衝突に含まれているか
	bool isBossAttack =
		IsHitObject<BossCharacter>(hitPair, CharacterID::BossNormalAtkID()) ||
		IsHitObject<BossCharacter>(hitPair, CharacterID::BossHitStampAtkID()) ||
		IsHitObject<BossCharacter>(hitPair, CharacterID::BossSpinAtkID()) ||
		IsHitObject<BossCharacter>(hitPair, CharacterID::BossThrowRockAtkID()) ||
		IsHitObject<BossCharacter>(hitPair, CharacterID::BossLaserWeakAtkID()) ||
		IsHitObject<BossCharacter>(hitPair, CharacterID::BossLaserStrongAtkID());
	if (!isBossAttack) return false;

	// プレイヤーがジャスト回避ウィンドウ中か
	PlayerStatus* status = player->GetStatus()->As<PlayerStatus>();
	return status && status->IsJustAvoiding();
}

void CollisionHitManager::OnJustAvoid(Pair& hitPair)
{
	// ジャスト回避したプレイヤーの感情レベルを上昇させる
	// デバフ中なら帳消しで Normal、通常/バフ中なら1段階アップ
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());
	if (player)
	{
		auto* status = player->GetStatus()->As<PlayerStatus>();
		if (status) { status->GetEmotionSystem().OnJustAvoid(); }
	}

	// TODO: スローモーション演出、カウンター攻撃ウィンドウの開放、専用エフェクト再生など
	// todo for test : 値の視覚化（ジャスト回避成功時に感情レベルが上がることを確認）
}

void CollisionHitManager::NotifyEmotionStrongHit(Pair& hitPair)
{
	// 強攻撃を受けたプレイヤーの感情レベルを下降させる
	// バフ中なら帳消しで Normal、通常/デバフ中なら1段階ダウン
	// HitStamp・Spin・チャージレーザーの3種が強攻撃に該当する
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());
	if (!player) { return; }
	auto* status = player->GetStatus()->As<PlayerStatus>();
	if (status) { status->GetEmotionSystem().OnStrongAttackHit(); }
}

bool CollisionHitManager::IsPlayerInvinciblePair(const Pair& hitPair)
{
	// プレイヤーが衝突に含まれているか
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());
	if (!player || !player->GetStatus()) return false;

	// プレイヤーが無敵状態か（enAvoid / enDamage / enSkill など何でも）
	PlayerStatus* status = player->GetStatus()->As<PlayerStatus>();
	return status && status->IsInvincible();
}



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

	// ボスのステータスがあるならダメージ処理
	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("NormalAttack");
		float vibrationTime = bossStatus->GetVibrationTimeValues("NormalAttack");
		float vibrationForce = bossStatus->GetVibrationForceValues("NormalAttack");
		DamageResult result = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(result.damage);
		if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::TakeHit, result.isCritical); // ダメージの情報を通知
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
		g_pad[0]->SetVibration(vibrationTime, vibrationForce);
	}

	// エフェクト
	GhostBody* bossAtkBody = GetGhostBodyById(hitPair, CharacterID::BossNormalAtkID());
	GhostBody* playerBody  = GetGhostBodyById(hitPair, CharacterID::PlayerID());
	Vector3 effectPos = CalcPlayerHitPos(bossAtkBody, playerBody);
	float scale = ParameterManager::Get().GetCollisionHitParam()->bossNormalAttackEffectScale;
	EffectManager::Get().PlayEffect(enEffectKind_AttackHitPlayer, effectPos, Quaternion::Identity, Vector3(scale));
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

	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("HitStamp");
		float vibrationTime = bossStatus->GetVibrationTimeValues("HitStamp");
		float vibrationForce = bossStatus->GetVibrationForceValues("HitStamp");
		DamageResult result = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(result.damage); // ダメージを与える
		if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::TakeHit, result.isCritical); // ダメージの情報を通知
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
		g_pad[0]->SetVibration(vibrationTime, vibrationForce);
		NotifyEmotionStrongHit(hitPair); // 強攻撃：感情レベルを下降させる
	}

	// エフェクト
	GhostBody* bossAtkBody = GetGhostBodyById(hitPair, CharacterID::BossHitStampAtkID());
	GhostBody* playerBody  = GetGhostBodyById(hitPair, CharacterID::PlayerID());
	Vector3 effectPos = CalcPlayerHitPos(bossAtkBody, playerBody);
	float scale = ParameterManager::Get().GetCollisionHitParam()->bossHitStampEffectScale;
	EffectManager::Get().PlayEffect(enEffectKind_AttackHitPlayer, effectPos, Quaternion::Identity, Vector3(scale));
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

	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("SpinAttack");
		float vibrationTime = bossStatus->GetVibrationTimeValues("SpinAttack");
		float vibrationForce = bossStatus->GetVibrationForceValues("SpinAttack");
		DamageResult result = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(result.damage);
		if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::TakeHit, result.isCritical); // ダメージの情報を通知
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
		g_pad[0]->SetVibration(vibrationTime, vibrationForce);
		NotifyEmotionStrongHit(hitPair); // 強攻撃：感情レベルを下降させる
	}
	

	// 当たった二人の座標を取得
	Vector3 playerPos = player->GetTransformPosition();
	Vector3 bossPos = boss->GetTransformPosition();

	// 方向を取得
	Vector3 dir= playerPos - bossPos;
	dir.Normalize();

	const auto* hitParam = ParameterManager::Get().GetCollisionHitParam();

	// ノックバック
	Vector3 knockBackVelocity = dir * hitParam->spinKnockBack;

	// エフェクト
	GhostBody* bossAtkBody = GetGhostBodyById(hitPair, CharacterID::BossSpinAtkID());
	GhostBody* playerBody  = GetGhostBodyById(hitPair, CharacterID::PlayerID());
	Vector3 effectPos = CalcPlayerHitPos(bossAtkBody, playerBody);
	EffectManager::Get().PlayEffect(enEffectKind_AttackHitPlayer, effectPos, Quaternion::Identity, Vector3(hitParam->bossSpinEffectScale));
}

bool CollisionHitManager::ContainsBossThrowRockPair(const Pair& hitPair)
{
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossThrowRockAtkID())) {
		return false;
	}
	if (!IsHitObject<Player>(hitPair, CharacterID::PlayerID())) {
		return false;
	}
	return true;
}

void CollisionHitManager::UpdateBossThrowRockPair(Pair& hitPair)
{
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());
	auto* boss = GetHitObject<BossCharacter>(hitPair, CharacterID::BossThrowRockAtkID());
	auto bossStatus = boss->GetStatus()->As<BossStatus>();

	// プレイヤーがいるか、もしくはプレイヤーのステータスがあるか
	if (player == nullptr || player->GetStatus() == nullptr) { return; }

	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("ThrowRock");
		float vibrationTime = bossStatus->GetVibrationTimeValues("ThrowRock");
		float vibrationForce = bossStatus->GetVibrationForceValues("ThrowRock");
		DamageResult result = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(result.damage); // ダメージを与える
		if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::TakeHit, result.isCritical); // ダメージの情報を通知
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
		g_pad[0]->SetVibration(vibrationTime, vibrationForce);
	}

	// エフェクト
	GhostBody* bossAtkBody = GetGhostBodyById(hitPair, CharacterID::BossThrowRockAtkID());
	GhostBody* playerBody  = GetGhostBodyById(hitPair, CharacterID::PlayerID());
	Vector3 effectPos = CalcPlayerHitPos(bossAtkBody, playerBody);
	float scale = ParameterManager::Get().GetCollisionHitParam()->bossThrowRockEffectScale;
	EffectManager::Get().PlayEffect(enEffectKind_AttackHitPlayer, effectPos, Quaternion::Identity, Vector3(scale));
}

bool CollisionHitManager::ContainsBossLaserWeakPair(const Pair& hitPair)
{
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossLaserWeakAtkID())) {
		return false;
	}
	if (!IsHitObject<Character>(hitPair, CharacterID::PlayerID())) {
		return false;
	}
	return true;
}

void CollisionHitManager::UpdateBossLaserWeakPair(Pair& hitPair)
{
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());
	auto* boss = GetHitObject<BossCharacter>(hitPair, CharacterID::BossLaserWeakAtkID());
	auto bossStatus = boss->GetStatus()->As<BossStatus>();

	// プレイヤーがいるか、もしくはプレイヤーのステータスがあるか
	if (player == nullptr || player->GetStatus() == nullptr) { return; }

	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("LaserWeak");
		float vibrationTime = bossStatus->GetVibrationTimeValues("LaserWeak");
		float vibrationForce = bossStatus->GetVibrationForceValues("LaserWeak");
		DamageResult result = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(result.damage); // ダメージを与える
		if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::TakeHit, result.isCritical); // ダメージの情報を通知
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
		g_pad[0]->SetVibration(vibrationTime, vibrationForce);
	}

	// エフェクト
	GhostBody* bossAtkBody = GetGhostBodyById(hitPair, CharacterID::BossLaserWeakAtkID());
	GhostBody* playerBody  = GetGhostBodyById(hitPair, CharacterID::PlayerID());
	Vector3 effectPos = CalcPlayerHitPos(bossAtkBody, playerBody);
	float scale = ParameterManager::Get().GetCollisionHitParam()->bossLaserWeakEffectScale;
	EffectManager::Get().PlayEffect(enEffectKind_AttackHitPlayer, effectPos, Quaternion::Identity, Vector3(scale));
}

bool CollisionHitManager::ContainsBossLaserStrongPair(const Pair& hitPair)
{
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossLaserStrongAtkID())) {
		return false;
	}
	if (!IsHitObject<Character>(hitPair, CharacterID::PlayerID())) {
		return false;
	}
	return true;
}

void CollisionHitManager::UpdateBossLaserStrongPair(Pair& hitPair)
{
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());
	auto* boss = GetHitObject<BossCharacter>(hitPair, CharacterID::BossLaserStrongAtkID());
	auto bossStatus = boss->GetStatus()->As<BossStatus>();

	// プレイヤーがいるか、もしくはプレイヤーのステータスがあるか
	if (player == nullptr || player->GetStatus() == nullptr) { return; }

	if (boss->GetStatus()) {
		float motion = bossStatus->GetSkillMotionValues("LaserStrong");
		float vibrationTime = bossStatus->GetVibrationTimeValues("LaserStrong");
		float vibrationForce = bossStatus->GetVibrationForceValues("LaserStrong");
		DamageResult result = Calculate(bossStatus, motion);
		player->GetStatus()->Damage(result.damage); // ダメージを与える
		if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::TakeHit, result.isCritical); // ダメージの情報を通知
		UpdateTakeHitSound(); // 攻撃が当たったSEを流す
		g_pad[0]->SetVibration(vibrationTime, vibrationForce);
		NotifyEmotionStrongHit(hitPair); // 強攻撃：感情レベルを下降させる
	}

	// エフェクト
	GhostBody* bossAtkBody = GetGhostBodyById(hitPair, CharacterID::BossLaserStrongAtkID());
	GhostBody* playerBody  = GetGhostBodyById(hitPair, CharacterID::PlayerID());
	Vector3 effectPos = CalcPlayerHitPos(bossAtkBody, playerBody);
	float scale = ParameterManager::Get().GetCollisionHitParam()->bossLaserStrongEffectScale;
	EffectManager::Get().PlayEffect(enEffectKind_AttackHitPlayer, effectPos, Quaternion::Identity, Vector3(scale));
}



/* ============================================ */
/* キャラクターの誰かが全員に対して攻撃 */
/* ============================================ */

bool CollisionHitManager::ContainsCharacterLandminePlayerPair(const Pair& hitPair)
{
	if (!IsHitObject<LandmineObject>(hitPair, CharacterID::CharaLandmineAtkID())) {
		return false;
	}
	if (!IsHitObject<Player>(hitPair, CharacterID::PlayerID())) {
		return false;
	}
	return true;
}

void CollisionHitManager::UpdateCharacterLandminePlayerPair(Pair& hitPair)
{
	// ownerはCharacterとして取得 → 攻撃者はどちらかわからないため
	auto* owner = GetHitObject<Character>(hitPair, CharacterID::CharaLandmineAtkID());
	auto* player = GetHitObject<Player>(hitPair, CharacterID::PlayerID());

	if (!owner || !player || !owner->GetStatus() || !player->GetStatus()) { return; }

	// プレイヤーの無敵チェック
	PlayerStatus* playerStatus = player->GetStatus()->As<PlayerStatus>();
	if (playerStatus && playerStatus->IsInvincible()) { return; }

	auto* ownerStatus = owner->GetStatus()->As<PlayerStatus>();
	float motion = ownerStatus->GetSkillMotionValues("SpecialAttack");
	float vibrationTime = ownerStatus->GetVibrationTimeValues("SpecialAttack");
	float vibrationForce = ownerStatus->GetVibrationForceValues("SpecialAttack");
	DamageResult result = Calculate(ownerStatus, motion);
	player->GetStatus()->Damage(result.damage);
	if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::TakeHit, result.isCritical); // ダメージの情報を通知
	UpdateTakeHitSound();
	g_pad[0]->SetVibration(vibrationTime, vibrationForce);
}

bool CollisionHitManager::ContainsCharacterLandmineBossPair(const Pair& hitPair)
{
	if (!IsHitObject<LandmineObject>(hitPair, CharacterID::CharaLandmineAtkID())) {
		return false;
	}
	if (!IsHitObject<BossCharacter>(hitPair, CharacterID::BossID())) {
		return false;
	}
	return true;
}

void CollisionHitManager::UpdateCharacterLandmineBossPair(Pair& hitPair)
{
	// ownerはCharacterとして取得 → 攻撃者はどちらかわからないため
	auto* owner = GetHitObject<Character>(hitPair, CharacterID::CharaLandmineAtkID());
	auto* boss = GetHitObject<BossCharacter>(hitPair, CharacterID::BossID());

	auto* ownerStatus = owner->GetStatus()->As<PlayerStatus>();
	float motion = ownerStatus->GetSkillMotionValues("SpecialAttack");
	DamageResult result = Calculate(ownerStatus, motion);
	boss->GetStatus()->Damage(result.damage);
	if (onDamageNotify) onDamageNotify(result.damage, DamageNotifyType::AttackHit, result.isCritical); // ダメージの情報を通知
	UpdateAttackHitSound();
}


