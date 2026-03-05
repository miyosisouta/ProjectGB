#pragma once


#define appState(name)\
	public:\
		/**
		 * ハッシュ値を設定
		 * IDを決めることで、処理の回数を削減・4バイトで省メモリ・入力ミスを防ぐ
		 */\
static constexpr uint32_t ID() { return appHash32(#name); }


static constexpr uint32_t INVALID_STATE_ID = -1;


/*
 * Stateの基底クラス
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