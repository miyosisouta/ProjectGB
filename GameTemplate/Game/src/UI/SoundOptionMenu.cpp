/**
 * SoundOptionMenu.cpp
 * サウンドオプションメニュー
 */
#include "stdafx.h"
#include "SoundOptionMenu.h"
#include "UIAnimationFactory.h"

#include "src/Util/TaskSchedulerSystem.h"


namespace
{
	// @todo for test
	static UIIcon* nikukyu[4];
	static TaskSchedulerSystem* taskScheduler = nullptr;


	static float masterVolume = 10.0f;
	static float bgmVolume = 10.0f;
	static float seVolume = 10.0f;

	static int selectVolumetType = 0;
	static constexpr int MASTER_VOLUME_TYPE = 0;
	static constexpr int BGM_VOLUME_TYPE = 1;
	static constexpr int SE_VOLUME_TYPE = 2;
	static constexpr int DEFAULT_VOLUME_TYPE = 3;

}


void SoundOptionMenu::Update()
{
	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());

	// ボリューム種類選択
	{
		if (g_pad[0]->IsTrigger(enButtonUp)) {
			selectVolumetType--;
			if (selectVolumetType < 0) {
				selectVolumetType = 0;
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonDown)) {
			selectVolumetType++;
			if (selectVolumetType > 3) {
				selectVolumetType = 3;
			}
		}
	}


	// Master
	
	{
		auto* gauge = GetUI<UIIcon>(Hash32("Ber_greenback1"));
		if (gauge) {
			if (selectVolumetType == MASTER_VOLUME_TYPE) {
				if (g_pad[0]->IsTrigger(enButtonLeft)) {
					masterVolume -= 1.0f;
					if (masterVolume < 0.0f) {
						masterVolume = 0.0f;
					}
				}
				else if (g_pad[0]->IsTrigger(enButtonRight)) {
					masterVolume += 1.0f;
					if (masterVolume > 10.0f) {
						masterVolume = 10.0f;
					}
				}
			}

			gauge->transform.localScale.x = (12.0f * (masterVolume / 10.0f));
		}

		// まる
		{
			float maxPos = 600.0f;
			float minPos = -380.0f;
			float length = minPos - maxPos;

			float pos = maxPos + (length * (1.0f - (masterVolume / 10.0f)));

			auto* maru = GetUI<UIIcon>(Hash32("bar_maru1"));
			maru->transform.localPosition.x = pos;
			auto* waku = GetUI<UIIcon>(Hash32("bar_waku1"));
			waku->transform.localPosition.x = pos;
		}


		// スピーカー
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker0"));
			speaker->isDraw = masterVolume <= 0.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker1"));
			speaker->isDraw = masterVolume > 0.0f && masterVolume < 3.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker2"));
			speaker->isDraw = masterVolume >= 3.0f && masterVolume < 6.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker3"));
			speaker->isDraw = masterVolume >= 6.0f;
		}
	}
	// BGM
	{
		auto* gauge = GetUI<UIIcon>(Hash32("Ber_greenback2"));
		if (gauge) {
			if (selectVolumetType == BGM_VOLUME_TYPE)
			{
				if (g_pad[0]->IsTrigger(enButtonLeft)) {
					bgmVolume -= 1.0f;
					if (bgmVolume < 0.0f) {
						bgmVolume = 0.0f;
					}
				}
				else if (g_pad[0]->IsTrigger(enButtonRight)) {
					bgmVolume += 1.0f;
					if (bgmVolume > 10.0f) {
						bgmVolume = 10.0f;
					}
				}
			}

			gauge->transform.localScale.x = (12.0f * (bgmVolume / 10.0f));
		}

		// まる
		{
			float maxPos = 600.0f;
			float minPos = -380.0f;
			float length = minPos - maxPos;

			float pos = maxPos + (length * (1.0f - (bgmVolume / 10.0f)));

			auto* maru = GetUI<UIIcon>(Hash32("bar_maru2"));
			maru->transform.localPosition.x = pos;
			auto* waku = GetUI<UIIcon>(Hash32("bar_waku2"));
			waku->transform.localPosition.x = pos;
		}


		// スピーカー
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker00"));
			speaker->isDraw = bgmVolume <= 0.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker01"));
			speaker->isDraw = bgmVolume > 0.0f && bgmVolume < 3.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker02"));
			speaker->isDraw = bgmVolume >= 3.0f && bgmVolume < 6.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker03"));
			speaker->isDraw = bgmVolume >= 6.0f;
		}
	}
	// SE
	{
		auto* gauge = GetUI<UIIcon>(Hash32("Ber_greenback3"));
		if (gauge) {
			if (selectVolumetType == SE_VOLUME_TYPE) {
				if (g_pad[0]->IsTrigger(enButtonLeft)) {
					seVolume -= 1.0f;
					if (seVolume < 0.0f) {
						seVolume = 0.0f;
					}
				}
				else if (g_pad[0]->IsTrigger(enButtonRight)) {
					seVolume += 1.0f;
					if (seVolume > 10.0f) {
						seVolume = 10.0f;
					}
				}
			}

			gauge->transform.localScale.x = (12.0f * (seVolume / 10.0f));
		}


		// まる
		{
			float maxPos = 600.0f;
			float minPos = -380.0f;
			float length = minPos - maxPos;

			float pos = maxPos + (length * (1.0f - (seVolume / 10.0f)));

			auto* maru = GetUI<UIIcon>(Hash32("bar_maru3"));
			maru->transform.localPosition.x = pos;
			auto* waku = GetUI<UIIcon>(Hash32("bar_waku3"));
			waku->transform.localPosition.x = pos;
		}


		// スピーカー
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker000"));
			speaker->isDraw = seVolume <= 0.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker001"));
			speaker->isDraw = seVolume > 0.0f && seVolume < 3.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker002"));
			speaker->isDraw = seVolume >= 3.0f && seVolume < 6.0f;
		}
		{
			auto* speaker = GetUI<UIIcon>(Hash32("speaker003"));
			speaker->isDraw = seVolume >= 6.0f;
		}
	}

	// デフォルト
	if (selectVolumetType == DEFAULT_VOLUME_TYPE) {
		if (g_pad[0]->IsTrigger(enButtonA)) {
			masterVolume = 10.0f;
			bgmVolume = 10.0f;
			seVolume = 10.0f;
		}
	}

	// 選択中
	{
		auto* selectFrame = GetUI<UIIcon>(Hash32("flame"));
		float posY = 250.0f;
		float offsetY = 225.0f;
		switch (selectVolumetType)
		{
			case MASTER_VOLUME_TYPE:
			{
				posY = 250.0f;
				selectFrame->isDraw = true;
				break;
			}
			case BGM_VOLUME_TYPE:
			{
				posY = 250.0f - offsetY;
				selectFrame->isDraw = true;
				break;
			}
			case SE_VOLUME_TYPE:
			{
				posY = 250.0f - (offsetY *2.0f);
				selectFrame->isDraw = true;
				break;
			}
			default:
			{
				selectFrame->isDraw = false;
				break;
			}
		}


		selectFrame->transform.localPosition.y = posY;
	}
	


	MenuBase::Update();
}


void SoundOptionMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void SoundOptionMenu::InitializeLogic()
{
	if (taskScheduler) {
		delete taskScheduler;
	}
	taskScheduler = new TaskSchedulerSystem();


	// @todo for test
	const int id = taskScheduler->CreateLoopSequence(20.0f);

	{
		nikukyu[0] = GetUI<UIIcon>(Hash32("nikukyu1"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyu[0], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 0.0f, [&]()
			{
				auto* animation = nikukyu[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 6.0f, [&]()
			{
				auto* animation = nikukyu[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			},true);
	}
	{
		nikukyu[1] = GetUI<UIIcon>(Hash32("nikukyu2"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyu[1], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 4.0f, [&]()
			{
				auto* animation = nikukyu[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 10.0f, [&]()
			{
				auto* animation = nikukyu[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyu[2] = GetUI<UIIcon>(Hash32("nikukyu3"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyu[2], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 8.0f, [&]()
			{
				auto* animation = nikukyu[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 14.0f, [&]()
			{
				auto* animation = nikukyu[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			},true);
	}
	{
		nikukyu[3] = GetUI<UIIcon>(Hash32("nikukyu4"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyu[3], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 12.0f, [&]()
			{
				auto* animation = nikukyu[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 18.0f, [&]()
			{
				auto* animation = nikukyu[3]->FindAnimation(Hash32("nikukyuu_fadein"));
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