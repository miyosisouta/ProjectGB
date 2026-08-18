#include "stdafx.h"
#include <time.h>
#include "NPCController.h"
#include "src/Actor/Player.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/ActorStatus.h"


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

    // ルールを元にビヘイビアツリーを組み立てる
    staggerNode_ = BuildStaggerNode();
    tree_ = BuildNormalBehaviorTree();

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
    if (boss_->GetStatus()->IsHpDepleted())
    {
        boss_->ChangeState(BossStateID::Death);
        return;
    }

    // プレイヤーの座標をボス本体に設定
    boss_->SetTargetPos(targerPlayer_->GetTransformPosition());

    // 怯み割り込みは通常行動ツリーとは独立に、毎フレーム最優先でチェックする
    // （通常行動ツリー内のSelector/Sequenceは「一度選んだ行動は最後までやり切る」メモリを持つため、
    //   ツリーの内側に混ぜてしまうと攻撃モーション中に割り込めなくなる。ここで外側から独立に見ることで解決する）
    if (staggerNode_ && staggerNode_->Tick() != BTStatus::Failure) { return; }

    // 怯み中でなければ、通常行動のビヘイビアツリーを評価する
    if (tree_) { tree_->Tick(); }
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

        AttackPattern pattern;
        pattern.attackID = stateId;
        pattern.weight = rule->weight;
        pattern.allowRepeat = rule->allowRepeat;
        pattern.laserModeWeightNormal = rule->laserModeWeightNormal;
        pattern.laserModeWeightMult   = rule->laserModeWeightMult;
        pattern.laserModeWeightCharge = rule->laserModeWeightCharge;

        currentRules_[static_cast<int>(phase)].attackList.push_back(pattern);
    }
}

DistancePhase NPCController::ChackDistancePhase(float distance) const
{
    const auto* p = ParameterManager::Get().GetNPCControllerParam();

    if (distance < p->shortDistance) { return DistancePhase::enShortAttackType; } // 近距離タイプ
    if (distance < p->midDistance)   { return DistancePhase::enMidAttackType; }   // 中距離タイプ
    if (distance < p->longDistance)  { return DistancePhase::enLongAttackType; }  // 遠距離タイプ
    return DistancePhase::enOutRange;                                              // 射程範囲外
}

DistancePhase NPCController::GetCurrentDistancePhase() const
{
    Vector3 diff = boss_->GetTransformPosition() - targerPlayer_->GetTransformPosition();
    return ChackDistancePhase(diff.Length());
}


/* ========================================== */
/* ビヘイビアツリー構築 */
/* ========================================== */

BTNodePtr NPCController::MakeStateAction(BossStateID id)
{
    // onEnter: このステートへ切り替える（既に同じステートなら何もしないのはChangeState側で保証済み）
    // isRunning: 自分が切り替えたステートのままかつ、まだ終わっていない間はRunning
    return std::make_unique<BTAction>(
        [this, id]() { boss_->ChangeState(id); },
        [this, id]() { return boss_->GetCurrentStateID() == id && !boss_->IsCurrentStateFinished(); }
    );
}

BTNodePtr NPCController::MakeAttackChoiceAction(const AttackPattern& pattern)
{
    const BossStateID id = pattern.attackID;

    return std::make_unique<BTAction>(
        [this, pattern]()
        {
            // レーザーが選ばれた場合、モード抽選用の重みを先に渡しておく（LaserState::Enter()が読む）
            if (pattern.attackID == BossStateID::Laser)
            {
                boss_->SetPendingLaserModeWeights(pattern.laserModeWeightNormal, pattern.laserModeWeightMult, pattern.laserModeWeightCharge);
            }

            boss_->ChangeState(pattern.attackID);

            // 連続回避の判定用に、今選んだ攻撃を記録する
            lastTopLevelAction_ = pattern.attackID;
        },
        [this, id]() { return boss_->GetCurrentStateID() == id && !boss_->IsCurrentStateFinished(); }
    );
}

BTNodePtr NPCController::BuildWeightedAttackNode(const std::vector<AttackPattern>& patterns)
{
    auto weighted = std::make_unique<BTWeightedSelector>();
    for (const auto& pattern : patterns)
    {
        weighted->AddEntry(
            MakeAttackChoiceAction(pattern),
            [this, pattern]()
            {
                // 連続を許さない攻撃かつ、直前に選んだのが自分自身なら今回は選ばれないようにする
                if (!pattern.allowRepeat && lastTopLevelAction_ == pattern.attackID) { return 0; }
                return pattern.weight;
            }
        );
    }
    return weighted;
}

BTNodePtr NPCController::BuildStaggerNode()
{
    // Condition: 怯み要求あり（かつ死亡中でない） → Action: Hitステートへ切り替えて再生し終わるまで待つ
    auto staggerSequence = std::make_unique<BTSequence>();

    staggerSequence->AddChild(std::make_unique<BTCondition>(
        [this]()
        {
            if (boss_->GetCurrentStateID() == BossStateID::Death) { return false; }

            auto* bossStatus = boss_->GetStatus()->As<BossStatus>();
            if (!bossStatus->IsStaggerRequested()) { return false; }

            // 特定の攻撃(ヒットスタンプ・回転・岩投げ・チャージレーザー等)の最中は怯まない。
            // その間に受けた怯み要求はここで消費して捨て、免疫が切れた後に発動しないようにする
            if (boss_->IsCurrentStateStaggerImmune())
            {
                bossStatus->ConsumeStaggerRequest();
                return false;
            }

            return true;
        }
    ));

    staggerSequence->AddChild(std::make_unique<BTAction>(
        [this]()
        {
            // 怯み要求を消費してからHitステートへ（多重発火防止）
            boss_->GetStatus()->As<BossStatus>()->ConsumeStaggerRequest();
            boss_->ChangeState(BossStateID::Hit);
        },
        [this]() { return boss_->GetCurrentStateID() == BossStateID::Hit && !boss_->IsCurrentStateFinished(); }
    ));

    return staggerSequence;
}

BTNodePtr NPCController::BuildNormalBehaviorTree()
{
    // 現在のステートが終わっている時だけ、距離帯ごとの重み付き抽選で次の行動を選ぶ
    // （攻撃モーションの途中は横取りしない。最初にBossCharacter::Start()で入るIdleの待機時間もここで尊重される）
    auto normalSequence = std::make_unique<BTSequence>();

    normalSequence->AddChild(std::make_unique<BTCondition>(
        [this]() { return boss_->IsCurrentStateFinished(); }
    ));

    auto distanceSelector = std::make_unique<BTSelector>();

    for (int i = 0; i < static_cast<int>(DistancePhase::enMax); ++i)
    {
        const DistancePhase phase = static_cast<DistancePhase>(i);
        const std::vector<AttackPattern>& patterns = currentRules_[i].attackList;

        auto phaseSequence = std::make_unique<BTSequence>();
        phaseSequence->AddChild(std::make_unique<BTCondition>(
            [this, phase]() { return GetCurrentDistancePhase() == phase; }
        ));

        auto attackThenIdle = std::make_unique<BTSequence>();
        attackThenIdle->AddChild(BuildWeightedAttackNode(patterns));
        attackThenIdle->AddChild(MakeStateAction(BossStateID::Idle));
        phaseSequence->AddChild(std::move(attackThenIdle));

        distanceSelector->AddChild(std::move(phaseSequence));
    }

    normalSequence->AddChild(std::move(distanceSelector));
    return normalSequence;
}
