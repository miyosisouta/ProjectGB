#pragma once
#include "Menu.h"
#include "src/Vfx/ParticleEffectRender.h"
#include <queue>


class Layout;


class MissionMenu : public MenuBase
{
private:
	struct MissionNotification
	{
		int  slot    = 0;
		bool isClear = false;
		int  count   = 0;
	};

	std::unique_ptr<TaskSchedulerSystem> missionTaskScheduler_ = nullptr;

	std::unique_ptr<UIAnimationSequence> missionSequence_      = nullptr;
	std::unique_ptr<UIAnimationSequence> missionObiSequence_   = nullptr;
	std::unique_ptr<UIAnimationSequence> missionMedaruSequence_= nullptr;
	std::unique_ptr<UIAnimationSequence> mission1Sequence_     = nullptr;
	std::unique_ptr<UIAnimationSequence> mission2Sequence_     = nullptr;
	std::unique_ptr<UIAnimationSequence> mission3Sequence_     = nullptr;
	std::unique_ptr<UIAnimationSequence> missionClearSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> skillCountSequence_        = nullptr;
	std::unique_ptr<UIAnimationSequence> kaihiCountSequence_       = nullptr;
	std::unique_ptr<UIAnimationSequence> normalAttackCountSequence_ = nullptr;

	std::unique_ptr<UIAnimationSequence> turtle1Sequence_      = nullptr;
	std::unique_ptr<UIAnimationSequence> turtle2Sequence_      = nullptr;
	std::unique_ptr<UIAnimationSequence> turtle3Sequence_      = nullptr;

	std::vector<std::unique_ptr<ParticleEffectRender>> effectRenderList_;

	std::queue<MissionNotification> notificationQueue_;
	bool isPlaying_       = false;
	bool pendingPlayNext_ = false;


public:
	void Update() override;
	void Render(RenderContext& rc) override;

	/** 使用するUIの初期設定 */
	void InitializeLogic() override;

	void TriggerMission(int slot, int count = 0);
	void TriggerClear(int slot, int count = 0);

	/** リザルト遷移時にキューに積まれた未再生アニメーションを破棄する */
	void ClearNotificationQueue()
	{
		notificationQueue_ = {};
		isPlaying_         = false;
		pendingPlayNext_   = false;
	}


private:
	void TryPlayNext();
	void PlayNotification(const MissionNotification& notif);
	void PlayCountDisplay(UIDigit* digit, UIAnimationSequence* seq, int count);
};
