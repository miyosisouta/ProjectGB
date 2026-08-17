#include "stdafx.h"
#include "BehaviorTree.h"


BTStatus BTSelector::Tick()
{
	for (size_t i = runningIndex_; i < children_.size(); ++i)
	{
		const BTStatus status = children_[i]->Tick();

		if (status == BTStatus::Running)
		{
			runningIndex_ = i;
			return BTStatus::Running;
		}
		if (status == BTStatus::Success)
		{
			runningIndex_ = 0;
			return BTStatus::Success;
		}
		// Failureなら次の子へ
	}

	runningIndex_ = 0;
	return BTStatus::Failure;
}


BTStatus BTSequence::Tick()
{
	for (size_t i = runningIndex_; i < children_.size(); ++i)
	{
		const BTStatus status = children_[i]->Tick();

		if (status == BTStatus::Running)
		{
			runningIndex_ = i;
			return BTStatus::Running;
		}
		if (status == BTStatus::Failure)
		{
			runningIndex_ = 0;
			return BTStatus::Failure;
		}
		// Successなら次の子へ
	}

	runningIndex_ = 0;
	return BTStatus::Success;
}


BTStatus BTWeightedSelector::Tick()
{
	// 実行中の候補がなければ重み付き抽選で1つ選ぶ
	if (selectedIndex_ < 0)
	{
		int totalWeight = 0;
		for (const auto& entry : entries_) { totalWeight += entry.weightFn(); }

		if (totalWeight <= 0) { return BTStatus::Failure; }

		const int lottery = rand() % totalWeight;
		int weightSum = 0;
		for (size_t i = 0; i < entries_.size(); ++i)
		{
			weightSum += entries_[i].weightFn();
			if (lottery < weightSum)
			{
				selectedIndex_ = static_cast<int>(i);
				break;
			}
		}
	}

	const BTStatus status = entries_[selectedIndex_].node->Tick();
	if (status != BTStatus::Running)
	{
		selectedIndex_ = -1; // 次回また抽選し直す
	}
	return status;
}
