/**
 * AttackObjectManager.cpp
 * 攻撃用オブジェクトを管理するクラス
 */


#include "stdafx.h"
#include "AttackObjectManager.h"
#include "AttackObject.h"



AttackObjectManager* AttackObjectManager::instance_ = nullptr; //初期化


AttackObjectManager::AttackObjectManager()
{
	
}


AttackObjectManager::~AttackObjectManager()
{

}


void AttackObjectManager::Update()
{
    std::vector<AttackObjectBase*> deleteList;

    // 全オブジェクトを更新
    for (auto& obj : objects_)
    {
        obj->Update();

        if (obj->IsFinished())
        {
            deleteList.push_back(obj.get());
        }
    }

    for (auto* deleteObject : deleteList)
    {
        auto it = std::find_if(objects_.begin(), objects_.end(), [deleteObject](auto& obj)
            {
                return obj.get() == deleteObject;
            });
        objects_.erase(it);
    }
}


void AttackObjectManager::Render(RenderContext& rc)
{
    for (auto& obj : objects_)
    {
        obj->Render(rc);
    }
}

void AttackObjectManager::CreateRock(BossCharacter* boss, const Vector3& startPos, const Vector3& direction,float collisionSize, float speed, float launchAngle, float gravity)
{
    auto rock = std::make_unique<ThrowRockObject>();

    // 飛翔パラメータを設定してから Start() を呼ぶ
    rock->SetBossData(boss);
    rock->SetupThrow(startPos, direction,collisionSize, speed, launchAngle, gravity);
    rock->Start();

    objects_.push_back(std::move(rock));
}
