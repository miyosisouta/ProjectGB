#include "stdafx.h"
#include "src/Actor/BossParamBuilder.h"


BossAnimID BossParamBuilder::ToAnimId(const std::string& animKey)
{
	static const std::unordered_map<std::string, BossAnimID> table = {
		{ "Idle",          BossAnimID::enAnimIdle },
		{ "Run",           BossAnimID::enAnimRun },
		{ "Jump",          BossAnimID::enAnimJump },
		{ "Attack",        BossAnimID::enAnimAttack },
		{ "Hit",           BossAnimID::enAnimHit },
		{ "Death",         BossAnimID::enAnimDeath },
		{ "Spin",          BossAnimID::enAnimSpin },
		{ "Clicked",       BossAnimID::enAnimClicked },
		{ "Antic",         BossAnimID::enAnimAntic },
		{ "ClickedWindUp", BossAnimID::enAnimClickedWindUp },
		{ "JumpImpact",    BossAnimID::enAnimJumpImpact },
	};

	auto it = table.find(animKey);
	return (it != table.end()) ? it->second : BossAnimID::enNone;
}


EnModelUpAxis BossParamBuilder::ToModelAxis(const std::string& axisStr)
{
	return (axisStr == "Y") ? EnModelUpAxis::enModelUpAxisY : EnModelUpAxis::enModelUpAxisZ;
}


BossParam BossParamBuilder::Build(const std::string& key)
{
	BossParam param;

	// CharacterMaster.json: モデルパス・アニメーションの置き場・カテゴリ
	const auto* master = ParameterManager::Get().GetCharacterMaster(key);
	K2_ASSERT(master != nullptr, ("CharacterMaster.jsonにキー'" + key + "'が見つかりません\n").c_str());
	if (!master) { return param; } // リリースビルドではK2_ASSERTが無効化されるため、明示的に打ち切る

	K2_ASSERT(master->category == "Boss", ("'" + key + "'はcategoryが'Boss'ではありません（BossSpawnerから生成しようとしています）\n").c_str());
	K2_ASSERT(!master->modelPath.empty(), ("CharacterMaster.json '" + key + "' のmodelPathが空です\n").c_str());

	param.characterKey_ = key;
	param.modelPath_    = master->modelPath;
	param.modelAxis_    = ToModelAxis(master->modelAxis);

	// AnimationMaster.json: アニメーションクリップ一覧
	const auto animations = ParameterManager::Get().GetAnimationsByCharacter(key);
	K2_ASSERT(!animations.empty(), ("AnimationMaster.jsonにキー'" + key + "'のアニメーションがありません\n").c_str());

	for (const auto* anim : animations)
	{
		const BossAnimID id = ToAnimId(anim->key);
		K2_ASSERT(id != BossAnimID::enNone, ("AnimationMaster.json '" + key + "' に未知のアニメーション種類キー'" + anim->key + "'があります\n").c_str());
		if (id == BossAnimID::enNone) { continue; } // 未知のキーはこのエントリだけ読み飛ばす

		param.anims[id].filePath = master->animationBasePath + anim->fileName;
		param.anims[id].isLoop   = anim->isLoop;
	}

	return param;
}
