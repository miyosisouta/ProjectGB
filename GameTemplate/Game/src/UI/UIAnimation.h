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
	UIBase* m_ui = nullptr;
	float m_timeSec = 0.0f;


public:
	UIAnimationBase() {}
	~UIAnimationBase() {}

	virtual void Update() = 0;
	virtual void Play() = 0;
	virtual void Stop() = 0;
	virtual bool IsPlay() = 0;
	virtual void Reset() = 0;


	void SetUI(UIBase* ui) { m_ui = ui; }
};


/** floatのアニメーション */
class UIFloatAnimation :public UIAnimationBase {
protected:
	FloatCurve m_curve;
	/** カーブ用のパラメーター */
	float m_start = 0.0f;
	float m_end = 0.0f;

	EasingType m_type = EasingType::Linear;
	LoopMode m_loopMode = LoopMode::Once;

	UIAnimationApplyFunc<float> m_applyFunc;

public:
	UIFloatAnimation() {}
	~UIFloatAnimation() {}

	void Update() override
	{
		// もし再生していないなら処理しない
		if (!IsPlay()) {
			return;
		}

		m_curve.Update(g_gameTime->GetFrameDeltaTime());

		//イージングされた現在の値を取得し、登録された関数を実行
		if (m_applyFunc) {
			m_applyFunc(m_curve.GetCurrentValue());
		}
	}

	/** 再生 */
	void Play() override
	{
		m_curve.Play();
	}

	/** 停止 */
	void Stop() override
	{
		m_curve.Stop();
	}

	/** リセット */
	void Reset() override
	{
		m_curve.Reset();
	}

	/** 再生中か */
	bool IsPlay() override
	{
		return m_curve.IsPlaying();
	}

	/** UIアニメーションのパラメーターを設定 */
	void SetParameter(float start, float end, float timeSec, EasingType type, LoopMode loopMode)
	{
		m_start = start;
		m_end = end;
		m_timeSec = timeSec;
		m_type = type;
		m_loopMode = loopMode;
		/* 即時反映のためにPlayと同じ引き数でセット */
		//m_curve.Play(start, end, timeSec, type, loopMode);
		m_curve.Initialize(m_start, m_end, m_timeSec, m_type, m_loopMode);
	}

	/** アニメーション中の現在の値を取得 */
	float GetCurrendtValue()
	{
		return m_curve.GetCurrentValue();
	}

	/** アニメーション後の情報を適用する関数を設定 */
	void SetFunc(const UIAnimationApplyFunc<float>& func)
	{
		m_applyFunc = func;
	}
};

/** 2Dアニメーション */
class  UIVector2Animation : public UIAnimationBase
{
protected:
	Vector2Curve m_curve;
	/** カーブ用のパラメーター */
	Vector2 m_start = Vector2::Zero;
	Vector2 m_end = Vector2::Zero;

	EasingType m_type = EasingType::Linear;
	LoopMode m_loopMode = LoopMode::Once;

	UIAnimationApplyFunc<Vector2> m_applyFunc;


public:
	UIVector2Animation() {}
	~UIVector2Animation() {}

	/** 更新 */
	void Update() override
	{
		m_curve.Update(g_gameTime->GetFrameDeltaTime());
		//m_applyFunc(m_curve.GetCurrentValue());
		if (m_applyFunc) {
			m_applyFunc(m_curve.GetCurrentValue());
		}
	}

	/** 再生 */
	void Play() override
	{
		m_curve.Play();
	}

	/** 再生してるか */
	bool IsPlay() override
	{
		return m_curve.IsPlaying();
	}

	/** 停止 */
	void Stop() override {
		m_curve.Stop();
	}


	/** リセット */
	void Reset() override
	{
		m_curve.Reset();
	}

	/** UIアニメーションの情報を設定 */
	void SetParameter(Vector2 start, Vector2 end, float timeSec, EasingType type, LoopMode loopMode)
	{
		m_start = start;
		m_end = end;
		m_timeSec = timeSec;
		m_type = type;
		m_loopMode = loopMode;
		//m_curve.Play(start, end, timeSec, type, loopMode);

	}

	/** アニメーション中の現在の値を取得 */
	Vector2 GetCurrentValue()
	{
		return m_curve.GetCurrentValue();
	}

	/** アニメーション後の情報を適用する関数を設定 */
	void SetFunc(const UIAnimationApplyFunc<Vector2>& func)
	{
		m_applyFunc = func;
	}
};




/***************************************/


/** Vector3のアニメーション(position, scale~~) */
class UIVector3Animation : public UIAnimationBase
{
protected:
	Vector3Curve m_curve;
	/** カーブ用のパラメーター */
	Vector3 m_start = Vector3::Zero;
	Vector3 m_end = Vector3::Zero;
	float m_timeSec = 0.0f;
	EasingType m_type = EasingType::Linear;
	LoopMode m_loopMode = LoopMode::Once;

	UIAnimationApplyFunc<Vector3> m_applyFunc;


public:
	UIVector3Animation() {}
	~UIVector3Animation() {}

	void Update() override
	{
		m_curve.Update(g_gameTime->GetFrameDeltaTime());
		//m_applyFunc(m_curve.GetCurrentValue());
		if (m_applyFunc) {
			m_applyFunc(m_curve.GetCurrentValue());
		}
		else {
			// Handle the error or log a message
		}
	}

	void Play() override
	{
		m_curve.Play();
	}

	bool IsPlay() override
	{
		return m_curve.IsPlaying();
	}

	void Stop() override {
		m_curve.Stop();
	}

	/** リセット */
	void Reset() override
	{
		m_curve.Reset();
	}

	void SetParameter(Vector3 start, Vector3 end, float timeSec, EasingType type, LoopMode loopMode)
	{
		m_start = start;
		m_end = end;
		m_timeSec = timeSec;
		m_type = type;
		m_loopMode = loopMode;
		m_curve.Initialize(start, end, timeSec, type, loopMode);
	}

	Vector3 GetCurrentValue()
	{
		return m_curve.GetCurrentValue();
	}

	void SetFunc(const UIAnimationApplyFunc<Vector3>& func)
	{
		m_applyFunc = func;
	}
};




/*****************************************/


/** Vector4のアニメーション(color~~) */
class UIVector4Animation : public UIAnimationBase
{
protected:
	Vector4Curve m_curve;
	/** カーブ用のパラメーター */
	Vector4 m_start = Vector4::White;
	Vector4 m_end = Vector4::White;
	float m_timeSec = 0.0f;
	EasingType m_type = EasingType::Linear;
	LoopMode m_loopMode = LoopMode::Once;

	UIAnimationApplyFunc<Vector4> m_applyFunc;

public:
	UIVector4Animation() {}
	~UIVector4Animation() {}

	void Update() override
	{
		m_curve.Update(g_gameTime->GetFrameDeltaTime());
		//m_applyFunc(m_curve.GetCurrentValue());
		if (m_applyFunc) {
			m_applyFunc(m_curve.GetCurrentValue());
		}
		else {
			// Handle the error or log a message
		}
	}

	void Play() override
	{
		m_curve.Play();
	}

	bool IsPlay() override
	{
		return m_curve.IsPlaying();
	}

	void Stop() override {
		m_curve.Stop();
	}

	/** リセット */
	void Reset() override
	{
		m_curve.Reset();
	}

	void SetParameter(Vector4 start, Vector4 end, float timeSec, EasingType type, LoopMode loopMode)
	{
		m_start = start;
		m_end = end;
		m_timeSec = timeSec;
		m_type = type;
		m_loopMode = loopMode;
		m_curve.Initialize(start, end, timeSec, type, loopMode);
	}


	Vector4 GetCurrendtValue()
	{
		return m_curve.GetCurrentValue();
	}

	void SetFunc(const UIAnimationApplyFunc<Vector4>& func)
	{
		m_applyFunc = func;
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
		m_curve.Update(g_gameTime->GetFrameDeltaTime());
		m_applyFunc(m_curve.GetCurrentValue());
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
		m_curve.Update(g_gameTime->GetFrameDeltaTime());
		m_applyFunc(m_curve.GetCurrentValue());
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
		m_curve.Update(g_gameTime->GetFrameDeltaTime());
		m_applyFunc(m_curve.GetCurrentValue());
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
		m_curve.Update(g_gameTime->GetFrameDeltaTime());
		m_applyFunc(m_curve.GetCurrentValue());
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
		m_curve.Update(g_gameTime->GetFrameDeltaTime());
		m_applyFunc(m_curve.GetCurrentValue());
	}

};