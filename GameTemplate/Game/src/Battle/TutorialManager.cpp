#include "stdafx.h"
#include "TutorialManager.h"
#include "src/Actor/Player.h"
#include "src/Actor/ActorStatus.h"
#include "src/Emotion/EmotionSystem.h"
#include "src/Core/ParameterManager.h"


namespace
{
	/** 段階に応じた表示テキストを取得 */
	const wchar_t* GetStageLabel(TutorialManager::Stage stage)
	{
		switch (stage)
		{
		case TutorialManager::Stage::Move:         return L"いどう";
		case TutorialManager::Stage::NormalAttack: return L"つうじょう";
		case TutorialManager::Stage::Skill:        return L"すきる";
		case TutorialManager::Stage::Avoid:        return L"かいひ";
		case TutorialManager::Stage::JustAvoid:    return L"ジャスト";
		case TutorialManager::Stage::Free:         return L"自由";
		default:                                   return L"";
		}
	}
}


TutorialManager* TutorialManager::instance_ = nullptr;


TutorialManager::TutorialManager()
{
	UpdateText();
}


TutorialManager::~TutorialManager()
{
	// 監視をやめる
	if (emotionSystem_) { emotionSystem_->RemoveObserver(this); }
}


void TutorialManager::Init(Player* player, BossCharacter* boss, EmotionSystem* emotionSystem)
{
	player_        = player;
	emotionSystem_ = emotionSystem;

	// ボスの感情レベル変化(バフ/デバフ)を監視し、ジャスト回避の成立を検知する
	if (emotionSystem_) { emotionSystem_->AddObserver(this); }

	// ボスの攻撃スクリプト制御を初期化（登場演出用の初期位置・向きもここで覚える）
	bossScript_.Init(boss);

	// 登場演出用にプレイヤーの初期位置・向きを覚えておく（RequestSkip()でここへ戻す）
	if (player_)
	{
		playerSpawnPosition_ = player_->GetTransformPosition();
		playerSpawnRotation_ = player_->GetTransformRotation();
	}
}


void TutorialManager::Update()
{
	// 前フレームで立てたUIAnimationフラグをリセット
	uiAnimationFlag_ = false;
	// プレイヤーがいない、もしくはEnterで終了要求済み（RequestSkip()でIdle・登場位置へ戻した直後）なら、
	// それ以上ボスを動かしたり段階を進めたりしない
	if (!player_ || skipRequested_) { return; }

	// 1フレーム当たりの時間を取得
	const float deltaTime = g_gameTime->GetFrameDeltaTime();
	// マジックナンバー・時間設定はTutorialParameter.jsonから読み込む
	const auto* param = ParameterManager::Get().GetTutorialParam();

	// 回避段階以降のボス攻撃ループは、テキストの「アニメーション」表示中や自由段階でも止めずに進める
	const bool shouldBossScriptBeActive = (stage_ == Stage::Avoid || stage_ == Stage::JustAvoid || stage_ == Stage::Free);
	bossScript_.Update(deltaTime, shouldBossScriptBeActive, player_->GetTransformPosition(), stage_ == Stage::Free);

	if (stage_ == Stage::Free) { return; }

	// アニメーション表示中は、時間経過だけを見て次の段階名表示への切り替えを待つ
	if (isTransitioning_)
	{
		transitionElapsed_ += deltaTime;
		if (transitionElapsed_ >= param->transitionDuration)
		{
			isTransitioning_ = false;
			AdvanceToNextStage();
		}
		return;
	}

	// プレイヤーのステートマシーンと状態を取得
	auto* stateMachine = player_->GetStateMachine();
	const PlayerStateID currentStateId = stateMachine->GetCurrentStateId();

	// チュートリアルの段階ごとの処理
	switch (stage_)
	{
	case Stage::Move:
		// 少しでも動いた（歩き/ダッシュのいずれかの状態になった）瞬間から計測を開始する
		if (!moveTimerActive_ && (currentStateId == PlayerStateID::Walk || currentStateId == PlayerStateID::Run))
		{
			moveTimerActive_ = true;
			moveElapsed_ = 0.0f;
		}
		if (moveTimerActive_)
		{
			moveElapsed_ += deltaTime;
			const bool isDashing = (currentStateId == PlayerStateID::Run);
			// ダッシュ状態になった、もしくは計測開始から既定秒数が経過したらクリア
			if (isDashing || moveElapsed_ >= param->moveTimeoutDuration)
			{
				StartTransition();
			}
		}
		break;

	case Stage::NormalAttack:
		// 通常攻撃(Bite)に入った回数を数え、既定回数に達したらクリア
		if (prevStateId_ != PlayerStateID::Bite && currentStateId == PlayerStateID::Bite)
		{
			// 攻撃回数の追加
			normalAttackCount_++;
			if (normalAttackCount_ >= param->requiredNormalAttackCount)
			{
				StartTransition();
			}
		}
		break;

	case Stage::Skill:
		// スキル(DefaultAttack/Landmine/FireMagicのいずれか)を1回使用したらクリア
		if (currentStateId == PlayerStateID::DefaultAttack ||
			currentStateId == PlayerStateID::Landmine ||
			currentStateId == PlayerStateID::FireMagic)
		{
			StartTransition();
		}
		break;

	case Stage::Avoid:
		// クリア判定は NotifyBossAttackAvoided() / OnEmotionChanged() 側で行う。
		// 回避キーを押してAvoid状態に入っただけではクリアにしない
		break;

	case Stage::JustAvoid:
		// ジャスト回避成立の検知は OnEmotionChanged() 側(ボスのバフ/デバフレベル低下)で行う
		break;

	default:
		break;
	}

	prevStateId_ = currentStateId;
}


void TutorialManager::OnEmotionChanged(int oldLevel, int newLevel)
{
	// ジャスト回避が成立するとボスの感情はDebuff方向(数値が下がる方向)へ動く
	// ※ボスの攻撃がプレイヤーに命中した場合は逆にBuff方向(数値が上がる方向)へ動くため、これで区別できる
	//
	// かいひ段階中にジャスト回避が成立した場合も、通常回避の上位互換としてクリア扱いにする
	// (CollisionHitManagerはジャスト回避と通常回避を排他的に判定しており、
	//  ジャスト回避成立時は NotifyBossAttackAvoided() が呼ばれないため、ここで拾う必要がある)
	const bool isAvoidStage = (stage_ == Stage::Avoid || stage_ == Stage::JustAvoid);
	if (isAvoidStage && !isTransitioning_ && newLevel < oldLevel)
	{
		StartTransition();
	}
}


void TutorialManager::NotifyBossAttackAvoided()
{
	// かいひ段階中、実際にボスの攻撃をよけられた瞬間だけクリア扱いにする
	if (stage_ == Stage::Avoid && !isTransitioning_)
	{
		StartTransition();
	}
}


bool TutorialManager::IsActionAllowed(Action action) const
{
	// Stage は宣言順(Move<NormalAttack<Skill<Avoid<JustAvoid<Free)に整数値が振られているため、
	// 「その段階以降になったら解禁」を大小比較だけで表せる
	const int currentOrder = static_cast<int>(stage_);

	switch (action)
	{
	case Action::NormalAttack: return currentOrder >= static_cast<int>(Stage::NormalAttack);
	case Action::Skill:        return currentOrder >= static_cast<int>(Stage::Skill);
	case Action::Avoid:        return currentOrder >= static_cast<int>(Stage::Avoid);
	default:                   return true;
	}
}


void TutorialManager::RequestSkip()
{
	skipRequested_ = true;
	stage_ = Stage::Free;
	isTransitioning_ = false;
	UpdateText();

	// ボスの攻撃ループを止め、登場演出用の初期位置・向き・Idle状態へ戻す
	bossScript_.ResetToSpawn();

	// プレイヤーも登場演出用の初期位置・向き・Idle状態・スタミナへ戻す
	if (player_)
	{
		auto* stateMachine = player_->GetStateMachine();
		stateMachine->ResetInput();
		stateMachine->SetRotation(playerSpawnRotation_);
		// InitialState()はnextStateIdを予約するだけなので、UpdateState()を直接呼んで即座にIdleへ切り替える
		// （StateMachine::Update()の遷移条件判定は経由せず、BossCharacter::ChangeState()と同様に強制遷移させる）
		stateMachine->InitialState(PlayerStateID::Idle);
		stateMachine->UpdateState();

		player_->SetMoveVelocity(Vector3::Zero);
		player_->transform_.localPosition = playerSpawnPosition_;
		player_->transform_.localRotation = playerSpawnRotation_;
		player_->transform_.UpdateTransform();
		// キャラクターコントローラーの内部座標も合わせないと、次のUpdate()でExecute()が
		// 古い内部座標から復元してしまい、直接書き換えた座標が反映されない
		player_->GetCharaCon()->SetPosition(playerSpawnPosition_);

		if (auto* status = player_->GetStatus()->As<PlayerStatus>())
		{
			status->ResetStamina();
			status->ClearInvincible();
		}
	}
}


TutorialManager::Stage TutorialManager::NextStage(Stage current)
{
	switch (current)
	{
	case Stage::Move:         return Stage::NormalAttack;
	case Stage::NormalAttack: return Stage::Skill;
	case Stage::Skill:        return Stage::Avoid;
	case Stage::Avoid:        return Stage::JustAvoid;
	case Stage::JustAvoid:    return Stage::Free;
	default:                  return Stage::Free;
	}
}


void TutorialManager::StartTransition()
{
	isTransitioning_ = true;
	transitionElapsed_ = 0.0f;
	uiAnimationFlag_ = true; // このフレームだけtrue
	UpdateText();
}


void TutorialManager::AdvanceToNextStage()
{
	stage_ = NextStage(stage_);
	UpdateText();
}


void TutorialManager::UpdateText()
{
	stageText_.SetText(isTransitioning_ ? L"アニメーション" : GetStageLabel(stage_));
}


void TutorialManager::Render(RenderContext& rc)
{
	stageText_.Render(rc);
}
