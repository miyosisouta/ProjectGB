/**
* GameClearMenu.cpp
* ゲームクリア時の演出等用
*/

#include "stdafx.h"
#include "GameClearMenu.h"
#include "UIAnimationFactory.h"
#include "src/Battle/MissionManager.h"
#include "src/Battle/MissionType.h"


void GameClearMenu::Update()
{
	const float deltaTime = g_gameTime->GetFrameDeltaTime();
	taskSchedulerSystem_->Update(deltaTime);
	gameClearSequence_->Update(deltaTime);

	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList_[i]->Update(deltaTime);
	}

	// Aボタンでタイトルへ（ミッション全表示後のみ受付）
	if (isMissionShown_ && g_pad[0]->IsTrigger(enButtonA))
	{
		isEnd_ = true;
	}

	MenuBase::Update();
}


void GameClearMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);

	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList_[i]->Draw(rc);
	}
}


void GameClearMenu::InitializeLogic()
{
	// GAME CLEAR バウンス
	{
		auto* gameClearBounce = GetUI<UIIcon>(Hash32("gameClear"));
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceA"));
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceB"));
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceC"));
		gameClearSequence_ = std::make_unique<UIAnimationSequence>();
		gameClearSequence_->Add(Hash32("GameClearStampBounceA")).Add(Hash32("GameClearStampBounceB")).Add(Hash32("GameClearStampBounceC"));
	}

	// 肉球ハンコ（フェードイン＋縮小のみアタッチ。退避アニメは2.0sタイマー内でアタッチする）
	{
		auto* hankoIcon = GetUI<UIIcon>(Hash32("gameClear_hanko"));
		UIAnimationFactory::Attach<UIColorAnimation>(hankoIcon, Hash32("GameClearStampFadeIn"));
		UIAnimationFactory::Attach<UIScaleAnimation>(hankoIcon, Hash32("GameClearStampScale"));
	}


	// ミッション結果カードを初期非表示
	{
		for (const char* name : { "missionResult1", "missionResult2", "missionResult3" })
		{
			auto* canvas = GetUI<UICanvas>(Hash32(name));
			if (canvas) canvas->isDraw = false;
		}
	}


	// 演出タイムライン
	// 1.0s : GAME CLEAR バウンス開始
	// 1.5s : ハンコ表示（FadeIn＋Scale）
	// 3.0s : 文字＆ハンコを同時に縮小・文字は上へ移動
	// 5.0s : ミッション1スライドイン
	// 5.25s: ミッション2スライドイン
	// 5.5s : ミッション3スライドイン
	// 7.0s : ミッション1リビール
	// 7.5s : ミッション2リビール
	// 8.0s : ミッション3リビール
	// 8.5s : Aボタン待ち開始（以降はAボタンでタイトルへ）
	taskSchedulerSystem_ = std::make_unique<TaskSchedulerSystem>();

	taskSchedulerSystem_->AddTimer(1.0f, [this]()
		{
			auto* gameClearBounce = GetUI<UIIcon>(Hash32("gameClear"));
			gameClearBounce->isDraw = true;
			gameClearSequence_->Play(gameClearBounce);
		});

	taskSchedulerSystem_->AddTimer(1.5f, [this]()
		{
			auto* hankoIcon = GetUI<UIIcon>(Hash32("gameClear_hanko"));
			hankoIcon->isDraw = true;
			hankoIcon->PlayAnimation();
		});

	// バウンス+ ハンコ表示 終了後、1秒待って退避
	taskSchedulerSystem_->AddTimer(3.0f, [this]()
		{
			// GAME CLEARテキスト：ここでアタッチして移動＆縮小を同時再生
			auto* gameClearTextIcon = GetUI<UIIcon>(Hash32("gameClear"));
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearTextIcon, Hash32("GameClearTextMove"));
			UIAnimationFactory::Attach<UIScaleAnimation>(gameClearTextIcon, Hash32("GameClearTextScaleDown"));
			auto* textMoveAnim  = gameClearTextIcon->FindAnimation(Hash32("GameClearTextMove"));
			auto* textScaleAnim = gameClearTextIcon->FindAnimation(Hash32("GameClearTextScaleDown"));
			if (textMoveAnim && textScaleAnim)
			{
				textMoveAnim->Clear();
				textScaleAnim->Clear();
				textMoveAnim->Play();
				textScaleAnim->Play();
			}

			// ハンコ：移動＆縮小を同時再生
			auto* hankoIcon = GetUI<UIIcon>(Hash32("gameClear_hanko"));
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(hankoIcon, Hash32("GameClearStampMove"));
			UIAnimationFactory::Attach<UIScaleAnimation>(hankoIcon, Hash32("GameClearStampScaleDown"));
			auto* hankoMoveAnim  = hankoIcon->FindAnimation(Hash32("GameClearStampMove"));
			auto* hankoScaleAnim = hankoIcon->FindAnimation(Hash32("GameClearStampScaleDown"));
			if (hankoMoveAnim && hankoScaleAnim)
			{
				hankoMoveAnim->Clear();
				hankoScaleAnim->Clear();
				hankoMoveAnim->Play();
				hankoScaleAnim->Play();
			}
		});
	// グレー背景スライドイン
	taskSchedulerSystem_->AddTimer(5.0f,  [this]() { ShowMissionBackground(0); });
	taskSchedulerSystem_->AddTimer(5.25f, [this]() { ShowMissionBackground(1); });
	taskSchedulerSystem_->AddTimer(5.5f,  [this]() { ShowMissionBackground(2); });

	// グレー背景スライド完了後にテキスト・メダル・色変化を順番に表示
	taskSchedulerSystem_->AddTimer(7.0f,  [this]() { RevealMission(0); });
	taskSchedulerSystem_->AddTimer(7.5f,  [this]() { RevealMission(1); });
	taskSchedulerSystem_->AddTimer(8.0f,  [this]() { RevealMission(2); });

	// 全ミッション表示完了後、Aボタンとテキストを表示してAボタン受付を開始
	taskSchedulerSystem_->AddTimer(8.5f, [this]()
		{
			isMissionShown_ = true;

			if (auto* aButton = GetUI<UIIcon>(Hash32("a_button")))
				aButton->isDraw = true;

			if (auto* returnText = GetUI<UIIcon>(Hash32("return_text")))
				returnText->isDraw = true;
		});

	// 丸エフェクト初期化
	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList_.push_back(std::make_unique<ParticleEffectRender>());
		char jsonPath[] = "Assets/ui/vfx/effect_mission_maru/effect_mission_maru_1.json";
		jsonPath[54] = '1' + static_cast<char>(i); // ファイル名末尾の"1"をi番目の連番(1～4)に書き換える
		effectRenderList_[i]->Init(jsonPath, "Assets/ui/inGameUI/mission/maru.dds", 150.0f, 150.0f);
		effectRenderList_[i]->EnableHotReload();
	}

	// クリア済みミッションの丸エフェクトをRevealの0.5秒後にスケジュール
	// IsCleared()をReveal時点で判定することでゴリラ/亀いずれも同じタイミングで再生
	const float revealTimes[]      = { 7.0f,  7.5f,  8.0f   };
	// X=-550: medal_gold の localX(-220) × scale(2.5) = ワールドX -550
	const float effectYPositions[] = { 60.0f, -90.0f, -240.0f };
	for (int i = 0; i < 3; i++)
	{
		const float effectDelay = revealTimes[i] + 0.5f;
		const float yPos = effectYPositions[i];
		const int missionIdx = i;
		taskSchedulerSystem_->AddTimer(effectDelay, [this, yPos, missionIdx]()
		{
			auto* mission = MissionManager::Get().GetMissionByIndex(missionIdx);
			if (!mission || !mission->IsCleared()) return;
			for (int j = 0; j < MAX_EFFECT_NUM; j++)
			{
				effectRenderList_[j]->SetPosition(Vector3(-550.0f, yPos, 0.0f));
				effectRenderList_[j]->Play();
			}
		});
	}
}


void GameClearMenu::ShowMissionBackground(int index)
{
	static const char* canvasNames[] = {
		"missionResult1",
		"missionResult2",
		"missionResult3"
	};
	const uint32_t slideKeys[] = {
		Hash32("GameClearMission1Move"),
		Hash32("GameClearMission2Move"),
		Hash32("GameClearMission3Move")
	};

	// 設定してあるミッションのインデックスを取得
	auto* mission = MissionManager::Get().GetMissionByIndex(index);
	if (!mission) return; // ないなら処理をかえす

	const char* canvasName = canvasNames[index];
	auto* canvas = GetUI<UICanvas>(Hash32(canvasName));
	if (!canvas) return;
	canvas->isDraw = true;

	// スライドSE再生
	SoundManager::Get().PlaySE(enSoundKind_Slide);

	// ミッションテキストをスライドと同時に表示
	{
		struct MissionTextEntry { MissionID id; const char* name; };
		static const MissionTextEntry textMap[] = {
			{ MissionID::enGorillaTime,        "text_gorilla_time"        }, // 時間内撃破
			{ MissionID::enGorillaAbility,     "text_gorilla_ability"     }, // スキルの使用
			{ MissionID::enGorillaUtility,     "text_gorilla_utility"     }, // 回避の使用
			{ MissionID::enTurtleTime,         "text_turtle_time"         }, // 時間内撃破
			{ MissionID::enTurtleNormalAttack, "text_turtle_normalAttack" }, // 通常攻撃の使用
			{ MissionID::enTurtleHpRate,       "text_turtle_hp"           }, // HP3割以上残す
		};

		// ミッションのIDと同一なら描画可能にする 
		for (const auto& entry : textMap)
		{
			std::string path = std::string(canvasName) + "/" + entry.name;
			auto* icon = GetUI<UIIcon>(Hash32(path.c_str()));
			if (icon) icon->isDraw = (mission->GetID() == entry.id);
		}
	}

	// カウント制ミッションはdigitも同時に表示
	{
		// カウント制のミッションのUIを取得
		std::string countPath = std::string(canvasName) + "/mission_count";
		auto* countDigit = GetUI<UIDigit>(Hash32(countPath.c_str()));
		if (countDigit)
		{
			// カウント制であること
			bool isCount = (mission->GetUpdateType() == MissionUpdateType::enCount);
			countDigit->isDraw = isCount;
			if (isCount)
			{
				// 現在のカウントを設定
				const int count = mission->GetCurrentCount();
				countDigit->SetNumber(count);
			}
		}
	}

	// スライドアニメーション（画面右から中央へ）
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(canvas, slideKeys[index]);
	auto* moveAnim = canvas->FindAnimation(slideKeys[index]);
	if (moveAnim)
	{
		moveAnim->Clear();
		moveAnim->Play();
	}
}


void GameClearMenu::RevealMission(int index)
{
	static const char* canvasNames[] = {
		"missionResult1",
		"missionResult2",
		"missionResult3"
	};

	auto* mission = MissionManager::Get().GetMissionByIndex(index);
	if (!mission) return;

	const char* canvasName = canvasNames[index];

	// クリア状態に応じてメダル・背景を切替（MissionMenuと同じアニメーション使用）
	{
		std::string backPath = std::string(canvasName) + "/mission_back";
		std::string goldPath = std::string(canvasName) + "/medal_gold";
		std::string noPath   = std::string(canvasName) + "/medal_no";
		auto* missionBack = GetUI<UIIcon>(Hash32(backPath.c_str()));
		auto* medalGold   = GetUI<UIIcon>(Hash32(goldPath.c_str()));
		auto* medalNo     = GetUI<UIIcon>(Hash32(noPath.c_str()));

		if (mission->IsCleared())
		{
			// 背景：灰色→金色アニメーション
			if (missionBack)
			{
				UIAnimationFactory::Attach<UIColorAnimation>(missionBack, Hash32("mission_color"));
				missionBack->FindAnimation(Hash32("mission_color"))->Play();
			}
			// 金メダル：拡大→通常のポップインアニメーション
			if (medalGold)
			{
				medalGold->isDraw = true;
				UIAnimationFactory::Attach<UIScaleAnimation>(medalGold, Hash32("mission_medaru_scale"));
				medalGold->FindAnimation(Hash32("mission_medaru_scale"))->Play();
			}
			if (medalNo) medalNo->isDraw = false;

			// クリアSE再生
			SoundManager::Get().PlaySE(enSoundKind_Mission_Clear);
		}
		else
		{
			// 未クリア：グレーメダルをそのまま表示
			if (medalNo)   medalNo->isDraw   = true;
			if (medalGold) medalGold->isDraw = false;
		}
	}
}


