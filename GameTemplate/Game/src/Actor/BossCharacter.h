#pragma once
#include "src/Actor/Character.h"
#include "src/Actor/NPCController.h"

class BossStateBase;
class BossStatus;
/* ボスキャラクターの基底クラス */
class BossCharacter : public Character
{
private:
	IState* currentState_ = nullptr;          //!< 現在のステート（状態）の実体
	BossStateID currentStateID_ = BossStateID::Idle; //!< 現在のステートID
	BossParam param_;

	/** あらかじめ生成したステートを保管するマップ */
	std::unordered_map<BossStateID, std::unique_ptr<IState>> stateMap_;
	Vector3 targetPlayerPos_ = Vector3::Zero; //!< プレイヤーのいる位置を保存
	Quaternion targetPlayerRot_ = Quaternion::Identity; // プレイヤーがいる方向
	bool isMoveStop_ = false; //!< 動きを止めるかどうか


public:
	/** プレイヤーのいる座標 */
	void SetTargetPos(const Vector3 targetPos) { targetPlayerPos_ = targetPos; }	//!< 設定
	Vector3 GetTargetPos() { return targetPlayerPos_; }								//!< 取得

	/** ボスの方向 */
	void SetTargetRot(const Quaternion targetRot) { targetPlayerRot_ = targetRot; }	//!< 設定
	Quaternion GetTargetRot() { return targetPlayerRot_; }							//!< 取得
	/** 動くかどうか */
	void SetMoveStop(const bool flg = true) { isMoveStop_ = flg; }	//!< 設定
	bool IsMoveStop() { return isMoveStop_; }						//!< 取得

	/**
	 * カットシーン演出用：基準の向き(baseRotation)に対して、前後(X軸)・左右(Y軸)の回転オフセットを加えた向きを毎フレーム絶対値で設定する。
	 * イージングなど「時間経過で角度が変化する」演出向けに、呼び出し側で計算した角度をそのまま渡す想定。
	 * isMoveStop_中はUpdate()内の回転同期処理(transform_.localRotation = targetPlayerRot_)がスキップされるため、
	 * AI/移動を介さずここで直接ローカル回転とモデルへ即反映する。
	 */
	void SetCutsceneTiltOffset(const Quaternion& baseRotation, const float pitchOffsetRad, const float yawOffsetRad)
	{
		Quaternion pitchOffset;
		pitchOffset.SetRotationX(pitchOffsetRad);
		Quaternion yawOffset;
		yawOffset.SetRotationY(yawOffsetRad);
		// 左右(Y)は基準の向きと同じ軸なのでbaseRotationと合成し、前後(X)はモデルのローカル軸で回すよう最後に掛ける
		transform_.localRotation = baseRotation * yawOffset * pitchOffset;
		transform_.UpdateTransform();
		modelRender_.SetRotation(transform_.rotation);
	}

/** =================================================================== */
/** アニメーション関連の関数 */
/** =================================================================== */

public:
	/** アニメーションの読み込み */
	void SetupAnimation();
	/** アニメーションの再生。interpolateTimeを指定すると、直前のアニメーションから指定秒数かけて滑らかにブレンドする */
	void PlayAnimation(const int id, const float animSpeed = 1.0f, const float interpolateTime = 0.0f);
	/** 現在のアニメーションが再生中か（ループしないアニメーションが最後まで再生し終わったか判定する用） */
	bool IsPlayingAnimation() const { return modelRender_.IsPlayingAnimation(); }
	/** 指定したアニメーションがこのボスに読み込まれているか（ボスの種類によって未使用のアニメーションIDがあるため、共有ステートから再生前に確認する用） */
	bool HasAnimation(const int id) const { return id >= 0 && id < BossAnimID::enAnimNum && !param_.anims[id].filePath.empty(); }


/** =================================================================== */
/** ステート関連の関数 */
/** =================================================================== */

private:
	/** ステートのインスタンスをあらかじめ作成 */
	void AddState(BossStateID id, IState* state)
	{
		stateMap_.emplace(id, std::unique_ptr<IState>(state));
	}

	/* ステートを見つける */
	IState* FindState(const BossStateID id)
	{
		auto it = stateMap_.find(id);
		if (it != stateMap_.end()) {
			return it->second.get();
		}
		return nullptr;
	}

	/** ステートのインスタンス作成 */
	void SetupTranslate();

public:
	/* 状態の変更を要求する */
	void ChangeState(BossStateID nextState);

	/* 現在のステートIDを取得する */
	BossStateID GetCurrentStateID() const { return currentStateID_; }

	/* 現在のステート処理が終了しているか */
	bool IsCurrentStateFinished() const {
		if (currentState_) return currentState_->IsFinished();
		return true; // ステートがない場合は「終わってる」扱いにして次を呼ぶ
	}

/** =================================================================== */
/** コンストラクタ関連の関数 */
/** =================================================================== */

public:
	BossCharacter();
	virtual ~BossCharacter();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc)override;

	/* ステータス設定 */
	//inline void SetupStatus(BossStatus* status) { status_ = status; }	//!< ボスのステータス設定
	inline void SetupParam(BossParam& param) { param_ = param; }		//!< ボスのパラメータ設定
};