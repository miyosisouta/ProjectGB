#pragma once
/**
 * AttackObjectManager.h
 * 攻撃オブジェクトの管理クラス
 */
class AttackObjectBase;
class ThrowRockObject;
class BossCharacter;
class AttackObjectManager
{
private:
    std::vector<std::unique_ptr<AttackObjectBase>> objects_;


private:
    AttackObjectManager();
    ~AttackObjectManager();


public:
    void Update();
    void Render(RenderContext& rc);

public:
    /** 岩オブジェクトを生成する。戻り値はLaunch()を呼ぶタイミングを制御するために呼び出し側で保持する想定 */
    ThrowRockObject* CreateRock(
        BossCharacter* boss,
        const Vector3& startPos,
        const Vector3& direction,
        float collisionSize,
        float spawnHeight = 10.0f,
        float speed = 1200.0f,
        float launchAngle = 15.0f,
        float gravity = 980.0f
    );

    void CreateLandmine(
        Character* owner,
        const Vector3& startPos,
        float motionValue
    );

/*=========================================*/
/* ここから先シングルトン */
/*=========================================*/
private:
    /** 自身のインスタンス */
    static AttackObjectManager* instance_;


public:
    /** インスタンスを作る */
    static void CreateInstance()
    {
        if (!instance_) instance_ = new AttackObjectManager();
    }


    /** インスタンスを取得 */
    static AttackObjectManager& Get()
    {
        if(instance_) return *instance_;
    }


    /** インスタンスを破棄 */
    static void DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }
};