/*!
 * @brief 円形ゲージシェーダー（任意範囲対応版）
 *        sprite.fx の cbuffer(b0) 構造をそのまま流用し、
 *        ゲージ専用パラメータを b1 に追加する。
 *
 *        任意の開始〜終了角度の範囲だけゲージを描画できる。
 *        例: 0度〜45度、90度〜270度、315度〜45度（折り返しあり）
 */

// b0 : エンジン共通（sprite.fx と同一レイアウト）
cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4 mulColor;
};

// b1 : ゲージ専用パラメータ
cbuffer GaugeCb : register(b1)
{
    float  g_startProgress;  // ゲージ描画の開始位置（0.0〜1.0）
                             //   0.0 = 12時, 0.25 = 3時, 0.5 = 6時, 0.75 = 9時
    float  g_endProgress;    // ゲージ描画の終了位置（0.0〜1.0）
                             //   start <= end → 通常範囲
                             //   start >  end → 折り返しあり（12時をまたぐ）
    float  g_innerRadius;    // 円環の内径（UV空間 0.0〜0.5）
    float  g_outerRadius;    // 円環の外径（UV空間 0.0〜0.5）太さ = outer - inner

    float  g_rotationAngle;  // 全体の回転オフセット（ラジアン）-PI/2 で 12時スタート
    float  g_padding0;       // 16バイトアライメント用パディング
    float  g_padding1;
    float  g_padding2;

    float4 g_gaugeColor;     // ゲージ部分の色 RGBA
    float4 g_bgColor;        // 背景リング部分の色 RGBA
};

struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv  = In.uv;
    return psIn;
}

float4 PSMain(PSInput In) : SV_Target0
{
    static const float PI = 3.14159265f;
    static const float AA = 0.005f;

    // UV を中心原点 (-0.5〜0.5) に変換
    float2 uv = In.uv - float2(0.5f, 0.5f);

    // 中心からの距離
    float r = length(uv);

    // 円環マスク（太さの制御）
    // innerRadius〜outerRadius の範囲のみ描画。smoothstep でAAを掛ける
    float ringMask = smoothstep(g_innerRadius - AA, g_innerRadius + AA, r)
                   * smoothstep(g_outerRadius + AA, g_outerRadius - AA, r);

    // 円環の完全外側は破棄
    if (ringMask <= 0.0f)
    {
        discard;
    }

    // 角度計算: atan2 → g_rotationAngle でオフセット → frac で 0〜1 正規化
    float angle = atan2(uv.y, uv.x) - g_rotationAngle;
    float normalizedAngle = frac((angle / (2.0f * PI)) + 0.5f);

    // 任意範囲ゲージマスク
    //
    // 【通常ケース】 startProgress <= endProgress
    //   例: 0度〜45度   → start=0.000, end=0.125
    //   例: 90度〜270度 → start=0.250, end=0.750
    //   normalizedAngle が start〜end の間なら 1、それ以外は 0
    //
    // 【折り返しケース】 startProgress > endProgress
    //   例: 315度〜45度 → start=0.875, end=0.125
    //   12時（0/1境界）をまたぐ範囲。
    //   normalizedAngle が start以上 または end以下なら 1
    float gaugeMask;

    if (g_startProgress <= g_endProgress)
    {
        // 通常ケース
        gaugeMask = step(g_startProgress, normalizedAngle)
                  * step(normalizedAngle, g_endProgress);
    }
    else
    {
        // 折り返しケース（OR 判定）
        gaugeMask = max(
            step(g_startProgress, normalizedAngle),  // start〜1.0 の部分
            step(normalizedAngle, g_endProgress)     // 0.0〜end  の部分
        );
    }

    float4 color = lerp(g_bgColor, g_gaugeColor, gaugeMask);
    color.a *= ringMask;

    return color * mulColor;
}
