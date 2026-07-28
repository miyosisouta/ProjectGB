/**
 * TutorialBossScript.h
 * チュートリアル中、ボスにHitStamp→Idle→HitStampの攻撃ループを行わせるスクリプト制御。
 *
 * TutorialManagerから「進行中か否か」だけを受け取り、実際のボスの状態遷移（Jump/Idle）や
 * 待機時間の管理はここに閉じ込める。プレイヤーの進行段階の判定（TutorialManagerの責務）とは分離する。
 */
#pragma once


class BossCharacter;


class TutorialBossScript
{
private:
	BossCharacter* boss_ = nullptr;

	bool  scriptActive_ = false; //!< 攻撃ループを開始済みか
	bool  waitingIdle_  = false; //!< 攻撃後、Idle待機中か
	float idleElapsed_  = 0.0f;  //!< Idle待機の経過秒数

	Vector3    spawnPosition_ = Vector3::Zero;       //!< Init()時点のボス座標（ResetToSpawn()で戻す先）
	Quaternion spawnRotation_ = Quaternion::Identity; //!< Init()時点のボス向き


public:
	/** 対象のボスを設定し、登場演出用の初期位置・向きを覚える */
	void Init(BossCharacter* boss);

	/**
	 * 更新処理
	 * @param deltaTime    1フレーム当たりの経過秒数
	 * @param shouldBeActive 攻撃ループを進行させるべきか（かいひ段階以降でtrue）
	 * @param targetPos    ボスが狙う対象（プレイヤー）の座標
	 * @param isFreeStage  自由段階中か（Idle待機時間の長さがteaching中と異なるため）
	 */
	void Update(float deltaTime, bool shouldBeActive, const Vector3& targetPos, bool isFreeStage);

	/** 攻撃ループを止め、登場演出用の初期位置・向き・Idle状態へ即座に戻す（チュートリアル全体スキップ時に使用） */
	void ResetToSpawn();
};
