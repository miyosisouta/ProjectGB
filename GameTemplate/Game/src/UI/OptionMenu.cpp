/**
 * OptionMenu.cpp
 * オプションメニュー
 */
#include "stdafx.h"
#include "OptionMenu.h"
#include "UIAnimationFactory.h"

#include "src/sound/SoundManager.h"


namespace
{
	static int selectOptionType = 0;
	static constexpr int SOUND_OPTION_TYPE = 0;
	static constexpr int KEY_OPTION_TYPE = 1;
	static constexpr int CAMERA_OPTION_TYPE = 2;
}



void OptionMenu::Update()
{
	// タスクスケジューラー
	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());

	// このフレームだけ差分をみたいので一時的に取得
	int oldSelectOptionType = selectOptionType;

	// 各種設定の選択
	{
		if (g_pad[0]->IsTrigger(enButtonRight)) {
			selectOptionType++;
			if (selectOptionType > CAMERA_OPTION_TYPE) {
				selectOptionType = CAMERA_OPTION_TYPE;
			} else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonLeft)) {
			selectOptionType--;
			if (selectOptionType < SOUND_OPTION_TYPE) {
				selectOptionType = SOUND_OPTION_TYPE;
			} else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}


	// 選択中のカーソル処理
	{
		// 青色の枠
		auto* selectFrame = GetUI<UIIcon>(Hash32("flame"));
		float startPosX = -550.0f;
		float offsetX = 550.0f;
		switch (selectOptionType)
		{
			case SOUND_OPTION_TYPE:
			{
				startPosX;
				selectFrame->isDraw = true;
				break;
			}
			case KEY_OPTION_TYPE:
			{
				startPosX += offsetX;
				selectFrame->isDraw = true;
				break;
			}
			case CAMERA_OPTION_TYPE:
			{
				startPosX += (offsetX * 2);
				selectFrame->isDraw = true;
				break;
			}
		}
		selectFrame->transform.localPosition.x = startPosX;
	}
	// 選択中のアイコン
	{
		if (oldSelectOptionType != selectOptionType)
		{
			// タスクスケジューラーを消す
			taskScheduler->ClearSchedule();

			// 各アイコンの初期値
			ResetIcon();

			switch (selectOptionType)
			{
			case SOUND_OPTION_TYPE:
			{
				SoundIconAnimation();
				break;
			}
			case KEY_OPTION_TYPE:
			{
				ControllerIconAnimation();
				break;
			}
			case CAMERA_OPTION_TYPE:
			{
				CameraIconAnimation();
				break;
			}
			}
			oldSelectOptionType = selectOptionType;
		}		
	}

	MenuBase::Update();
}


void OptionMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void OptionMenu::InitializeLogic()
{
	taskScheduler = std::make_unique<TaskSchedulerSystem>();

	// アニメーションアタッチ
	AnimationIconAttach();

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
			}, true);
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
			}, true);
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
			}, true);
	}

	// 歯車が回る
	auto* gear = GetUI<UIIcon>(Hash32("title_icon"));
	UIAnimationFactory::Attach<UIRotationAnimation>(gear, Hash32("gear_rotation"));
	{
		auto* animation = gear->FindAnimation(Hash32("gear_rotation"));
		animation->Play();
	}

	// 初めに選択されているときに一度だけ再生
	SoundIconAnimation();
}


void OptionMenu::AnimationIconAttach()
{
	// サウンドアイコンのアニメーションアタッチ
	auto* sound = GetUI<UIIcon>(Hash32("icon_sound"));
	UIAnimationFactory::Attach<UIScaleAnimation>(sound, Hash32("Option_soundIcon_scaleUp"));
	//auto* sound = GetUI<UIIcon>(Hash32("icon_sound"));
	UIAnimationFactory::Attach<UIRotationAnimation>(sound, Hash32("sound_rotation"));

	//
	auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
	UIAnimationFactory::Attach<UIScaleAnimation>(controller, Hash32("Option_controllerIcon_scaleUp"));
	//auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(controller, Hash32("controller_posYUp"));
	//auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(controller, Hash32("controller_posYDown"));
	//auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
	UIAnimationFactory::Attach<UIScaleAnimation>(controller, Hash32("controller_scaleDown"));
	//auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
	UIAnimationFactory::Attach<UIScaleAnimation>(controller, Hash32("controller_default"));

	//
	auto* camera = GetUI<UIIcon>(Hash32("icon_camera"));
	UIAnimationFactory::Attach<UIScaleAnimation>(camera, Hash32("Option_cameraIcon_scaleUp"));
	auto* gear1 = GetUI<UIIcon>(Hash32("icon_camera_gear"));
	UIAnimationFactory::Attach<UIScaleAnimation>(gear1, Hash32("Option_gearIcon_scaleUp"));
	auto* gear2 = GetUI<UIIcon>(Hash32("icon_camera_gear2"));
	UIAnimationFactory::Attach<UIScaleAnimation>(gear2, Hash32("Option_gear2Icon_scaleUp"));
	//auto* gear1 = GetUI<UIIcon>(Hash32("icon_camera_gear"));
	UIAnimationFactory::Attach<UIRotationAnimation>(gear1, Hash32("gear_rotation_Left"));
	//auto* gear2 = GetUI<UIIcon>(Hash32("icon_camera_gear2"));
	UIAnimationFactory::Attach<UIRotationAnimation>(gear2, Hash32("sound_rotation_Right"));

}

void OptionMenu::SoundIconAnimation()
{
	// 拡大
	auto* sound = GetUI<UIIcon>(Hash32("icon_sound"));
	//UIAnimationFactory::Attach<UIScaleAnimation>(sound, Hash32("Option_soundIcon_scaleUp"));
	auto* animation = sound->FindAnimation(Hash32("Option_soundIcon_scaleUp"));
	animation->Clear();
	animation->Play();
	// タスクスケジューラー
	const int id = taskScheduler->CreateLoopSequence(4.0f);
	taskScheduler->AddLoopTimer(id, 2.0f, [&]()
		{
			auto* sound = GetUI<UIIcon>(Hash32("icon_sound"));
			//UIAnimationFactory::Attach<UIRotationAnimation>(sound, Hash32("sound_rotation"));
			auto* animation = sound->FindAnimation(Hash32("sound_rotation"));
			animation->Play();
		});
	taskScheduler->AddLoopTimer(id, 3.4f, [&]()
		{
			auto* sound = GetUI<UIIcon>(Hash32("icon_sound"));
			//UIAnimationFactory::Attach<UIRotationAnimation>(sound, Hash32("sound_rotation"));
			auto* animation = sound->FindAnimation(Hash32("sound_rotation"));
			animation->Stop();
		}, false);
}


void OptionMenu::ControllerIconAnimation()
{
	// 拡大
	auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
	//UIAnimationFactory::Attach<UIScaleAnimation>(controller, Hash32("Option_controllerIcon_scaleUp"));
	auto* animation = controller->FindAnimation(Hash32("Option_controllerIcon_scaleUp"));
	animation->Clear();
	animation->Play();
	// タスクスケジューラーでループ再生
	const int id = taskScheduler->CreateLoopSequence(4.0f);
	taskScheduler->AddLoopTimer(id, 2.0, [&]()
		{
			auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
			//UIAnimationFactory::Attach<UITranslateOffsetAnimation>(controller, Hash32("controller_posYUp"));
			auto* animation = controller->FindAnimation(Hash32("controller_posYUp"));
			animation->Clear();
			animation->Play();
		});
	taskScheduler->AddLoopTimer(id, 2.3f, [&]()
		{
			auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
			//UIAnimationFactory::Attach<UITranslateOffsetAnimation>(controller, Hash32("controller_posYUp"));
			auto* animation = controller->FindAnimation(Hash32("controller_posYUp"));
			animation->Stop();
		}, false);
	taskScheduler->AddLoopTimer(id, 2.4, [&]()
		{
			auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
			//UIAnimationFactory::Attach<UITranslateOffsetAnimation>(controller, Hash32("controller_posYDown"));
			auto* animation = controller->FindAnimation(Hash32("controller_posYDown"));
			animation->Clear();
			animation->Play();
		});
	taskScheduler->AddLoopTimer(id, 2.7f, [&]()
		{
			auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
			//UIAnimationFactory::Attach<UITranslateOffsetAnimation>(controller, Hash32("controller_posYDown"));
			auto* animation = controller->FindAnimation(Hash32("controller_posYDown"));
			animation->Stop();
		}, false);
	taskScheduler->AddLoopTimer(id, 2.6, [&]()
		{
			auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
			//controller->SetPivot(Vector2(0.5f, 0.0f));
			//UIAnimationFactory::Attach<UIScaleAnimation>(controller, Hash32("controller_scaleDown"));
			auto* animation = controller->FindAnimation(Hash32("controller_scaleDown"));
			animation->Clear();
			animation->Play();
		});
	taskScheduler->AddLoopTimer(id, 3.0f, [&]()
		{
			auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
			//UIAnimationFactory::Attach<UIScaleAnimation>(controller, Hash32("controller_scaleDown"));
			auto* animation = controller->FindAnimation(Hash32("controller_scaleDown"));
			animation->Stop();
		}, false);
	taskScheduler->AddLoopTimer(id, 2.9, [&]()
		{
			auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
			//UIAnimationFactory::Attach<UIScaleAnimation>(controller, Hash32("controller_default"));
			auto* animation = controller->FindAnimation(Hash32("controller_default"));
			animation->Clear();
			animation->Play();
		});
	taskScheduler->AddLoopTimer(id, 3.3f, [&]()
		{
			auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
			//UIAnimationFactory::Attach<UIScaleAnimation>(controller, Hash32("controller_default"));
			auto* animation = controller->FindAnimation(Hash32("controller_default"));
			animation->Stop();
		}, false);
}


void OptionMenu::CameraIconAnimation()
{
	// 拡大
	auto* camera = GetUI<UIIcon>(Hash32("icon_camera"));
	//UIAnimationFactory::Attach<UIScaleAnimation>(camera, Hash32("Option_cameraIcon_scaleUp"));
	auto* animation = camera->FindAnimation(Hash32("Option_cameraIcon_scaleUp"));
	animation->Clear();
	animation->Play();
	// 拡大
	auto* gear1 = GetUI<UIIcon>(Hash32("icon_camera_gear"));
	//UIAnimationFactory::Attach<UIScaleAnimation>(gear1, Hash32("Option_gearIcon_scaleUp"));
	auto* gear1Anim = gear1->FindAnimation(Hash32("Option_gearIcon_scaleUp"));
	animation->Clear();
	gear1Anim->Play();
	// 拡大
	auto* gear2 = GetUI<UIIcon>(Hash32("icon_camera_gear2"));
	//UIAnimationFactory::Attach<UIScaleAnimation>(gear2, Hash32("Option_gear2Icon_scaleUp"));
	auto* gear2Anim = gear2->FindAnimation(Hash32("Option_gear2Icon_scaleUp"));
	animation->Clear();
	gear2Anim->Play();

	// タスクスケジューラーでループ再生
	const int id = taskScheduler->CreateLoopSequence(4.0f);
	taskScheduler->AddLoopTimer(id, 2.0, [&]()
		{
			auto* gear1 = GetUI<UIIcon>(Hash32("icon_camera_gear"));
			//UIAnimationFactory::Attach<UIRotationAnimation>(gear1, Hash32("gear_rotation_Left"));
			auto* gear1Anim = gear1->FindAnimation(Hash32("gear_rotation_Left"));
			//
			auto* gear2 = GetUI<UIIcon>(Hash32("icon_camera_gear2"));
			//UIAnimationFactory::Attach<UIRotationAnimation>(gear2, Hash32("sound_rotation_Right"));
			auto* gear2Anim = gear2->FindAnimation(Hash32("sound_rotation_Right"));
			gear1Anim->Play();
			gear2Anim->Play();
		});
	taskScheduler->AddLoopTimer(id, 3.2f, [&]()
		{
			auto* gear1 = GetUI<UIIcon>(Hash32("icon_camera_gear"));
			//UIAnimationFactory::Attach<UIRotationAnimation>(gear1, Hash32("gear_rotation_Left"));
			auto* gear1Anim = gear1->FindAnimation(Hash32("gear_rotation_Left"));
			auto* gear2 = GetUI<UIIcon>(Hash32("icon_camera_gear2"));
			//UIAnimationFactory::Attach<UIRotationAnimation>(gear2, Hash32("sound_rotation_Right"));
			auto* gear2Anim = gear2->FindAnimation(Hash32("sound_rotation_Right"));
			gear2Anim->Stop();
			gear1Anim->Stop();
		}, false);
}


void OptionMenu::ResetIcon()
{
	// サウンドアイコン
	auto* sound = GetUI<UIIcon>(Hash32("icon_sound"));
	auto* soundInit = GetUI<UIDummy>(Hash32("InitSoundIcon"));
	// アニメーション停止
	if (auto* animation = GetUI<UIIcon>(Hash32("icon_sound")))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	sound->transform.localScale = soundInit->transform.localScale;
	sound->transform.localRotation = soundInit->transform.localRotation;

	// コントローラーアイコン
	auto* controller = GetUI<UIIcon>(Hash32("icon_controller"));
	auto* controllerInit = GetUI<UIDummy>(Hash32("InitControllerIcon"));
	// アニメーション停止
	if (auto* animation = GetUI<UIIcon>(Hash32("icon_controller")))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	controller->transform.localScale = controllerInit->transform.localScale;
	controller->transform.localPosition = controllerInit->transform.localPosition;

	// カメラアイコン
	auto* camera = GetUI<UIIcon>(Hash32("icon_camera"));
	auto* cameraInit = GetUI<UIDummy>(Hash32("InitCameraIcon"));
	// アニメーション停止
	if (auto* animation = GetUI<UIIcon>(Hash32("icon_camera")))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	camera->transform.localScale = cameraInit->transform.localScale;
	
	// 歯車1アイコン
	auto* gear1 = GetUI<UIIcon>(Hash32("icon_camera_gear"));
	auto* gear1Init = GetUI<UIDummy>(Hash32("InitGear1Icon"));
	// アニメーション停止
	if (auto* animation = GetUI<UIIcon>(Hash32("icon_camera_gear")))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	gear1->transform.localScale = gear1Init->transform.localScale;
	gear1->transform.localRotation = gear1Init->transform.localRotation;

	// 歯車2アイコン
	auto* gear2 = GetUI<UIIcon>(Hash32("icon_camera_gear2"));
	auto* gear2Init = GetUI<UIDummy>(Hash32("InitGear2Icon"));
	// アニメーション停止
	if (auto* animation = GetUI<UIIcon>(Hash32("icon_camera_gear2")))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	gear2->transform.localScale = gear2Init->transform.localScale;
	gear2->transform.localRotation = gear2Init->transform.localRotation;
}


bool OptionMenu::IsSelectSound() const
{
	// 0は「サウンド」なので
	//return selector_->GetValue() == 0;
	return selectOptionType == SOUND_OPTION_TYPE;
}


bool OptionMenu::IsSelectKeyConfig() const
{
	// 1は「キー」なので
	//return selector_->GetValue() == 1;
	return selectOptionType == KEY_OPTION_TYPE;
}


bool OptionMenu::IsSelectCamera() const
{
	// 2は「カメラ」なので
	//return selector_->GetValue() == 2;
	return selectOptionType == CAMERA_OPTION_TYPE;
}