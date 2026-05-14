/*!
 * @brief  草専用GBufferシェーダー
 *
 * ComputeAnimationVertexBuffer が全インスタンスの頂点を
 * 事前にワールド空間へ変換してドローコールを1回で行う仕組みのため、
 * VSGrass エントリーは vsIn.pos をそのままワールド座標として受け取る。
 * (VSMainInstancing のように per-instance ワールド行列で再変換してはいけない)
 *
 * 高さの判定: vsIn.uv.y を使用する (草メッシュの UV が根元=0, 先端=1 を前提)。
 * もし UV が逆 (根元=1, 先端=0) なら VSGrass 内の heightRatio 計算を反転すること。
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

///////////////////////////////////////
// 頂点シェーダーの共通処理をインクルード
// (SVSIn, mView, mProj, g_worldMatrixArray 等を提供)
///////////////////////////////////////
#include "ModelVSCommon.h"

///////////////////////////////////////
// ディザリング用定数バッファ (b1)
///////////////////////////////////////
cbuffer DitherCB : register(b1)
{
    float3 g_playerPos;
    float  g_ditherNear;
    float3 g_cameraPos;
    float  g_ditherFar;
    float  g_ditherdValue;
    float  g_ditherAlpha;
    float2 g_ditherPad;
};

///////////////////////////////////////
// 草曲げ用リソース
///////////////////////////////////////
#define MAX_BEND_SOURCES 8

struct GrassBendSource
{
    float3 position;      // 攻撃・移動の中心座標 (ワールド)
    float  force;         // 最大曲げ量 (ワールド単位)
    float  radius;        // 影響半径 (ワールド単位)
    float  elapsed;       // 経過時間 (秒)
    float  duration;      // 持続時間 (秒) — 0以下で無効
    float  recoverySpeed; // 将来拡張用
};

// slot1 = t11  (ModelRender::SetExtraGBufferSRV(0, ...) で渡す)
StructuredBuffer<GrassBendSource> g_bendSources : register(t11);

///////////////////////////////////////
// テクスチャ / サンプラー
///////////////////////////////////////
Texture2D<float4> g_albedo   : register(t0);
Texture2D<float4> g_normal   : register(t1);
Texture2D<float4> g_spacular : register(t2);
sampler g_sampler : register(s0);

////////////////////////////////////////////////
// VSMainCore の定義
// ModelVSCommon.h が前方宣言しているため定義が必要。
// VSGrass から直接は呼ばれないが、コンパイルを通すために必要。
////////////////////////////////////////////////
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;
    float4 worldPos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.worldPos = worldPos.xyz;
    psIn.pos      = mul(mView, worldPos);
    psIn.pos      = mul(mProj, psIn.pos);
    CalcVertexNormalTangentBiNormalInWorldSpace(
        psIn.normal, psIn.tangent, psIn.biNormal,
        mWorldLocal, vsIn.normal, vsIn.tangent, vsIn.biNormal,
        isUsePreComputedVertexBuffer
    );
    psIn.uv = vsIn.uv;
    return psIn;
}

////////////////////////////////////////////////
// 草専用頂点シェーダー (エントリーポイント)
//
// vsIn.pos はすでにワールド空間 (ComputeAnimationVertexBuffer が変換済み)。
// vsIn.normal / tangent / biNormal もワールド空間。
// vsIn.uv.y = 高さ比率 (根元=0, 先端=1) として草の曲げ量を制御する。
//
// 曲げは「根元を支点とした剛体回転」で実装。
//   水平変位 = heightRatio * F * bendDir
//   垂直補正 = heightRatio * (cosθ - 1) * GRASS_HEIGHT
// これにより草の全長が変わらず、伸びずに曲がるだけになる。
//
// GRASS_HEIGHT: 草モデルのワールド空間での高さ (実際のモデルに合わせて調整)。
////////////////////////////////////////////////
#define GRASS_HEIGHT 60.0f

SPSIn VSGrass(SVSIn vsIn)
{
    float4 worldPos   = vsIn.pos;
    float heightRatio = saturate(vsIn.uv.y);  // 根元=0, 先端=1

    // ---- 全ソースの力をXZ平面でベクトル合成 ----
    // 複数ソースをループ内で個別に回転適用すると Y 補正が重複するため、
    // まず合力を求めてから 1 回だけ回転を適用する。
    float3 totalForceVec = float3(0.0f, 0.0f, 0.0f);

    [unroll]
    for (int i = 0; i < MAX_BEND_SOURCES; i++)
    {
        GrassBendSource src = g_bendSources[i];
        if (src.duration <= 0.0f)
            continue;

        float3 toGrass = worldPos.xyz - src.position;
        toGrass.y      = 0.0f;
        float dist     = length(toGrass);

        if (dist < src.radius && dist > 0.001f)
        {
            float falloff    = 1.0f - saturate(dist / src.radius);
            falloff          = falloff * falloff;
            float timeFactor = 1.0f - saturate(src.elapsed / src.duration);

            totalForceVec += normalize(toGrass) * (src.force * falloff * timeFactor);
        }
    }

    // ---- 合力を 1 回の剛体回転として適用 ----
    float totalF = length(totalForceVec);
    if (totalF > 0.001f)
    {
        // sinθ = totalF / GRASS_HEIGHT としてクランプ (最大約80度)
        float sinTheta = clamp(totalF / GRASS_HEIGHT, 0.0f, 0.99f);
        float cosTheta = sqrt(max(0.0f, 1.0f - sinTheta * sinTheta));
        float3 bendDir = totalForceVec / totalF;  // normalize

        // 水平変位 = heightRatio * sinθ * GRASS_HEIGHT
        worldPos.x += bendDir.x * heightRatio * sinTheta * GRASS_HEIGHT;
        worldPos.z += bendDir.z * heightRatio * sinTheta * GRASS_HEIGHT;

        // 垂直補正 = heightRatio * (cosθ - 1) * GRASS_HEIGHT
        // 先端が曲がった分だけ下がり、草の全長が変わらない
        worldPos.y += heightRatio * (cosTheta - 1.0f) * GRASS_HEIGHT;
    }

    SPSIn psIn;
    psIn.worldPos = worldPos.xyz;
    psIn.pos      = mul(mView, worldPos);
    psIn.pos      = mul(mProj, psIn.pos);
    psIn.normal   = vsIn.normal;
    psIn.tangent  = vsIn.tangent;
    psIn.biNormal = vsIn.biNormal;
    psIn.uv       = vsIn.uv;
    return psIn;
}

////////////////////////////////////////////////
// ピクセルシェーダー
////////////////////////////////////////////////

float3 GetNormalFromNormalMap(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
    float3 binSpaceNormal = g_normal.SampleLevel(g_sampler, uv, 0.0f).xyz;
    binSpaceNormal        = (binSpaceNormal * 2.0f) - 1.0f;
    return tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;
}

SPSOut PSMainCore(SPSIn psIn, int isShadowReciever)
{
    static const float BayerMatrix[16] =
    {
         0.0f / 16.0f,  8.0f / 16.0f,  2.0f / 16.0f, 10.0f / 16.0f,
        12.0f / 16.0f,  4.0f / 16.0f, 14.0f / 16.0f,  6.0f / 16.0f,
         3.0f / 16.0f, 11.0f / 16.0f,  1.0f / 16.0f,  9.0f / 16.0f,
        15.0f / 16.0f,  7.0f / 16.0f, 13.0f / 16.0f,  5.0f / 16.0f
    };

    if (g_ditherdValue > 0.5f)
    {
        float dist         = length(g_cameraPos - psIn.worldPos);
        float distStrength = 1.0f - saturate(
            (dist - g_ditherNear) / max(g_ditherFar - g_ditherNear, 0.001f)
        );
        float ditherStrength = distStrength * (1.0f - g_ditherAlpha);

        uint px = (uint)psIn.pos.x % 4;
        uint py = (uint)psIn.pos.y % 4;
        if (BayerMatrix[py * 4 + px] < ditherStrength)
            discard;
    }

    SPSOut psOut;
    psOut.albedo = g_albedo.Sample(g_sampler, psIn.uv);
    clip(psOut.albedo.a - 0.2f);
    psOut.albedo.w             = psIn.pos.z;
    psOut.normal.xyz           = GetNormalFromNormalMap(psIn.normal, psIn.tangent, psIn.biNormal, psIn.uv);
    psOut.normal.w             = 1.0f;
    psOut.metaricShadowSmooth  = g_spacular.Sample(g_sampler, psIn.uv);
    psOut.metaricShadowSmooth.g = 255.0f * isShadowReciever;
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
