/**
 * UIBase.h
 * UIの基本的な処理をするクラス群
 */
#pragma once
#include "UIAnimation.h"
#include <memory>
#include <vector>
#include "src/Math/Transform.h"

 //class UIAnimationBase;

 /** 基底クラス */
class UIBase : public Noncopyable
{
public:
	Transform transform;
	Vector4 color = Vector4::White;

	/*bool isStart = false;
	bool isUpdate = true;*/
	bool isDraw = true;

public:
	// deleteがいらない
	//std::vector<std::unique_ptr<UIAnimationBase>> m_uiAnimationList;
	std::unordered_map<uint32_t, std::unique_ptr<UIAnimationBase>> uiAnimationList;


public:
	UIBase()
	{
		uiAnimationList.clear();
	}
	virtual ~UIBase()
	{
		// 明示的に消しているだけ。本来は要らない
		uiAnimationList.clear();
	}

	//virtual bool Start() = 0;
	virtual void Update() = 0;
	virtual void Render(RenderContext& rc) = 0;


public:
	void UpdateAnimation()
	{
		ForEachAnimation([](UIAnimationBase* animation)
			{
				animation->Update();
			});
	}
	void PlayAnimation()
	{
		ForEachAnimation([](UIAnimationBase* animation)
			{
				animation->Play();
			});
	}
	void ResetAnimation()
	{
		ForEachAnimation([](UIAnimationBase* animation)
			{
				animation->Reset();
			});
	}
	bool IsPlayAniamtion()
	{
		auto it = std::find_if(uiAnimationList.begin(), uiAnimationList.end(), [&](const auto& animationPair)
			{
				auto* animation = animationPair.second.get();
				if (animation->IsPlay()) {
					return true;
				}
				return false;
			});
		return it != uiAnimationList.end();
	}
	void StopSpriteAnimation()
	{
		ForEachAnimation([](UIAnimationBase* animation)
			{
				animation->Stop();
			});
	}
	bool IsComplted() const
	{
		// すべて再生済みか
		auto it = std::find_if(uiAnimationList.begin(), uiAnimationList.end(), [&](const auto& animationPair)
			{
				auto* animation = animationPair.second.get();
				return !animation->IsPlay();
			});
		return it != uiAnimationList.end();
	}


	void AddAnimation(const uint32_t key, std::unique_ptr<UIAnimationBase> animation)
	{
		animation->SetUI(this);
		uiAnimationList.emplace(key, std::move(animation));
	}

	void RemoveAnimation(const uint32_t key)
	{
		uiAnimationList.erase(key);
	}

	void ForEachAnimation(const std::function<void(UIAnimationBase*)>& func)
	{
		for (auto& animation : uiAnimationList)
		{
			func(animation.second.get());
		}
	}

	UIAnimationBase* FindAnimaion(const uint32_t key)
	{
		auto it = uiAnimationList.find(key);
		if (it != uiAnimationList.end())
		{
			return it->second.get();
		}
		return nullptr;
	}
};




// ============================================
// 画像を使うUI関連
// ============================================


class UIImage : public UIBase
{
protected:
	SpriteRender spriteRender_;


protected:
	UIImage();
	~UIImage();


public:
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

public:
	/** スプライトレンダーの取得 */
	SpriteRender* GetSpriteRender() { return &spriteRender_; }
};



/**
 * ゲージUI
 */
class UIGauge : public UIImage
{
	friend class UICanvas;


private:
	UIGauge();
	~UIGauge();


public:
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

public:
	void Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation);
};


/**
 * アイコンUI
 */
class UIIcon : public UIImage
{
	friend class UICanvas;

	//private:
public:
	UIIcon();
	~UIIcon();


public:
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;


public:
	void Initialize(const char* assetName, const float width, const float height);
};




// ============================================
// 文字を使うUI関連
// ============================================


class UIText : public UIBase
{
protected:
	FontRender fontRender_;


private:
	UIText();
	~UIText();


public:
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;

	void SetText(const wchar_t* text)
	{
		fontRender_.SetText(text);
	}
};



// ============================================
// ボタンを使うUI関連
// ============================================
class UIButton : public UIImage
{
private:
	/** ボタンが押されたときの処理(外部から委譲される) */
	std::function<void()> delegate_;


private:
	UIButton();
	~UIButton();


public:
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;
};



// ============================================
// UI桁表示(スコア表示などで使用)
// ============================================
class UIDigit : public UIBase
{
private:
	/** 画像表示機能の可変長配列 */
	std::vector<SpriteRender*> renderList_;
	/** 表示される数字 */
	int number_;
	int requestNumber_;
	int digit_;
	/** 数字表示に必要な画像が入った */
	std::string assetPath_;

	int w;
	int h;



public:
	UIDigit();
	~UIDigit();


public:
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;


public:
	/**
	 * ・アセットの名前
	 * ・何桁かの情報（数）
	 * ・表示する数
	 * ・横
	 * ・高さ
	 * ・位置
	 * ・大きさ
	 * ・回転
	 */
	void Initialize(const char* assetPath, const int digit, const int number, const float widht, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation);

	/** 数字を設定 */
	void SetNumber(const int number) { requestNumber_ = number; }

	std::vector<SpriteRender*>& GetSpriteRenderList() { return renderList_; }

	void ForEach(const std::function<void(SpriteRender*)>& func)
	{
		for (auto* render : renderList_) {
			func(render);
		}
	}


private:
	void UpdateNumber(const int targetDigit, const int number);
	void UpdatePosition(const int index);

	/** 対象の桁 */
	int GetDigit(int digit);
};




// ============================================
// キャンバス
// ============================================


/**
 * 絵を書くキャンバスのイメージ
 * UIを作るときにこのクラスを作ってください
 */
class UICanvas : public UIBase
{
	friend class UIBase;
	friend class UIImage;
	friend class UIGauge;
	friend class UIIcon;
	friend class UIText;
	friend class UIButton;


private:
	/**
	 * NOTE: 各UI自体に親子関係持たせたいけど使わない可能性があるので、一旦ここだけにしてみる
	 */
	 //std::vector<UIBase*> m_uiList;
	std::unordered_map<uint32_t, std::unique_ptr<UIBase>> uiList_;


public:
	UICanvas();
	~UICanvas();


	void Update() override;
	void Render(RenderContext& rc) override;


public:
	template <typename T>
	T* CreateUI(const uint32_t key)
	{
		auto ui = std::make_unique<T>();
		// transformの名前が違うかも もしエラーはいたら確認
		ui->transform.SetParent(&transform);
		uiList_.emplace(key, std::move(ui));
		return static_cast<T*>(uiList_[key].get());
	}

	void RemoveUI(const uint32_t key)
	{
		uiList_.erase(key);
	}

	template <typename T>
	T* FindUI(const uint32_t key)
	{
		auto it = uiList_.find(key);
		if (it != uiList_.end())
		{
			return dynamic_cast<T*>(it->second.get());
		}
		return nullptr;
	}

	/*template <typename T>
	T* CreateUI()
	{
		T* ui = new T();
		ui->m_transform.SetParent(&m_transform);
		m_uiList.push_back(ui);
		return ui;
	}*/
};