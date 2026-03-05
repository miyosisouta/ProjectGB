/**
 * Transform.cpp
 * 座標、回転、拡縮をまとめたもの
 * 親子構造をした座標計算などをしやすい
 */
#include "stdafx.h"
#include "Transform.h"


namespace app
{
	namespace math
	{
		Transform::Transform()
			:m_position(Vector3::Zero)
			, m_localPosition(Vector3::Zero)
			, m_scale(Vector3::One)
			, m_localScale(Vector3::One)
			, m_rotation(Quaternion::Identity)
			, m_localRotation(Quaternion::Identity)
			, m_rotationMatrix_(Matrix::Identity)
			, m_worldMatrix_(Matrix::Identity)
			, m_parent_(nullptr)
		{
			m_children_.clear();
		}

		Transform::~Transform()
		{
			if (m_parent_) {
				m_parent_->RemoveChild(this);
			}
			Release();
		}

		void Transform::UpdateTransform()
		{

			if (m_parent_) {
				//座標計算
				Matrix localPos;
				localPos.MakeTranslation(m_localPosition);

				Matrix pos;
				pos.Multiply(localPos, m_parent_->m_worldMatrix_);

				//多分平行移動の部分を取ってるだけ
				m_position.x = pos.m[3][0];
				m_position.y = pos.m[3][1];
				m_position.z = pos.m[3][2];

				//スケール
				m_scale.x = m_localScale.x * m_parent_->m_scale.x;
				m_scale.y = m_localScale.y * m_parent_->m_scale.y;
				m_scale.z = m_localScale.z * m_parent_->m_scale.z;

				//回転
				m_rotation = m_parent_->m_rotation * m_localRotation;

			}
			else
			{
				//ローカルの値をそのままコピー
				m_position = m_localPosition;
				m_scale = m_localScale;
				m_rotation = m_localRotation;
			}

			//回転行列
			m_rotationMatrix_.MakeRotationFromQuaternion(m_rotation);
			//ワールド行列更新
			UpdateWorldMatrix();
		}

		void Transform::UpdateWorldMatrix()
		{
			Matrix scal, pos, world;
			scal.MakeScaling(m_scale);
			pos.MakeTranslation(m_position);

			world.Multiply(scal, m_rotationMatrix_);
			m_worldMatrix_.Multiply(world, pos);

			//子も更新
			for (Transform* child : m_children_)
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
			for (auto* child : m_children_)
			{
				if (child) {
					child->m_parent_ = nullptr;
				}
			}

			// 2. リストを空にする
			m_children_.clear();
		}

		void Transform::RemoveChild(Transform* t)
		{
			//イテレータ生成
			std::vector<Transform*>::iterator it = m_children_.begin();
			//vectorを回す
			while (it != m_children_.end())
			{
				//イテレータから子トランスフォームのポインタを受け取る
				Transform* child = (*it);
				//受け取った子トランスフォームがしてされた物なら
				if (child == t)
				{
					//紐づけを外す
					child->m_parent_ = nullptr;
					m_children_.erase(it);
					//処理を終了
					return;
				}

				//イテレータを進める
				++it;
			}
		}


		void Transform::ResetLocalPosition()
		{
			m_localPosition.x = 0.0f;
			m_localPosition.y = 0.0f;
			m_localPosition.z = 0.0f;
		}


		void Transform::ResetLocalRotation()
		{
			m_localRotation.x = 0.0f;
			m_localRotation.y = 0.0f;
			m_localRotation.z = 0.0f;
		}
	}
}