#pragma once

/**
 * DamageNotify.h
 * ダメージ通知の型定義と通知キューの管理
 */

 /** ダメージ通知の種別 */
enum class DamageNotifyType
{
    AttackHit,  //!< プレイヤーがボスに与えたダメージ
    TakeHit,    //!< プレイヤーが受けたダメージ
};

/** ダメージ通知1件分 */
struct DamageNotify
{
    int              damage = 0;                             //!< 確定したダメージ値
    DamageNotifyType type = DamageNotifyType::AttackHit;      //!< 与えた/受けたの種別
    bool             isCritical = false;                      //!< クリティカルかどうか
};

/**
 * ダメージ通知を管理するリングバッファ
 * BattleManager、TutorialManagerなど各シーンで使い回せる
 */
class DamageNotifyQueue
{
private:
    static constexpr int kMaxNotify = 16; //!< 通知の最大件数
    DamageNotify buffer_[kMaxNotify];     //!< 固定長の通知バッファ
    int head_ = 0;                 //!< 読み取り位置
    int tail_ = 0;                 //!< 書き込み位置
    int notifyCount_ = 0;                 //!< 現在の通知件数

public:
    /**
     * ダメージ通知を積む
     * 上限を超えた場合は捨てる（クラッシュしない）
     * @param damage      確定したダメージ値
     * @param type        AttackHit（与えた）か TakeHit（受けた）か
     * @param isCritical  クリティカルかどうか
     */
    void Push(int damage, DamageNotifyType type, bool isCritical)
    {
        if (notifyCount_ >= kMaxNotify) return;

        buffer_[tail_].damage = damage;
        buffer_[tail_].type = type;
        buffer_[tail_].isCritical = isCritical;

        tail_ = (tail_ + 1) % kMaxNotify;
        notifyCount_++;
    }

    /**
     * ダメージ通知を1件取得する
     * 取得と同時に自動的に削除される
     * @return 通知1件分の構造体（通知がなければ damage が -1）
     */
    DamageNotify Pop()
    {
        // 通知がなければ damage が -1 の構造体を返す
        if (notifyCount_ <= 0) return { -1, DamageNotifyType::AttackHit, false };

        DamageNotify notify = buffer_[head_];

        // 読み取り位置を1つ進める（末端に達したら先頭に戻る）
        head_ = (head_ + 1) % kMaxNotify;
        notifyCount_--;

        return notify;
    }
};