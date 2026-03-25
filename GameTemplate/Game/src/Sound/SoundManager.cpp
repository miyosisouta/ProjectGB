/**
 * SoundManager.cpp
 * 
 * サウンド管理
 * 必要なサウンドファイルを読み込んだり再生したりなど管理する
 */


#include "stdafx.h"
#include "SoundManager.h"


SoundManager* SoundManager::myInstance_ = nullptr; //初期化


SoundManager::SoundManager()
{
	seList_.clear();

	// サウンドの登録
	for (int i = 0; i < ARRAYSIZE(soundInformation); ++i) {
		const auto& info = soundInformation[i];
		g_soundEngine->ResistWaveFileBank(i, info.assetPath.c_str());
	}
}


SoundManager::~SoundManager()
{
}


void SoundManager::Update()
{
	std::vector<SoundHandle> eraseList;
	for (auto& it : seList_) {
		const auto key = it.first;
		auto* se = it.second;
		// 再生が終わっているなら削除
		if (!se->IsPlaying()) {
			eraseList.push_back(key);
		} else {
			// SEのボリュームを変更する必要があるなら変更
			if (isChangeVolume_) {
				se->SetVolume(ComputeVolume(SoundVolumeType::SE));
			}
		}
	}
	for (const auto& key : eraseList) {
		seList_.erase(key);
	}

	if (isChangeVolume_) {
		// BGMのボリュームを変更する必要があるなら変更
		if (bgm_ != nullptr) {
			bgm_->SetVolume(ComputeVolume(SoundVolumeType::BGM));
		}
		isChangeVolume_ = false;
	}
}


void SoundManager::PlayBGM(const int kind)
{
	// BGMが生成されていない
	if (bgm_ == nullptr) {
		// 生成
		bgm_ = NewGO<SoundSource>(0, "bgm");
	}
	else {
		// すでに生成されているならBGMを停止する
		bgm_->Stop();
	}
	// 初期化
	bgm_->Init(kind);
	bgm_->SetVolume(ComputeVolume(SoundVolumeType::BGM));
	bgm_->Play(true);	// BGMなのでループ再生する
}


void SoundManager::StopBGM()
{
	if (bgm_ == nullptr) {
		return;
	}
	bgm_->Stop();
}


SoundHandle SoundManager::PlaySE(const int kind, const bool isLood, const bool is3D)
{
	// ハンドルが最大数になったら使えない
	// NOTE: そんなに再生するはずがない
	if (soundHandleCount_ == INVALID_SOUND_HANDLE) {
		K2_ASSERT(false, "サウンドの再生が多いです。\n");
		return INVALID_SOUND_HANDLE;
	}
	auto* se = NewGO<SoundSource>(0, "se");
	se->Init(kind, is3D);
	se->SetVolume(ComputeVolume(SoundVolumeType::SE));
	se->Play(isLood);

	seList_.emplace(soundHandleCount_++, se);

	return soundHandleCount_;
}


void SoundManager::StopSE(const SoundHandle handle)
{
	auto* se = FindSE(handle);
	if (se == nullptr) {
		return;
	}
	se->Stop();
}


/******************************************************************************/


SoundManagerObject::SoundManagerObject()
{
	SoundManager::CreateInstance();
}


SoundManagerObject::~SoundManagerObject()
{
	SoundManager::DestroyInstance();
}


bool SoundManagerObject::Start()
{
	return true;
}


void SoundManagerObject::Update()
{
	SoundManager::Get().Update();
}	