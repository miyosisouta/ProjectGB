#include "stdafx.h"
#include "BossCharacter.h"
#include "NPCController.h"


/* ============================================== */
/* ボスの基底クラス */
/* ============================================== */
void BossCharacter::PlayAnimation(const int id)
{
	modelRender_.PlayAnimation(id); // Idをもとにそのアニメーションの再生
}


BossCharacter::BossCharacter()
{
}

BossCharacter::~BossCharacter()
{
}

bool BossCharacter::Start()
{
	// 1. アニメーションの読み込み（for文で一気に回す！）
	const int animCount = BossAnimID::enAnimNum; // アニメーションの総数
	animationClipList_.Create(animCount); // メモリをあらかじめ確保

	for (int i = 0; i < animCount; ++i)
	{
		// filePath が空っぽじゃない（ちゃんとセットされている）時だけ読み込む
		if (!param_.anims[i].filePath.empty())
		{
			// c_str()を使い、stringからconst charに変える
			animationClipList_[i].Load(param_.anims[i].filePath.c_str());
			animationClipList_[i].SetLoopFlag(param_.anims[i].isLoop);
		}
	}

	// 2. モデルの初期化
	modelRender_.Init(
		param_.modelPath_.c_str(),
		animationClipList_.data(),
		animationClipList_.size(),
		enModelUpAxisZ
	);


	// モデルの設定
	transform_.localScale = Vector3(2.5f, 2.5f, 2.5f);
	transform_.UpdateTransform();
	modelRender_.SetPosition(transform_.position);
	modelRender_.SetRotation(transform_.rotation);
	modelRender_.SetScale(transform_.scale);

	PlayAnimation(enAnimRun);
	return true;
}

void BossCharacter::Update()
{
	modelRender_.Update();
}

void BossCharacter::Render(RenderContext& rc)
{
	modelRender_.Draw(rc);
}
