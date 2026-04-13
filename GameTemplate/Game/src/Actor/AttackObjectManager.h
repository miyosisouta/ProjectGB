#pragma once
/**
 * AttackObjectManager.h
 * 攻撃オブジェクトの管理クラス
 */
class AttackObjectBase;
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
    void CreateRock(
        BossCharacter* boss,
        const Vector3& startPos,
        const Vector3& direction,
        float collisionSize,
        float speed = 1200.0f,
        float launchAngle = 15.0f,
        float gravity = 980.0f
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