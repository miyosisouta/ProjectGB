/*!
 * @brief  攻撃範囲インジケーター (Box) 用 GBuffer シェーダー – 9スライス UV タイリング
 *
 * 9スライス構成 (16頂点):
 *   ┌──────┬──────────────┬──────┐
 *   │ 角TL │   上エッジ   │ 角TR │
 *   ├──────┼──────────────┼──────┤
 *   │ 左   │   中央       │ 右   │
 *   │ エッジ│ (タイリング) │ エッジ│
 *   ├──────┼──────────────┼──────┤
 *   │ 角BL │   下エッジ   │ 角BR │
 *   └──────┴──────────────┴──────┘
 *
 * 角・端領域: UV そのまま (変形しない)
 * 中央領域 : objectSize / baseSize でタイリング数を計算してリピート
 *
 * 定数バッファ (b2 = SetSplatColorParams):
 *   param0 = tilingU  : X方向タイリング数 (objectSize.x / baseSize.x)
 *   param1 = tilingV  : Z方向タイリング数 (objectSize.z / baseSize.z)
 *
 * シェーダー内定数 (テクスチャのレイアウトに合わせて調整すること):
 *   BORDER_U : 左右の角領域の幅 (UV比率)
 *   BORDER_V : 上下の角領域の高さ (UV比率)
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
    float g_tilingU;  // param0: X方向タイリング数
    float g_tilingV;  // param1: Z方向タイリング数
    float2 _cbpad;
};

////////////////////////////////////////////////
// テクスチャ / サンプラー
////////////////////////////////////////////////

Texture2D<float4> g_albedo   : register(t0);
Texture2D<float4> g_normal   : register(t1);
Texture2D<float4> g_spacular : register(t2);
sampler           g_sampler  : register(s0);

// ── テクスチャのレイアウトに合わせて調整 ──────────────────
static const float BORDER_U = 0.25f; // 左右の角領域の UV 幅比率
static const float BORDER_V = 0.25f; // 上下の角領域の UV 高さ比率
// ──────────────────────────────────────────────────────────

/// 1次元の 9スライス UV を計算する
/// border: 端領域の幅比率, tiling: 中央のタイリング数
float NineSlice1D(float t, float border, float tiling)
{
    if (t < border)
        return t;                                          // 左/上端: そのまま

    if (t > 1.0f - border)
        return t;                                          // 右/下端: そのまま

    // 中央: 正規化してタイリング
    float centerSpan   = 1.0f - 2.0f * border;
    float centerNorm   = (t - border) / centerSpan;       // 0〜1 に正規化
    float tiled        = frac(centerNorm * tiling);       // タイリング
    return border + tiled * centerSpan;                   // テクスチャの中央領域にマッピング
}

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
    // 9スライス UV を計算
    float2 slicedUV;
    slicedUV.x = NineSlice1D(psIn.uv.x, BORDER_U, g_tilingU);
    slicedUV.y = NineSlice1D(psIn.uv.y, BORDER_V, g_tilingV);

    float4 color = g_albedo.Sample(g_sampler, slicedUV);

    // α=0 のピクセルは破棄
    clip(color.a - 0.01f);

    SPSOut psOut;
    psOut.albedo.rgb          = color.rgb;
    psOut.albedo.a            = psIn.pos.z;
    // normal=(0,0,0) → DeferredLighting が albedo を直接出力 (自発光)
    psOut.normal              = float4(0.0f, 0.0f, 0.0f, 0.0f);
    psOut.metaricShadowSmooth = float4(0.0f, 0.0f, 0.0f, 0.0f);
    return psOut;
}

SPSOut PSMain(SPSIn psIn)             { return PSMainCore(psIn, 0); }
SPSOut PSMainShadowReciever(SPSIn psIn) { return PSMainCore(psIn, 1); }
