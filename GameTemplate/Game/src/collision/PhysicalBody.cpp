#include "stdafx.h"
#include "PhysicalBody.h"


PhysicalBody::PhysicalBody()
{
}


PhysicalBody::~PhysicalBody()
{
}


void PhysicalBody::CreateFromModel(Model& model, const Matrix& worldMatrix, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags, const float restitution)
{
	auto* meshCollider = new MeshCollider();
	meshCollider->CreateFromModel(model, worldMatrix);
	collider_.reset(meshCollider);
	CreateCore([](RigidBodyInitData& data) {}, collisionAttribute, collisionFlags,  restitution);
}


void PhysicalBody::CreateCapsule(float radius, float height, const Vector3& position, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags, const float restitution)
{
	auto* capsuleCollider = new CCapsuleCollider();
	capsuleCollider->Init(radius, height);
	collider_.reset(capsuleCollider);
	CreateCore([position](RigidBodyInitData& data)
		{
			data.pos = position;
		},
		collisionAttribute, collisionFlags, restitution);
}


void PhysicalBody::CreateBox(const Vector3& size, const Vector3& position, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags, const float restitution)
{
	auto* boxCollider = new BoxCollider();
	boxCollider->Create(size);
	collider_.reset(boxCollider);
	CreateCore([position](RigidBodyInitData& data)
		{
			data.pos = position;
		},
		collisionAttribute, collisionFlags, restitution);
}


void PhysicalBody::CreateSphere(float radius, const Vector3& position, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags, const float restitution)
{
	auto* sphereCollider = new SphereCollider();
	sphereCollider->Create(radius);
	collider_.reset(sphereCollider);
	CreateCore([position](RigidBodyInitData& data)
		{
			data.pos = position;
		},
		collisionAttribute, collisionFlags, restitution);
}


void PhysicalBody::CreateCore(const std::function<void(RigidBodyInitData& data)>& func, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags, const float restitution)
{
	// 剛体の初期化データを組み立てる（コライダー・質量・反発係数）
	RigidBodyInitData rbInfo;
	rbInfo.collider = collider_.get(); // コライダー
	rbInfo.mass = 0.0f;                // 質量（0=静的オブジェクト）
	rbInfo.restitution = restitution;  // 反発係数
	// カスタム設定（呼び出し元ごとの座標設定など）
	func(rbInfo);
	rigidBody_.Init(rbInfo);

	rigidBody_.GetBody()->setUserIndex(collisionAttribute);
	rigidBody_.GetBody()->setCollisionFlags(collisionFlags);
}


void PhysicalBody::SetPosition(const Vector3& position)
{
	btTransform& trans = rigidBody_.GetBody()->getWorldTransform();
	trans.setOrigin(btVector3(position.x, position.y, position.z));
	rigidBody_.GetBody()->setActivationState(DISABLE_DEACTIVATION);
}