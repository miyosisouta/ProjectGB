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
	Vector3 m_localPosition;
	Quaternion m_localRotation;
	Vector3 m_localScale;

	/** 親トランスフォームを考慮したパラメータ */
	// 原点となるもの
	Vector3 m_position;
	Quaternion m_rotation;
	Vector3 m_scale;


	/**
	 * private変数
	 * 基本的に関数以外から弄る必要なし
	 */
private:
	Matrix m_rotationMatrix_;
	Matrix m_worldMatrix_;

	Transform* m_parent_;
	std::vector<Transform*> m_children_;


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
		m_parent_ = p;
		m_parent_->m_children_.push_back(this);
	}

	void ResetLocalPosition();
	void ResetLocalRotation();

	const Matrix& GetWorldMatrix() const { return m_worldMatrix_; }


	const std::vector<Transform*>& GetChildren() { return m_children_; }
	bool HasChild() { return m_children_.size() > 0; }
};