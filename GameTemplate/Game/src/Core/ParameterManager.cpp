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
			p.motionValues = j.value("motionValues",  0);
		}
	);
}