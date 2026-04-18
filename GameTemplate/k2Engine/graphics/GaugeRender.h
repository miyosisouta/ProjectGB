#pragma once

namespace nsK2Engine {

/// <summary>
/// 円形ゲージレンダラー（任意範囲対応版）。
/// SpriteRender と同じ使い勝手で円形ゲージを描画する。
/// テクスチャは使用せず、シェーダーだけで円形ゲージを描く。
///
/// 任意の開始〜終了角度の範囲だけゲージを描画できる。
///   例: 0度〜45度、90度〜270度、315度〜45度（折り返しあり）
/// </summary>
class GaugeRender : public IRenderer
{
public:
    /// <summary>
    /// ゲージ用定数バッファ構造体（gauge.fx の GaugeCb と同一レイアウト）。
    /// ※ メンバの順番・型・バイト数を gauge.fx と完全一致させること。
    /// </summary>
    struct GaugeConstantBuffer
    {
        float   startProgress;   // ゲージ描画の開始位置（0.0〜1.0）
        float   endProgress;     // ゲージ描画の終了位置（0.0〜1.0）
        float   innerRadius;     // 円環の内径（UV空間 0.0〜0.5）
        float   outerRadius;     // 円環の外径（UV空間 0.0〜0.5）太さ = outer - inner
        float   rotationAngle;   // 全体の回転オフセット（ラジアン）-PI/2 で 12時スタート
        float   padding0;        // 16バイトアライメント用パディング
        float   padding1;
        float   padding2;
        Vector4 gaugeColor;      // ゲージ部分の色 RGBA
        Vector4 bgColor;         // 背景リング部分の色 RGBA
    };

public:
    /// <summary>
    /// 初期化。
    /// </summary>
    /// <param name="w">スプライトの横幅（ピクセル）</param>
    /// <param name="h">スプライトの縦幅（ピクセル）</param>
    void Init(float w, float h);

    // -------------------------------------------------------
    // 座標・回転・スケール（SpriteRender と同一インターフェース）
    // -------------------------------------------------------

    /// <summary>座標を設定。z は 0.0f で。</summary>
    void SetPosition(const Vector3& pos)        { m_position = pos; }
    /// <summary>座標を取得。</summary>
    const Vector3& GetPosition() const          { return m_position; }

    /// <summary>大きさを設定。z は 1.0f で。</summary>
    void SetScale(const Vector3& scale)         { m_scale = scale; }
    /// <summary>大きさを取得。</summary>
    const Vector3& GetScale() const             { return m_scale; }

    /// <summary>回転を設定。</summary>
    void SetRotation(const Quaternion& rot)     { m_rotation = rot; }
    /// <summary>回転を取得。</summary>
    const Quaternion& GetRotation() const       { return m_rotation; }

    /// <summary>ピボットを設定。</summary>
    void SetPivot(const Vector2& pivot)         { m_pivot = pivot; }
    /// <summary>ピボットを取得。</summary>
    const Vector2& GetPivot() const             { return m_pivot; }

    /// <summary>乗算カラーを設定。</summary>
    void SetMulColor(const Vector4& color)      { m_sprite.SetMulColor(color); }
    /// <summary>乗算カラーを取得。</summary>
    const Vector4& GetMulColor() const          { return m_sprite.GetMulColor(); }

    // -------------------------------------------------------
    // ゲージ範囲の指定
    // -------------------------------------------------------

    /// <summary>
    /// ゲージの描画範囲を 0.0〜1.0 の割合で指定。
    /// startProgress <= endProgress → 通常範囲
    /// startProgress >  endProgress → 折り返しあり（12時をまたぐ範囲）
    /// </summary>
    /// <param name="startProgress">開始位置（0.0〜1.0）</param>
    /// <param name="endProgress">終了位置（0.0〜1.0）</param>
    void SetProgressRange(float startProgress, float endProgress)
    {
        m_gaugeCb.startProgress = max(0.0f, min(1.0f, startProgress));
        m_gaugeCb.endProgress   = max(0.0f, min(1.0f, endProgress));
    }

    /// <summary>
    /// ゲージの描画範囲を度数（°）で指定。
    /// 0度 = 12時、時計回りで増加。
    /// 折り返しも自動で処理される（例: 315度〜45度）。
    /// </summary>
    /// <param name="startDeg">開始角度（度）例: 0.0f</param>
    /// <param name="endDeg">終了角度（度）例: 45.0f</param>
    void SetProgressRangeDeg(float startDeg, float endDeg)
    {
        m_gaugeCb.startProgress = fmodf(startDeg / 360.0f, 1.0f);
        if (m_gaugeCb.startProgress < 0.0f) m_gaugeCb.startProgress += 1.0f;
        m_gaugeCb.endProgress   = fmodf(endDeg   / 360.0f, 1.0f);
        if (m_gaugeCb.endProgress   < 0.0f) m_gaugeCb.endProgress   += 1.0f;
    }

    /// <summary>
    /// ゲージを 0 から progress までの通常ゲージとして設定。
    /// SetProgressRange(0.0f, progress) の短縮版。
    /// </summary>
    /// <param name="progress">0.0（空）〜1.0（満タン）</param>
    void SetProgress(float progress)
    {
        SetProgressRange(0.0f, max(0.0f, min(1.0f, progress)));
    }

    /// <summary>現在の開始位置を取得。</summary>
    float GetStartProgress() const { return m_gaugeCb.startProgress; }
    /// <summary>現在の終了位置を取得。</summary>
    float GetEndProgress()   const { return m_gaugeCb.endProgress; }

    // -------------------------------------------------------
    // 円環の太さ
    // -------------------------------------------------------

    /// <summary>
    /// 円環の太さを中心半径＋太さで指定する（直感的な指定方法）。
    /// </summary>
    /// <param name="radius">円環の中心半径（UV空間 例: 0.40f）</param>
    /// <param name="thickness">太さ（UV空間 例: 0.10f）</param>
    void SetThickness(float radius, float thickness)
    {
        m_gaugeCb.innerRadius = radius - thickness * 0.5f;
        m_gaugeCb.outerRadius = radius + thickness * 0.5f;
    }

    /// <summary>
    /// 円環の内径・外径を直接指定する。
    /// </summary>
    /// <param name="innerRadius">内径（UV空間 例: 0.35f）</param>
    /// <param name="outerRadius">外径（UV空間 例: 0.45f）</param>
    void SetRadius(float innerRadius, float outerRadius)
    {
        m_gaugeCb.innerRadius = innerRadius;
        m_gaugeCb.outerRadius = outerRadius;
    }

    // -------------------------------------------------------
    // その他パラメータ
    // -------------------------------------------------------

    /// <summary>
    /// 全体の回転オフセットを設定。
    /// デフォルトは -PI/2（12時スタート）。
    /// </summary>
    /// <param name="angle">ラジアン</param>
    void SetRotationAngle(float angle)          { m_gaugeCb.rotationAngle = angle; }
    /// <summary>回転オフセットを取得。</summary>
    float GetRotationAngle() const              { return m_gaugeCb.rotationAngle; }

    /// <summary>ゲージ部分の色を設定。</summary>
    void SetGaugeColor(const Vector4& color)    { m_gaugeCb.gaugeColor = color; }
    /// <summary>ゲージ部分の色を取得。</summary>
    const Vector4& GetGaugeColor() const        { return m_gaugeCb.gaugeColor; }

    /// <summary>背景リング部分の色を設定。</summary>
    void SetBgColor(const Vector4& color)       { m_gaugeCb.bgColor = color; }
    /// <summary>背景リング部分の色を取得。</summary>
    const Vector4& GetBgColor() const           { return m_gaugeCb.bgColor; }

    // -------------------------------------------------------
    // 更新・描画
    // -------------------------------------------------------

    /// <summary>
    /// 更新処理。毎フレーム呼ぶこと。
    /// 定数バッファの内容をGPUへ送信し、スプライトのワールド行列を更新する。
    /// </summary>
    void Update();

    /// <summary>
    /// 描画登録。RenderingEngine の 2D 描画パスに登録する。
    /// </summary>
    /// <param name="rc">レンダーコンテキスト。</param>
    void Draw(RenderContext& rc);

private:
    /// <summary>
    /// 2D 描画パスから呼ばれる処理。
    /// </summary>
    void OnRender2D(RenderContext& rc) override
    {
        m_sprite.Draw(rc);
    }

private:
    Sprite                  m_sprite;                               // スプライト本体
    GaugeConstantBuffer     m_gaugeCb;                              // ゲージ用定数バッファ

    Vector3                 m_position  = Vector3::Zero;            // 座標
    Quaternion              m_rotation  = Quaternion::Identity;     // 回転
    Vector3                 m_scale     = Vector3::One;             // 大きさ
    Vector2                 m_pivot     = Sprite::DEFAULT_PIVOT;    // ピボット
};

} // namespace nsK2Engine
