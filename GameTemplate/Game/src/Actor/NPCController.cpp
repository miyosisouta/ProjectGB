#include "stdafx.h"
#include <time.h>
#include "NPCController.h"
#include "src/Actor/Player.h"
#include "src/Actor/BossCharacter.h"


namespace
{
	// NPCAttackRuleParameter.jsonの"distance"文字列 → DistancePhase の変換（唯一の変換場所）
	DistancePhase ToDistancePhase(const std::string& distance)
	{
		if (distance == "Short")    { return DistancePhase::enShortAttackType; }
		if (distance == "Mid")      { return DistancePhase::enMidAttackType; }
		if (distance == "Long")     { return DistancePhase::enLongAttackType; }
		if (distance == "OutRange") { return DistancePhase::enOutRange; }

		K2_ASSERT(false, ("NPCAttackRuleParameter.json: 未知のdistanceです（distance='" + distance + "'）\n").c_str());
		return DistancePhase::enOutRange;
	}

	// NPCAttackRuleParameter.jsonの"stateId"文字列 → BossStateID の変換（唯一の変換場所）
	BossStateID ToBossStateId(const std::string& stateId)
	{
		static const std::unordered_map<std::string, BossStateID> table = {
			{ "Idle",      BossStateID::Idle },
			{ "Run",       BossStateID::Run },
			{ "HitStamp",  BossStateID::HitStamp },
			{ "Attack",    BossStateID::Attack },
			{ "Hit",       BossStateID::Hit },
			{ "Spin",      BossStateID::Spin },
			{ "ThrowRock", BossStateID::ThrowRock },
			{ "Laser",     BossStateID::Laser },
			{ "Death",     BossStateID::Death },
		};

		auto it = table.find(stateId);
		K2_ASSERT(it != table.end(), ("NPCAttackRuleParameter.json: 未知のstateIdです（stateId='" + stateId + "'）\n").c_str());
		return (it != table.end()) ? it->second : BossStateID::Idle;
	}
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
    BuildRulesFromParam();

	return true;
}

void NPCController::Update()
{
    // 処理をかえす処理
    {
        // 動作対象と攻撃対象がいない場合
        if (!boss_ || !targerPlayer_) return;
        // ボスが死んでいる場合
        if (boss_->GetCurrentStateID() == BossStateID::Death) return;
    }


    // ボスのHPが0なら死亡ステートへ移行
    if (boss_->GetStatus()->IsHpDepleted()) {
        boss_->ChangeState(BossStateID::Death);
    }

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

void NPCController::BuildRulesFromParam()
{
    // 距離帯ごとの攻撃リストを初期化
    for (int i = 0; i < static_cast<int>(DistancePhase::enMax); ++i)
    {
        currentRules_[i].distanceType_ = static_cast<DistancePhase>(i);
        currentRules_[i].attackList.clear();
    }

    const auto rules = ParameterManager::Get().GetNPCAttackRulesByCharacter(bossKey_);
    K2_ASSERT(!rules.empty(), ("NPCAttackRuleParameter.jsonにキー'" + bossKey_ + "'の攻撃ルールがありません\n").c_str());

    for (const auto* rule : rules)
    {
        const DistancePhase phase = ToDistancePhase(rule->distance);
        const BossStateID   stateId = ToBossStateId(rule->stateId);
        currentRules_[static_cast<int>(phase)].attackList.push_back({ stateId, rule->weight });
    }
}

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

    // 現在の距離フェーズに対応するルールを取得
    // enumの値をintにキャストして配列のインデックスとして使います
    const DistanceRule& currentRule = currentRules_[static_cast<int>(currentPhase)];

    // 重みの合計を求める（JSON側の値をそのまま使うため、合計値を固定値に決め打ちしない）
    int totalWeight = 0;
    for (const auto& pattern : currentRule.attackList) { totalWeight += pattern.weight; }

    // 決定した攻撃を入れる変数（初期値は安全のためIdleにしておく）
    BossStateID selectedAttack = BossStateID::Idle;

    if (totalWeight > 0)
    {
        // どの攻撃を使うか抽選
        int actionLottery = rand() % totalWeight;

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
    }

    // 抽選結果に基づくアクションの実行
    boss_->ChangeState(selectedAttack);
}

DistancePhase NPCController::ChackDistancePhase(float distance)
{
    const auto* p = ParameterManager::Get().GetNPCControllerParam();

    if (distance < p->shortDistance) { return DistancePhase::enShortAttackType; } // 近距離タイプ
    if (distance < p->midDistance)   { return DistancePhase::enMidAttackType; }   // 中距離タイプ
    if (distance < p->longDistance)  { return DistancePhase::enLongAttackType; }  // 遠距離タイプ
    return DistancePhase::enOutRange;                                              // 射程範囲外
}
