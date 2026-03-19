/**
 * Layout.h
 * UIのレイアウト管理
 */
#pragma once
#include "Menu.h"


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


    /**
     * コード側からプレハブ内のUIを取得するためのキー生成ヘルパー
     *
     * 使用例:
     *   auto* hp = menu->GetUI<UIIcon>(Layout::MakeScopedKey("Player1Status", "HpIcon"));
     *   // ルート直下のUIは今まで通り Hash32("Background") でOK
     */
    static uint32_t MakeScopedKey(const std::string& prefabName, const std::string& childName)
    {
        std::string scoped = prefabName + "/" + childName;
        return Hash32(scoped.c_str());
    }


private:
    /**
     * UIを1つ生成して canvas に追加する
     * @param prefix プレハブのスコープ名（空ならルート直下）
     */
    UIBase* CreateUI(UICanvas* canvas, const std::string& type,
        const uint32_t key, const nlohmann::json& item,
        const std::string& prefix);

    /**
     * プレハブ(別JSON)を子Canvasとして読み込み、親Canvasに埋め込む
     * @param prefix 親から引き継ぐスコープ名（ネスト時に連結される）
     * @param depth 再帰の深さ（無限ループ防止用）
     */
    UIBase* LoadPrefab(UICanvas* parentCanvas, const uint32_t key,
        const nlohmann::json& item,
        const std::string& prefix, int depth = 0);

    /** プレハブの最大ネスト深度 */
    static const int kMaxPrefabDepth = 8;
};