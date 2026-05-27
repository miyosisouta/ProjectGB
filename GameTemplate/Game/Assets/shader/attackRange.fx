/*!
 * @brief  攻撃範囲インジケーター用 GBuffer シェーダー
 *
 * 描画ルール:
 *   alpha >= g_lineThreshold  → DDS 色 (線の部分)
 *   alpha <  g_lineThreshold  → オレンジ × alpha (外側グロー)
 *   + パルスリング             → 中央から外側の線に向かって一定速で拡大するオレンジリング
 *
 * 定数バッファ (b2 = SetSplatColorParams):
 *   param0 = g_emissiveIntensity : グロー / リングの明るさ倍率
 *   param1 = g_lineThreshold     : 線とグローを分ける alpha 閾値 (0〜1)
 *   param2 = g_time              : 累積経過時間 (AttackRange::Update() で毎フレーム更新)
 *   param3 = g_pulseSpeed        : パルスの速さ (cycles/秒, 例: 0.5 = 2秒で1周)
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
    float g_emissiveIntensity; // param0: グロー / リングの明るさ倍率
    float g_lineThreshold;     // param1: 線とグローを分ける alpha 閾値
    float g_time;              // param2: 累積経過時間
    float g_pulseSpeed;        // param3: パルスの速さ (cycles/秒)
};

////////////////////////////////////////////////
// テクスチャ / サンプラー
////////////////////////////////////////////////

Texture2D<float4> g_albedo   : register(t0);
Texture2D<float4> g_normal   : register(t1);
Texture2D<float4> g_spacular : register(t2);
sampler           g_sampler  : register(s0);

// グロー / パルスのオレンジ色
static const float3 GLOW_COLOR     = float3(1.0f, 0.45f, 0.0f);
// パルスリングの幅 (UV 半径比, 0.0〜1.0)
static const float  RING_THICKNESS = 0.04f;
// パルスリングが到達する最大半径 (1.0=円の端, 小さくすると内側で止まる)
static const float  RING_RADIUS_MAX = 0.85f;

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
    float4 color = g_albedo.Sample(g_sampler, psIn.uv);

    // ── パルスリング計算 ──────────────────────────────────
    // UV 中心 (0.5, 0.5) からの距離を 0〜1 に正規化
    float dist = length(psIn.uv - float2(0.5f, 0.5f)) * 2.0f;

    // 0→RING_RADIUS_MAX へ一方向に拡大し、最大値で停止 (インジケーター消滅タイミングで端に到達)
    float pulseRadius  = min(g_time * g_pulseSpeed, 1.0f) * RING_RADIUS_MAX;
    float ringDist     = abs(dist - pulseRadius);
    float ringMask     = step(dist, 1.0f); // circle boundary: suppress ring beyond edge
    float ringStrength = saturate(1.0f - ringDist / RING_THICKNESS) * g_emissiveIntensity * ringMask;

    // ── クリップ ─────────────────────────────────────────
    // DDS alpha が有効 OR パルスリングが通過中のピクセルは残す
    clip(max(color.a - 0.02f, ringStrength - 0.01f));

    // ── DDS 色 (線 / 外側グロー) ─────────────────────────
    float showDDS    = step(0.02f, color.a);
    float lineFactor = saturate((color.a - g_lineThreshold)
                                / (1.0f - g_lineThreshold + 0.001f));
    float3 glowColor = GLOW_COLOR * g_emissiveIntensity
                       * (color.a / (g_lineThreshold + 0.001f));
    float3 ddsColor  = lerp(glowColor, color.rgb, lineFactor) * showDDS;

    // ── パルスリングを加算 ────────────────────────────────
    float3 outColor = ddsColor + GLOW_COLOR * ringStrength;

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
