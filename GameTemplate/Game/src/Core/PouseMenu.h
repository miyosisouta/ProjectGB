#pragma once


class Layout;


class PouseMenu : public IGameObject
{
private:
	/** 表示状態の可否 */
	bool isActive_ = false;
	/** タイトルシーンへ戻るフラグ */
	bool isReturnTitle_ = false;

	Layout* layout_ = nullptr;


public:
	/** メニューの表示状態を設定 */
	inline void SetActive(const bool flg) { isActive_ = flg; }
	/** メニューが表示状態かのフラグを取得 */
	inline bool GetActive() { return isActive_; }
	/** タイトル変戻るフラグを取得 */
	inline const bool IsReturnTitle() const { return isReturnTitle_; }


public:
	PouseMenu();
	~PouseMenu();


private:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};

