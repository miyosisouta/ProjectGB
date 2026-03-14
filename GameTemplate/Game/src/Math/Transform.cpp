/**
 * Transform.cpp
 * 座標、回転、拡縮をまとめたもの
 * 親子構造をした座標計算などをしやすい
 */
#include "stdafx.h"
#include "Transform.h"


Transform::Transform()
	: position(Vector3::Zero)
	, localPosition(Vector3::Zero)
	, scale(Vector3::One)
	, localScale(Vector3::One)
	, rotation(Quaternion::Identity)
	, localRotation(Quaternion::Identity)
	, rotationMatrix_(Matrix::Identity)
	, worldMatrix_(Matrix::Identity)
	, parent_(nullptr)
{
	children_.clear();
}

Transform::~Transform()
{
	if (parent_) {
		parent_->RemoveChild(this);
	}
	Release();
}

void Transform::UpdateTransform()
{

	if (parent_) {
		//座標計算
		Matrix localPos;
		localPos.MakeTranslation(localPosition);

		Matrix pos;
		pos.Multiply(localPos, parent_->worldMatrix_);

		//多分平行移動の部分を取ってるだけ
		position.x = pos.m[3][0];
		position.y = pos.m[3][1];
		position.z = pos.m[3][2];

		//スケール
		scale.x = localScale.x * parent_->scale.x;
		scale.y = localScale.y * parent_->scale.y;
		scale.z = localScale.z * parent_->scale.z;

		//回転
		rotation = parent_->rotation * localRotation;

	}
	else
	{
		//ローカルの値をそのままコピー
		position = localPosition;
		scale = localScale;
		rotation = localRotation;
	}

	//回転行列
	rotationMatrix_.MakeRotationFromQuaternion(rotation);
	//ワールド行列更新
	UpdateWorldMatrix();
}

void Transform::UpdateWorldMatrix()
{
	Matrix scal, pos, world;
	scal.MakeScaling(scale);
	pos.MakeTranslation(position);

	world.Multiply(scal, rotationMatrix_);
	worldMatrix_.Multiply(world, pos);

	//子も更新
	for (Transform* child : children_)
	{
		child->UpdateTransform();
	}
}


void Transform::Release()
{
	////イテレータ生成
	//std::vector<Transform*>::iterator it = m_children_.begin();
	////vectorの終わりまで回す
	//while (it != m_children_.end())
	//{
	//	//子トランスフォームからの紐づけを外す
	//	(*it)->m_parent_ = nullptr;
	//	//子トランスフォームへの紐づけを外す
	//	m_children_.erase(it);
	//	if (m_children_.size() <= 0) {
	//		break;
	//	}
	//	//イテレータを進める
	//	++it;
	//}
	////念のため？vectorの要素を全削除
	//m_children_.clear();

	// 1. 全ての子どもの親設定を解除
	for (auto* child : children_)
	{
		if (child) {
			child->parent_ = nullptr;
		}
	}

	// 2. リストを空にする
	children_.clear();
}

void Transform::RemoveChild(Transform* t)
{
	//イテレータ生成
	std::vector<Transform*>::iterator it = children_.begin();
	//vectorを回す
	while (it != children_.end())
	{
		//イテレータから子トランスフォームのポインタを受け取る
		Transform* child = (*it);
		//受け取った子トランスフォームがしてされた物なら
		if (child == t)
		{
			//紐づけを外す
			child->parent_ = nullptr;
			children_.erase(it);
			//処理を終了
			return;
		}

		//イテレータを進める
		++it;
	}
}


void Transform::ResetLocalPosition()
{
	localPosition = Vector3::Zero;
}


void Transform::ResetLocalRotation()
{
	localRotation = Quaternion::Identity;
}