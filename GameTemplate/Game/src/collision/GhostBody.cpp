#include "stdafx.h"
#include "GhostBody.h"
#include "GhostBodyManager.h"



GhostBody::GhostBody()
{
}


GhostBody::~GhostBody()
{
	if (GhostBodyManager::IsAvailable()) {
		GhostBodyManager::Get().RemoveBody(this);
	}
}


void GhostBody::CreateSphere(IGameObject* owner, const uint32_t id, const float radius, const uint32_t attr, const uint32_t mask)
{
	shape_ = std::make_unique<GhostSphere>(radius);
	CreateCore(owner, id, attr, mask);
}


void GhostBody::CreateBox(IGameObject* owner, const uint32_t id, const Vector3& half, const uint32_t attr, const uint32_t mask)
{
	shape_ = std::make_unique<GhostBox>(half);
	CreateCore(owner, id, attr, mask);
}


void GhostBody::CreateCapsule(IGameObject* owner, const uint32_t id, const float r, const float h, const uint32_t attr, const uint32_t mask)
{
	shape_ = std::make_unique<GhostCapsule>(r, h);
	CreateCore(owner, id, attr, mask);
}


void GhostBody::CreateCore(IGameObject* owner, const uint32_t id, const uint32_t attr, const uint32_t mask)
{
	// 所有者・属性情報を設定（所有者本体・識別ID・自分の属性・判定する相手のマスク）
	owner_ = owner;         // 所有者
	attribute_ = attr;      // 自分の属性
	mask_ = mask;           // 判定する相手の属性マスク
	ownerId_ = id;          // 所有者を識別するID
	RebuildBulletObject();
	if (GhostBodyManager::IsAvailable()) {
		GhostBodyManager::Get().AddBody(this);
	}
}


void GhostBody::SetPosition(const Vector3& pos)
{
	if (!position_.IsEqual(pos)) {
		position_ = pos;
		isDirty_ = true;
		// BulletObjectも同期
		if (bulletObject_) {
			bulletObject_->setWorldTransform(GetBtTransform());
		}
	}
}


void GhostBody::SetRotation(const Quaternion& rot)
{
	if (!rotation_.IsEqual(rot)) {
		rotation_ = rot;
		isDirty_ = true;
		if (bulletObject_) {
			bulletObject_->setWorldTransform(GetBtTransform());
		}
	}
}


void GhostBody::ComputeAabb(btVector3& min, btVector3& max) const
{
	if (shape_) {
		shape_->GetAabb(GetBtTransform(), min, max);
	}
}


btTransform GhostBody::GetBtTransform() const
{
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(position_.x, position_.y, position_.z));
	t.setRotation(btQuaternion(rotation_.x, rotation_.y, rotation_.z, rotation_.w));
	return t;
}


void GhostBody::RebuildBulletObject()
{
	bulletShape_.reset(shape_->CreateBulletShape());
	bulletObject_ = std::make_unique<btGhostObject>();
	bulletObject_->setCollisionShape(bulletShape_.get());
	bulletObject_->setCollisionShape(bulletShape_.get());
	bulletObject_->setWorldTransform(GetBtTransform());
	// Bullet側でCollisionのフラグが必要ならここで設定
	// m_bulletObject->setCollisionFlags(m_bulletObject->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
}