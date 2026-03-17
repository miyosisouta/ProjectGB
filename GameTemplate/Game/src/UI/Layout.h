/**
 * Layout.h
 * UIのレイアウト管理
 */
#pragma once
#include "Menu.h"
#include "src/json/json.hpp"


#ifdef _DEBUG
#define APP_ENABLE_LAYOUT_HOTRELOAD
#endif


class Layout : public Noncopyable
{
private:
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
    time_t lastUpdateTime_ = 0;
#endif // APP_ENABLE_LAYOUT_HOTRELOAD

    std::string filePath_ = "";
    std::unique_ptr<MenuBase> menu_ = nullptr;

public:
    Layout() {}
    ~Layout() {}

    template <typename T>
    void Initialize(const std::string& path)
    {
        filePath_ = path;
        menu_ = std::make_unique<T>();
        Reload();
    }

    MenuBase* GetMenu() const { return menu_.get(); }

    void Update();
    void Render(RenderContext& rc);

    void Reload();

    /** ファイルパスを取得（Swap の戻り先再構築用） */
    const std::string& GetFilePath() const { return filePath_; }

    /**
     * パスを指定して再ロードする（Swap の戻り先再構築用）
     * Initialize と異なり MenuBase の型を変えずにレイアウトだけ再読み込みする
     */
    void ReloadFromPath(const std::string& path)
    {
        filePath_ = path;
        Reload();
    }


private:
    static UIBase* CreateUI(UICanvas* canvas, const std::string& type, const uint32_t key, const nlohmann::json& item);
};