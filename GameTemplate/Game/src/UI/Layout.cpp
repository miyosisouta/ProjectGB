/**
 * Layout.cpp
 * UIのレイアウト管理
 */
#include "stdafx.h"
#include "Layout.h"
#include <fstream>
#include <sys/stat.h>
#include <Windows.h>



namespace
{
    // UTF-8 (JSONの文字列) を Shift-JIS (Windowsアプリ用) に変換する
    std::wstring Utf8ToShiftJis(const std::string& utf8Str)
    {
        if (utf8Str.empty()) return std::wstring();

        // 1. UTF-8 を Unicode (UTF-16) に変換
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), &wstrTo[0], size_needed);

        return wstrTo;
    }


    /**
     * ==========================================
     * パース関連
     * ==========================================
     */
    Vector2 ParseVector2(const nlohmann::json& arr)
    {
        return Vector2(
            arr[0].get<float>(),
            arr[1].get<float>()
        );
    }


    Vector3 ParseVector3(const nlohmann::json& arr)
    {
        return Vector3(
            arr[0].get<float>(),
            arr[1].get<float>(),
            arr[2].get<float>()
        );
    }


    Vector4 ParseVector4(const nlohmann::json& arr)
    {
        return Vector4(
            arr[0].get<float>(),
            arr[1].get<float>(),
            arr[2].get<float>(),
            arr[3].get<float>()
        );
    }


    Vector4 ParseColor(const nlohmann::json& arr)
    {
        return Vector4(
            arr[0].get<float>() / 255.0f,
            arr[1].get<float>() / 255.0f,
            arr[2].get<float>() / 255.0f,
            arr[3].get<float>() / 255.0f
        );
    }


    Quaternion ParseRotation(const float rotation)
    {
        Quaternion q;
        q.SetRotationDegZ(rotation);
        return q;
    }


    /**
     * ==========================================
     * 初期化関連
     * ==========================================
     */
    template <typename T>
    void InitializeUIParts(T* parts, const nlohmann::json& item)
    {
        K2_ASSERT(false, "未実装\n");
    }

    void InitializeUIParts(UIIcon* image, const nlohmann::json& item)
    {
        const std::string assetName = item["asset"].get<std::string>();
        const float w = item["width"].get<float>();
        const float h = item["height"].get<float>();
        const Vector3 position = ParseVector3(item["position"]);
        const Vector3 scale = ParseVector3(item["scale"]);
        const Quaternion rotation = ParseRotation(item["rotation"].get<float>());
        const Vector4 color = ParseVector4(item["color"]);

        image->Initialize(assetName.c_str(), w, h);
        image->transform.localPosition = position;
        image->transform.localScale = scale;
        image->transform.localRotation = rotation;
        image->color = color;
        if (item.contains("isDraw")) {
            image->isDraw = item["isDraw"].get<bool>();
        }
        if (item.contains("pivot")) {
            image->SetPivot(ParseVector2(item["pivot"]));
        }
    }
    void InitializeUIParts(UIText* text, const nlohmann::json& item)
    {
        const Vector3 position = ParseVector3(item["position"]);
        const Vector3 scale = ParseVector3(item["scale"]);
        const Vector4 color = ParseVector4(item["color"]);
        const auto str = item["text"].get<std::string>();
        const auto wstr = Utf8ToShiftJis(str);

        text->SetText(wstr.c_str());
        text->transform.localPosition = position;
        text->transform.localScale = scale;
        text->color = color;
        if (item.contains("isDraw")) {
            text->isDraw = item["isDraw"].get<bool>();
        }
        if (item.contains("pivot")) {
            text->SetPivot(ParseVector2(item["pivot"]));
        }
    }
    void InitializeUIParts(UIDigit* text, const nlohmann::json& item)
    {
        const std::string assetName = item["asset"].get<std::string>();
        const int digitCount = item["digit"].get<int>();
        const float w = item["width"].get<float>();
        const float h = item["height"].get<float>();
        const Vector3 position = ParseVector3(item["position"]);
        const Vector3 scale = ParseVector3(item["scale"]);
        const Vector4 color = ParseVector4(item["color"]);
        const Quaternion rotation = ParseRotation(item["rotation"].get<float>());

        // 初期値の数値は0としておく
        text->Initialize(assetName.c_str(), digitCount, 0, w, h, position, scale, rotation);
        text->color = color;
    }
    void InitializeUIParts(UIDummy* dummy, const nlohmann::json& item)
    {
        // オプション（ダミーだが、情報として持てる）
        if (item.contains("position")) {
            const Vector3 position = ParseVector3(item["position"]);
            dummy->transform.localPosition = position;
        }
        if (item.contains("scale")) {
            const Vector3 scale = ParseVector3(item["scale"]);
            dummy->transform.localScale = scale;
        }
        if (item.contains("rotation")) {
            const Quaternion rotation = ParseRotation(item["rotation"].get<float>());
            dummy->transform.localRotation = rotation;
        }
        if (item.contains("color")) {
            dummy->color = ParseVector4(item["color"]);
        }
    }


    /**
     * スコープ付きキーを生成する
     * prefix が空なら Hash32(name) をそのまま返す
     * prefix があれば Hash32("prefix/name") を返す
     */
    uint32_t MakeScopedKey(const std::string& prefix, const std::string& name)
    {
        if (prefix.empty()) {
            return Hash32(name.c_str());
        }
        std::string scoped = prefix + "/" + name;
        return Hash32(scoped.c_str());
    }
}


void Layout::Update()
{
    menu_->Update();
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
    // ホットリロードチェック
    struct stat st;
    if (stat(filePath_.c_str(), &st) == 0) {
        if (lastUpdateTime_ != st.st_mtime) {
            lastUpdateTime_ = st.st_mtime;
            Reload();
        }
    }
#endif // APP_ENABLE_LAYOUT_HOTRELOAD
}


#ifdef K2_DEBUG
bool Layout::s_isDrawPaused = false;
#endif


void Layout::Render(RenderContext& rc)
{
#ifdef K2_DEBUG
    if (s_isDrawPaused) { return; }
#endif
    menu_->Render(rc);
}


void Layout::Reload()
{
    // Canvas は必ず生成する。
    // ファイルが開けなくても canvas が nullptr のままになると
    // MenuBase::Update でクラッシュするため、先に生成しておく。
    menu_->Clear();
    menu_->SetCanvas(new UICanvas());

    std::ifstream file(filePath_);
    if (!file.is_open())
    {
        K2_LOG("Layout::Reload: file not found [%s]", filePath_.c_str());
        return;
    }

    nlohmann::json j;
    file >> j;

    auto* canvas = menu_->GetCanvas();
    auto& elements = j["canvas"]["elements"];

    // ルート直下は prefix 空
    const std::string emptyPrefix;

    for (auto& item : elements) {
        std::string type = item["type"];
        std::string name = item["name"];

        // ルート直下なので prefix なしのキー
        const uint32_t key = Hash32(name.c_str());
        if (menu_->HasUI(key)) {
            menu_->UnregisterUI(key);
            canvas->RemoveUI(key);
        }
        auto* ui = CreateUI(canvas, type, key, item, emptyPrefix);
        if (ui == nullptr) {
            K2_LOG("Layout::Reload: unknown UI type [%s] name=[%s]\n", type.c_str(), name.c_str());
            continue;
        }
        menu_->RegisterUI(key, ui);
    }

    menu_->InitializeLogic();
}


UIBase* Layout::CreateUI(UICanvas* canvas, const std::string& type, const uint32_t key,
    const nlohmann::json& item, const std::string& prefix)
{
    if (type == "UIIcon") {
        canvas->CreateUI<UIIcon>(key);
        auto* image = canvas->FindUI<UIIcon>(key);
        InitializeUIParts(image, item);
        return image;
    }
    if (type == "UIText") {
        canvas->CreateUI<UIText>(key);
        auto text = canvas->FindUI<UIText>(key);
        InitializeUIParts(text, item);
        return text;
    }
    if (type == "UIDigit") {
        canvas->CreateUI<UIDigit>(key);
        auto* digit = canvas->FindUI<UIDigit>(key);
        InitializeUIParts(digit, item);
        return digit;
    }
    if (type == "UIDummy") {
        canvas->CreateUI<UIDummy>(key);
        auto* dummy = canvas->FindUI<UIDummy>(key);
        InitializeUIParts(dummy, item);
        return dummy;
    }
    // -------------------------------------------------------
    // プレハブ: 別JSONで定義されたUIの塊を子Canvasとして埋め込む
    // -------------------------------------------------------
    if (type == "UIPrefab") {
        return LoadPrefab(canvas, key, item, prefix);
    }
    //if (type == "UIButton") return canvas->CreateUI<UIButton>(key);
    //if (type == "UIGauge")  return canvas->CreateUI<UIGauge>(key);
    return nullptr;
}


UIBase* Layout::LoadPrefab(UICanvas* parentCanvas, const uint32_t key,
    const nlohmann::json& item,
    const std::string& prefix, int depth)
{
    // 無限再帰ガード
    if (depth >= kMaxPrefabDepth) {
        K2_LOG("Layout::LoadPrefab: max depth reached (%d)\n", kMaxPrefabDepth);
        return nullptr;
    }

    // 子JSONのパスを取得
    if (!item.contains("source")) {
        K2_LOG("Layout::LoadPrefab: 'source' field missing\n");
        return nullptr;
    }
    const std::string sourcePath = item["source"].get<std::string>();

    // このプレハブの name を取得してスコープを構築
    // 親JSON側の "name" が "Player1Status" なら childPrefix = "Player1Status"
    // ネストしている場合は "Player1Status/InnerGroup" のように連結される
    const std::string prefabName = item.value("name", "");
    std::string childPrefix;
    if (prefix.empty()) {
        childPrefix = prefabName;
    }
    else {
        childPrefix = prefix + "/" + prefabName;
    }

    // 子JSONを読み込む
    std::ifstream file(sourcePath);
    if (!file.is_open()) {
        K2_LOG("Layout::LoadPrefab: file not found [%s]\n", sourcePath.c_str());
        return nullptr;
    }

    nlohmann::json childJson;
    file >> childJson;

    if (!childJson.contains("canvas") || !childJson["canvas"].contains("elements")) {
        K2_LOG("Layout::LoadPrefab: invalid format [%s]\n", sourcePath.c_str());
        return nullptr;
    }

    // 子Canvas を親Canvas の子UIとして生成
    parentCanvas->CreateUI<UICanvas>(key);
    auto* childCanvas = parentCanvas->FindUI<UICanvas>(key);
    if (!childCanvas) return nullptr;

    // 親JSON側の overrides を子Canvasに適用
    if (item.contains("position")) {
        childCanvas->transform.localPosition = ParseVector3(item["position"]);
    }
    if (item.contains("scale")) {
        childCanvas->transform.localScale = ParseVector3(item["scale"]);
    }
    if (item.contains("color")) {
        childCanvas->color = ParseVector4(item["color"]);
    }

    // 子JSONの elements を走査して、子Canvas上にUIを生成
    auto& elements = childJson["canvas"]["elements"];
    for (auto& childItem : elements) {
        std::string childType = childItem["type"];
        std::string childName = childItem["name"];

        // スコープ付きキー: "Player1Status/HpIcon" → Hash32
        const uint32_t childKey = MakeScopedKey(childPrefix, childName);

        UIBase* childUI = nullptr;

        if (childType == "UIPrefab") {
            // プレハブの中にプレハブ（再帰）
            // childPrefix をさらに引き継ぐ
            childUI = LoadPrefab(childCanvas, childKey, childItem, childPrefix, depth + 1);
        }
        else {
            childUI = CreateUI(childCanvas, childType, childKey, childItem, childPrefix);
        }

        if (childUI == nullptr) {
            K2_LOG("Layout::LoadPrefab: unknown type [%s] in [%s]\n",
                childType.c_str(), sourcePath.c_str());
            continue;
        }

        // 子UIも menu_ の uiMap_ にフラットに登録 → GetUI で取れる
        menu_->RegisterUI(childKey, childUI);
    }

    return childCanvas;
}