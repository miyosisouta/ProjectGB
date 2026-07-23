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

    // 現存するオブジェクトの更新
    for (auto& obj : objects_)
    {
        obj->Update();

        if (obj->IsFinished())
        {
            deleteList.push_back(obj.get());
        }
    }

    // 削除予定のオブジェクトオブジェクトを削除
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

ThrowRockObject* AttackObjectManager::CreateRock(BossCharacter* boss, const Vector3& startPos, const Vector3& direction,float collisionSize, float spawnHeight, float speed, float launchAngle, float gravity)
{
    // オブジェクトを作成
    auto rock = std::make_unique<ThrowRockObject>();

    // 必要なパラメータを設定
    rock->SetBossData(boss);
    rock->Setup(startPos, direction,collisionSize, speed, launchAngle, gravity, spawnHeight);
    rock->Start();

    ThrowRockObject* rockPtr = rock.get();

    // 所有権を移動
    objects_.push_back(std::move(rock));

    return rockPtr;
}

void AttackObjectManager::CreateLandmine(Character* owner, const Vector3& startPos, float motionValue)
{
    // オブジェクトを作成
    auto bomb = std::make_unique<LandmineObject>();

    // 必要なパラメータを設定
    bomb->SetOwnerData(owner);
    bomb->Setup(startPos,motionValue);
    bomb->Start();

    // 所有権を移動
    objects_.push_back(std::move(bomb));
}
