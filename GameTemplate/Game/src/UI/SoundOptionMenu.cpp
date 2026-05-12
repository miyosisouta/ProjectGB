/**
 * SoundOptionMenu.cpp
 * サウンドオプションメニュー
 */
#include "stdafx.h"
#include "SoundOptionMenu.h"
#include "UIAnimationFactory.h"

#include "src/sound/SoundManager.h"


namespace
{
	static int selectVolumetType = 0;
	static constexpr int MASTER_VOLUME_TYPE = 0;
	static constexpr int BGM_VOLUME_TYPE = 1;
	static constexpr int SE_VOLUME_TYPE = 2;
	static constexpr int DEFAULT_VOLUME_TYPE = 3;
}


void SoundOptionMenu::Update()
{
	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());

	float masterVolume = static_cast<float>(SoundManager::Get().GetVolume(SoundVolumeType::Master));
	float bgmVolume = static_cast<float>(SoundManager::Get().GetVolume(SoundVolumeType::BGM));
	float seVolume = static_cast<float>(SoundManager::Get().GetVolume(SoundVolumeType::SE));

	// ボリューム種類選択
	{
		if (g_pad[0]->IsTrigger(enButtonUp)) {
			selectVolumetType--;
			if (selectVolumetType < MASTER_VOLUME_TYPE) {
				selectVolumetType = MASTER_VOLUME_TYPE;
			} else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonDown)) {
			selectVolumetType++;
			if (selectVolumetType > DEFAULT_VOLUME_TYPE) {
				selectVolumetType = DEFAULT_VOLUME_TYPE;
			} else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}

	// Master
	{
		UpdateBar(Hash32("Ber_greenback1"), masterVolume, MASTER_VOLUME_TYPE);
		UpdateMaru(Hash32("bar_maru1"), Hash32("bar_waku1"), masterVolume);
		UpdateSpeaker(Hash32("speaker0"), Hash32("speaker1"), Hash32("speaker2"), Hash32("speaker3"), masterVolume);
	}
	// BGM
	{
		UpdateBar(Hash32("Ber_greenback2"), bgmVolume, BGM_VOLUME_TYPE);
		UpdateMaru(Hash32("bar_maru2"), Hash32("bar_waku2"), bgmVolume);
		UpdateSpeaker(Hash32("speaker00"), Hash32("speaker01"), Hash32("speaker02"), Hash32("speaker03"), bgmVolume);
	}
	// SE
	{
		UpdateBar(Hash32("Ber_greenback3"), seVolume, SE_VOLUME_TYPE);
		UpdateMaru(Hash32("bar_maru3"), Hash32("bar_waku3"), seVolume);
		UpdateSpeaker(Hash32("speaker000"), Hash32("speaker001"), Hash32("speaker002"), Hash32("speaker003"), seVolume);
	}

	// デフォルト
	if (selectVolumetType == DEFAULT_VOLUME_TYPE) {
		if (g_pad[0]->IsTrigger(enButtonA)) {
			masterVolume = DEFAULT_VOLUME;
			bgmVolume = DEFAULT_VOLUME;
			seVolume = DEFAULT_VOLUME;

			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
		}

		// 選択中の時にでかくなる
		auto* buttonCanvs = GetUI<UICanvas>(Hash32("DefaultSoundIcon"));
		buttonCanvs->transform.localScale = Vector3(1.2f);
		// 背景の色が変わる
		auto* normalBack = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_back"));
		auto* selectColorDummy = GetUI<UIDummy>(Hash32("DefaultSoundIcon/BackSelectColorDummy"));
		if (normalBack){
			normalBack->color = selectColorDummy->color;
		}
		// 枠の色が変わる
		auto* nomalFlame = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_flame"));
		auto* selectFlameColorDummy = GetUI<UIDummy>(Hash32("DefaultSoundIcon/FlameSelectColorDummy"));
		if (nomalFlame) {
			nomalFlame->color = selectFlameColorDummy->color;
		}
	}
	else {
		auto* buttonCanvs = GetUI<UICanvas>(Hash32("DefaultSoundIcon"));
		buttonCanvs->transform.localScale = Vector3(1.0f);
		// 背景の色が変わる
		auto* normalBack = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_back"));
		auto* normalColorDummy = GetUI<UIDummy>(Hash32("DefaultSoundIcon/BackNormalColorDummy"));
		normalBack->color = normalColorDummy->color;
		// 枠の色が変わる
		auto* nomalFlame = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_flame"));
		auto* selectFlameColorDummy = GetUI<UIDummy>(Hash32("DefaultSoundIcon/FlameNormalColorDummy"));
		nomalFlame->color = selectFlameColorDummy->color;
		
	}

	// 選択中
	{
		// 黄色の背景
		auto* selectFrame = GetUI<UIIcon>(Hash32("flame"));
		// 青緑の枠
		auto* selectFrame2 = GetUI<UIIcon>(Hash32("flame2"));
		float posY = 250.0f;
		float offsetY = 225.0f;
		switch (selectVolumetType)
		{
			case MASTER_VOLUME_TYPE:
			{
				posY = 250.0f;
				selectFrame->isDraw = true;
				selectFrame2->isDraw = true;
				break;
			}
			case BGM_VOLUME_TYPE:
			{
				posY = 250.0f - offsetY;
				selectFrame->isDraw = true;
				selectFrame2->isDraw = true;
				break;
			}
			case SE_VOLUME_TYPE:
			{
				posY = 250.0f - (offsetY *2.0f);
				selectFrame->isDraw = true;
				selectFrame2->isDraw = true;
				break;
			}
			default:
			{
				selectFrame->isDraw = false;
				selectFrame2->isDraw = false;
				break;
			}
		}
		selectFrame->transform.localPosition.y = posY;
		selectFrame2->transform.localPosition.y = posY;
	}


	// ボリュームの反映
	SoundManager::Get().SetVolume(SoundVolumeType::Master, static_cast<int>(masterVolume));
	SoundManager::Get().SetVolume(SoundVolumeType::BGM, static_cast<int>(bgmVolume));
	SoundManager::Get().SetVolume(SoundVolumeType::SE, static_cast<int>(seVolume));

	MenuBase::Update();
}


void SoundOptionMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void SoundOptionMenu::InitializeLogic()
{
	taskScheduler = std::make_unique<TaskSchedulerSystem>();

	const int id = taskScheduler->CreateLoopSequence(20.0f);
	{
		nikukyuList[0] = GetUI<UIIcon>(Hash32("optionBack/nikukyu1"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList[0], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 0.0f, [&]()
			{
				auto* animation = nikukyuList[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 6.0f, [&]()
			{
				auto* animation = nikukyuList[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			},true);
	}
	{
		nikukyuList[1] = GetUI<UIIcon>(Hash32("optionBack/nikukyu2"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList[1], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 4.0f, [&]()
			{
				auto* animation = nikukyuList[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 10.0f, [&]()
			{
				auto* animation = nikukyuList[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList[2] = GetUI<UIIcon>(Hash32("optionBack/nikukyu3"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList[2], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 8.0f, [&]()
			{
				auto* animation = nikukyuList[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 14.0f, [&]()
			{
				auto* animation = nikukyuList[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			},true);
	}
	{
		nikukyuList[3] = GetUI<UIIcon>(Hash32("optionBack/nikukyu4"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList[3], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 12.0f, [&]()
			{
				auto* animation = nikukyuList[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 18.0f, [&]()
			{
				auto* animation = nikukyuList[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			},true);
	}

	
	auto* gear = GetUI<UIIcon>(Hash32("Gear"));
	UIAnimationFactory::Attach<UIRotationAnimation>(gear, Hash32("gear_rotation"));
	{
		auto* animation = gear->FindAnimation(Hash32("gear_rotation"));
		animation->Play();
	}


	auto* gauge1 = GetUI<UIIcon>(Hash32("Ber_greenback1"));
	gauge1->SetPivot(Vector2(0.0f, 0.5f));
	auto* gauge2 = GetUI<UIIcon>(Hash32("Ber_greenback2"));
	gauge2->SetPivot(Vector2(0.0f, 0.5f));
	auto* gauge3 = GetUI<UIIcon>(Hash32("Ber_greenback3"));
	gauge3->SetPivot(Vector2(0.0f, 0.5f));
}


void SoundOptionMenu::UpdateBar(const uint32_t gaugeId, float& volume, const uint32_t volumeType)
{
	auto* gauge = GetUI<UIIcon>(gaugeId);
	if (gauge == nullptr) {
		return;
	}
	if (selectVolumetType == volumeType) {
		if (g_pad[0]->IsTrigger(enButtonLeft)) {
			volume -= 1.0f;
			if (volume < 0.0f) {
				volume = 0.0f;
			} else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}

		} else if (g_pad[0]->IsTrigger(enButtonRight)) {
			volume += 1.0f;
			if (volume > static_cast<float>(MAX_VOLUME)) {
				volume = static_cast<float>(MAX_VOLUME);
			} else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}
	// ゲージはデフォルトに戻すがあるので、選択中限らず更新する
	gauge->transform.localScale.x = (12.0f * (volume / 10.0f));
}


void SoundOptionMenu::UpdateMaru(const uint32_t maruId, const uint32_t wakuId, const float volume)
{
	float maxPos = 600.0f;
	float minPos = -380.0f;
	float length = minPos - maxPos;

	float pos = maxPos + (length * (1.0f - (volume / 10.0f)));

	auto* maru = GetUI<UIIcon>(maruId);
	maru->transform.localPosition.x = pos;
	auto* waku = GetUI<UIIcon>(wakuId);
	waku->transform.localPosition.x = pos;
}


void SoundOptionMenu::UpdateSpeaker(const uint32_t idA, const uint32_t idB, const uint32_t idC, const uint32_t idD, const float volume)
{
	// スピーカー
	{
		auto* speaker = GetUI<UIIcon>(idA);
		speaker->isDraw = volume <= 0.0f;
	}
	{
		auto* speaker = GetUI<UIIcon>(idB);
		speaker->isDraw = volume > 0.0f && volume < 3.0f;
	}
	{
		auto* speaker = GetUI<UIIcon>(idC);
		speaker->isDraw = volume >= 3.0f && volume < 6.0f;
	}
	{
		auto* speaker = GetUI<UIIcon>(idD);
		speaker->isDraw = volume >= 6.0f;
	}
}