/**
 * UIAnimation.h
 * UIAnimationをアニメーションさせる機能群
 */
#pragma once
#include "src/Util/Curve.h"

class UIBase;

template <typename T>
using UIAnimationApplyFunc = std::function<void(const T&)>;

/** UIアニメーション基底クラス */
class UIAnimationBase : public Noncopyable
{
protected:
	UIBase* ui_ = nullptr;
	float timeSec_ = 0.0f;


public:
	UIAnimationBase() {}
	~UIAnimationBase() {}

	virtual void Update() = 0;
	virtual void Play() = 0;
	virtual void Stop() = 0;
	virtual bool IsPlay() = 0;
	virtual void Reset() = 0;


	void SetUI(UIBase* ui) { ui_ = ui; }
};


/** floatのアニメーション */
class UIFloatAnimation :public UIAnimationBase {
protected:
	FloatCurve curve_;
	/** カーブ用のパラメーター */
	float start_ = 0.0f;
	float end_ = 0.0f;

	EasingType type_ = EasingType::Linear;
	LoopMode loopMode_ = LoopMode::Once;

	UIAnimationApplyFunc<float> applyFunc_;

public:
	UIFloatAnimation() {}
	~UIFloatAnimation() {}

	void Update() override
	{
		// もし再生していないなら処理しない
		if (!IsPlay()) {
			return;
		}

		curve_.Update(g_gameTime->GetFrameDeltaTime());

		//イージングされた現在の値を取得し、登録された関数を実行
		if (applyFunc_) {
			applyFunc_(curve_.GetCurrentValue());
		}
	}

	/** 再生 */
	void Play() override
	{
		curve_.Play();
	}

	/** 停止 */
	void Stop() override
	{
		curve_.Stop();
	}

	/** リセット */
	void Reset() override
	{
		curve_.Reset();
	}

	/** 再生中か */
	bool IsPlay() override
	{
		return curve_.IsPlaying();
	}

	/** UIアニメーションのパラメーターを設定 */
	void SetParameter(float start, float end, float timeSec, EasingType type, LoopMode loopMode)
	{
		start_ = start;
		end_ = end;
		timeSec_ = timeSec;
		type_ = type;
		loopMode_ = loopMode;
		/* 即時反映のためにPlayと同じ引き数でセット */
		//m_curve.Play(start, end, timeSec, type, loopMode);
		curve_.Initialize(start_, end_, timeSec_, type_, loopMode_);
	}

	/** アニメーション中の現在の値を取得 */
	float GetCurrendtValue()
	{
		return curve_.GetCurrentValue();
	}

	/** アニメーション後の情報を適用する関数を設定 */
	void SetFunc(const UIAnimationApplyFunc<float>& func)
	{
		applyFunc_ = func;
	}
};

/** 2Dアニメーション */
class  UIVector2Animation : public UIAnimationBase
{
protected:
	Vector2Curve curve_;
	/** カーブ用のパラメーター */
	Vector2 start_ = Vector2::Zero;
	Vector2 end_ = Vector2::Zero;

	EasingType type_ = EasingType::Linear;
	LoopMode loopMode_ = LoopMode::Once;

	UIAnimationApplyFunc<Vector2> applyFunc_;


public:
	UIVector2Animation() {}
	~UIVector2Animation() {}

	/** 更新 */
	void Update() override
	{
		curve_.Update(g_gameTime->GetFrameDeltaTime());
		//m_applyFunc(m_curve.GetCurrentValue());
		if (applyFunc_) {
			applyFunc_(curve_.GetCurrentValue());
		}
	}

	/** 再生 */
	void Play() override
	{
		curve_.Play();
	}

	/** 再生してるか */
	bool IsPlay() override
	{
		return curve_.IsPlaying();
	}

	/** 停止 */
	void Stop() override {
		curve_.Stop();
	}


	/** リセット */
	void Reset() override
	{
		curve_.Reset();
	}

	/** UIアニメーションの情報を設定 */
	void SetParameter(Vector2 start, Vector2 end, float timeSec, EasingType type, LoopMode loopMode)
	{
		start_ = start;
		end_ = end;
		timeSec_ = timeSec;
		type_ = type;
		loopMode_ = loopMode;
		//m_curve.Play(start, end, timeSec, type, loopMode);

	}

	/** アニメーション中の現在の値を取得 */
	Vector2 GetCurrentValue()
	{
		return curve_.GetCurrentValue();
	}

	/** アニメーション後の情報を適用する関数を設定 */
	void SetFunc(const UIAnimationApplyFunc<Vector2>& func)
	{
		applyFunc_ = func;
	}
};




/***************************************/


/** Vector3のアニメーション(position, scale~~) */
class UIVector3Animation : public UIAnimationBase
{
protected:
	Vector3Curve curve_;
	/** カーブ用のパラメーター */
	Vector3 start_ = Vector3::Zero;
	Vector3 end_ = Vector3::Zero;
	float timeSec_ = 0.0f;
	EasingType type_ = EasingType::Linear;
	LoopMode loopMode_ = LoopMode::Once;

	UIAnimationApplyFunc<Vector3> applyFunc_;


public:
	UIVector3Animation() {}
	~UIVector3Animation() {}

	void Update() override
	{
		curve_.Update(g_gameTime->GetFrameDeltaTime());
		//m_applyFunc(m_curve.GetCurrentValue());
		if (applyFunc_) {
			applyFunc_(curve_.GetCurrentValue());
		}
		else {
			// Handle the error or log a message
		}
	}

	void Play() override
	{
		curve_.Play();
	}

	bool IsPlay() override
	{
		return curve_.IsPlaying();
	}

	void Stop() override {
		curve_.Stop();
	}

	/** リセット */
	void Reset() override
	{
		curve_.Reset();
	}

	void SetParameter(Vector3 start, Vector3 end, float timeSec, EasingType type, LoopMode loopMode)
	{
		start_ = start;
		end_ = end;
		timeSec_ = timeSec;
		type_ = type;
		loopMode_ = loopMode;
		curve_.Initialize(start, end, timeSec, type, loopMode);
	}

	Vector3 GetCurrentValue()
	{
		return curve_.GetCurrentValue();
	}

	void SetFunc(const UIAnimationApplyFunc<Vector3>& func)
	{
		applyFunc_ = func;
	}
};




/*****************************************/


/** Vector4のアニメーション(color~~) */
class UIVector4Animation : public UIAnimationBase
{
protected:
	Vector4Curve curve_;
	/** カーブ用のパラメーター */
	Vector4 start_ = Vector4::White;
	Vector4 end_ = Vector4::White;
	float timeSec_ = 0.0f;
	EasingType type_ = EasingType::Linear;
	LoopMode loopMode_ = LoopMode::Once;

	UIAnimationApplyFunc<Vector4> applyFunc_;

public:
	UIVector4Animation() {}
	~UIVector4Animation() {}

	void Update() override
	{
		curve_.Update(g_gameTime->GetFrameDeltaTime());
		//m_applyFunc(m_curve.GetCurrentValue());
		if (applyFunc_) {
			applyFunc_(curve_.GetCurrentValue());
		}
		else {
			// Handle the error or log a message
		}
	}

	void Play() override
	{
		curve_.Play();
	}

	bool IsPlay() override
	{
		return curve_.IsPlaying();
	}

	void Stop() override {
		curve_.Stop();
	}

	/** リセット */
	void Reset() override
	{
		curve_.Reset();
	}

	void SetParameter(Vector4 start, Vector4 end, float timeSec, EasingType type, LoopMode loopMode)
	{
		start_ = start;
		end_ = end;
		timeSec_ = timeSec;
		type_ = type;
		loopMode_ = loopMode;
		curve_.Initialize(start, end, timeSec, type, loopMode);
	}


	Vector4 GetCurrendtValue()
	{
		return curve_.GetCurrentValue();
	}

	void SetFunc(const UIAnimationApplyFunc<Vector4>& func)
	{
		applyFunc_ = func;
	}
};




/*************************************************/


/** カラーアニメーション */
class UIColorAnimation : public UIVector4Animation
{
public:
	UIColorAnimation();
	~UIColorAnimation() {}

	void Update() override
	{
		curve_.Update(g_gameTime->GetFrameDeltaTime());
		applyFunc_(curve_.GetCurrentValue());
	}
};




/** 拡縮アニメーション */
class UIScaleAnimation : public UIVector3Animation
{
public:
	UIScaleAnimation();
	~UIScaleAnimation() {}

	void Update() override
	{
		curve_.Update(g_gameTime->GetFrameDeltaTime());
		applyFunc_(curve_.GetCurrentValue());
	}
};


/** 座標アニメーション */
class UITranslateAniamtion : public UIVector3Animation
{
public:
	UITranslateAniamtion();
	~UITranslateAniamtion() {}

	void Update() override
	{
		curve_.Update(g_gameTime->GetFrameDeltaTime());
		applyFunc_(curve_.GetCurrentValue());
	}
};

/** 元座標との差分アニメーション */
class UITranslateOffsetAnimation : public UIVector3Animation
{
public:
	UITranslateOffsetAnimation();
	~UITranslateOffsetAnimation() {}

	void Update() override
	{
		curve_.Update(g_gameTime->GetFrameDeltaTime());
		applyFunc_(curve_.GetCurrentValue());
	}
};


/** 回転アニメーション */
class UIRotationAnimation : public UIFloatAnimation
{
public:
	UIRotationAnimation();
	~UIRotationAnimation() {}

	void Update() override
	{
		curve_.Update(g_gameTime->GetFrameDeltaTime());
		applyFunc_(curve_.GetCurrentValue());
	}

};