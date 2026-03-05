#pragma once
#include "Actor.h"


class Character : public Actor
{
protected:
	AllocatedArray<AnimationClip> m_animationClipList;


public:
	Character() {}
	~Character() {}

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;
};