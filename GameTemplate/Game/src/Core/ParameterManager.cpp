/**
 * ParameterManager.cpp
 *
 * パラメーター管理
 * ステータスなどの数値を外部ファイルから読み込んで使用する
 */
#include "stdafx.h"
#include "ParameterManager.h"


/*==================================*/
/* パース関連						*/
/*==================================*/

Vector3 ParseVector3(const nlohmann::json& arr)
{
	if (!arr.is_array() || arr.size() < 3) {
		return Vector3::Zero;
	}
	return Vector3(
		arr[0].get<float>(),
		arr[1].get<float>(),
		arr[2].get<float>()
	);
}


Vector4 ParseVector4(const nlohmann::json& arr)
{
	return Vector4(
		arr[0].get<float>(),
		arr[1].get<float>(),
		arr[2].get<float>(),
		arr[3].get<float>()
	);
}


Quaternion ParseRotation(const nlohmann::json& arr)
{
	Vector4 vec4 = ParseVector4(arr);
	return Quaternion(vec4.x,vec4.y,vec4.z,vec4.w);
}


// ネストしたJSONオブジェクトを安全に取得する（無ければ空オブジェクトを返す）
// BossStateParameter.json のように「攻撃ごとにネストしたオブジェクト」を読むときに使う
nlohmann::json GetSubObject(const nlohmann::json& j, const char* key)
{
	return (j.contains(key) && j[key].is_object()) ? j[key] : nlohmann::json::object();
}


/*====================================*/
/* パラメータマネージャー */
/*====================================*/
ParameterManager* ParameterManager::instance_ = nullptr; //初期化


ParameterManager::ParameterManager()
{
	// リストの削除
	parameterMap_.clear();
}

ParameterManager::~ParameterManager()
{
	// リストの削除
	for (auto& it : parameterMap_) {
		auto& parameters = it.second;
		for (auto* parameter : parameters) {
			delete parameter;
		}
		parameters.clear();
	}
	parameterMap_.clear();
}


// ============================================================
//  CharacterStatusData.json 読み込みヘルパー
//  ゲーム起動時などに一度だけ呼ぶ
// ============================================================

void ParameterManager::LoadCharacterStatusData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterCharacterStatusParameter>(
		path,
		"Character",   // JSONのトップレベルキー名
		[](const nlohmann::json& j, MasterCharacterStatusParameter& p)
		{
			// key: キャラクター識別子 (必須)
			p.key = j.value("key", "");

			// ステータス
			p.position							= ParseVector3(j["position"]);
			p.rotation							= ParseRotation(j["rotation"]);
			p.scale								= ParseVector3(j["scale"]);
			p.collisionPosUp					= ParseVector3(j["collisionPosUp"]);
			p.collisionSizeRadius				= j.value("collisionSizeRadius",		0.0f);
			p.collisionSizeHeight				= j.value("collisionSizeHeight",		0.0f);
			p.charaConSizeRadius				= j.value("charaConSizeRadius",			0.0f);
			p.charaConSizeHeight				= j.value("charaConSizeHeight",			0.0f);
			p.hp								= j.value("hp",							0);
			p.attack							= j.value("attack",						0);
			p.criticalRate						= j.value("criticalRate",				0);
			p.criticalDamageMultiplier			= j.value("criticalDamageMultiplier",	0.0f);
			const nlohmann::json staminaJson =
				(j.contains("stamina") && j["stamina"].is_object())
				? j["stamina"]
				: nlohmann::json::object();

			p.stamina.maxStamina				= staminaJson.value("maxStamina", j.value("stamina", 0.0f));
			p.stamina.drainPerSec				= staminaJson.value("drainPerSec", j.value("drainPerSec", 0.0f));
			p.stamina.recoverPerSec				= staminaJson.value("recoverPerSec", j.value("recoverPerSec", 0.0f));
			p.stamina.exhaustedRecoverPerSec	= staminaJson.value("exhaustedRecoverPerSec", j.value("exhaustedRecoverPerSec", 0.0f));
			p.stamina.exhaustedThreshold		= staminaJson.value("exhaustedThreshold", j.value("exhaustedThreshold", 0.0f));
			p.stamina.exhaustedSpeedRate		= staminaJson.value("exhaustedSpeedRate", j.value("exhaustedSpeedRate", 0.0f));
			p.moveSpeedBase						= j.value("walkSpeedBase",				0.0f);
			p.runSpeedBase						= j.value("runSpeedBase",				0.0f);
		}
	);
}


// ============================================================
//  PlayerSkillStatus.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadPlayerSkillStatusData(const char* path)
{
	ParameterManager::Get().LoadParameterFromNestedArray<MasterPlayerSkillParameter>(
		path,
		[](const std::string& category, const nlohmann::json& j, MasterPlayerSkillParameter& p)
		{
			p.category			= category;
			p.key				= j.value("key",			   "");
			p.motionValues		= j.value("motionValues",       0);
			p.cooldown			= j.value("cooldown",		 0.0f);
			p.decreaseStamina	= j.value("decreaseStamina", 0.0f);
			p.vibrationTime		= j.value("vibrationTime", 0.0f);
			p.vibrationForce	= j.value("vibrationForce", 0.0f);
		}
	);
}


// ============================================================
//  BossSkillStatus.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadBossSkillStatusData(const char* path)
{
	ParameterManager::Get().LoadParameterFromNestedArray<MasterBossSkillParameter>(
		path,
		[](const std::string& category, const nlohmann::json& j, MasterBossSkillParameter& p)
		{
			p.category = category;
			p.key      = j.value("key",    "");
			p.motionValues = j.value("motionValues",  0.0f);
			p.vibrationTime = j.value("vibrationTime",  0.0f);
			p.vibrationForce = j.value("vibrationForce",  0.0f);
		}
	);
}

// ============================================================
//  grass_attack_params.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadGrassBendParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterGrassBendParameter>(
		path,
		"attacks",
		[](const nlohmann::json& j, MasterGrassBendParameter& p)
		{
			p.key          = j.value("key",           "");
			p.force        = j.value("force",        0.0f);
			p.radius       = j.value("radius",       0.0f);
			p.duration     = j.value("duration",     0.0f);
			p.recoverySpeed= j.value("recoverySpeed",1.0f);
		}
	);
}

// ============================================================
//  PlayerStateParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadPlayerStateParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterPlayerStateParameter>(
		path,
		"PlayerState",
		[](const nlohmann::json& j, MasterPlayerStateParameter& p)
		{
			p.walkSeInterval     = j.value("walkSeInterval",     0.0f);
			p.walkLoopTime       = j.value("walkLoopTime",       0.0f);
			p.walkEffectScale    = j.value("walkEffectScale",    0.0f);
			p.runSeInterval      = j.value("runSeInterval",      0.0f);
			p.deathAnimationTime = j.value("deathAnimationTime", 0.0f);
			p.moveThreshold      = j.value("moveThreshold",      0.0f);
		}
	);
}

// ============================================================
//  NPCControllerParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadNPCControllerParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterNPCControllerParameter>(
		path,
		"NPCController",
		[](const nlohmann::json& j, MasterNPCControllerParameter& p)
		{
			p.attackLotteryMax = j.value("attackLotteryMax", 0);
			p.shortDistance    = j.value("shortDistance",    0.0f);
			p.midDistance      = j.value("midDistance",      0.0f);
			p.longDistance     = j.value("longDistance",     0.0f);
		}
	);
}

// ============================================================
//  BossSpawnerParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadBossSpawnerParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterBossSpawnerParameter>(
		path,
		"BossSpawner",
		[](const nlohmann::json& j, MasterBossSpawnerParameter& p)
		{
			p.modeAttackMultiplier = j.value("modeAttackMultiplier", 0.0f);
			p.modeHpMultiplier     = j.value("modeHpMultiplier",     0.0f);
		}
	);
}

// ============================================================
//  SkillParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadSkillParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterSkillParameter>(
		path,
		"Skill",
		[](const nlohmann::json& j, MasterSkillParameter& p)
		{
			p.key                      = j.value("key",                      "");
			p.collisionRadius          = j.value("collisionRadius",          0.0f);
			p.collisionForward         = j.value("collisionForward",         0.0f);
			p.collisionHeight          = j.value("collisionHeight",          0.0f);
			p.targetPosForward         = j.value("targetPosForward",         0.0f);
			p.avoidMoveSpeed           = j.value("avoidMoveSpeed",           0.0f);
			p.avoidEffectRotation      = j.value("avoidEffectRotation",      0.0f);
			p.avoidEffectScale         = j.value("avoidEffectScale",         0.0f);
			p.avoidStartTime           = j.value("avoidStartTime",           0.0f);
			p.avoidInvincibleStartTime = j.value("avoidInvincibleStartTime", 0.0f);
			p.avoidJustStartTime       = j.value("avoidJustStartTime",       0.0f);
			p.avoidJustEndTime         = j.value("avoidJustEndTime",         0.0f);
			p.avoidEndTime             = j.value("avoidEndTime",             0.0f);
			p.effectScaleFactor        = j.value("effectScaleFactor",        0.0f);
			p.fireMagicCollisionSize   = j.value("fireMagicCollisionSize",   0.0f);
			p.fireMagicCollisionDepth  = j.value("fireMagicCollisionDepth",  0.0f);
			p.fireMagicCollisionForward= j.value("fireMagicCollisionForward",0.0f);
			p.fireMagicCollisionHeight = j.value("fireMagicCollisionHeight", 0.0f);
			p.fireMagicHitCount        = j.value("fireMagicHitCount",        1);
			p.fireMagicHitInterval     = j.value("fireMagicHitInterval",     0.0f);
			p.fireMagicAttackStartTime = j.value("fireMagicAttackStartTime", 0.0f);
			p.fireMagicAttackEndTime   = j.value("fireMagicAttackEndTime",   0.0f);
		}
	);
}

// ============================================================
//  AttackObjectParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadAttackObjectParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterAttackObjectParameter>(
		path,
		"AttackObject",
		[](const nlohmann::json& j, MasterAttackObjectParameter& p)
		{
			p.landmineModelScale          = j.value("landmineModelScale",          0.0f);
			p.landmineCollisionSize       = j.value("landmineCollisionSize",       0.0f);
			p.landmineIndicatorRadius     = j.value("landmineIndicatorRadius",     0.0f);
			p.effectScaleFactorDamageLing = j.value("effectScaleFactorDamageLing", 0.0f);
			p.effectScaleFactorExplode    = j.value("effectScaleFactorExplode",    0.0f);
		}
	);
}

// ============================================================
//  StageManagerParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadStageManagerParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterStageManagerParameter>(
		path,
		"StageManager",
		[](const nlohmann::json& j, MasterStageManagerParameter& p)
		{
			p.ditheringAlphaGround   = j.value("ditheringAlphaGround",   0.0f);
			p.ditheringAlphaFence    = j.value("ditheringAlphaFence",    0.0f);
			p.ditheringAlphaTree     = j.value("ditheringAlphaTree",     0.0f);
			p.splatMapGrassLuminance = j.value("splatMapGrassLuminance", 0.0f);
			p.splatMapClayLuminance  = j.value("splatMapClayLuminance",  0.0f);
			p.splatMapHumusLuminance = j.value("splatMapHumusLuminance", 0.0f);
			p.splatMapWholeLuminance = j.value("splatMapWholeLuminance", 0.0f);
			p.splatMapSaturation     = j.value("splatMapSaturation",     0.0f);
			p.collisionUp            = j.value("collisionUp",            0.0f);
			p.meterToCentimeter      = j.value("meterToCentimeter",      0.0f);
		}
	);
}

// ============================================================
//  CollisionHitParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadCollisionHitParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterCollisionHitParameter>(
		path,
		"CollisionHit",
		[](const nlohmann::json& j, MasterCollisionHitParameter& p)
		{
			p.spinKnockBack                 = j.value("spinKnockBack",                 0.0f);
			p.effectYCap                    = j.value("effectYCap",                    0.0f);
			p.playerNormalAttackEffectScale = j.value("playerNormalAttackEffectScale", 0.0f);
			p.playerSkillAttackEffectScale  = j.value("playerSkillAttackEffectScale",  0.0f);
			p.bossNormalAttackEffectScale   = j.value("bossNormalAttackEffectScale",   0.0f);
			p.bossHitStampEffectScale       = j.value("bossHitStampEffectScale",       0.0f);
			p.bossSpinEffectScale           = j.value("bossSpinEffectScale",           0.0f);
			p.bossThrowRockEffectScale      = j.value("bossThrowRockEffectScale",      0.0f);
			p.bossLaserWeakEffectScale      = j.value("bossLaserWeakEffectScale",      0.0f);
			p.bossLaserStrongEffectScale    = j.value("bossLaserStrongEffectScale",    0.0f);
		}
	);
}

// ============================================================
//  MissionParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadMissionParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterMissionParameter>(
		path,
		"Mission",
		[](const nlohmann::json& j, MasterMissionParameter& p)
		{
			p.key                    = j.value("key",                    "");
			p.targetTime             = j.value("targetTime",             0.0f);
			p.abilityTargetCount     = j.value("abilityTargetCount",     static_cast<uint8_t>(0));
			p.utilityTargetCount     = j.value("utilityTargetCount",     static_cast<uint8_t>(0));
			p.normalAttackTargetCount= j.value("normalAttackTargetCount",static_cast<uint8_t>(0));
			p.goalHpRate             = j.value("goalHpRate",             0.0f);
		}
	);
}

// ============================================================
//  TitleBackgroundParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadTitleBGParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterTitleBGParameter>(
		path, "TitleBackground",
		[](const nlohmann::json& j, MasterTitleBGParameter& p)
		{
			p.treeSpeed          = j.value("treeSpeed",          p.treeSpeed);
			p.grassSpeed         = j.value("grassSpeed",         p.grassSpeed);
			p.fenceSpeed         = j.value("fenceSpeed",         p.fenceSpeed);
			p.treeSpacing        = j.value("treeSpacing",        p.treeSpacing);
			p.treeZ              = j.value("treeZ",              p.treeZ);
			p.treeZ2             = j.value("treeZ2",             p.treeZ2);
			p.grassSpacing       = j.value("grassSpacing",       p.grassSpacing);
			p.grassZ             = j.value("grassZ",             p.grassZ);
			p.fenceSpacing       = j.value("fenceSpacing",       p.fenceSpacing);
			p.fenceZ             = j.value("fenceZ",             p.fenceZ);
			p.groundY            = j.value("groundY",            p.groundY);
			p.groundScale        = j.value("groundScale",        p.groundScale);
			p.cullingMargin      = j.value("cullingMargin",      p.cullingMargin);
			p.spawnX             = j.value("spawnX",             p.spawnX);
			p.treeSpawnX         = j.value("treeSpawnX",         p.treeSpawnX);
			p.playerX            = j.value("playerX",            p.playerX);
			p.playerY            = j.value("playerY",            p.playerY);
			p.playerZ            = j.value("playerZ",            p.playerZ);
			p.playerRotYDeg      = j.value("playerRotYDeg",      p.playerRotYDeg);
			p.camPosX            = j.value("camPosX",            p.camPosX);
			p.camPosY            = j.value("camPosY",            p.camPosY);
			p.camPosZ            = j.value("camPosZ",            p.camPosZ);
			p.camTargetX         = j.value("camTargetX",         p.camTargetX);
			p.camTargetY         = j.value("camTargetY",         p.camTargetY);
			p.camTargetZ         = j.value("camTargetZ",         p.camTargetZ);
			p.camFovDeg          = j.value("camFovDeg",          p.camFovDeg);
			p.camNear            = j.value("camNear",            p.camNear);
			p.camFar             = j.value("camFar",             p.camFar);
			p.skyCubeScale       = j.value("skyCubeScale",       p.skyCubeScale);
			p.treeMaxConsecutive  = j.value("treeMaxConsecutive",  p.treeMaxConsecutive);
			p.treeMinGap          = j.value("treeMinGap",          p.treeMinGap);
			p.treeBaseGap         = j.value("treeBaseGap",         p.treeBaseGap);
			p.treeMaxGap          = j.value("treeMaxGap",          p.treeMaxGap);
			p.grassMaxConsecutive = j.value("grassMaxConsecutive", p.grassMaxConsecutive);
			p.grassMinGap         = j.value("grassMinGap",         p.grassMinGap);
			p.grassBaseGap        = j.value("grassBaseGap",        p.grassBaseGap);
			p.grassMaxGap         = j.value("grassMaxGap",         p.grassMaxGap);
			p.fenceMaxConsecutive = j.value("fenceMaxConsecutive", p.fenceMaxConsecutive);
			p.fenceMinGap         = j.value("fenceMinGap",         p.fenceMinGap);
			p.fenceBaseGap        = j.value("fenceBaseGap",        p.fenceBaseGap);
			p.fenceMaxGap         = j.value("fenceMaxGap",         p.fenceMaxGap);
			p.playerRunSpeed      = j.value("playerRunSpeed",      p.playerRunSpeed);
		}
	);
}


// ============================================================
//  EmotionParameter.json 読み込みヘルパー
// ============================================================
void ParameterManager::LoadEmotionParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterEmotionParameter>(
		path,
		"Emotion",
		[](const nlohmann::json& j, MasterEmotionParameter& p)
		{
			// 倍率テーブル（7段階）
			if (j.contains("modifiers") && j["modifiers"].is_array())
			{
				const auto& arr = j["modifiers"];
				for (int i = 0; i < 7 && i < static_cast<int>(arr.size()); i++)
				{
					p.modifiers[i].attackMul         = arr[i].value("attackMul",         1.0f);
					p.modifiers[i].attackSpeedMul    = arr[i].value("attackSpeedMul",    1.0f);
					p.modifiers[i].damageTakenMul    = arr[i].value("damageTakenMul",    1.0f);
					p.modifiers[i].animationSpeedMul = arr[i].value("animationSpeedMul", 1.0f);
					p.modifiers[i].effectSpeedMul    = arr[i].value("effectSpeedMul",    1.0f);
				}
			}
			// effect scale
			p.buffEffectScale   = j.value("buffEffectScale",   1.0f);
			p.debuffEffectScale = j.value("debuffEffectScale", 1.0f);
			// effect position offset（Yのみ。X/Zはボス座標をそのまま使う）
			p.buffEffectOffsetY   = j.value("buffEffectOffsetY",   0.0f);
			p.debuffEffectOffsetY = j.value("debuffEffectOffsetY", 0.0f);
		}
	);
}


// ============================================================
//  BossStateParameter.json 読み込みヘルパー
//  攻撃ごとにネストしたオブジェクトから読み込む（timing/collision/effect/movement の分類はコメント参照）
// ============================================================
void ParameterManager::LoadBossStateParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterBossStateParameter>(
		path,
		"BossState",
		[](const nlohmann::json& j, MasterBossStateParameter& p)
		{
			// 共通
			{
				const auto o = GetSubObject(j, "common");
				p.common.shortDistance         = o.value("shortDistance",         p.common.shortDistance);
				p.common.midDistance           = o.value("midDistance",           p.common.midDistance);
				p.common.longDistance          = o.value("longDistance",          p.common.longDistance);
				p.common.rotateSpeed           = o.value("rotateSpeed",           p.common.rotateSpeed);
				p.common.damageRingEffectScale = o.value("damageRingEffectScale", p.common.damageRingEffectScale);
			}

			// 待機
			{
				const auto o = GetSubObject(j, "idle");
				p.idle.endTime = o.value("endTime", p.idle.endTime);
			}

			// 走る
			{
				const auto o = GetSubObject(j, "run");
				p.run.moveSpeed          = o.value("moveSpeed",          p.run.moveSpeed);
				p.run.seLoopInterval     = o.value("seLoopInterval",     p.run.seLoopInterval);
				p.run.effectLoopInterval = o.value("effectLoopInterval", p.run.effectLoopInterval);
				if (o.contains("effectScale")) { p.run.effectScale = ParseVector3(o["effectScale"]); }
			}

			// 通常攻撃
			{
				const auto o = GetSubObject(j, "normalAttack");
				p.normalAttack.beginTime          = o.value("beginTime",          p.normalAttack.beginTime);
				p.normalAttack.collisionResetTime = o.value("collisionResetTime", p.normalAttack.collisionResetTime);
				p.normalAttack.endTime            = o.value("endTime",            p.normalAttack.endTime);
				p.normalAttack.collisionForward    = o.value("collisionForward",   p.normalAttack.collisionForward);
				p.normalAttack.collisionHeight     = o.value("collisionHeight",    p.normalAttack.collisionHeight);
				p.normalAttack.collisionSize        = o.value("collisionSize",       p.normalAttack.collisionSize);
			}

			// ヒットスタンプ
			{
				const auto o = GetSubObject(j, "hitStamp");
				p.hitStamp.upBeginTime          = o.value("upBeginTime",          p.hitStamp.upBeginTime);
				p.hitStamp.overheadMoveTime     = o.value("overheadMoveTime",     p.hitStamp.overheadMoveTime);
				p.hitStamp.fallBeginTime        = o.value("fallBeginTime",        p.hitStamp.fallBeginTime);
				p.hitStamp.rangeSize            = o.value("rangeSize",            p.hitStamp.rangeSize);
				if (o.contains("jumpHeight")) { p.hitStamp.jumpHeight = ParseVector3(o["jumpHeight"]); }
				p.hitStamp.verticalVelocity     = o.value("verticalVelocity",     p.hitStamp.verticalVelocity);
				p.hitStamp.upSpeed              = o.value("upSpeed",              p.hitStamp.upSpeed);
				p.hitStamp.downSpeed            = o.value("downSpeed",            p.hitStamp.downSpeed);
				p.hitStamp.gravityPower         = o.value("gravityPower",         p.hitStamp.gravityPower);
				p.hitStamp.effectScaleBasis     = o.value("effectScaleBasis",     p.hitStamp.effectScaleBasis);
				p.hitStamp.smokeEffectScale      = o.value("smokeEffectScale",      p.hitStamp.smokeEffectScale);
				p.hitStamp.shockWaveEffectScale = o.value("shockWaveEffectScale", p.hitStamp.shockWaveEffectScale);
			}

			// 回転攻撃
			{
				const auto o = GetSubObject(j, "spin");
				p.spin.attackStartTime    = o.value("attackStartTime",    p.spin.attackStartTime);
				p.spin.attackEndTime      = o.value("attackEndTime",      p.spin.attackEndTime);
				p.spin.seLoopInterval     = o.value("seLoopInterval",     p.spin.seLoopInterval);
				p.spin.effectLoopInterval = o.value("effectLoopInterval", p.spin.effectLoopInterval);
				p.spin.moveSpeed          = o.value("moveSpeed",          p.spin.moveSpeed);
				p.spin.overMoveDistance   = o.value("overMoveDistance",   p.spin.overMoveDistance);
				p.spin.effectScaleBasis   = o.value("effectScaleBasis",   p.spin.effectScaleBasis);
				p.spin.indicatorRangeSize = o.value("indicatorRangeSize", p.spin.indicatorRangeSize);
				p.spin.indicatorLength    = o.value("indicatorLength",    p.spin.indicatorLength);
				p.spin.indicatorForward   = o.value("indicatorForward",   p.spin.indicatorForward);
			}

			// 岩を投げる攻撃
			{
				const auto o = GetSubObject(j, "throwRock");
				p.throwRock.beginTime          = o.value("beginTime",          p.throwRock.beginTime);
				p.throwRock.endTime            = o.value("endTime",            p.throwRock.endTime);
				p.throwRock.overMoveDistance    = o.value("overMoveDistance",   p.throwRock.overMoveDistance);
				p.throwRock.indicatorLength     = o.value("indicatorLength",    p.throwRock.indicatorLength);
				p.throwRock.indicatorBaseSize   = o.value("indicatorBaseSize",  p.throwRock.indicatorBaseSize);
				p.throwRock.indicatorForward    = o.value("indicatorForward",   p.throwRock.indicatorForward);
				p.throwRock.indicatorRangeSize = o.value("indicatorRangeSize", p.throwRock.indicatorRangeSize);
				p.throwRock.rockCollisionSize   = o.value("rockCollisionSize",  p.throwRock.rockCollisionSize);
			}

			// レーザー攻撃
			{
				const auto o = GetSubObject(j, "laser");
				p.laser.initialShotTime       = o.value("initialShotTime",       p.laser.initialShotTime);
				p.laser.attackTime            = o.value("attackTime",            p.laser.attackTime);
				p.laser.shotIntervalNormal    = o.value("shotIntervalNormal",    p.laser.shotIntervalNormal);
				p.laser.shotIntervalCharge    = o.value("shotIntervalCharge",    p.laser.shotIntervalCharge);
				p.laser.shotCountNormal       = o.value("shotCountNormal",       p.laser.shotCountNormal);
				p.laser.shotCountMult         = o.value("shotCountMult",         p.laser.shotCountMult);
				if (o.contains("chargeScale")) { p.laser.chargeScale = ParseVector3(o["chargeScale"]); }
				p.laser.collisionScale        = o.value("collisionScale",        p.laser.collisionScale);
				p.laser.indicatorRadiusNormal = o.value("indicatorRadiusNormal", p.laser.indicatorRadiusNormal);
				p.laser.indicatorRadiusCharge = o.value("indicatorRadiusCharge", p.laser.indicatorRadiusCharge);
				p.laser.effectScaleFactor     = o.value("effectScaleFactor",     p.laser.effectScaleFactor);
			}

			// 死亡
			{
				const auto o = GetSubObject(j, "death");
				p.death.animationTime = o.value("animationTime", p.death.animationTime);
			}
		}
	);
}


void ParameterManager::LoadBattleCommonParamData(const char* path)
{
	ParameterManager::Get().LoadParameterFromArray<MasterBattleCommonParameter>(
		path,
		"Battle",
		[](const nlohmann::json& j, MasterBattleCommonParameter& p)
		{
			p.cameraParam.nearClip	= j.value("nearClip",	0.0f);
			p.cameraParam.farClip	= j.value("farClip",	0.0f);
			p.cameraParam.fovy		= j.value("fovy",		0.0f);
			p.cameraParam.height	= j.value("height",		0.0f);
			p.cameraParam.distance	= j.value("distance",	0.0f);
			p.cameraParam.rotSpeed	= j.value("rotSpeed",	0.0f);
			p.cameraParam.invert = j.value("invert", false);
			p.cameraParam.sensitivity = j.value("sensitivity", 0.0f);
			p.cameraParam.lookAtOffsetY = j.value("lookAtOffsetY", 0.0f);

			p.skyCubeScale = j.value("scale", 0);

			p.gameTimeParam.limitTime	= j.value("limitTime",		0.0f);
			p.gameTimeParam.warningTime = j.value("warningTime",	0.0f);

			p.cutSceneParam.firstCutTime		= j.value("firstCutTime",	0.0f);
			p.cutSceneParam.secondCutTime		= j.value("secondCutTime",	0.0f);
			p.cutSceneParam.thirdCutTime		= j.value("thirdCutTime",	0.0f);
			p.cutSceneParam.endCutTime			= j.value("endCutTime",		0.0f);
			p.cutSceneParam.firstCutCameraPos	= ParseVector3(j["firstCutCameraPos"]);
			p.cutSceneParam.secondCutCameraPos	= ParseVector3(j["secondCutCameraPos"]);
			p.cutSceneParam.thirdCutCameraPos	= ParseVector3(j["thirdCutCameraPos"]);
			p.cutSceneParam.cutSceneTargetPos	= ParseVector3(j["cutTargetPos"]);
		}
	);
}
