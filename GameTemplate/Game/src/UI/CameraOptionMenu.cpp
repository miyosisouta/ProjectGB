/**
 * CameraOptionMenu.cpp
 * カメラオプションメニュー
 */
#include "stdafx.h"
#include "CameraOptionMenu.h"
#include "UIAnimationFactory.h"
#include "Layout.h"

#include "src/Camera/CameraManager.h"

namespace
{
	// カメラ設定の選択
	static int selectCameraType = 0;
	static constexpr int CAMERA_SENS_TYPE = 0;
	static constexpr int CAMERA_LENGTH_TYPE = 1;
	static constexpr int CAMERA_INVERSION_TYPE = 2;
	static constexpr int DEFAULT_CAMERA_TYPE = 3;

	// カメラ距離の選択
	static int selectDistanceType = 0;
	static constexpr int BUTTON_DISTANCE_NEAR = 0;
	static constexpr int BUTTON_DISTANCE_NORMAL = 1;
	static constexpr int BUTTON_DISTANCE_FAR = 2;

	// 左右反転の選択
	static int selectInversionType = 0;
	static constexpr int BUTTON_OFF = 0;
	static constexpr int BUTTON_ON = 1;
}

void CameraOptionMenu::Update()
{
	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());


	// ボリューム種類選択
	{
		if (g_pad[0]->IsTrigger(enButtonUp)) {
			selectCameraType--;
			if (selectCameraType < CAMERA_SENS_TYPE) {
				selectCameraType = CAMERA_SENS_TYPE;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonDown)) {
			selectCameraType++;
			if (selectCameraType > DEFAULT_CAMERA_TYPE) {
				selectCameraType = DEFAULT_CAMERA_TYPE;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}



	//	カメラ距離のみの選択
	{
		// カメラ距離が選択中のときのみ
		if (selectCameraType == CAMERA_LENGTH_TYPE)
		{
			if (g_pad[0]->IsTrigger(enButtonRight)) {
				selectDistanceType++;
				if (selectDistanceType > BUTTON_DISTANCE_FAR) {
					selectDistanceType = BUTTON_DISTANCE_FAR;
				}
				else {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
				}
			}
			else if (g_pad[0]->IsTrigger(enButtonLeft)) {
				selectDistanceType--;
				if (selectDistanceType < BUTTON_DISTANCE_NEAR) {
					selectDistanceType = BUTTON_DISTANCE_NEAR;
				}
				else {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
				}
			}
		}

		// Type選択をCameraManagerに設定
		switch (selectDistanceType)
		{
			case BUTTON_DISTANCE_NEAR:
			{
				CameraManager::Get().SetDistance(DISTANCE_NEAR);
				break;
			}
			case BUTTON_DISTANCE_NORMAL:
			{
				CameraManager::Get().SetDistance(DISTANCE_NORMAL);
				break;
			}
			case BUTTON_DISTANCE_FAR:
			{
				CameraManager::Get().SetDistance(DISTANCE_FAR);
				break;
			}
		}
	}


	// 左右反転のみの選択
	{
		// 左右反転が選択中のときのみ
		if (selectCameraType == CAMERA_INVERSION_TYPE)
		{
			if (g_pad[0]->IsTrigger(enButtonRight)) {
				selectInversionType++;
				if (selectInversionType > BUTTON_ON) {
					selectInversionType = BUTTON_ON;
				}
				else {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
				}
			}
			else if (g_pad[0]->IsTrigger(enButtonLeft)) {
				selectInversionType--;
				if (selectInversionType < BUTTON_OFF) {
					selectInversionType = BUTTON_OFF;
				}
				else {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
				}
			}
		}
	}



	// デフォルト
	if (selectCameraType == DEFAULT_CAMERA_TYPE) {
		if (g_pad[0]->IsTrigger(enButtonA)) {
			// デフォルトの値
			sensitiityStage_ = static_cast<float>(SENSITIVITY_DEFAULT_STAGE);
			selectDistanceType = BUTTON_DISTANCE_NORMAL;
			selectInversionType = BUTTON_OFF;

			CameraManager::Get().SetSensitivityStage(SENSITIVITY_DEFAULT_STAGE);
			CameraManager::Get().SetDistance(DISTANCE_NORMAL);
			CameraManager::Get().SetInvert(INVERT_OFF);

			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
		}

		// 選択中の時にでかくなる
		auto* buttonCanvs = GetUI<UICanvas>(Hash32("DefaultIcon"));
		buttonCanvs->transform.localScale = Vector3(1.2f);
		// 背景の色が変わる
		auto* normalBack = GetUI<UIIcon>(Hash32("DefaultIcon/Button_Default_back"));
		auto* selectColorDummy = GetUI<UIDummy>(Hash32("DefaultIcon/BackSelectColorDummy"));
		if (normalBack) {
			normalBack->color = selectColorDummy->color;
		}
		// 枠の色が変わる
		auto* nomalFlame = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_flame"));
		auto* selectFlameColorDummy = GetUI<UIDummy>(Hash32("DefaultIcon/FlameSelectColorDummy"));
		if (nomalFlame) {
			nomalFlame->color = selectFlameColorDummy->color;
		}
	}
	else {
		auto* buttonCanvs = GetUI<UICanvas>(Hash32("DefaultIcon"));
		buttonCanvs->transform.localScale = Vector3(1.0f);
		// 背景の色が変わる
		auto* normalBack = GetUI<UIIcon>(Hash32("DefaultIcon/Button_Default_back"));
		auto* normalColorDummy = GetUI<UIDummy>(Hash32("DefaultIcon/BackNormalColorDummy"));
		normalBack->color = normalColorDummy->color;
		// 枠の色が変わる
		auto* nomalFlame = GetUI<UIIcon>(Hash32("DefaultIcon/Button_Default_flame"));
		auto* selectFlameColorDummy = GetUI<UIDummy>(Hash32("DefaultIcon/FlameNormalColorDummy"));
		nomalFlame->color = selectFlameColorDummy->color;

	}


	// 選択中
	{
		// 青緑の枠
		auto* selectFrame = GetUI<UIIcon>(Hash32("flame"));
		float posY = 250.0f;
		float offsetY = 225.0f;
		switch (selectCameraType)
		{
		case CAMERA_SENS_TYPE:
		{
			posY = 250.0f;
			selectFrame->isDraw = true;
			break;
		}
		case CAMERA_LENGTH_TYPE:
		{
			posY = 250.0f - offsetY;
			selectFrame->isDraw = true;
			break;
		}
		case CAMERA_INVERSION_TYPE:
		{
			posY = 250.0f - (offsetY * 2.0f);
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

	// カメラ距離の選択中
	{
		// 選択されているときにボタンの色が緑色になる
		auto* selectNear = GetUI<UIIcon>(Hash32("selected_back_near"));
		auto* selectNormal = GetUI<UIIcon>(Hash32("selected_back_normal"));
		auto* selectFar = GetUI<UIIcon>(Hash32("selected_back_far"));
		switch (selectDistanceType)
		{
		case BUTTON_DISTANCE_NEAR:
		{
			selectNear->isDraw = true;
			selectNormal->isDraw = false;
			selectFar->isDraw = false;
			break;
		}
		case BUTTON_DISTANCE_NORMAL:
		{
			selectNear->isDraw = false;
			selectNormal->isDraw = true;
			selectFar->isDraw = false;
			break;
		}
		case BUTTON_DISTANCE_FAR:
		{
			selectNear->isDraw = false;
			selectNormal->isDraw = false;
			selectFar->isDraw = true;
			break;
		}
		}
	}

	// カメラ反転の選択中
	{
		// 選択されているときにボタンの色が緑色になる
		auto* selectButtonOff = GetUI<UIIcon>(Hash32("inversion_selected_button_off"));
		auto* selectButtonOn = GetUI<UIIcon>(Hash32("inversion_selected_button_on"));
		switch (selectInversionType)
		{
			case BUTTON_OFF:
			{
				selectButtonOff->isDraw = true;
				selectButtonOn->isDraw = false;
				break;
			}
			case BUTTON_ON:
			{
				selectButtonOn->isDraw = true;
				selectButtonOff->isDraw = false;
				break;
			}
		}

		CameraManager::Get().SetInvert(selectInversionType == BUTTON_ON ? INVERT_ON : INVERT_OFF);

	}


	// カメラ感度ゲージの更新
	UpdateSensitivityBar(Hash32("Ber_greenback1"), CAMERA_SENS_TYPE);
	UpdateMaru(Hash32("bar_maru1"), Hash32("bar_waku1"), sensitiityStage_);

	MenuBase::Update();
}


void CameraOptionMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void CameraOptionMenu::InitializeLogic()
{
	taskScheduler = std::make_unique<TaskSchedulerSystem>();

	// カメラ設定の初期値を読み込む
	sensitiityStage_ = static_cast<float>(CameraManager::Get().GetSensitivityStage());

	// 距離の初期設定
	const float currendtDistance = CameraManager::Get().GetDistance();
	if (currendtDistance <= DISTANCE_NEAR) {
		selectDistanceType = BUTTON_DISTANCE_NEAR;
	}
	else if (currendtDistance >= DISTANCE_FAR) {
		selectDistanceType = BUTTON_DISTANCE_FAR;
	}
	else {
		selectDistanceType = BUTTON_DISTANCE_NORMAL;
	}

	// 反転の初期選択
	selectInversionType = CameraManager::Get().GetInvert() ? BUTTON_ON : BUTTON_OFF;

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

	auto* gauge1 = GetUI<UIIcon>(Hash32("Ber_greenback1"));
	gauge1->SetPivot(Vector2(0.0f, 0.5f));
}

void CameraOptionMenu::UpdateSensitivityBar(const uint32_t gaugeId, const int volumeType)
{
	auto* gauge = GetUI<UIIcon>(gaugeId);
	if (gauge == nullptr) {
		return;
	}
	if (selectCameraType == volumeType) {
		if (g_pad[0]->IsTrigger(enButtonLeft)) {
			sensitiityStage_ -= 1.0f;
			if (sensitiityStage_ < static_cast<float>(SENSITIVITY_MIN_STAGE)) {
				sensitiityStage_ = static_cast<float>(SENSITIVITY_MIN_STAGE);
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}

		}
		else if (g_pad[0]->IsTrigger(enButtonRight)) {
			sensitiityStage_ += 1.0f;
			if (sensitiityStage_ > static_cast<float>(SENSITIVITY_MAX_STAGE)) {
				sensitiityStage_ = static_cast<float>(SENSITIVITY_MAX_STAGE);
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}

	// 段階を実数値にして変換
	//const float sensitivityValue = SENSITIVITY_BASE + (sensitiityStage_ * SENSITIVITY_STEP);
	CameraManager::Get().SetSensitivityStage(static_cast<int>(sensitiityStage_));

	// ゲージはデフォルトに戻すがあるので、選択中限らず更新する
	gauge->transform.localScale.x = 12.0f * ((sensitiityStage_ - static_cast<float>(SENSITIVITY_MIN_STAGE)) / static_cast<float>(SENSITIVITY_MAX_STAGE - SENSITIVITY_MIN_STAGE));
}


void CameraOptionMenu::UpdateMaru(const uint32_t maruId, const uint32_t wakuId, const float volume)
{
	float maxPos = 600.0f;
	float minPos = -380.0f;
	float length = minPos - maxPos;

	// 11段階(1~11)の計算
	 // stage1のとき minPos、stage11のとき maxPos になるよう正規化
	const float stage = (volume - static_cast<float>(SENSITIVITY_MIN_STAGE)) / static_cast<float>(SENSITIVITY_MAX_STAGE - SENSITIVITY_MIN_STAGE);
	float pos = maxPos + (length * (1.0f - stage));

	auto* maru = GetUI<UIIcon>(maruId);
	maru->transform.localPosition.x = pos;
	auto* waku = GetUI<UIIcon>(wakuId);
	waku->transform.localPosition.x = pos;
}
