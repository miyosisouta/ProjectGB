#pragma once
/**
 * 実体のあるものすべてで必要なものをまとめている基底クラス
 * IGameObjectを継承させているので、自動更新・描画が行われる
 */
class ActorStatus;

class Actor : public IGameObject
{
public:
	/** 例外としてpublic */
	Transform transform_; //!< トランスフォーム

protected:
	ModelRender modelRender_; //!< モデルレンダー
	ActorStatus* status_ = nullptr; //!< ステータス

protected:
	bool isDraw_ = true; //!< 描画するか（falseにするとRenderで描画をスキップする）

public:
	/** グローバルな座標を取得 */
	inline const Vector3 GetTransformPosition() const { return transform_.position; }
	/** グローバルな回転を取得 */
	inline const Quaternion GetTransformRotation() const { return transform_.rotation; }
	/** グローバルな大きさを取得 */
	inline const Vector3 GetTransformScale() const { return transform_.scale; }

	/* ステータスを取得 */
	inline ActorStatus* GetStatus() { return status_; }

	/** 描画の可否を設定する（一時的に非表示にしたい場合に使用） */
	inline void SetDraw(bool isDraw) { isDraw_ = isDraw; }
	/** 描画中か */
	inline bool GetDraw() const { return isDraw_; }


public:
	/* コンストラクタ */
	Actor() {}
	/* デストラクタ */
	~Actor() {}

	/* スタート処理 */
	virtual bool Start() override;
	/* 更新処理 */
	virtual void Update() override;
	/* 描画処理 */
	virtual void Render(RenderContext& rc) override;
};