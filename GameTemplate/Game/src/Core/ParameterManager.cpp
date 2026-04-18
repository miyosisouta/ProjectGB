/**
 * ParameterManager.cpp
 *
 * パラメーター管理
 * ステータスなどの数値を外部ファイルから読み込んで使用する
 */
#include "stdafx.h"
#include "ParameterManager.h"


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
			p.hp								= j.value("hp",							0);
			p.attack							= j.value("attack",						0);
			p.criticalRate						= j.value("criticalRate",				0);
			p.criticalDamageMultiplier			= j.value("criticalDamageMultiplier",	0.0f);
			p.stamina.maxStamina				= j.value("stamina",					0.0f);
			p.stamina.drainPerSec				= j.value("drainPerSec",				0.0f);
			p.stamina.recoverPerSec				= j.value("recoverPerSec",				0.0f);
			p.stamina.exhaustedRecoverPerSec	= j.value("exhaustedRecoverPerSec",		0.0f);
			p.stamina.exhaustedThreshold		= j.value("exhaustedThreshold",			0.0f);
			p.stamina.exhaustedSpeedRate		= j.value("exhaustedSpeedRate",			0.0f);
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
