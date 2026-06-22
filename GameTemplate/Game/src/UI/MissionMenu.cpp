/**
 * MissionMenu.cpp
 * ミッションメニュー
 */
#include "stdafx.h"
#include "MissionMenu.h"
#include "UIAnimationFactory.h"
#include "src/Battle/MissionManager.h"
#include "src/CharacterDataBase.h"
#include "src/Util/TaskSchedulerSystem.h"

namespace
{
	constexpr int MAX_EFFECT_NUM = 4;
}

void MissionMenu::Update()
{
	const float deltaTime = g_gameTime->GetFrameDeltaTime();

	// クリア済みミッションのアイコンを表示状態にする
	const bool isGorilla = (CharacterDataBase::Get().GetStageType() == BossType::enGorilla);
	auto* slot1Icon = isGorilla ? GetUI<UIIcon>(Hash32("gollira_mission_1")) : GetUI<UIIcon>(Hash32("turtle_mission_1"));
	auto* slot2Icon = isGorilla ? GetUI<UIIcon>(Hash32("gollira_mission_2")) : GetUI<UIIcon>(Hash32("turtle_mission_2"));
	auto* slot3Icon = isGorilla ? GetUI<UIIcon>(Hash32("gollira_mission_3")) : GetUI<UIIcon>(Hash32("turtle_mission_3"));
	const int missionCount = MissionManager::Get().GetMissionCount();
	for (int idx = 0; idx < missionCount; idx++)
	{
		const auto* m = MissionManager::Get().GetMissionByIndex(idx);
		if (!m || !m->IsCleared()) continue;
		if (m->GetUISlot() == 1) slot1Icon->isDraw = true;
		if (m->GetUISlot() == 2) slot2Icon->isDraw = true;
		if (m->GetUISlot() == 3) slot3Icon->isDraw = true;
	}

	// タスクスケジューラー
	if (missionTaskScheduler_)
	{
		missionTaskScheduler_->Update(deltaTime);
	}

	// シーケンス更新
	missionSequence_->Update(deltaTime);
	missionObiSequence_->Update(deltaTime);
	missionMedaruSequence_->Update(deltaTime);
	mission1Sequence_->Update(deltaTime);
	mission2Sequence_->Update(deltaTime);
	mission3Sequence_->Update(deltaTime);
	missionClearSequence_->Update(deltaTime);
	skillCountSequence_->Update(deltaTime);
	kaihiCountSequence_->Update(deltaTime);
	normalAttackCountSequence_->Update(deltaTime);
	turtle1Sequence_->Update(deltaTime);
	turtle2Sequence_->Update(deltaTime);
	turtle3Sequence_->Update(deltaTime);

	// エフェクト更新
	for (int effectIdx = 0; effectIdx < MAX_EFFECT_NUM; effectIdx++)
	{
		effectRenderList_[effectIdx]->Update(deltaTime);
	}

	MenuBase::Update();

	// タイマーコールバック内でスケジューラーを壊さないよう、次再生はUpdate末尾で行う
	if (pendingPlayNext_)
	{
		pendingPlayNext_ = false;
		TryPlayNext();
	}
}


void MissionMenu::TriggerMission(int slot, int count)
{
	notificationQueue_.push({ slot, false, count });
	TryPlayNext();
}


void MissionMenu::TriggerClear(int slot, int count)
{
	notificationQueue_.push({ slot, true, count });
	TryPlayNext();
}


void MissionMenu::TryPlayNext()
{
	if (isPlaying_ || notificationQueue_.empty()) return;
	isPlaying_ = true;
	auto notif = notificationQueue_.front();
	notificationQueue_.pop();
	PlayNotification(notif);
}


void MissionMenu::PlayCountDisplay(UIDigit* digit, UIAnimationSequence* seq, int count)
{
	digit->isDraw = true;
	digit->SetNumber(count);
	digit->transform.localPosition.x = (count >= 10) ? 95.0f : 130.0f;
	seq->Play(digit);
}


void MissionMenu::PlayNotification(const MissionNotification& notif)
{
	auto* mission       = GetUI<UIIcon>(Hash32("mission_back"));
	auto* missionObi    = GetUI<UIIcon>(Hash32("mission_obi"));
	auto* missionMedaru = GetUI<UIIcon>(Hash32("noClear_medaru"));

	missionSequence_->Play(mission);
	missionObiSequence_->Play(missionObi);
	missionMedaruSequence_->Play(missionMedaru);

	const bool isGorilla = (CharacterDataBase::Get().GetStageType() == BossType::enGorilla);
	if (notif.slot == 1)
	{
		if (isGorilla)
		{
			auto* icon = GetUI<UIIcon>(Hash32("gollira_mission_1"));
			icon->isDraw = true;
			mission1Sequence_->Play(icon);
		}
		else
		{
			auto* icon = GetUI<UIIcon>(Hash32("turtle_mission_1"));
			icon->isDraw = true;
			turtle1Sequence_->Play(icon);
		}
	}
	else if (notif.slot == 2)
	{
		if (isGorilla)
		{
			auto* icon = GetUI<UIIcon>(Hash32("gollira_mission_2"));
			icon->isDraw = true;
			mission2Sequence_->Play(icon);
			PlayCountDisplay(GetUI<UIDigit>(Hash32("kaihi_count")), kaihiCountSequence_.get(), notif.count);
		}
		else
		{
			auto* icon = GetUI<UIIcon>(Hash32("turtle_mission_2"));
			icon->isDraw = true;
			turtle2Sequence_->Play(icon);
			PlayCountDisplay(GetUI<UIDigit>(Hash32("normalAttack_count")), normalAttackCountSequence_.get(), notif.count);
		}
	}
	else if (notif.slot == 3)
	{
		if (isGorilla)
		{
			auto* icon = GetUI<UIIcon>(Hash32("gollira_mission_3"));
			icon->isDraw = true;
			mission3Sequence_->Play(icon);
			PlayCountDisplay(GetUI<UIDigit>(Hash32("skill_count")), skillCountSequence_.get(), notif.count);
		}
		else
		{
			auto* icon = GetUI<UIIcon>(Hash32("turtle_mission_3"));
			icon->isDraw = true;
			turtle3Sequence_->Play(icon);
		}
	}

	missionTaskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	if (notif.isClear)
	{
		auto* clearMedaru = GetUI<UIIcon>(Hash32("clear_medaru"));
		missionClearSequence_->Play(clearMedaru);

		// 2秒後：背景を金色に、クリアメダルを出現
		missionTaskScheduler_->AddTimer(2.0f, [this]()
		{
			auto* missionBack = GetUI<UIIcon>(Hash32("mission_back"));
			UIAnimationFactory::Attach<UIColorAnimation>(missionBack, Hash32("mission_color"));
			missionBack->FindAnimation(Hash32("mission_color"))->Play();

			auto* clearMedaru = GetUI<UIIcon>(Hash32("clear_medaru"));
			clearMedaru->isDraw = true;
			UIAnimationFactory::Attach<UIScaleAnimation>(clearMedaru, Hash32("mission_medaru_scale"));
			clearMedaru->FindAnimation(Hash32("mission_medaru_scale"))->Play();
		});

		// 2.5秒後：丸エフェクト再生
		missionTaskScheduler_->AddTimer(2.5f, [this]()
		{
			for (int effectIdx = 0; effectIdx < MAX_EFFECT_NUM; effectIdx++)
			{
				effectRenderList_[effectIdx]->Play();
			}
		});

		// 5秒後：リセット → 次のキューを確認
		missionTaskScheduler_->AddTimer(5.0f, [this]()
		{
			auto* missionNormal     = GetUI<UIIcon>(Hash32("mission_back"));
			auto* missionColorDummy = GetUI<UIDummy>(Hash32("missionDummy"));
			missionNormal->color = missionColorDummy->color;

			auto* clearMedaru = GetUI<UIIcon>(Hash32("clear_medaru"));
			clearMedaru->isDraw = false;
			clearMedaru->FindAnimation(Hash32("mission_medaru_scale"))->Clear();

			isPlaying_       = false;
			pendingPlayNext_ = true; // Update()末尾で呼ぶ（コールバック内でschedulerを破壊しないため）
		});
	}
	else
	{
		// スライドアウト完了後 → 次のキューを確認
		missionTaskScheduler_->AddTimer(5.0f, [this]()
		{
			isPlaying_       = false;
			pendingPlayNext_ = true;
		});
	}
}


void MissionMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);

	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList_[i]->Draw(rc);
	}
}


void MissionMenu::InitializeLogic()
{
	GetCanvas()->transform.localPosition = Vector3{1200.0f,330.0f,0.0f};

	// ミッション背景
	auto* mission = GetUI<UIIcon>(Hash32("mission_back"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission, Hash32("missionEventEnd"));
	missionSequence_ = std::make_unique<UIAnimationSequence>();
	missionSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// ミッションの白帯
	auto* missionObi = GetUI<UIIcon>(Hash32("mission_obi"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(missionObi, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(missionObi, Hash32("missionEventEnd"));
	missionObiSequence_ = std::make_unique<UIAnimationSequence>();
	missionObiSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// ミッションのメダル
	auto* missionMedaru = GetUI<UIIcon>(Hash32("noClear_medaru"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(missionMedaru, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(missionMedaru, Hash32("missionEventEnd"));
	missionMedaruSequence_ = std::make_unique<UIAnimationSequence>();
	missionMedaruSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// ミッション１ (2分以内にクリア)
	auto* mission1 = GetUI<UIIcon>(Hash32("gollira_mission_1"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission1, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission1, Hash32("missionEventEnd"));
	mission1Sequence_ = std::make_unique<UIAnimationSequence>();
	mission1Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// ミッション2 (回避できた)
	auto* mission2 = GetUI<UIIcon>(Hash32("gollira_mission_2"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission2, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission2, Hash32("missionEventEnd"));
	mission2Sequence_ = std::make_unique<UIAnimationSequence>();
	mission2Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// ミッション3 (スキルを?回使った)
	auto* mission3 = GetUI<UIIcon>(Hash32("gollira_mission_3"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission3, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission3, Hash32("missionEventEnd"));
	mission3Sequence_ = std::make_unique<UIAnimationSequence>();
	mission3Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// カメミッション1 (2分以内にクリア)
	auto* turtle1 = GetUI<UIIcon>(Hash32("turtle_mission_1"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(turtle1, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(turtle1, Hash32("missionEventEnd"));
	turtle1Sequence_ = std::make_unique<UIAnimationSequence>();
	turtle1Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// カメミッション2 (通常攻撃10回以上)
	auto* turtle2 = GetUI<UIIcon>(Hash32("turtle_mission_2"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(turtle2, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(turtle2, Hash32("missionEventEnd"));
	turtle2Sequence_ = std::make_unique<UIAnimationSequence>();
	turtle2Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// カメミッション3 (HP30%以上でクリア)
	auto* turtle3 = GetUI<UIIcon>(Hash32("turtle_mission_3"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(turtle3, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(turtle3, Hash32("missionEventEnd"));
	turtle3Sequence_ = std::make_unique<UIAnimationSequence>();
	turtle3Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// クリアメダル(退場)
	auto* clearMedaru = GetUI<UIIcon>(Hash32("clear_medaru"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(clearMedaru, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(clearMedaru, Hash32("missionEventEnd"));
	missionClearSequence_ = std::make_unique<UIAnimationSequence>();
	missionClearSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// スキル使用回数カウント
	auto* skillCount = GetUI<UIDigit>(Hash32("skill_count"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(skillCount, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(skillCount, Hash32("missionEventEnd"));
	skillCountSequence_ = std::make_unique<UIAnimationSequence>();
	skillCountSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// 回避使用回数カウント
	auto* kaihiCount = GetUI<UIDigit>(Hash32("kaihi_count"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(kaihiCount, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(kaihiCount, Hash32("missionEventEnd"));
	kaihiCountSequence_ = std::make_unique<UIAnimationSequence>();
	kaihiCountSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// 通常攻撃回数カウント
	auto* normalAttackCount = GetUI<UIDigit>(Hash32("normalAttack_count"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(normalAttackCount, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(normalAttackCount, Hash32("missionEventEnd"));
	normalAttackCountSequence_ = std::make_unique<UIAnimationSequence>();
	normalAttackCountSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 4.0f);

	// 丸のエフェクト
	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList_.push_back(std::make_unique<ParticleEffectRender>());
		char jsonPath[] = "Assets/ui/vfx/effect_mission_maru/effect_mission_maru_1.json";
		jsonPath[54] = '1' + static_cast<char>(i);
		effectRenderList_[i]->Init(jsonPath, "Assets/ui/inGameUI/mission/maru.dds", 128.0f, 128.0f);
		effectRenderList_[i]->SetPosition(Vector3(528.0f, 330, 0.0f));
		effectRenderList_[i]->EnableHotReload();
	}
}