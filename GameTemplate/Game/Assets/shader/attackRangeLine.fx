/*!
 * @brief  攻撃範囲インジケーター (ライン) 用 GBuffer シェーダー
 *
 * 描画ルール:
 *   UV.y < g_drawProgress の範囲のみ描画 (ボス側から徐々に伸びる)
 *   中央領域のみ g_uvTilingY 倍でタイリング (両端キャップは固定)
 *   先端に明るいエッジグロー (伸びている感を強調)
 *
 * 定数バッファ (b2 = SetSplatColorParams):
 *   param0 = g_emissiveIntensity : グローの明るさ倍率
 *   param1 = g_lineThreshold     : 線とグローを分ける alpha 閾値
 *   param2 = g_drawProgress      : 描画進捗 (0→1, 0=何も見えない 1=全表示)
 *   param3 = g_uvTilingY         : V方向タイリング倍率 (距離 / タイル単位)
 */

////////////////////////////////////////////////
// 構造体
////////////////////////////////////////////////

struct SPSIn
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

struct SPSOut
{
    float4 albedo              : SV_Target0;
    float4 normal              : SV_Target1;
    float4 metaricShadowSmooth : SV_Target2;
};

////////////////////////////////////////////////
// 頂点シェーダー共通処理
////////////////////////////////////////////////
#include "ModelVSCommon.h"

////////////////////////////////////////////////
// 定数バッファ
////////////////////////////////////////////////

cbuffer DitherCB : register(b1)
{
    float3 g_playerPos;
    float  g_ditherNear;
    float3 g_cameraPos;
    float  g_ditherFar;
    float  g_ditherdValue;
    float  g_ditherAlpha;
    float  g_grassLodDitherStartDist;
    float  g_grassLodDitherAlphaMin;
    float  g_isGrassLodDither;
    float  g_grassLodDitherEndDist;
    float2 g_pad2;
};

cbuffer AttackRangeCB : register(b2)
{
    float g_emissiveIntensity; // param0: グロー / エッジの明るさ倍率
    float g_lineThreshold;     // param1: 線とグローを分ける alpha 閾値
    float g_drawProgress;      // param2: 描画進捗 0→1 (ボス側から伸びる)
    float g_uvTilingY;         // param3: V方向タイリング倍率
};

////////////////////////////////////////////////
// テクスチャ / サンプラー
////////////////////////////////////////////////

Texture2D<float4> g_albedo   : register(t0);
Texture2D<float4> g_normal   : register(t1);
Texture2D<float4> g_spacular : register(t2);
sampler           g_sampler  : register(s0);

static const float3 GLOW_COLOR  = float3(1.0f, 0.45f, 0.0f);
static const float  EDGE_WIDTH  = 0.04f;  // 先端エッジの幅 (UV.y 比)
static const float  BORDER_V    = 0.25f;  // 両端キャップの UV.y 比率

////////////////////////////////////////////////
// 頂点シェーダー
////////////////////////////////////////////////

SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;
    float4 worldPos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.worldPos   = worldPos.xyz;
    psIn.pos        = mul(mView, worldPos);
    psIn.pos        = mul(mProj, psIn.pos);
    CalcVertexNormalTangentBiNormalInWorldSpace(
        psIn.normal, psIn.tangent, psIn.biNormal,
        mWorldLocal, vsIn.normal, vsIn.tangent, vsIn.biNormal,
        isUsePreComputedVertexBuffer
    );
    psIn.uv = vsIn.uv;
    return psIn;
}

////////////////////////////////////////////////
// ピクセルシェーダー
////////////////////////////////////////////////

SPSOut PSMainCore(SPSIn psIn, int isShadowReciever)
{
    float rawU = psIn.uv.x; // 長さ方向 (0=ボス側, 1=矢印/ターゲット側, メッシュは X 軸方向に伸びている)

    // drawProgress を超えた部分は破棄
    clip(g_drawProgress - rawU);

    // 9スライス: 両端キャップは UV をそのまま、中央だけタイリング
    float sampledU;
    if (rawU < BORDER_V)
    {
        sampledU = rawU;                                            // ボス側キャップ
    }
    else if (rawU > 1.0f - BORDER_V)
    {
        sampledU = rawU;                                            // 矢印キャップ
    }
    else
    {
        float centerSpan = 1.0f - 2.0f * BORDER_V;
        float centerNorm = (rawU - BORDER_V) / centerSpan;         // 0-1 に正規化
        float tiled      = frac(centerNorm * g_uvTilingY);         // タイリング
        sampledU = BORDER_V + tiled * centerSpan;                  // キャップ内にマッピング
    }

    float4 color = g_albedo.Sample(g_sampler, float2(sampledU, psIn.uv.y));

    // アルファテスト
    clip(color.a - 0.02f);

    // ── DDS 色 (線 / 外側グロー) ──────────────────────────
    float lineFactor = saturate((color.a - g_lineThreshold)
                                / (1.0f - g_lineThreshold + 0.001f));
    float3 glowColor = GLOW_COLOR * g_emissiveIntensity
                       * (color.a / (g_lineThreshold + 0.001f));
    float3 ddsColor  = lerp(glowColor, color.rgb, lineFactor);

    // ── 先端エッジグロー (進捗境界を強調) ───────────────────
    float edgeDist    = g_drawProgress - rawU;
    float edgeStrength = saturate(1.0f - edgeDist / EDGE_WIDTH) * g_emissiveIntensity * 1.5f;
    float3 edgeColor  = GLOW_COLOR * edgeStrength;

    float3 outColor = ddsColor + edgeColor;

    SPSOut psOut;
    psOut.albedo.rgb          = outColor;
    psOut.albedo.a            = psIn.pos.z;
    // normal=(0,0,0) → DeferredLighting が albedo を直接出力 (自発光)
    psOut.normal              = float4(0.0f, 0.0f, 0.0f, 0.0f);
    psOut.metaricShadowSmooth = float4(0.0f, 0.0f, 0.0f, 0.0f);
    return psOut;
}

SPSOut PSMain(SPSIn psIn)
{
    return PSMainCore(psIn, 0);
}

SPSOut PSMainShadowReciever(SPSIn psIn)
{
    return PSMainCore(psIn, 1);
}
