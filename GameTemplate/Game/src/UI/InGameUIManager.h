/**
 * InGameUIManager.h
 * 
 * インゲームのUIを管理
 */


#pragma once


class InGameUIManager
{
private:
    InGameUIManager();
    ~InGameUIManager();


public:
    void Update();


private:
    /** 自身のインスタンス */
    static InGameUIManager* myInstance_;


public:
    /** インスタンスを作る */
    static void CreateInstance()
    {
        if (!myInstance_) myInstance_ = new InGameUIManager();
    }


    /** インスタンスを取得 */
    static InGameUIManager& Get()
    {
        if(myInstance_) return *myInstance_;
    }


    /** インスタンスを破棄 */
    static void DestroyInstance()
    {
        if (myInstance_) {
            delete myInstance_;
            myInstance_ = nullptr;
        }
    }
};