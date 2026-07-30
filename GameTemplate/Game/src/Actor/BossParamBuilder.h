#pragma once

/**
 * CharacterMaster.json / AnimationMaster.json から BossParam を組み立てるクラス。
 * ゲームオブジェクトの生成（NewGO等）には一切関与せず、「JSONデータ → BossParamへの変換」だけに責任を持つ。
 * アニメーション種類キー文字列(AnimationMaster.jsonの"key") ⇔ BossAnimID の変換は、
 * ここだけが知っていればよく、他のクラスには漏らさない。
 */
class BossParamBuilder
{
public:
    /**
     * 指定したキーのボスのBossParamを組み立てる
     * CharacterMaster.json / AnimationMaster.json に必須データが見つからない場合はアサートで停止する（デバッグ時）
     */
    static BossParam Build(const std::string& key);

private:
    /** AnimationMaster.jsonのアニメーション種類キー文字列を、BossAnimIDへ変換する（唯一の変換場所） */
    static BossAnimID ToAnimId(const std::string& animKey);

    /** CharacterMaster.jsonのmodelAxis文字列("Y"/"Z")を、EnModelUpAxisへ変換する */
    static EnModelUpAxis ToModelAxis(const std::string& axisStr);
};
