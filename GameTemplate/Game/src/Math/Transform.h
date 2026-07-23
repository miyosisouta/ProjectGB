/**
 * Transform.h
 * 座標、回転、拡縮をまとめたもの
 * 親子構造をした座標計算などをしやすい
 */
#pragma once


class Transform : public Noncopyable
{
	/**
	 * private変数
	 * 基本的に関数以外から弄る必要なし
	 */
private:
	Matrix rotationMatrix_; //!< 回転行列
	Matrix worldMatrix_; //!< ワールド行列

	Transform* parent_; //!< 親トランスフォーム
	std::vector<Transform*> children_; //!< 子トランスフォームのリスト


public:
	/** 自身の現在のパラメータ */
	Vector3 localPosition; //!< ローカル座標
	Quaternion localRotation; //!< ローカル回転
	Vector3 localScale; //!< ローカル拡縮

	/** 親トランスフォームを考慮したパラメータ */
	// 原点となるもの
	Vector3 position; //!< ワールド座標
	Quaternion rotation; //!< ワールド回転
	Vector3 scale; //!< ワールド拡縮


public:
	/** コンストラクタ */
	Transform();
	/** デストラクタ */
	~Transform();

	/** 更新処理 */
	void UpdateTransform();
	/** ワールド行列更新、UpdateTransformの方で呼ばれるので呼び出す必要なし */
	void UpdateWorldMatrix();

	/** 全ての子トランスフォームとの紐づけを外す */
	void Release();

	/** 特定の子トランスフォームとの紐づけを外す */
	void RemoveChild(Transform* t);

	/** 親トランスフォームを設定 */
	void SetParent(Transform* p)
	{
		parent_ = p;
		parent_->children_.push_back(this);
	}

	/** ローカル座標をリセット */
	void ResetLocalPosition();
	/** ローカル回転をリセット */
	void ResetLocalRotation();

	/** ワールド行列の取得 */
	inline const Matrix& GetWorldMatrix() const { return worldMatrix_; }


	/** 子トランスフォームのリストを取得 */
	inline const std::vector<Transform*>& GetChildren() { return children_; }
	/** 子を持っているか */
	inline bool HasChild() { return children_.size() > 0; }
};