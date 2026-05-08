#include "stdafx.h"
#include <time.h>
#include "NPCController.h"
#include "src/Actor/Player.h"
#include "src/Actor/BossCharacter.h"


// ボスの攻撃ルール
namespace bossRuleData
{
    // ゴリラのルールセットを返す関数（std::array版）
    inline std::array<DistanceRule, static_cast<int>(DistancePhase::enMax)> GetGorillaRules()
    {
        // std::array を返すため、全体を {{ }} で囲みます
        return { 
            {
                // ▼ 近距離ルールの設定
                { DistancePhase::enShortAttackType, {
                    { BossStateID::Attack, 5 }, // 通常攻撃
                    { BossStateID::Spin,   3 }, // 回転攻撃
                    { BossStateID::Jump,   2 }  // ヒットスタンプ
                }},

                // ▼ 中距離ルールの設定
                { DistancePhase::enMidAttackType, {
                    { BossStateID::Run,     1 },    // 走る
                    { BossStateID::Jump,    3 },    // ヒットスタンプ
                    { BossStateID::Spin,    3 },    // 回転攻撃
                    { BossStateID::Clicked_Gollira, 3 }     // 岩を投げる
                }},

                // ▼ 遠距離ルールの設定
                { DistancePhase::enLongAttackType, {
                    { BossStateID::Run,     1 },    // 走る
                    { BossStateID::Jump,    2 },    // ジャンプ
                    { BossStateID::Spin,    2 },    // 回転攻撃
                    { BossStateID::Clicked_Gollira, 5 }     // 岩を投げる
                }},

                // ▼ 攻撃範囲外にいる場合
                { DistancePhase::enOutRange, {
                    { BossStateID::Run, 10 }    // 追跡
                    // 攻撃範囲外なら1つでもOK
                }}
            } 
        };
    }

    // ここから下に新しいボスのルールを書く
    // カメのルールセットを返す関数（std::array版）
    inline std::array<DistanceRule, static_cast<int>(DistancePhase::enMax)> GetTurtleRules()
    {
        // std::array を返すため、全体を {{ }} で囲みます
        return {
            {
                // ▼ 近距離ルールの設定
                { DistancePhase::enShortAttackType, {
                    { BossStateID::Attack, 3 }, // 通常攻撃
                    { BossStateID::Spin,   2 }, // 回転攻撃
                    { BossStateID::Jump,   2 },  // ヒットスタンプ
                    { BossStateID::Clicked_Turtle, 3 }     // レーザー
                }},

            // ▼ 中距離ルールの設定
            { DistancePhase::enMidAttackType, {
                { BossStateID::Run,     1 },    // 走る
                { BossStateID::Jump,    2 },    // ヒットスタンプ
                { BossStateID::Spin,    2 },    // 回転攻撃
                { BossStateID::Clicked_Turtle, 5 }     // レーザー
            }},

            // ▼ 遠距離ルールの設定
            { DistancePhase::enLongAttackType, {
                { BossStateID::Jump,    3 },    // ジャンプ
                { BossStateID::Spin,    2 },    // 回転攻撃
                { BossStateID::Clicked_Turtle, 5 }     // レーザー
            }},

            // ▼ 攻撃範囲外にいる場合
            { DistancePhase::enOutRange, {
                { BossStateID::Run, 10 }    // 追跡
                // 攻撃範囲外なら1つでもOK
            }}
        }
        };
    }
}

namespace
{
    constexpr int ATTACK_LOTTERY_MAX = 10;  //!< 抽選合計確率

    /** 距離ごとの定数 */
    constexpr float SHORT_DISTANCE = 500.0f;        //!< 近距離
    constexpr float MID_DISTANCE = 1000.0f;          //!< 中距離
    constexpr float LONG_DISTANCE = 1500.0f;        //!< 遠距離
    
}


/* ========================================== */
/* スタート処理と更新処理 */
/* ========================================== */

NPCController::NPCController()
{
}

NPCController::~NPCController()
{
}

bool NPCController::Start()
{
    // 乱数をランダムで生成
    srand(time(nullptr));


    // スポーナーから渡されたボスの種類によって、AIのルール（確率や行動パターン）を切り替える
    {
        switch (bossType_)
        {
        case BossType::enGorilla:
            currentRules_ = bossRuleData::GetGorillaRules(); // ゴリラ用の設定を取得
            break;

        case BossType::enTurtle:
            currentRules_ = bossRuleData::GetTurtleRules(); // カメ用の設定を取得
            break;

        default:
            K2_ASSERT(true, "ボスの種類が設定されていません");
            break;
        }
    }


	return true;
}

void NPCController::Update()
{
    // 動作対象と攻撃対象がいない場合は処理を返す
    if (!boss_ || !targerPlayer_) return;

    // プレイヤーの座標をボス本体に設定
    boss_->SetTargetPos(targerPlayer_->GetTransformPosition());

    // 行動ノードを選択
    if (boss_->IsCurrentStateFinished()) 
    {
        // 終わったのがIdle以外なら、強制的にIdleを挟む
        if (boss_->GetCurrentStateID() != BossStateID::Idle)
        {
            boss_->ChangeState(BossStateID::Idle);
        }
        // 今終わったのがIdleなら、次の行動を決める！
        else
        {
            SelectActionNode();
        }
    }

}


/* ========================================== */
/* 選択処理 */
/* ========================================== */

void NPCController::SelectActionNode()
{
    // 自分と攻撃の対象の座標を取得
    Vector3 bossPos = boss_->GetTransformPosition();
    Vector3 targetPos = targerPlayer_->GetTransformPosition();

    // プレイヤーがいる方向と距離を計算
    Vector3 diff = bossPos - targetPos;
    float distance = diff.Length();

    // 現在どの距離にいるかを取得
    DistancePhase currentPhase = ChackDistancePhase(distance);


    // どの攻撃を使うか抽選
    int actionLottery = rand() % ATTACK_LOTTERY_MAX;


    // 現在の距離フェーズに対応するルールを取得
    // enumの値をintにキャストして配列のインデックスとして使います
    const DistanceRule& currentRule = currentRules_[static_cast<int>(currentPhase)];

    // 決定した攻撃を入れる変数（初期値は安全のためIdleにしておく）
    BossStateID selectedAttack = BossStateID::Idle;

    // ここから抽選処理
    int currentWeightSum = 0; // 重みの合計値

    for (const auto& pattern : currentRule.attackList)
    {
        currentWeightSum += pattern.weight;

        // 乱数が現在の重みの合計値を下回ったら、その攻撃に決定！
        if (actionLottery < currentWeightSum)
        {
            selectedAttack = pattern.attackID;
            break; // 攻撃が決まったのでループを抜ける
        }
    }

    // 抽選結果に基づくアクションの実行
    boss_->ChangeState(selectedAttack);
}

DistancePhase NPCController::ChackDistancePhase(float distance)
{
    if (distance < SHORT_DISTANCE) 
    { 
        return DistancePhase::enShortAttackType; 
    } // 近距離タイプ
    if (distance < MID_DISTANCE)
    {
        return DistancePhase::enMidAttackType;
    }
    // 中距離タイプ
    if (distance < LONG_DISTANCE) { 
        return DistancePhase::enLongAttackType; 
    }   // 遠距離タイプ
    if (distance > LONG_DISTANCE)
    { 
        return DistancePhase::enOutRange; 
    }    // 射程範囲外

    return DistancePhase::enLongAttackType;
}
