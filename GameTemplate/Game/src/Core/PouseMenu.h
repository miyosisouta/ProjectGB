#pragma once



class PouseMenu : public IGameObject
{
private:
	/** 表示状態の可否 */
	bool isActive_ = false;
	/** タイトルシーンへ戻るフラグ */
	bool isReturnTitle_ = false;
	/** 現在のメニューインデックス */
	uint8_t currentMenuIndex_ = 0;
	/** メニューの文字列 */
	FontRender menuText_;


public:
	/** メニューの表示状態を設定 */
	inline void SetActive(const bool flg) { isActive_ = flg; }
	/** タイトル変戻るフラグを取得 */
	inline const bool IsReturnTitle() const { return isReturnTitle_; }



	public:
		PouseMenu();
		~PouseMenu();



	private:
		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;



private:
	void MoveMenu();
	void SelectMenu();

};

