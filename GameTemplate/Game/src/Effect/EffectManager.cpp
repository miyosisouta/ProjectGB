/**
 * EffectManager.cpp
 * 
 * エフェクト管理
 * 必要なエフェクトファイルを読み込んだり再生したりなど管理する
 */


#include "stdafx.h"
#include "EffectManager.h"


EffectManager* EffectManager::myInstance_ = nullptr; //初期化


EffectManager::EffectManager()
{
	effectList_.clear();

	// エフェクトの登録
	for (int i = 0; i < ARRAYSIZE(effectInformation); ++i) {
		const auto& info = effectInformation[i];
		EffectEngine::GetInstance()->ResistEffect(i, info.assetPath);
	}
}


EffectManager::~EffectManager()
{
}


void EffectManager::Update()
{
}


EffectHandle EffectManager::PlayEffect(const int kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
	// ハンドルが最大数になったら使えない
	// NOTE: そんなに再生するはずがない
	if (effectHandleCount_ == INVALID_EFFECT_HANDLE) {
		K2_ASSERT(false, "エフェクトの再生が多いです。\n");
		return INVALID_EFFECT_HANDLE;
	}
	EffectEmitter* m_effect = NewGO<EffectEmitter>(0);
	m_effect->Init(kind);
	m_effect->SetPosition(position);
	m_effect->SetRotation(rotation);
	m_effect->SetScale(scale);
	m_effect->Play();

	return effectHandleCount_;
}


void EffectManager::StopEffect(const EffectHandle handle)
{
	auto* effect = FindEffect(handle);
	if (effect == nullptr) {
		return;
	}
	effect->Stop();
}

	


/******************************************************************************/


EffectManagerObject::EffectManagerObject()
{
	
}


EffectManagerObject::~EffectManagerObject()
{
	EffectManager::DestroyInstance();
}


bool EffectManagerObject::Start()
{
	EffectManager::CreateInstance();

	return true;
}


void EffectManagerObject::Update()
{
	EffectManager::Get().Update();
}