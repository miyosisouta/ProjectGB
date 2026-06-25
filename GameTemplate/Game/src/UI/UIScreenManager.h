/**
 * UIScreenManager.h
 * UI画面遷移管理システム
 *
 * ============================================================
 * 【遷移モード一覧】
 *
 *   Push    : 旧画面を残したまま新画面を上に重ねる（戻れる）
 *   Replace : 旧画面を破棄して置き換える（戻れない）
 *   Swap    : 旧画面を破棄するが戻り先情報を残す（戻れる・メモリ節約）
 *
 * ============================================================
 * 【アニメーション完了の安全な通知方法】
 *
 *   UICanvasFadeAnimation 等の Update() の中で onComplete を直接呼ぶと、
 *   呼び出しスタック上に canvas->UpdateAnimation() があるため、
 *   onComplete の中でスタック操作（pop_back, erase 等）を行うと
 *   自己破壊する。
 *
 *   このクラスでは UIScreenAnimRunner という仕組みを使い、
 *   アニメーション完了を「フラグ」で記録して
 *   UIScreenManager::Update() の末尾フェーズで安全に後処理を行う。
 *
 * ============================================================
 * 【使用例】
 *
 *   UIScreenManager::Get().Boot<TitleMenu>("ui/title.json");
 *   UIScreenManager::Get().Boot<TitleMenu>("ui/title.json",
 *       UIScreenTransitionPreset::FadeInOut());
 *
 *   UIScreenManager::Get().Push<PauseMenu>("ui/pause.json",
 *       UITransitionMode::Push,
 *       UIScreenTransitionPreset::FadeInOut(),
 *       { .onResume = [this](){ ResumeBGM(); } });
 *
 *   UIScreenManager::Get().Push<GameOverMenu>("ui/gameover.json",
 *       UITransitionMode::Replace,
 *       UIScreenTransitionPreset::FadeInOut());
 *
 *   UIScreenManager::Get().Push<ShopMenu>("ui/shop.json",
 *       UITransitionMode::Swap,
 *       UIScreenTransitionPreset::SlideFromRight(),
 *       { .onResume = [this](){ RefreshAfterShop(); } });
 *
 *   UIScreenManager::Get().Pop();
 */
#pragma once
#include "Layout.h"
#include "UIAnimation.h"
#include "UIAnimationFactory.h"



// ============================================
// 遷移モード / 画面状態
// ============================================

enum class UITransitionMode { Push, Replace, Swap };

enum class UIScreenState
{
    EnterAnim,  // 入場アニメ中
    Active,     // 操作可能
    ExitAnim,   // 退場アニメ中
    Inactive,   // 背面（Push で隠れた状態）
    Dead,       // 破棄待ち
};


// ============================================
// コールバック
// ============================================

struct UIScreenCallbacks
{
    std::function<void()> onEnter;    // EnterAnim 開始直前
    std::function<void()> onActive;   // Active になった瞬間
    std::function<void()> onExit;     // ExitAnim 開始直前
    std::function<void()> onResume;   // 前の画面に戻ってきた瞬間
    std::function<void()> onDestroy;  // 破棄直前
};


// ============================================
// 遷移アニメーション定義
// ============================================

/**
 * 1方向分（入場 or 退場）のアニメーション定義
 *
 * buildFunc: canvas と一時キーを受け取り、
 *            アニメーション（unique_ptr）を返す関数。
 *            nullptr を返せばアニメーションなし（即完了）。
 *
 * animKeys:  buildFunc が nullptr のとき UIAnimationParameter の
 *            JSON キー列をシーケンスで使う（未実装は即完了）。
 */
struct UIScreenTransitionAnim
{
    // canvas と一時キーを受け取りアニメを生成して返す
    std::function<std::unique_ptr<UIAnimationBase>(UICanvas* canvas, uint32_t tmpKey)> buildFunc;

    // buildFunc が nullptr のとき使う JSON キー列
    std::vector<uint32_t> animKeys;
};

struct UIScreenTransitionDef
{
    UIScreenTransitionAnim enter;
    UIScreenTransitionAnim exit;
};


// ============================================
// アニメーション完了の安全な通知
// ============================================

/**
 * UIScreenAnimRunner
 *
 * canvas に一時アニメを登録して再生し、
 * 完了を isAnimDone_ フラグで記録する。
 *
 * 完了通知（onEnd_）は UIScreenManager::Update() の
 * 末尾フェーズ（FlushIfDone）で実行されるため、
 * canvas->UpdateAnimation() のスタック上では実行されない。
 */
struct UIScreenAnimRunner
{
    bool      isRunning_ = false;
    bool      isAnimDone_ = false;
    uint32_t  animKey_ = 0;
    std::function<void()> onEnd_;

    void Reset()
    {
        isRunning_ = false;
        isAnimDone_ = false;
        animKey_ = 0;
        onEnd_ = nullptr;
    }

    bool IsRunning() const { return isRunning_; }

    /**
     * アニメを canvas に登録して開始する
     * @param canvas  対象キャンバス（nullptr なら即 onEnd_ を予約）
     * @param anim    再生するアニメ（nullptr なら即完了）
     * @param key     canvas への一時登録キー
     * @param onEnd   完了時に呼ぶコールバック
     */
    void Start(UICanvas* canvas,
        std::unique_ptr<UIAnimationBase> anim,
        uint32_t key,
        std::function<void()> onEnd)
    {
        Reset();
        onEnd_ = std::move(onEnd);
        isRunning_ = true;

        if (!canvas || !anim)
        {
            // アニメなし → 即完了フラグ
            isAnimDone_ = true;
            return;
        }

        animKey_ = key;
        anim->Play();
        canvas->AddAnimation(key, std::move(anim));
    }

    /**
     * canvas->UpdateAnimation() より後に呼ぶ。
     * アニメが完了していれば isAnimDone_ を立てる。
     */
    void Tick(UICanvas* canvas)
    {
        if (!isRunning_ || isAnimDone_) return;
        if (!canvas) { isAnimDone_ = true; return; }

        auto* anim = canvas->FindAnimation(animKey_);
        if (!anim || !anim->IsPlay())
        {
            if (anim) canvas->RemoveAnimation(animKey_);
            isAnimDone_ = true;
        }
    }

    /**
     * Update の末尾フェーズで呼ぶ。
     * isAnimDone_ が立っていれば onEnd_ を実行する。
     */
    void FlushIfDone()
    {
        if (!isRunning_ || !isAnimDone_) return;
        auto cb = std::move(onEnd_);
        Reset(); // 先にリセットして再入を防ぐ
        if (cb) cb();
    }
};


// ============================================
// プリセット
// ============================================

struct UIScreenTransitionPreset
{
public:
    // フェードイン / アウト
    static UIScreenTransitionDef FadeInOut(float duration = 0.3f)
    {
        UIScreenTransitionDef def;

        def.enter.buildFunc = [duration](UICanvas* canvas, uint32_t) -> std::unique_ptr<UIAnimationBase>
            {
                if (!canvas) return nullptr;
                canvas->color.w = 0.0f;
                auto anim = std::make_unique<UICanvasFadeAnimation>();
                anim->SetParameter(0.0f, 1.0f, duration,
                    EasingType::EaseOut, LoopMode::Once);
                anim->SetUI(canvas);
                return anim;
            };

        def.exit.buildFunc = [duration](UICanvas* canvas, uint32_t) -> std::unique_ptr<UIAnimationBase>
            {
                if (!canvas) return nullptr;
                // 現在の alpha から 0 へフェード。
                // 呼び出し元が canvas.w を手動で 0 にしていた場合でも
                // 開始値を 1.0f に戻してフラッシュさせないようにする。
                const float fromAlpha = canvas->color.w;
                auto anim = std::make_unique<UICanvasFadeAnimation>();
                anim->SetParameter(fromAlpha, 0.0f, duration,
                    EasingType::EaseIn, LoopMode::Once);
                anim->SetUI(canvas);
                return anim;
            };

        return def;
    }

    // 右からスライドイン / 右へスライドアウト
    static UIScreenTransitionDef SlideFromRight(float duration = 0.35f, float offsetX = 1920.0f)
    {
        UIScreenTransitionDef def;

        def.enter.buildFunc = [duration, offsetX](UICanvas* canvas, uint32_t) -> std::unique_ptr<UIAnimationBase>
            {
                if (!canvas) return nullptr;
                const Vector3 origin = canvas->transform.localPosition;
                Vector3 start = origin; start.x += offsetX;
                auto anim = std::make_unique<UITranslateAnimation>();
                anim->SetParameter(start, origin, duration,
                    EasingType::EaseOut, LoopMode::Once);
                anim->SetUI(canvas);
                return anim;
            };

        def.exit.buildFunc = [duration, offsetX](UICanvas* canvas, uint32_t) -> std::unique_ptr<UIAnimationBase>
            {
                if (!canvas) return nullptr;
                const Vector3 origin = canvas->transform.localPosition;
                Vector3 end = origin; end.x += offsetX;
                auto anim = std::make_unique<UITranslateAnimation>();
                anim->SetParameter(origin, end, duration,
                    EasingType::EaseIn, LoopMode::Once);
                anim->SetUI(canvas);
                return anim;
            };

        return def;
    }

    // 左からスライドイン / 左へスライドアウト
    static UIScreenTransitionDef SlideFromLeft(float duration = 0.35f, float offsetX = 1920.0f)
    {
        UIScreenTransitionDef def;

        def.enter.buildFunc = [duration, offsetX](UICanvas* canvas, uint32_t) -> std::unique_ptr<UIAnimationBase>
            {
                if (!canvas) return nullptr;
                const Vector3 origin = canvas->transform.localPosition;
                Vector3 start = origin; start.x -= offsetX;
                auto anim = std::make_unique<UITranslateAnimation>();
                anim->SetParameter(start, origin, duration,
                    EasingType::EaseOut, LoopMode::Once);
                anim->SetUI(canvas);
                return anim;
            };

        def.exit.buildFunc = [duration, offsetX](UICanvas* canvas, uint32_t) -> std::unique_ptr<UIAnimationBase>
            {
                if (!canvas) return nullptr;
                const Vector3 origin = canvas->transform.localPosition;
                Vector3 end = origin; end.x -= offsetX;
                auto anim = std::make_unique<UITranslateAnimation>();
                anim->SetParameter(origin, end, duration,
                    EasingType::EaseIn, LoopMode::Once);
                anim->SetUI(canvas);
                return anim;
            };

        return def;
    }

    // スケールアップ / ダウン
    static UIScreenTransitionDef ScaleInOut(float duration = 0.25f)
    {
        UIScreenTransitionDef def;

        def.enter.buildFunc = [duration](UICanvas* canvas, uint32_t) -> std::unique_ptr<UIAnimationBase>
            {
                if (!canvas) return nullptr;
                auto anim = std::make_unique<UIScaleAnimation>();
                anim->SetParameter(
                    Vector3(0.0f, 0.0f, 1.0f), Vector3::One, duration,
                    EasingType::EaseOut, LoopMode::Once);
                anim->SetUI(canvas);
                return anim;
            };

        def.exit.buildFunc = [duration](UICanvas* canvas, uint32_t) -> std::unique_ptr<UIAnimationBase>
            {
                if (!canvas) return nullptr;
                auto anim = std::make_unique<UIScaleAnimation>();
                anim->SetParameter(
                    Vector3::One, Vector3(0.0f, 0.0f, 1.0f), duration,
                    EasingType::EaseIn, LoopMode::Once);
                anim->SetUI(canvas);
                return anim;
            };

        return def;
    }

    // アニメなし（即完了）
    static UIScreenTransitionDef None()
    {
        UIScreenTransitionDef def;
        // buildFunc が nullptr → UIScreenAnimRunner::Start で即完了扱い
        return def;
    }
};


// ============================================
// スクリーンエントリー
// ============================================

struct UIScreenEntry
{
    std::unique_ptr<Layout>  layout;
    UIScreenState            state = UIScreenState::EnterAnim;
    UITransitionMode         mode = UITransitionMode::Push;
    UIScreenCallbacks        callbacks;
    UIScreenTransitionDef    transition;
    bool                     updateWhenInactive = false;

    // 入退場アニメーションの完了監視
    UIScreenAnimRunner       animRunner;
};


// ============================================
// Swap 用の戻り先情報
// ============================================

struct UIScreenHistoryEntry
{
    std::function<UIScreenEntry()> restoreBuilder;
    UIScreenTransitionDef          transition;
    UIScreenCallbacks              callbacks;
};


// ============================================
// UIScreenManager
// ============================================

class UIScreenManager : public Noncopyable
{
private:
    std::vector<UIScreenEntry>        stack_;
    std::vector<UIScreenHistoryEntry> history_;
    bool isTransitioning_ = false;

    struct PendingEntry
    {
        std::function<UIScreenEntry()> builder;
        bool valid = false;
    };
    PendingEntry pending_;


    // ============================================
    // シングルトン
    // ============================================
public:
    static UIScreenManager& Get()
    {
        static UIScreenManager instance;
        return instance;
    }

private:
    UIScreenManager() {}
    ~UIScreenManager() {}


public:
    // ============================================
    // Boot（初期化専用）
    // ============================================

    /**
     * 最初の1画面を起動する。スタックが空のときだけ呼ぶこと。
     */
    template <typename TMenu>
    void Boot(
        const std::string& filePath,
        UIScreenTransitionDef transition = UIScreenTransitionPreset::None(),
        UIScreenCallbacks callbacks = {})
    {
        K2_ASSERT(stack_.empty(), "Boot はスタックが空のときだけ呼んでください\n");

        UIScreenEntry entry;
        entry.mode = UITransitionMode::Replace;
        entry.callbacks = std::move(callbacks);
        entry.transition = std::move(transition);
        entry.state = UIScreenState::EnterAnim;
        entry.layout = std::make_unique<Layout>();
        entry.layout->Initialize<TMenu>(filePath);

        if (entry.callbacks.onEnter) entry.callbacks.onEnter();

        stack_.push_back(std::move(entry));
        StartEnterAnim(stack_.back());
    }


    // ============================================
    // Push
    // ============================================

    /**
     * 新しい画面を積む。
     *
     * Push    : 旧画面を Inactive → 新画面を即構築 → EnterAnim → Active
     * Replace : 旧画面 ExitAnim → 破棄 → 新画面構築 → EnterAnim → Active
     * Swap    : 旧画面 ExitAnim → 破棄（history_ に保存）→ 新画面構築 → EnterAnim → Active
     */
    template <typename TMenu>
    void Push(
        const std::string& filePath,
        UITransitionMode mode,
        UIScreenTransitionDef transition,
        UIScreenCallbacks callbacks = {},
        bool updateWhenInactive = false)
    {
        if (isTransitioning_) return;
        isTransitioning_ = true;

        // 新画面の構築ファクトリを保留
        pending_.builder = [=]() mutable
            {
                UIScreenEntry entry;
                entry.mode = mode;
                entry.callbacks = callbacks;
                entry.transition = transition;
                entry.updateWhenInactive = updateWhenInactive;
                entry.state = UIScreenState::EnterAnim;
                entry.layout = std::make_unique<Layout>();
                entry.layout->Initialize<TMenu>(filePath);
                return entry;
            };
        pending_.valid = true;

        if (!stack_.empty())
        {
            auto& top = stack_.back();

            if (mode == UITransitionMode::Push)
            {
                if (top.callbacks.onExit) top.callbacks.onExit();
                top.state = UIScreenState::Inactive;
                BuildAndPushPending();
                return;
            }

            if (mode == UITransitionMode::Swap)
            {
                SaveHistory(top, transition, callbacks);
            }

            // Replace / Swap: 旧画面の退場アニメ完了後に新画面を積む
            StartExitAnim(top, [this]()
                {
                    if (!stack_.empty())
                    {
                        if (stack_.back().callbacks.onDestroy)
                            stack_.back().callbacks.onDestroy();
                        stack_.back().state = UIScreenState::Dead;
                    }
                    CleanupDead();
                    BuildAndPushPending();
                });
            return;
        }

        BuildAndPushPending();
    }


    // ============================================
    // Pop
    // ============================================

    void Pop()
    {
        if (isTransitioning_) return;
        if (stack_.empty())   return;
        isTransitioning_ = true;

        auto& top = stack_.back();
        if (top.callbacks.onExit) top.callbacks.onExit();

        const UITransitionMode currentMode = top.mode;

        StartExitAnim(top, [this, currentMode]()
            {
                if (!stack_.empty())
                {
                    if (stack_.back().callbacks.onDestroy)
                        stack_.back().callbacks.onDestroy();
                    stack_.pop_back();
                }

                if (currentMode == UITransitionMode::Swap)
                    RestoreFromHistory();
                else
                    ResumeInactiveTop();
            });
    }

    void AllPop()
    {
        if (isTransitioning_) return;
        if (stack_.empty())   return;
        isTransitioning_ = true;

        auto& top = stack_.back();
        if (top.callbacks.onExit) top.callbacks.onExit();

        const UITransitionMode currentMode = top.mode;

        StartExitAnim(top, [this, currentMode]()
            {
                while (!stack_.empty())
                {
                    if (stack_.back().callbacks.onDestroy)
                        stack_.back().callbacks.onDestroy();
                    stack_.pop_back();
                }
            });
    }


    // ============================================
    // Update / Render
    // ============================================

    void Update()
    {
        const float dt = g_gameTime->GetFrameDeltaTime();

        // ① 各エントリーの layout を更新（UpdateAnimation 含む）
        for (auto& entry : stack_)
        {
            if (entry.state == UIScreenState::Dead) continue;

            switch (entry.state)
            {
            case UIScreenState::EnterAnim:
            case UIScreenState::Active:
            case UIScreenState::ExitAnim:
                entry.layout->Update();
                break;
            case UIScreenState::Inactive:
                if (entry.updateWhenInactive)
                    entry.layout->Update();
                break;
            default:
                break;
            }
        }

        // ② layout->Update() の後でアニメ完了チェック
        //    （UpdateAnimation のスタック上では実行しない）
        for (auto& entry : stack_)
        {
            if (entry.state == UIScreenState::Dead) continue;
            if (!entry.animRunner.IsRunning())      continue;

            auto* canvas = GetCanvas(entry);
            entry.animRunner.Tick(canvas);
        }

        // ③ 完了した animRunner の onEnd_ をここで安全に実行
        //    （スタック操作はすべてここで行われる）
        //    FlushIfDone の中で stack_ が変化する可能性があるため、
        //    インデックスアクセスで処理し、変化したら終了する。
        const int stackSizeBefore = static_cast<int>(stack_.size());
        for (int i = 0; i < static_cast<int>(stack_.size()); ++i)
        {
            if (stack_[i].state == UIScreenState::Dead) continue;
            if (!stack_[i].animRunner.IsRunning()) continue;

            stack_[i].animRunner.FlushIfDone();

            // FlushIfDone で stack_ が変化した場合はここで終了
            // 次フレームで残りを処理する
            if (static_cast<int>(stack_.size()) != stackSizeBefore) break;
        }
    }

    void Render(RenderContext& rc)
    {
        for (auto& entry : stack_)
        {
            if (entry.state != UIScreenState::Dead)
                entry.layout->Render(rc);
        }
    }


    // ============================================
    // ユーティリティ
    // ============================================

    MenuBase* GetActiveMenu() const
    {
        for (int i = static_cast<int>(stack_.size()) - 1; i >= 0; --i)
        {
            if (stack_[i].state == UIScreenState::Active)
                return stack_[i].layout->GetMenu();
        }
        return nullptr;
    }

    template <typename TMenu>
    TMenu* FindMenu() const
    {
        for (auto& entry : stack_)
        {
            if (auto* m = dynamic_cast<TMenu*>(entry.layout->GetMenu()))
                return m;
        }
        return nullptr;
    }

    bool IsEmpty()         const { return stack_.empty(); }
    bool IsTransitioning() const { return isTransitioning_; }
    int  GetStackSize()    const { return static_cast<int>(stack_.size()); }

    void SetUpdateWhenInactive(int stackIndex, bool enable)
    {
        if (stackIndex < 0 || stackIndex >= GetStackSize()) return;
        stack_[stackIndex].updateWhenInactive = enable;
    }

    /** Inactive 状態のエントリーをスタックから即座に除去する（遷移中は呼ばないこと） */
    void ClearInactive()
    {
        stack_.erase(
            std::remove_if(stack_.begin(), stack_.end(),
                [](const UIScreenEntry& e) { return e.state == UIScreenState::Inactive; }),
            stack_.end());
    }


private:
    // ============================================
    // 内部: アニメーション開始
    // ============================================

    UICanvas* GetCanvas(UIScreenEntry& entry)
    {
        if (!entry.layout) return nullptr;
        auto* menu = entry.layout->GetMenu();
        return menu ? menu->GetCanvas() : nullptr;
    }

    void StartEnterAnim(UIScreenEntry& entry)
    {
        auto* canvas = GetCanvas(entry);
        const auto& animDef = entry.transition.enter;
        const uint32_t tmpKey = Hash32("__Enter__");

        std::unique_ptr<UIAnimationBase> anim;
        if (animDef.buildFunc)
            anim = animDef.buildFunc(canvas, tmpKey);

        entry.animRunner.Start(canvas, std::move(anim), tmpKey, [this]()
            {
                if (stack_.empty()) return;
                auto& top = stack_.back();
                top.state = UIScreenState::Active;
                if (top.callbacks.onActive) top.callbacks.onActive();
                isTransitioning_ = false;
            });
    }

    void StartExitAnim(UIScreenEntry& entry, std::function<void()> onComplete)
    {
        entry.state = UIScreenState::ExitAnim;
        auto* canvas = GetCanvas(entry);
        const auto& animDef = entry.transition.exit;
        const uint32_t tmpKey = Hash32("__Exit__");

        std::unique_ptr<UIAnimationBase> anim;
        if (animDef.buildFunc)
            anim = animDef.buildFunc(canvas, tmpKey);

        entry.animRunner.Start(canvas, std::move(anim), tmpKey, std::move(onComplete));
    }


    // ============================================
    // 内部: 保留画面の構築
    // ============================================

    void BuildAndPushPending()
    {
        if (!pending_.valid) return;

        UIScreenEntry entry = pending_.builder();
        pending_.valid = false;
        pending_.builder = nullptr;

        if (entry.callbacks.onEnter) entry.callbacks.onEnter();

        stack_.push_back(std::move(entry));
        StartEnterAnim(stack_.back());
    }


    // ============================================
    // 内部: Swap の戻り先保存
    // ============================================

    void SaveHistory(
        const UIScreenEntry& top,
        const UIScreenTransitionDef& transition,
        const UIScreenCallbacks& newCallbacks)
    {
        const std::string   filePath = top.layout ? top.layout->GetFilePath() : "";
        const UIScreenTransitionDef prevTrans = top.transition;
        const UIScreenCallbacks     prevCbs = top.callbacks;
        const bool                  prevUpdate = top.updateWhenInactive;
        const UITransitionMode      prevMode = top.mode;

        UIScreenHistoryEntry hist;
        hist.restoreBuilder = [filePath, prevTrans, prevCbs,
            prevUpdate, prevMode, newCallbacks]()
            {
                UIScreenEntry entry;
                entry.mode = prevMode;
                entry.transition = prevTrans;
                entry.updateWhenInactive = prevUpdate;
                entry.state = UIScreenState::EnterAnim;
                entry.layout = std::make_unique<Layout>();
                entry.layout->ReloadFromPath(filePath);
                entry.callbacks = prevCbs;
                entry.callbacks.onResume = newCallbacks.onResume;
                return entry;
            };
        hist.transition = transition;
        hist.callbacks = newCallbacks;
        history_.push_back(std::move(hist));
    }


    // ============================================
    // 内部: Pop 後の復帰
    // ============================================

    void RestoreFromHistory()
    {
        if (history_.empty())
        {
            isTransitioning_ = false;
            return;
        }

        UIScreenHistoryEntry hist = std::move(history_.back());
        history_.pop_back();

        UIScreenEntry entry = hist.restoreBuilder();
        entry.transition.enter = hist.transition.enter;

        if (entry.callbacks.onEnter) entry.callbacks.onEnter();

        stack_.push_back(std::move(entry));

        // Swap 復帰専用の EnterAnim 開始
        // 完了後に onResume → onActive の順で呼ぶ
        auto* canvas = GetCanvas(stack_.back());
        const auto& animDef = stack_.back().transition.enter;
        const uint32_t tmpKey = Hash32("__Enter__");

        std::unique_ptr<UIAnimationBase> anim;
        if (animDef.buildFunc)
            anim = animDef.buildFunc(canvas, tmpKey);

        stack_.back().animRunner.Start(canvas, std::move(anim), tmpKey, [this]()
            {
                if (stack_.empty()) return;
                auto& top = stack_.back();
                top.state = UIScreenState::Active;
                if (top.callbacks.onResume) top.callbacks.onResume();
                if (top.callbacks.onActive) top.callbacks.onActive();
                isTransitioning_ = false;
            });
    }

    void ResumeInactiveTop()
    {
        if (!stack_.empty())
        {
            auto& prev = stack_.back();
            prev.state = UIScreenState::Active;
            if (prev.callbacks.onResume) prev.callbacks.onResume();
        }
        isTransitioning_ = false;
    }


    // ============================================
    // 内部: Dead エントリー削除
    // ============================================

    void CleanupDead()
    {
        // animRunner を先にリセット（canvas への参照を持つ可能性があるため）
        for (auto& e : stack_)
        {
            if (e.state == UIScreenState::Dead)
                e.animRunner.Reset();
        }

        stack_.erase(
            std::remove_if(stack_.begin(), stack_.end(),
                [](const UIScreenEntry& e) { return e.state == UIScreenState::Dead; }),
            stack_.end());
    }
};