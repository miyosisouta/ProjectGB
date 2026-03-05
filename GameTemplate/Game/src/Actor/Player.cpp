#include "stdafx.h"
#include "Player.h"


namespace
{
	// アニメーション設定
	struct AnimationData
	{
		const char* filePath;
		bool loopFlag;
	};

	// 読み込みたいアニメーションのファイルパスを並べる
	// ここにパスを追加・削除するだけで、下の処理がすべて自動で適応される
	static AnimationData sAnimPaths[] = {
		AnimationData{"Assets/animData/idle.tka",true},
		AnimationData{"Assets/animData/walk.tka",true},
		AnimationData{"Assets/animData/run.tka",true}
		// アニメーションを増やすときはここから
	};
}


Player::Player()
{
}

bool Player::Start()
{
	// 自分はPlayerグループであることを宣言する
	SetUpdateGroup(UpdateGroup::Player);

	// アニメーション
	{
		// アニメーションクリップリストの配列数を決定
		const size_t animCount = ARRAYSIZE(sAnimPaths);// アニメーションの数を数える
		m_animationClipList.Create(animCount); // 配列を作る

		// すべてのアニメーションを登録
		for (size_t i = 0; i < animCount; ++i)
		{
			m_animationClipList[i].Load(sAnimPaths[i].filePath);			// ファイルパス
			m_animationClipList[i].SetLoopFlag(sAnimPaths[i].loopFlag);  // ループ
		}
	}
	

	// モデルの設定
	{
		// モデルのファイルパスとアニメーションを設定
		m_modelRender.Init(
			"Assets/modelData/unityChan.tkm",	// ファイルパス
			m_animationClipList.data(),			// アニメーションデータ
			m_animationClipList.size(),			// リストの数
			enModelUpAxisY						// モデルの上方向
		);

		// モデルの座標を更新・初期化
		m_modelRender.SetPosition(m_transform.m_position);
		m_modelRender.SetScale(m_transform.m_scale);
		m_modelRender.Update();
	}
	return Character::Start();
}


void Player::Update()
{
	Character::Update();
}


void Player::Render(RenderContext& rc)
{
	Character::Render(rc);
}