#pragma once
#include "Character.h"


class Player : public Character
{
public:
	Player();
	~Player() {}

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;
};