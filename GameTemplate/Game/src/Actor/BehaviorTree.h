#pragma once
#include <functional>
#include <memory>
#include <vector>

/**
 * ビヘイビアツリー
 * BossCharacter/NPCController等、特定のキャラクタークラスには一切依存しない汎用実装。
 * キャラクター固有の処理はBTAction/BTConditionにstd::functionのクロージャとして渡す想定。
 * 将来ボス以外の敵を追加する時も、このファイルはそのまま流用できる。
 */


/** ノードの評価結果 */
enum class BTStatus
{
	Success, //!< 成功
	Failure, //!< 失敗
	Running  //!< 実行中（次のTickも同じノードから継続する）
};


/** ビヘイビアツリーのノード基底クラス */
class BTNode
{
public:
	virtual ~BTNode() = default;

	/** 毎フレーム呼ぶ */
	virtual BTStatus Tick() = 0;
};

using BTNodePtr = std::unique_ptr<BTNode>;


/** 複合ノードの基底（子ノードを複数持つ） */
class BTComposite : public BTNode
{
protected:
	std::vector<BTNodePtr> children_; //!< 子ノード

public:
	/** 子ノードを追加する（構築時のみ呼ぶ想定） */
	BTComposite& AddChild(BTNodePtr child)
	{
		children_.push_back(std::move(child));
		return *this;
	}
};


/**
 * セレクター（複数の選択肢から1つを選んで実行する）
 * 子を先頭から順に試し、Failure以外（Success/Running）を返した時点でそれを返す。
 * 全ての子がFailureならFailureを返す。
 *
 * 前回Runningで抜けた子から再開する（Sequenceと同様のメモリを持つ）。
 * これにより「一度選んだ選択肢が実行中の間は、他の選択肢が新たに条件を満たしても
 * 途中で乗っ取られない」という挙動になる（例：攻撃の実行中に距離帯が変わっても、
 * その攻撃自体は最後までやり切ってから次の選択に移る）。
 *
 * 「他の分岐よりつねに優先して毎フレーム条件をチェックしたい」割り込み処理
 * （例：怯み）はこのノードの子として組み込まず、呼び出し側で別のツリーとして
 * 独立に毎フレームTickする（NPCController::Update()を参照）。
 */
class BTSelector : public BTComposite
{
private:
	size_t runningIndex_ = 0; //!< 前回Runningで抜けた子のインデックス

public:
	BTStatus Tick() override;
};


/**
 * シーケンス（手順の連結）
 * 子を先頭から順に実行し、全てSuccessして初めてSuccessを返す。
 * 途中でFailureが出たらそこで打ち切ってFailureを返す。
 *
 * 前回Runningで抜けた子から再開し、既にSuccess済みの子（例：距離帯判定のCondition）は
 * 再評価しない。これにより「攻撃を始めた後に距離が変わっても、その攻撃自体は最後まで実行される」
 * という自然な挙動になる。
 */
class BTSequence : public BTComposite
{
private:
	size_t runningIndex_ = 0; //!< 前回Runningで抜けた子のインデックス

public:
	BTStatus Tick() override;
};


/**
 * 条件ノード
 * 条件を満たしていればSuccess、そうでなければFailureを返す（Runningにはならない）
 */
class BTCondition : public BTNode
{
private:
	std::function<bool()> condition_; //!< 条件判定コールバック

public:
	explicit BTCondition(std::function<bool()> condition) : condition_(std::move(condition)) {}

	BTStatus Tick() override
	{
		return condition_() ? BTStatus::Success : BTStatus::Failure;
	}
};


/**
 * アクションノード（葉）
 * onEnterはこのアクションに入った最初のTickで一度だけ呼ぶ。
 * isRunningがtrueを返す間はRunningを返し続け、falseになったらSuccessを返す
 * （外部要因で途中中断された場合も、次にisRunningがfalseを返した時点でSuccess扱いになる）。
 */
class BTAction : public BTNode
{
private:
	std::function<void()> onEnter_;   //!< 開始時に一度だけ呼ぶ
	std::function<bool()> isRunning_; //!< 毎Tick呼ぶ。trueなら継続中
	bool isStarted_ = false;          //!< onEnter_を呼び済みか

public:
	BTAction(std::function<void()> onEnter, std::function<bool()> isRunning)
		: onEnter_(std::move(onEnter)), isRunning_(std::move(isRunning)) {}

	BTStatus Tick() override
	{
		if (!isStarted_)
		{
			onEnter_();
			isStarted_ = true;
		}

		if (isRunning_())
		{
			return BTStatus::Running;
		}

		isStarted_ = false; // 次に選ばれた時、また最初からonEnterできるようにする
		return BTStatus::Success;
	}
};


/**
 * 重み付き抽選セレクター
 * 実行中の候補がなければ重み付き乱数で候補を1つ選び、
 * その候補がRunning以外を返すまでTickを委譲し続ける。
 * （距離帯ごとに「複数の攻撃候補から確率で1つ選ぶ」既存の抽選ロジックをノード化したもの）
 *
 * 重みはstd::functionで受け取る（固定値ではなく、抽選のたびに評価し直す）。
 * ツリー自体は起動時に1回だけ組み立てて使い回すため、「直前に何を選んだか」のような
 * 実行時の状態に応じて重みを変えたい場合（例：同じ攻撃の連続を避ける）に対応するため。
 */
class BTWeightedSelector : public BTNode
{
private:
	struct Entry
	{
		BTNodePtr node;
		std::function<int()> weightFn;
	};

	std::vector<Entry> entries_; //!< 候補と重み評価関数のリスト
	int selectedIndex_ = -1;     //!< 現在実行中の候補のインデックス（-1は未選択）

public:
	/** 候補を追加する（構築時のみ呼ぶ想定） */
	BTWeightedSelector& AddEntry(BTNodePtr node, std::function<int()> weightFn)
	{
		entries_.push_back({ std::move(node), std::move(weightFn) });
		return *this;
	}

	BTStatus Tick() override;
};
