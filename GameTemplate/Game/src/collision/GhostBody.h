/**
 * GhostBody.h
 * ゴースト用の衝突体本体
 */
#pragma once
#include "GhostPrimitive.h"


class btCollisionObject;
class btCollisionShape;


class GhostBodyManager;


/**
 * ゴースト用衝突体本体
 */
class GhostBody : public Noncopyable
{
	friend class GhostBodyManager;


private:
	IGameObject* owner_ = nullptr;			//!< 所有者
	uint32_t ownerId_ = 0;					//!< 所有者を識別するID
	std::unique_ptr<IGhostShape> shape_;	//!< 形状

	/** Broadphase用ハンドル */
	void* broadphaseHandle_ = nullptr;	//!< Broadphase側のノードハンドル
	bool isDirty_ = true;				//!< AABBの再計算が必要か
	bool isActive_ = true;				//!< 判定対象として有効か

	/** 属性 */
	uint32_t attribute_ = ghost::CollisionAttribute::None;	//!< 自分の属性
	uint32_t mask_ = ghost::CollisionAttributeMask::All;	//!< 判定する相手の属性マスク

	/** トランスフォーム */
	Vector3 position_ = Vector3::Zero;			 //!< 座標
	Quaternion rotation_ = Quaternion::Identity; //!< 回転

	/** 詳細判定用BulletObject */
	std::unique_ptr<btGhostObject> bulletObject_;	//!< Bullet側のゴーストオブジェクト
	std::unique_ptr<btCollisionShape> bulletShape_; //!< Bullet側の衝突形状


private:
	/** 形状からBulletのオブジェクトを再構築する */
	void RebuildBulletObject();
	/** 現在の座標・回転からBulletのTransformを作成する */
	btTransform GetBtTransform() const;


public:
	/** コンストラクタ */
	GhostBody();
	/** デストラクタ */
	~GhostBody();

	/** 球形状で初期化 */
	void CreateSphere(IGameObject* owner, const uint32_t id, const float radius, const uint32_t attr, const uint32_t mask);
	/** ボックス形状で初期化 */
	void CreateBox(IGameObject* owner, const uint32_t id, const Vector3& half, const uint32_t attr, const uint32_t mask);
	/** カプセル形状で初期化 */
	void CreateCapsule(IGameObject* owner, const uint32_t id, const float r, const float h, const uint32_t attr, const uint32_t mask);
	/** 形状生成後の共通初期化（所有者・属性の設定とマネージャーへの登録） */
	void CreateCore(IGameObject* owner, const uint32_t id, const uint32_t attr, const uint32_t mask);


	/** 座標の設定 */
	void SetPosition(const Vector3& pos);
	/** 座標の取得 */
	inline const Vector3& GetPosition() const { return position_; }
	/** 回転の設定 */
	void SetRotation(const Quaternion& rot);
	/** 回転の取得 */
	inline const Quaternion& GetRotation() const { return rotation_; }

	/** 所有者の取得 */
	inline IGameObject* GetOwner() const { return owner_; }
	/** 所有者IDの取得 */
	inline uint32_t GetOwnerId() const { return ownerId_; }
	/** 自分の属性の取得 */
	inline uint32_t GetAttribute() const { return attribute_; }
	/** 判定する相手の属性マスクの取得 */
	inline uint32_t GetMask() const { return mask_; }
	/** 判定対象の有効/無効を設定 */
	inline void SetActive(bool active) { isActive_ = active; }
	/** 判定対象として有効か */
	inline bool IsActive() const { return isActive_; }

	/** 形状タイプの取得 */
	inline GhostShapeType GetShapeType() const { return shape_ ? shape_->GetType() : GhostShapeType::Sphere; }
	/** 包含球の半径の取得（早期リターン用） */
	inline float GetBoundingRadius() const { return shape_ ? shape_->GetBoundingRadius() : 0.0f; }


	/** Broadphase連携用 */
	/** Broadphase側のノードハンドルを設定 */
	inline void SetBroadphaseHandle(void* handle) { broadphaseHandle_ = handle; }
	/** Broadphase側のノードハンドルを取得 */
	inline void* GetBroadphaseHandle() const { return broadphaseHandle_; }
	/** AABBを計算する */
	void ComputeAabb(btVector3& min, btVector3& max) const;

	/** 再計算不要フラグを立てる */
	inline void ClearDirty() { isDirty_ = false; }
	/** AABBの再計算が必要か */
	inline bool IsDirty() const { return isDirty_; }

	/** 詳細判定用 */
	inline btCollisionObject* GetBulletObject() { return bulletObject_.get(); }
};