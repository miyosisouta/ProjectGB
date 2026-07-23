/**
 * PhysicalBody.h
 * 物理オブジェクト
 */
#pragma once
#include "src/collision/Types.h"


/**
 * 物理オブジェクト
 */
class PhysicalBody : public Noncopyable
{
private:
	/** コライダー */
	std::unique_ptr<ICollider> collider_;
	/** 剛体 */
	RigidBody rigidBody_;


private:
	/** コライダー生成後の共通初期化（剛体の生成） */
	void CreateCore(const std::function<void(RigidBodyInitData& data)>& func, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags, const float restitution = 0.0f);


public:
	/** コンストラクタ */
	PhysicalBody();
	/** デストラクタ */
	~PhysicalBody();

	/**
	 * モデルからの静的オブジェクトの作成。
	 */
	void CreateFromModel(Model& model, const Matrix& worldMatrix, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags = btCollisionObject::CF_STATIC_OBJECT, const float restitution = 0.0f);
	/** カプセル形状のオブジェクトを作成 */
	void CreateCapsule(float radius, float height, const Vector3& position, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags = btCollisionObject::CF_STATIC_OBJECT, const float restitution = 0.0f);
	/** ボックス形状のオブジェクトを作成 */
	void CreateBox(const Vector3& size, const Vector3& position, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags = btCollisionObject::CF_STATIC_OBJECT, const float restitution = 0.0f);
	/** 球形状のオブジェクトを作成 */
	void CreateSphere(float radius, const Vector3& position, const uint32_t collisionAttribute, const btCollisionObject::CollisionFlags collisionFlags = btCollisionObject::CF_STATIC_OBJECT, const float restitution = 0.0f);

	/** 剛体を破棄 */
	inline void Release()
	{
		rigidBody_.Release();
	}

	/** 座標設定 */
	void SetPosition(const Vector3& position);

	/**
	 * 摩擦力を設定
	 * NOTE: 10が最大値
	 */
	inline void SetFriction(float friction)
	{
		rigidBody_.SetFriction(friction);
	}
	/** btCollisionObjectを取得 */
	inline btCollisionObject* GetbtCollisionObject()
	{
		return rigidBody_.GetBody();
	}
};