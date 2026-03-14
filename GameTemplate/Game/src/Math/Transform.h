/**
 * Transform.h
 * 座標、回転、拡縮をまとめたもの
 * 親子構造をした座標計算などをしやすい
 */
#pragma once


class Transform : public Noncopyable
{
public:
	/** 自身の現在のパラメータ */
	Vector3 localPosition;
	Quaternion localRotation;
	Vector3 localScale;

	/** 親トランスフォームを考慮したパラメータ */
	// 原点となるもの
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;


	/**
	 * private変数
	 * 基本的に関数以外から弄る必要なし
	 */
private:
	Matrix rotationMatrix_;
	Matrix worldMatrix_;

	Transform* parent_;
	std::vector<Transform*> children_;


public:
	Transform();
	~Transform();

	//更新処理
	void UpdateTransform();
	//ワールド行列更新、UpdateTransformの方で呼ばれるので呼び出す必要なし
	void UpdateWorldMatrix();

	//全ての子トランスフォームとの紐づけを外す
	void Release();

	//特定の子トランスフォームとの紐づけを外す
	void RemoveChild(Transform* t);

	//親トランスフォームを設定
	void SetParent(Transform* p)
	{
		parent_ = p;
		parent_->children_.push_back(this);
	}

	void ResetLocalPosition();
	void ResetLocalRotation();

	const Matrix& GetWorldMatrix() const { return worldMatrix_; }


	const std::vector<Transform*>& GetChildren() { return children_; }
	bool HasChild() { return children_.size() > 0; }
};