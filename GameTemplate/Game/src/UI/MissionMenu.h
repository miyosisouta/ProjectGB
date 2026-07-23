#pragma once
#include "Menu.h"
#include "src/Vfx/ParticleEffectRender.h"
#include <queue>


class Layout;


class MissionMenu : public MenuBase
{
private:
	/** 1件分のミッション通知（表示待ちキューに積む） */
	struct MissionNotification
	{
		int  slot    = 0; //!< UIスロット番号
		bool isClear = false; //!< クリア通知か（falseはカウント更新通知）
		int  count   = 0; //!< 表示するカウント値
	};

	std::unique_ptr<TaskSchedulerSystem> missionTaskScheduler_ = nullptr; //!< 演出タイマーの管理

	std::unique_ptr<UIAnimationSequence> missionSequence_      = nullptr; //!< ミッション表示全体のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> missionObiSequence_   = nullptr; //!< 帯部分のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> missionMedaruSequence_ = nullptr; //!< メダルのアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> mission1Sequence_     = nullptr; //!< ミッション1のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> mission2Sequence_     = nullptr; //!< ミッション2のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> mission3Sequence_     = nullptr; //!< ミッション3のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> missionClearSequence_ = nullptr; //!< クリア演出のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> skillCountSequence_        = nullptr; //!< 特殊スキル使用回数表示のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> kaihiCountSequence_       = nullptr; //!< 回避使用回数表示のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> normalAttackCountSequence_ = nullptr; //!< 通常攻撃使用回数表示のアニメーションシーケンス

	std::unique_ptr<UIAnimationSequence> turtle1Sequence_      = nullptr; //!< カメ用ミッション1のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> turtle2Sequence_      = nullptr; //!< カメ用ミッション2のアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> turtle3Sequence_      = nullptr; //!< カメ用ミッション3のアニメーションシーケンス

	std::vector<std::unique_ptr<ParticleEffectRender>> effectRenderList_; //!< クリア演出の丸エフェクトリスト

	std::queue<MissionNotification> notificationQueue_; //!< 表示待ちの通知キュー
	bool isPlaying_       = false; //!< 通知アニメーションを再生中か
	bool pendingPlayNext_ = false; //!< Update()末尾で次の通知再生を開始するフラグ


private:
	/** キューの先頭を再生開始する（再生中でなければ） */
	void TryPlayNext();
	/** 1件分の通知アニメーションを再生する */
	void PlayNotification(const MissionNotification& notif);
	/** カウント数字表示のアニメーションを再生する */
	void PlayCountDisplay(UIDigit* digit, UIAnimationSequence* seq, int count);


public:
	/** 更新処理 */
	void Update() override;
	/** 描画処理 */
	void Render(RenderContext& rc) override;

	/** 使用するUIの初期設定 */
	void InitializeLogic() override;

	/** カウント更新のミッション通知をキューに積む */
	void TriggerMission(int slot, int count = 0);
	/** クリア通知をキューに積む */
	void TriggerClear(int slot, int count = 0);

	/** リザルト遷移時にキューに積まれた未再生アニメーションを破棄する */
	void ClearNotificationQueue()
	{
		notificationQueue_ = {};
		isPlaying_         = false;
		pendingPlayNext_   = false;
	}
};
