#pragma once
/*
 * State‚ÌŠî’êƒNƒ‰ƒX
 */
class IState : public Noncopyable
{	
public:
	IState() {}
	virtual ~IState() {}

	virtual void Enter() = 0;
	virtual void Update() = 0;
	virtual void Exit() = 0;
};