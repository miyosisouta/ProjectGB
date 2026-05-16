///////////////////////////////////////
// 構造体。
///////////////////////////////////////

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

// ピクセルシェーダーからの出力
struct SPSOut
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 metaricShadowSmooth : SV_Target2;
};

///////////////////////////////////////
// 頂点シェーダーの共通処理をインクルードする。
///////////////////////////////////////
#include "../ModelVSCommon.h"

// ディザリング用定数バッファ
cbuffer DitherCB : register(b1)
{
    float3 g_playerPos;               // 12  row0
    float  g_ditherNear;              //  4
    float3 g_cameraPos;               // 12  row1
    float  g_ditherFar;               //  4
    float  g_ditherdValue;            //  4  row2
    float  g_ditherAlpha;             //  4  per-objectの透明度ディザ(0.0=完全透明, 1.0=不透明)
    float  g_grassLodDitherStartDist; //  4  草LODディザ開始距離
    float  g_grassLodDitherAlphaMin;  //  4  草LODディザ最小アルファ
    float  g_isGrassLodDither;        //  4  row3 草LODディザ有効フラグ
    float  g_grassLodDitherEndDist;   //  4  草LODディザ終了距離
    float2 g_pad2;                    //  8
};
///////////////////////////////////////
// シェーダーリソース
///////////////////////////////////////
Texture2D<float4> g_albedo : register(t0);
Texture2D<float4> g_normal : register(t1);
Texture2D<float4> g_spacular : register(t2);

///////////////////////////////////////
// サンプラーステート
///////////////////////////////////////
sampler g_sampler : register(s0);

///////////////////////////////////////
// 関数
///////////////////////////////////////

float3 GetNormalFromNormalMap(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
    float3 binSpaceNormal = g_normal.SampleLevel(g_sampler, uv, 0.0f).xyz;
    binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;
    float3 newNormal = tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;
    return newNormal;
}

// モデル用の頂点シェーダーのエントリーポイント
SPSIn VSMainCore(SVSIn vsIn, float4x4 mWorldLocal, uniform bool isUsePreComputedVertexBuffer)
{
    SPSIn psIn;
    float4 worldPos = CalcVertexPositionInWorldSpace(vsIn.pos, mWorldLocal, isUsePreComputedVertexBuffer);
    psIn.worldPos = worldPos.xyz;
    psIn.pos = mul(mView, worldPos);
    psIn.pos = mul(mProj, psIn.pos);

    CalcVertexNormalTangentBiNormalInWorldSpace(
        psIn.normal,
        psIn.tangent,
        psIn.biNormal,
        mWorldLocal,
        vsIn.normal,
        vsIn.tangent,
        vsIn.biNormal,
        isUsePreComputedVertexBuffer
    );

    psIn.uv = vsIn.uv;
    return psIn;
}

SPSOut PSMainCore(SPSIn psIn, int isShadowReciever)
{
    static const float BayerMatrix[16] =
    {
        0.0 / 16.0, 8.0 / 16.0, 2.0 / 16.0, 10.0 / 16.0,
        12.0 / 16.0, 4.0 / 16.0, 14.0 / 16.0, 6.0 / 16.0,
         3.0 / 16.0, 11.0 / 16.0, 1.0 / 16.0, 9.0 / 16.0,
        15.0 / 16.0, 7.0 / 16.0, 13.0 / 16.0, 5.0 / 16.0
    };

    // プレイヤー近接ディザ（既存）
    if (g_ditherdValue > 0.5f)
    {
        float dist = length(g_cameraPos - psIn.worldPos);
        float distStrength = 1.0f - saturate(
            (dist - g_ditherNear) / max(g_ditherFar - g_ditherNear, 0.001f)
        );
        float ditherStrength = distStrength * (1.0f - g_ditherAlpha);

        uint px = (uint)psIn.pos.x % 4;
        uint py = (uint)psIn.pos.y % 4;
        if (BayerMatrix[py * 4 + px] < ditherStrength)
            discard;
    }

    // 草LOD遷移ディザ: XZ距離でインスタンスごとにGPU側計算
    if (g_isGrassLodDither > 0.5f)
    {
        float2 camDiff = psIn.worldPos.xz - g_cameraPos.xz;
        float  dist    = length(camDiff);
        float  t       = saturate((dist - g_grassLodDitherStartDist)
                       / max(g_grassLodDitherEndDist - g_grassLodDitherStartDist, 0.001f));
        float  alpha   = lerp(1.0f, g_grassLodDitherAlphaMin, t);

        uint px = (uint)psIn.pos.x % 4;
        uint py = (uint)psIn.pos.y % 4;
        if (BayerMatrix[py * 4 + px] < (1.0f - alpha))
            discard;
    }


    SPSOut psOut;
    psOut.albedo = g_albedo.Sample(g_sampler, psIn.uv);
    clip(psOut.albedo.a - 0.2f);
    psOut.albedo.w = psIn.pos.z;
    psOut.normal.xyz = GetNormalFromNormalMap(
        psIn.normal, psIn.tangent, psIn.biNormal, psIn.uv);
    psOut.normal.w = 1.0f;
    psOut.metaricShadowSmooth = g_spacular.Sample(g_sampler, psIn.uv);
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