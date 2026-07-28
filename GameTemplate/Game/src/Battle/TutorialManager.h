/**
 * TutorialManager.h
 * チュートリアルの進行管理。
 *
 * 移動→通常攻撃3回→スキル→通常回避→ジャスト回避→自由 の6段階を、
 * 実際のプレイヤー/ボスの状態から検知して進行する。
 * 各段階をクリアすると、UIAnimation開始フラグ(GetUIAnimationFlag())が1フレームだけtrueになり、
 * アニメーション中の時間がとられる。
 * その間は、次の行動の許可がない
 *
 * NOTE: ボスの攻撃スクリプト制御はTutorialBossScript、段階名の画面表示はTutorialStageTextに
 *       責務を分離している。ここでは「プレイヤーの進行状態を判定し、それらへ指示を出す」ことに専念する。
 */
#pragma once
#include "src/Emotion/IEmotionObserver.h"
#include "src/Actor/StateMachine.h"
#include "src/Battle/TutorialBossScript.h"
#include "src/Battle/TutorialStageText.h"


class Player;
class BossCharacter;
class EmotionSystem;

class TutorialManager : public IEmotionObserver
{
public:
	/** チュートリアルの段階 */
	enum class Stage
	{
		Move,			//!< 移動
		NormalAttack,	//!< 通常攻撃
		Skill,			//!< スキル
		Avoid,			//!< 通常回避
		JustAvoid,		//!< ジャスト回避
		Free,			//!< 自由（全ての行動が可能になる）
	};

	/** ロック対象のアクション */
	enum class Action
	{
		NormalAttack,	//!< 通常攻撃
		Skill,			//!< スキル
		Avoid,			//!< 回避（ジャスト回避も同じ入力なので、かいひ段階以降で許可）
	};


private:
	Player*        player_        = nullptr;
	EmotionSystem* emotionSystem_ = nullptr; //!< 監視解除(RemoveObserver)用に保持

	Stage         stage_             = Stage::Move;
	PlayerStateID prevStateId_       = PlayerStateID::None; //!< 1フレーム前のプレイヤーステート（立ち上がり検知用）
	int           normalAttackCount_ = 0;                    //!< 通常攻撃(Bite)に入った回数

	bool  isTransitioning_   = false; //!< クリア後、次の段階名を表示するまでの「アニメーション」表示中か
	float transitionElapsed_ = 0.0f;  //!< アニメーション表示の経過秒数

	bool  moveTimerActive_ = false; //!< 移動段階：計測を開始したか
	float moveElapsed_     = 0.0f;  //!< 移動段階：計測開始からの経過秒数

	bool skipRequested_    = false; //!< Enterによる即時終了が要求されたか
	bool uiAnimationFlag_  = false; //!< UIAnimation開始フラグ。Update()の先頭で毎フレームfalseに戻すため、1フレームだけtrueが取れる

	Vector3    playerSpawnPosition_ = Vector3::Zero;       //!< Init()時点のプレイヤー座標（RequestSkip()で戻す先）
	Quaternion playerSpawnRotation_ = Quaternion::Identity; //!< Init()時点のプレイヤー向き

	TutorialBossScript bossScript_; //!< かいひ段階以降のボス攻撃ループ制御（責務を分離）
	TutorialStageText  stageText_;  //!< 画面右側の段階名デバッグ表示（責務を分離）


private:
	/** 次の段階のIDを返す（Freeより後は無い） */
	static Stage NextStage(Stage current);
	/** クリア判定：アニメーション表示状態にし、UIAnimation開始フラグを立てる */
	void StartTransition();
	/** アニメーション表示が終わった後、実際に次の段階へ進め、表示テキストを更新する */
	void AdvanceToNextStage();
	/** 現在の状態（アニメーション表示中/各段階）に応じた表示テキストへ差し替える */
	void UpdateText();


public:
	/** コンストラクタ */
	TutorialManager();
	/** デストラクタ */
	~TutorialManager();

	/** プレイヤー・ボス・ボスの感情システムを設定し、監視を開始する（ボスのStart()完了後に1回だけ呼ぶ） */
	void Init(Player* player, BossCharacter* boss, EmotionSystem* emotionSystem);

	/** 更新処理 */
	void Update();
	/** 描画処理 */
	void Render(RenderContext& rc);

	/** 指定アクションが現在解禁されているか */
	bool IsActionAllowed(Action action) const;

	/** UIAnimationを開始すべきタイミングか */
	inline bool GetUIAnimationFlag() const { return uiAnimationFlag_; }

	/** Enterによる終了要求が来ているか */
	inline bool IsFinished() const { return skipRequested_; }

	/** チュートリアルを即座に終了させる（Enterによる全体スキップ） */
	void RequestSkip();

	/**
	 * ボスの攻撃を回避の無敵中に受けた瞬間に呼ぶ
	 * 回避段階では、単に回避キーを押しただけでなく、実際にボスの攻撃をよけられた時だけクリア扱いにする
	 */
	void NotifyBossAttackAvoided();

	/**
	 * 感情のレベルが変化したときに呼ばれる（IEmotionObserver の実装）
	 * ジャスト回避が成立するとボスの感情はDebuff方向（数値が下がる方向）へ動くため、これでジャスト回避成立を検知する
	 */
	void OnEmotionChanged(int oldLevel, int newLevel) override;


	/*========================================*/
	/* シングルトン関連 */
	/*========================================*/
private:
	static TutorialManager* instance_;

public:
	/** インスタンスを作る */
	static void CreateInstance()
	{
		if (!instance_) instance_ = new TutorialManager();
	}

	/** インスタンスを取得 */
	static TutorialManager& Get()
	{
		return *instance_;
	}

	/** インスタンスを破棄 */
	static void DestroyInstance()
	{
		if (instance_) {
			delete instance_;
			instance_ = nullptr;
		}
	}
};
